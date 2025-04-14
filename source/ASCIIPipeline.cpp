///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"
#include <bitset>


/// Descriptor constructor                                                    
///   @param producer - the pipeline producer                                 
///   @param descriptor - the pipeline descriptor                             
ASCIIPipeline::ASCIIPipeline(ASCIIRenderer* producer, const Many& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing graphics pipeline from: ", descriptor);
   descriptor.ForEach(
      [this](const ASCIILayer& layer) {
         if (layer.GetStyle() & ASCIILayer::Hierarchical)
            mDepthTest = false;
      },
      [this](ASCIIStyle style) {
         mStyle = style;
      }
   );

   // Deside intermediate buffer size                                   
   switch (mStyle) {
   case ASCIIStyle::Halfblocks:
      mBufferScale = 2;
      break;
   case ASCIIStyle::Braille:
      mBufferScale = {2, 4};
      break;
   default:
      break;
   }
}

/// Resize the pipeline's internal buffer                                     
///   @param color - uniform color value                                      
///   @param depth - uniform depth value                                      
void ASCIIPipeline::Clear(const RGBAf& color, float depth) {
   mBuffer.Fill(color);
   mDepth.Fill(depth);
}

/// Resize the pipeline's internal buffer                                     
///   @param x - buffer width                                                 
///   @param y - buffer height                                                
void ASCIIPipeline::Resize(int x, int y) {
   x *= mBufferScale.x;
   y *= mBufferScale.y;

   mBuffer.Resize(x, y);
   mDepth.Resize(x, y);
}

/// Draw a single renderable (used in hierarchical drawing)                   
///   @param layer - the layer that we're rendering to                        
///   @param pv - the projection-view matrix                                  
///   @param sub - prepared renderable instance LOD to draw                   
///   @param lights - list of lights to apply                                 
void ASCIIPipeline::Render(
   const ASCIILayer* layer,
   const Mat4& pv,
   const PipeSubscriber& sub,
   const TMany<LightSubscriber>& lights
) const {
   LANGULUS(PROFILE);
   if (not sub.mesh)
      return;

   PipelineState ps {layer, mBuffer.GetView().GetScale(), pv, sub, lights};
   RasterizeMesh(ps);
}

/// Credit: https://github.com/Gaukler/Software-Rasterizer                    
/// However the mentioned code clips in NDC space, which presumably works     
/// only if there's no chance at anything getting behind the camera.          
/// I modified the code, so that it works in clipspace.                       
template<int AXIS>
std::vector<Vec4> ClipLine(const std::vector<Vec4>& vertices) {
   auto isInside = [](const Vec4& p) {
      return p[AXIS] > -p.w and p[AXIS] < p.w;
   };

   std::vector<Vec4> clipped;
   for (size_t i = 0; i < vertices.size(); i++) {
      Vec4 v1 = vertices[i];
      Vec4 v2 = vertices[(i + 1) % vertices.size()];

      if (isInside(v1) and isInside(v2)) {
         // Both points in                                              
         clipped.emplace_back(v2);
      }
      else if (not isInside(v1) and not isInside(v2)) {
         // Both points out, nothing to draw                            
      }
      else if (v1[AXIS] > v1.w) {
         // Mixed                                                       
         auto t = (v1[AXIS] - v1.w) / ((v1[AXIS] - v1.w) - (v2[AXIS] - v2.w));
         clipped.emplace_back(t * v2 + (1 - t) * v1);
         clipped.emplace_back(v2);
      }
      else if (v1[AXIS] < -v1.w) {
         // Mixed                                                       
         auto t = (v1[AXIS] + v1.w) / ((v1[AXIS] + v1.w) - (v2[AXIS] + v2.w));
         clipped.emplace_back(t * v2 + (1 - t) * v1);
         clipped.emplace_back(v2);
      }
      else if (v2[AXIS] >  v2.w) {
         // Mixed (notice this branch depends on the other branches     
         // being executed first on a prior iteration)                  
         auto t = (v2[AXIS] - v2.w) / ((v2[AXIS] - v2.w) - (v1[AXIS] - v1.w));
         clipped.emplace_back(t * v1 + (1 - t) * v2);
      }
      else if (v2[AXIS] < -v2.w) {
         // Mixed (notice this branch depends on the other branches     
         // being executed first on a prior iteration)                  
         auto t = (v2[AXIS] + v2.w) / ((v2[AXIS] + v2.w) - (v1[AXIS] + v1.w));
         clipped.emplace_back(t * v1 + (1 - t) * v2);
      }
   }

   return clipped;
}

