///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "ASCII.hpp"


/// Descriptor constructor                                                    
///   @param producer - the pipeline producer                                 
///   @param descriptor - the pipeline descriptor                             
ASCIIPipeline::ASCIIPipeline(ASCIIRenderer* producer, const Neat& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing graphics pipeline from: ", descriptor);
   descriptor.ForEach(
      [this](const ASCIILayer& layer) {
         if (layer.GetStyle() & ASCIILayer::Hierarchical)
            mDepth = false;
      },
      [this](ASCIIStyle style) {
         mStyle = style;
      }
   );

   // Deside intermediate buffer size                                   
   switch (mStyle) {
   case ASCIIStyle::Halfblocks:
      mBufferXScale = mBufferYScale = 2;
      break;
   case ASCIIStyle::Braille:
      mBufferXScale = 2;
      mBufferYScale = 4;
      break;
   default:
      break;
   }
}

/// Resize the pipeline's internal buffer                                     
///   @param x - buffer width                                                 
///   @param y - buffer height                                                
void ASCIIPipeline::Resize(int x, int y) {
   mBuffer.Resize(x * mBufferXScale, y * mBufferYScale);
   mBuffer.Fill({0, 0, 0, 0});
}

/// Draw a single renderable (used in hierarchical drawing)                   
///   @param layer - the layer that we're rendering to                        
///   @param pv - the projection-view matrix                                  
///   @param sub - prepared renderable instance LOD to draw                   
void ASCIIPipeline::Render(
   const ASCIILayer* layer, const Mat4& pv, const PipeSubscriber& sub
) const {
   if (not sub.mesh)
      return;

   PipelineState ps {layer, mBuffer.GetView().GetScale(), pv, sub};
   RasterizeMesh(ps);
   
   // Depth and normals are written directly into layer, but this       
   // pipeline might have some odd ways of deciding color and symbols,  
   // so assemble those here, and write to layer                        
   for (uint32_t y = 0; y < layer->mImage.GetView().mHeight; ++y) {
      for (uint32_t x = 0; x < layer->mImage.GetView().mWidth; ++x) {
         // mBufferXScale x mBufferYScale pixels -> 1 layer pixel       
         auto to = layer->mImage.GetPixel(x, y);

         if (mBufferXScale == 1 and mBufferYScale == 1) {
            // Pixels map 1:1                                           
            auto d = layer->mDepth.Get(x, y);
            if (d > 0.0f and d < 1000.0f) {
               // Write pixel only if in valid depth range              
               auto& from = mBuffer.Get(x, y);
               to.mSymbol = "█";
               to.mFgColor = from;
               to.mBgColor = from;
            }
         }
         else TODO();
      }
   }
}

/// Rasterize all primitives inside a mesh                                    
///   @param camera - the camera state                                        
///   @param mesh - the mesh we want to rasterize                             
void ASCIIPipeline::RasterizeMesh(const PipelineState& ps) const {
   const auto MVP = ps.mProjectedView * ps.mSubscriber.transform;

   // Create a buffer for each relevant data trait                      
   // Each data request will generate that data, if it hasn't yet       
   auto& mesh = *ps.mSubscriber.mesh;
   if (mesh.MadeOfTriangles()) {
      Triangle pos;
      Triangle nor;
      //TTriangle<RGBA> col;
      Offset counter = 0;

      mesh.ForEachVertex(
         [&](const Traits::Place& p, const Traits::Aim& n/*, const Traits::Color& c*/) {
            if (p.IsSimilar<Vec3>())
               pos[counter] = p.GetRaw<Vec3>()[0];
            else if (p.IsSimilar<Vec2>())
               pos[counter] = Vec3(p.GetRaw<Vec2>()[0], 0);
            else
               LANGULUS_OOPS(Access, "Unsupported place type");

            if (n.IsSimilar<Vec3>())
               nor[counter] = n.GetRaw<Vec3>()[0];
            else
               LANGULUS_OOPS(Access, "Unsupported normal type");

            //col[counter] = c.AsCast<RGBA, false>();
            if (++counter == 3) {
               // A triangle is ready                                   
               RasterizeTriangle(ps, MVP, pos, nor/*, col*/);
               counter = 0;
            }
         }
      );
   }
   else TODO();
}