/// Clip a triangle depending on how many vertices are in viewport            
///   @param MVP - model*view*projection matrix                               
///   @param triangle - the triangle to clip                                  
///   @param rasterizer - rasterizer to use                                   
void ASCIIPipeline::ClipTriangle(
   const Mat4& MVP, const ASCIIGeometry::Vertex* triangle, auto&& rasterizer
) const {
   // Transform to viewspace                                            
   std::vector<Vec4> points;
   points.emplace_back(MVP * triangle[0].mPos);
   points.emplace_back(MVP * triangle[1].mPos);
   points.emplace_back(MVP * triangle[2].mPos);

   // Clip                                                              
   //points = ClipLine<0>(points);
   //points = ClipLine<1>(points);
   points = ClipLine<2>(points);  // Clipping only z is enough for me,  
                                  // but also clipping along x and y    
                                  // produces undesired artifacts       
   if (points.size() < 3)
      return;

   // Do perspective division (collapses Z data)                        
   for (auto& p : points)
      p /= p.w;

   // Create a triangle fan                                             
   for (size_t i = 1; i < points.size() - 1; ++i)
      rasterizer(Triangle4 {points[0], points[i], points[i + 1]});
}

/// Rasterize a single triangle                                               
///   @tparam LIT - whether or not to calculate lights and speculars          
///   @tparam DEPTH - whether or not to perform depth test and write depth    
///   @tparam SMOOTH - interpolate normals/colors inside trianlges            
///   @tparam FOG - apply fog                                                 
///   @tparam COLORIZE - apply vertex colors                                  
///   @tparam SHADOWED - apply shadowmaps                                     
///   @param ps - the pipeline state                                          
///   @param M - precomputed world matrix for light computation               
///   @param triangle - three consecutive original vertices (object space)    
///   @param clipped - a clipped triangle in NDC space                        
template<bool LIT, bool DEPTH, bool SMOOTH, bool FOG, bool COLORIZE, bool SHADOWED>
void ASCIIPipeline::RasterizeTriangle(
   const PipelineState& ps, const Mat4& M,
   const ASCIIGeometry::Vertex* triangle,
   const Triangle4& clipped
) const {
   LANGULUS(PROFILE);
   const Vec3 p0 = clipped[0].xyz();
   const Vec3 p1 = clipped[1].xyz();
   const Vec3 p2 = clipped[2].xyz();
   const auto a = 0.5_real * (
      -p1.y *   p2.x + 
       p0.y * (-p1.x + p2.x) + 
       p0.x * ( p1.y - p2.y) + 
       p1.x *   p2.y
   );

   // Cull based on winding if enabled                                  
   switch (mCull) {
   case CullBack:  if (a  > 0) return; break;
   case CullFront: if (a <= 0) return; break;
   case NoCulling: break;
   }

   // If reached, then triangle is visible                              
   const auto term_a  = 1.0_real / (2.0_real * a);
   const auto term_s1 = p0.y * p2.x - p0.x * p2.y;
   const auto term_s2 = p2.y - p0.y;
   const auto term_s3 = p0.x - p2.x;
   const auto term_t1 = p0.x * p1.y - p0.y * p1.x;
   const auto term_t2 = p0.y - p1.y;
   const auto term_t3 = p1.x - p0.x;

   const auto term_s1_a = term_a * term_s1;
   const auto term_t1_a = term_a * term_t1;
   const auto term_s2_a = term_a * term_s2;
   const auto term_t2_a = term_a * term_t2;
   const auto term_s3_a = term_a * term_s3;
   const auto term_t3_a = term_a * term_t3;

   // p0, p1, and p2 should be in NDC space                             
   Vec2i minp = Math::Floor(Math::Min(p0.xy(), p1.xy(), p2.xy()) * ps.mResolution + 0.5);
   minp.y -= ps.mResolution.y * 2;
   minp = Math::Min(Math::Max(minp, -ps.mResolution), ps.mResolution);
   minp = (minp + ps.mResolution) / 2;

   Vec2i maxp = Math::Ceil(Math::Max(p0.xy(), p1.xy(), p2.xy()) * ps.mResolution + 0.5);
   maxp.y += ps.mResolution.y * 2;
   maxp = Math::Min(Math::Max(maxp, -ps.mResolution), ps.mResolution);
   maxp = (maxp + ps.mResolution) / 2;

   // The normal                                                        
   [[maybe_unused]] Vec3  n {0, 0, 1};
   // The accumulated light colors                                      
   [[maybe_unused]] RGBAf lit = 0;

   if constexpr (LIT and not SMOOTH) {
      // Get an average normal for the triangle for flat rendering      
      n = Mat3(M) * (triangle[0].mNor + triangle[1].mNor + triangle[2].mNor);
      n = n.Normalize();

      // Just get the center of the triangle (in world space)           
      auto p = M * (( triangle[0].mPos 
                    + triangle[1].mPos
                    + triangle[2].mPos ) / 3);

      // Accumulate all lights                                          
      for (auto& light : ps.mLights) {
         // Determine light direction                                   
         switch (light.type) {
         case A::Light::Directional:
         case A::Light::Spot:
            // Direction is taken from the light instance               
            lit += light.color * n.Dot(light.direction);
            break;
         case A::Light::Point:
            // Direction is relative to light position                  
            lit += light.color * n.Dot((light.position - p).Normalize());
            break;
         case A::Light::Domain:
            TODO();
         }
      }

      // And then clamp                                                 
      if (lit.r > 1) lit.r = 1;
      if (lit.g > 1) lit.g = 1;
      if (lit.b > 1) lit.b = 1;
      lit.a = 1;
   }

   // Iterate all pixels in the area of interest                        
   for (int y = minp.y; y < maxp.y; ++y) {
      bool row_started = false;
      const auto screenv = -(y * 2 - ps.mResolution.y + 0.5_real) / ps.mResolution.y;
      const auto term_s3_v = term_s1_a + term_s3_a * screenv;
      const auto term_t3_v = term_t1_a + term_t3_a * screenv;

      for (int x = minp.x; x < maxp.x; ++x) {
         const auto screenu = (x * 2 - ps.mResolution.x + 0.5_real) / ps.mResolution.x;
         const auto s = term_s2_a * screenu + term_s3_v;
         const auto t = term_t2_a * screenu + term_t3_v;
         const auto d = 1 - s - t;

         if (s < 0 or t < 0 or d < 0) {
            // Pixel discarded (not inside the triangle)                
            // Was a row started? If so, then there's not any chance to 
            // find a point in the triangle again on this row - just    
            // jump to the next row by breaking                         
            if (row_started) {
               row_started = false;
               break;
            }
            else continue;
         }

         // If reached, then pixel is inside triangle                   
         row_started = true;

         // Interpolate depth at the current pixel                      
         [[maybe_unused]] const Real z = p1.z * s + p2.z * t + p0.z * d;
         if constexpr (DEPTH) {
            auto& global_depth = ps.mLayer->mDepth.Get(x / mBufferScale.x, y / mBufferScale.y);

            // Do depth test                                            
            if (z >= global_depth or z <= 0 or z >= 1)
               continue;

            //                                                          
            // If reached, pixel depth is overwritten                   
            global_depth = mDepth.Get(x, y) = z;
         }

         if constexpr (FOG or COLORIZE or (LIT and SMOOTH)) {
            //                                                          
            // If reached, pixel color is overwritten                   
            auto& pixel = mBuffer.Get(x, y);

            /*const auto fog = Clamp(
               (fogRange.GetMax() - z) / fogRange.Length(),
               0_real, 1_real
            );*/ //TODO clamp not working in this context, check TODO.md
            [[maybe_unused]] RGBAf fogColor = mFogColor;
            [[maybe_unused]] Real  fog = 0;
            if constexpr (FOG) {
               fog = (mFogRange.GetMax() - (1 - z) * 1000) / mFogRange.Length();
               if (fog >= 1) {
                  // Fog can optimize-out far pixels                    
                  pixel = fogColor;
                  continue;
               }
               else if (fog < 0)
                  fog = 0;

               fogColor *= fog;
            }

            if constexpr (COLORIZE) {
               // Interpolate the color                                 
               //TODO fix color multiplication with normalization, see todo.md
               pixel = triangle[1].mCol * s
                     + triangle[2].mCol * t
                     + triangle[0].mCol * d;
            }

            if constexpr (LIT and SMOOTH) {
               // Interpolate and transform the normal per-pixel        
               n = Mat3(M) * Vec3( triangle[0].mNor * d
                                 + triangle[1].mNor * s
                                 + triangle[2].mNor * t );
               n = n.Normalize();

               // Interpolate and transform the position per-pixel      
               // (in world space)                                      
               auto p  = M * ( triangle[0].mPos * d
                             + triangle[1].mPos * s
                             + triangle[2].mPos * t );

               // Accumulate all lights                                 
               for (auto& light : ps.mLights) {
                  // Determine light direction                          
                  switch (light.type) {
                  case A::Light::Directional:
                  case A::Light::Spot:
                     // Direction is taken from the light instance      
                     lit += light.color * n.Dot(light.direction);
                     break;
                  case A::Light::Point:
                     // Direction is relative to light position         
                     lit += light.color * n.Dot((light.position - p).Normalize());
                     break;
                  case A::Light::Domain:
                     TODO();
                  }
               }

               // And then clamp                                        
               if (lit.r > 1) lit.r = 1;
               if (lit.g > 1) lit.g = 1;
               if (lit.b > 1) lit.b = 1;
               lit.a = 1;

               if constexpr (COLORIZE)
                  // Blend with vertex colors                           
                  pixel *= ps.mSubscriber.color * lit;
               else
                  // Just assign the instance color * light color       
                  pixel  = ps.mSubscriber.color * lit;
            }
            else if constexpr (COLORIZE) {
               // Blend with vertex colors                              
               if constexpr (LIT)
                  pixel *= ps.mSubscriber.color * lit;
               else
                  pixel *= ps.mSubscriber.color;
            }
            else {
               // Just assign the instance color                        
               if constexpr (LIT)
                  pixel = ps.mSubscriber.color * lit;
               else
                  pixel = ps.mSubscriber.color;
            }

            if constexpr (FOG)
               pixel = fogColor + pixel * (1 - fog);
         }
      }
   }
}

#define MAP_ARGUMENT_TO_TEMPLATE(Arg, tArgId, Nest) \
   if (Arg) { \
      constexpr bool tArg##tArgId = true;  \
      Nest \
   } \
   else { \
      constexpr bool tArg##tArgId = false; \
      Nest \
   }

/// Rasterize all primitives inside a mesh                                    
///   @param ps - pipeline state                                              
void ASCIIPipeline::RasterizeMesh(const PipelineState& ps) const {
   const auto M   = ps.mSubscriber.transform;
   const auto MVP = ps.mProjectedView * M;
   auto& mesh     = ps.mSubscriber.mesh;

   if (mesh->MadeOfTriangles()) {
      // Rasterize triangles...                                         
      auto vertices = mesh->GetVertices().GetRaw();
      bool firstVertex = true;
      Range4 dataRangeBeforeW;
      Range4 dataRangeAfterW;
      for (Offset i = 0; i < mesh->GetVertices().GetCount(); ++i) {
         auto p = MVP * vertices[i].mPos;
         if (firstVertex)
            dataRangeBeforeW.mMin = dataRangeBeforeW.mMax = p;
         else
            dataRangeBeforeW.Embrace(p);

         p /= p.w;

         if (firstVertex)
            dataRangeAfterW.mMin = dataRangeAfterW.mMax = p;
         else
            dataRangeAfterW.Embrace(p);

         firstVertex = false;
      }

      MAP_ARGUMENT_TO_TEMPLATE(mLit,      0,
      MAP_ARGUMENT_TO_TEMPLATE(mDepthTest,1,
      MAP_ARGUMENT_TO_TEMPLATE(mSmooth,   2,
      MAP_ARGUMENT_TO_TEMPLATE(mFog,      3,
      MAP_ARGUMENT_TO_TEMPLATE(mColorize, 4,
      MAP_ARGUMENT_TO_TEMPLATE(mShadows,  5,
         for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3) {
            ClipTriangle(MVP, vertices + i, [&](const Triangle4& t) {
               RasterizeTriangle<tArg0, tArg1, tArg2, tArg3, tArg4, tArg5>(ps, M, vertices + i, t);
            });
         }
      ))))));
   }
   else TODO();
}