/// Rasterize a single triangle                                               
///   @param ps - the pipeline state                                          
///   @param MVP - precomputed model*view*projection matrix                   
///   @param triangle - triangle to rasterize                                 
///   @param normals - the normals for each triangle corner                   
void ASCIIPipeline::RasterizeTriangle(
   const PipelineState& ps,
   const Mat4& MVP,
   const Triangle& triangle,
   const Triangle& normals/*,
   const TTriangle<RGBA>& colors*/
) const {
   /*if (colors[0].a * colors[1].a * colors[2].a == 0) {
      // Whole triangle discarded by alpha test                         
      return;
   }*/

   // Transform to eye space                                            
   const Vec4f pt0 = MVP * Vec4f(triangle[0], 1);
   const Vec4f pt1 = MVP * Vec4f(triangle[1], 1);
   const Vec4f pt2 = MVP * Vec4f(triangle[2], 1);

   const Vec2f p0 = pt0.xy() / pt0.w;
   const Vec2f p1 = pt1.xy() / pt1.w;
   const Vec2f p2 = pt2.xy() / pt2.w;

   const auto a = 0.5f * (
      -p1.y * p2.x + 
       p0.y * (-p1.x + p2.x) + 
       p0.x * ( p1.y - p2.y) + 
       p1.x * p2.y
   );
   const bool front = a <= 0.0f;

   // Cull if we have to                                                
   switch (mCull) {
   case CullBack:
      if (not front)
         return;
      break;
   case CullFront:
      if (front)
         return;
      break;
   case NoCulling:
      break;
   }

   // If reached, then triangle is visible                              
   const auto term_a = 1.0f / (2.0f * a);
   const auto term_s1 = p0.y * p2.x - p0.x * p2.y;
   const auto term_s2 = p2.y - p0.y;
   const auto term_s3 = p0.x - p2.x;
   const auto term_t1 = p0.x * p1.y - p0.y * p1.x;
   const auto term_t2 = p0.y - p1.y;
   const auto term_t3 = p1.x - p0.x;

   //const auto minp = Math::Floor(Math::Min(p0, p1, p2) * cam.mResolution * 0.5f);
   //const auto maxp = Math::Ceil (Math::Max(p0, p1, p2) * cam.mResolution * 0.5f);

   // Clip                                                              
   /*if (maxp.x < 0.0 or maxp.y < 0.0
   or minp.x >= cam.mResolution.x or minp.y >= cam.mResolution.y) {
      // Triangle is fully outside view                                 
      return;
   }*/

   // Iterate all pixels in the area of interest                        
   /*for (int y = static_cast<int>(minp.y);
            y < static_cast<int>(maxp.y); ++y) {
      for (int x = static_cast<int>(minp.x);
               x < static_cast<int>(maxp.x); ++x) {
         const Vec2f screenuv = (Vec2f(x, y) - minp) / cam.mResolution.x;*/
   for (int y = 0; y < static_cast<int>(ps.mResolution.y) * mBufferYScale; ++y) {
      bool row_started = false;

      for (int x = 0; x < static_cast<int>(ps.mResolution.x) * mBufferXScale; ++x) {
         const Vec2f screenuv = (Vec2f(x, y) * 2.0f - ps.mResolution + Vec2f(0.5)) / ps.mResolution;

         const float s = term_a * (term_s1
            + term_s2 *  screenuv.x
            + term_s3 * -screenuv.y);
         const float t = term_a * (term_t1
            + term_t2 *  screenuv.x
            + term_t3 * -screenuv.y);

         if (s <= 0 or t <= 0 or 1 - s - t <= 0) {
            // Pixel discarded (not inside the triangle)                
            // Was a row started? If so, then there's not any chance to 
            // find a point in the triangle again on this row.          
            if (row_started) {
               row_started = false;
               break;
            }
            else continue;
         }

         // If reached, then pixel is inside triangle                   
         // Depth test                                                  
         row_started = true;
         Vec4f test {1, s, t, 1 - s - t};
         float denominator = 1.0f / (test.y / pt1.w + test.z / pt2.w + test.w / pt0.w);
         float z = (
               (pt1.z * test.y) / pt1.w +
               (pt2.z * test.z) / pt2.w +
               (pt0.z * test.w) / pt0.w
            ) * denominator;

         auto& depth = ps.mLayer->mDepth.Get(x / mBufferXScale, y / mBufferYScale);
         if (z > depth or z <= 0) {
            // Pixel discarded by depth test                            
            continue;
         }

         // If reached, pixel is overwritten                            
         const Normal interpolatedNormal = normals[1] * s + normals[2] * t + normals[0] * (1 - s - t);

         mBuffer.Get(x, y) = /*colors[0] **/ ps.mSubscriber.color * interpolatedNormal.Dot(Normal(1,1,0).Normalize()); //TODO color interpolation

         // Only last pixel of a group is allowed to write depth and    
         // normal, otherwise subtle bugs might occur                   
         if ((x % mBufferXScale) == mBufferXScale - 1
         and (y % mBufferYScale) == mBufferYScale - 1) {
            depth = z;
            ps.mLayer->mNormals.Get(x / mBufferXScale, y / mBufferYScale) = normals[0]; //TODO normal interpolation
         }
      }
   }
}