/// Merge the pipeline with the layer's image, assembling any symbols         
///   @param layer - the layer that we're rendering to                        
void ASCIIPipeline::Assemble(const ASCIILayer* layer) const {
   LANGULUS(PROFILE);

   auto gradient3x3 = [&](
      ::std::array<RGBAf, 9>&& colors,
      ::std::array<Real,  9>&& gather
   ) -> ::std::string_view {
      //  [0][1][2]                                                     
      //  [3][4][5]                                                     
      //  [6][7][8]                                                     
      const auto center = gather[4];
      for (auto& n : gather)
         n -= center;

      constexpr Real colorThreshold = 0.025;
      auto mid40c = (colors[4] + colors[0]) / 2;
      auto mid48c = (colors[4] + colors[8]) / 2;
      auto mid40  = (gather[4] + gather[0]) / 2;
      auto mid48  = (gather[4] + gather[8]) / 2;
      if (gather[3] < mid40     and mid40     > gather[1]
      and gather[6] < gather[4] and gather[4] > gather[2]
      and gather[5] < mid48     and mid48     > gather[7]) {
         auto dia0 = (colors[1] + colors[2] + colors[5]).Length() / 3;
         auto dia1 = (colors[0] + colors[4] + colors[8]).Length() / 3;
         auto dia2 = (colors[3] + colors[6] + colors[7]).Length() / 3;
         if (Abs(dia0 - dia1) < colorThreshold and Abs(dia2 - dia1) < colorThreshold)
            return "╲";
      }

      if (gather[3] > mid40     and mid40     < gather[1]
      and gather[6] > gather[4] and gather[4] < gather[2]
      and gather[5] > mid48     and mid48     < gather[7]) {
         // Add detail only if colors don't already provide it          
         auto dia0 = (colors[1] + colors[2] + colors[5]).Length() / 3;
         auto dia1 = (colors[0] + colors[4] + colors[8]).Length() / 3;
         auto dia2 = (colors[3] + colors[6] + colors[7]).Length() / 3;
         if (Abs(dia0 - dia1) < colorThreshold and Abs(dia2 - dia1) < colorThreshold)
            return "╲";
      }

      auto mid42c = (colors[4] + colors[2]) / 2;
      auto mid46c = (colors[4] + colors[6]) / 2;
      auto mid42  = (gather[4] + gather[2]) / 2;
      auto mid46  = (gather[4] + gather[6]) / 2;
      if (gather[1] < mid42     and mid42     > gather[5]
      and gather[0] < gather[4] and gather[4] > gather[8]
      and gather[3] < mid46     and mid46     > gather[7]) {
         // Add detail only if colors don't already provide it          
         auto dia0 = (colors[0] + colors[1] + colors[3]).Length() / 3;
         auto dia1 = (colors[2] + colors[4] + colors[6]).Length() / 3;
         auto dia2 = (colors[5] + colors[7] + colors[8]).Length() / 3;
         if (Abs(dia0 - dia1) < colorThreshold and Abs(dia2 - dia1) < colorThreshold)
            return "╱";
      }

      if (gather[1] > mid42     and mid42     < gather[5]
      and gather[0] > gather[4] and gather[4] < gather[8]
      and gather[3] > mid46     and mid46     < gather[7]) {
         // Add detail only if colors don't already provide it          
         auto dia0 = (colors[0] + colors[1] + colors[3]).Length() / 3;
         auto dia1 = (colors[2] + colors[4] + colors[6]).Length() / 3;
         auto dia2 = (colors[5] + colors[7] + colors[8]).Length() / 3;
         if (Abs(dia0 - dia1) < colorThreshold and Abs(dia2 - dia1) < colorThreshold)
            return "╱";
      }

      if (gather[0] < gather[1] and gather[1] > gather[2]
      and gather[3] < gather[4] and gather[4] > gather[5]
      and gather[6] < gather[7] and gather[7] > gather[8]) {
         // Add detail only if colors don't already provide it          
         auto col0 = (colors[0] + colors[3] + colors[6]).Length() / 3;
         auto col1 = (colors[1] + colors[4] + colors[7]).Length() / 3;
         auto col2 = (colors[2] + colors[5] + colors[8]).Length() / 3;
         if (Abs(col0 - col1) < colorThreshold and Abs(col2 - col1) < colorThreshold)
            return "│";
      }

      if (gather[0] > gather[1] and gather[1] < gather[2]
      and gather[3] > gather[4] and gather[4] < gather[5]
      and gather[6] > gather[7] and gather[7] < gather[8]) {
         // Add detail only if colors don't already provide it          
         auto col0 = (colors[0] + colors[3] + colors[6]).Length() / 3;
         auto col1 = (colors[1] + colors[4] + colors[7]).Length() / 3;
         auto col2 = (colors[2] + colors[5] + colors[8]).Length() / 3;
         if (Abs(col0 - col1) < colorThreshold and Abs(col2 - col1) < colorThreshold)
            return "│";
      }

      if (gather[0] < gather[3] and gather[3] > gather[6]
      and gather[1] < gather[4] and gather[4] > gather[7]
      and gather[2] < gather[5] and gather[5] > gather[8]) {
         // Add detail only if colors don't already provide it          
         auto row0 = (colors[0] + colors[1] + colors[2]).Length() / 3;
         auto row1 = (colors[3] + colors[4] + colors[5]).Length() / 3;
         auto row2 = (colors[6] + colors[7] + colors[8]).Length() / 3;
         if (Abs(row0 - row1) < colorThreshold and Abs(row2 - row1) < colorThreshold)
            return "─";
      }

      if (gather[0] > gather[3] and gather[3] < gather[6]
      and gather[1] > gather[4] and gather[4] < gather[7]
      and gather[2] > gather[5] and gather[5] < gather[8]) {
         // Add detail only if colors don't already provide it          
         auto row0 = (colors[0] + colors[1] + colors[2]).Length() / 3;
         auto row1 = (colors[3] + colors[4] + colors[5]).Length() / 3;
         auto row2 = (colors[6] + colors[7] + colors[8]).Length() / 3;
         if (Abs(row0 - row1) < colorThreshold and Abs(row2 - row1) < colorThreshold)
            return "─";
      }

      // Map depth values to 1 if slope is >= 0, 0 if otherwise         
      // [ 4][ 2][-1]      [ 1][ 1][ 0]                                 
      // [ 2][ 0][-2]  ->  [ 1]    [ 0]                                 
      // [-1][-2][-2]      [ 0][ 0][ 0]                                 
      /*::std::bitset<8> pattern = 0;
      pattern |= (gather[0] >= 0) << 0;
      pattern |= (gather[1] >= 0) << 1;
      pattern |= (gather[2] >= 0) << 2;
      pattern |= (gather[3] >= 0) << 3;
      // ...skip center cell...                                         
      pattern |= (gather[5] >= 0) << 4;
      pattern |= (gather[6] >= 0) << 5;
      pattern |= (gather[7] >= 0) << 6;
      pattern |= (gather[8] >= 0) << 7;

      // Check key combinations that correspond to characters           
      // [0][0][0]                                                      
      // [1][1][1]  ->  '─'                                             
      // [0][0][0]                                                      
      if (pattern == 0b00011000 or pattern == ~0b00011000)
         return "─";

      // [0][0][0]                                                      
      // [1][1][0]  ->  '╴'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b00010000 or pattern == ~0b00010000)
         return "╴";

      // [0][0][0]                                                      
      // [0][1][1]  ->  '╶'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b00001000 or pattern == ~0b00001000)
         return "╶";

      // [0][1][0]                                                      
      // [0][1][0]  ->  '│'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b01000010 or pattern == ~0b01000010)
         return "│";

      // [0][0][0]                                                      
      // [0][1][0]  ->  '╷'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b00000010 or pattern == ~0b00000010)
         return "╷";

      // [0][1][0]                                                      
      // [0][1][0]  ->  '╵'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b01000000 or pattern == ~0b01000000)
         return "╵";

      // [0][1][0]                                                      
      // [0][1][1]  ->  '└'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b01001000 or pattern == ~0b01001000)
         return "└";

      // [0][1][0]                                                      
      // [0][1][1]  ->  '├'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b01001010 or pattern == ~0b01001010)
         return "├";

      // [0][1][0]                                                      
      // [1][1][0]  ->  '┘'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b01010000 or pattern == ~0b01010000)
         return "┘";

      // [0][1][0]                                                      
      // [1][1][1]  ->  '┴'                                             
      // [0][0][0]                                                      
      else if (pattern == 0b01011000 or pattern == ~0b01011000)
         return "┴";

      // [0][0][0]                                                      
      // [1][1][0]  ->  '┐'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b00010010 or pattern == ~0b00010010)
         return "┐";

      // [0][1][0]                                                      
      // [1][1][0]  ->  '┤'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b01010010 or pattern == ~0b01010010)
         return "┤";

      // [0][0][0]                                                      
      // [0][1][1]  ->  '┌'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b00001010 or pattern == ~0b00001010)
         return "┌";

      // [0][0][0]                                                      
      // [1][1][1]  ->  '┬'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b00011010 or pattern == ~0b00011010)
         return "┬";

      // [0][1][0]                                                      
      // [1][1][1]  ->  '┼'                                             
      // [0][1][0]                                                      
      else if (pattern == 0b01011010 or pattern == ~0b01011010)
         return "┼";

      // [1][0][0]                                                      
      // [0][1][0]  ->  '╲'                                             
      // [0][0][1]                                                      
      else if (pattern == 0b10000001 or pattern == ~0b10000001
      or       pattern == 0b10010011 or pattern == ~0b10010011
      or       pattern == 0b11001001 or pattern == ~0b11001001)
         return "╲";

      // [0][0][1]                                                      
      // [0][1][0]  ->  '╱'                                             
      // [1][0][0]                                                      
      else if (pattern == 0b00100100 or pattern == ~0b00100100
      or       pattern == 0b01110100 or pattern == ~0b01110100
      or       pattern == 0b00101110 or pattern == ~0b00101110)
         return "╱";

      // [1][0][1]                                                      
      // [0][1][0]  ->  '╳'                                             
      // [1][0][1]                                                      
      else if (pattern == 0b10100101 or pattern == ~0b10100101)
         return "╳";*/

      return " ";
   };

   // Depth and normals are written directly into layer, but this       
   // pipeline might have some odd ways of deciding color and symbols,  
   // so assemble those here, and write to layer                        
   // mBufferXScale x mBufferYScale pixels -> 1 layer pixel             
   if (mBufferScale == 1) {
      // Pixels map 1:1                                                 
      for (uint32_t y = 0; y < layer->mImage.GetView().mHeight; ++y) {
         for (uint32_t x = 0; x < layer->mImage.GetView().mWidth; ++x) {
            auto to = layer->mImage.GetPixel(x, y);
            auto& from = mBuffer.Get(x, y);
            ::std::string_view c = " ";

            if (x and y and y < layer->mImage.GetView().mHeight-1
                        and x < layer->mImage.GetView().mWidth-1
            ) {
               c = gradient3x3({
                  mBuffer.Get(x-1, y-1), mBuffer.Get(x, y-1), mBuffer.Get(x+1, y-1),
                  mBuffer.Get(x-1, y  ), mBuffer.Get(x, y  ), mBuffer.Get(x+1, y  ),
                  mBuffer.Get(x-1, y+1), mBuffer.Get(x, y+1), mBuffer.Get(x+1, y+1),
               }, {
                  layer->mDepth.Get(x-1, y-1), layer->mDepth.Get(x, y-1), layer->mDepth.Get(x+1, y-1),
                  layer->mDepth.Get(x-1, y  ), layer->mDepth.Get(x, y  ), layer->mDepth.Get(x+1, y  ),
                  layer->mDepth.Get(x-1, y+1), layer->mDepth.Get(x, y+1), layer->mDepth.Get(x+1, y+1),
               });
            }

            //if (d > 0.0f and d < 1000.0f) {
               // Write pixel only if in valid depth range              
               to.mSymbol = c;
               to.mFgColor = from;
               to.mBgColor = from;
            //}
         }
      }
   }
   else TODO();
}
