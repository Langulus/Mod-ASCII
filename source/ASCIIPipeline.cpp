///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"


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
void ASCIIPipeline::Clear(RGBA color, float depth) {
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
void ASCIIPipeline::Render(
   const ASCIILayer* layer, const Mat4& pv, const PipeSubscriber& sub
) const {
   if (not sub.mesh)
      return;

   PipelineState ps {layer, mBuffer.GetView().GetScale(), pv, sub};
   RasterizeMesh(ps);
}

/// Merge the pipeline with the layer's image, assembling any ASCII symbols   
///   @param layer - the layer that we're rendering to                        
void ASCIIPipeline::Assemble(const ASCIILayer* layer) const {
   // Depth and normals are written directly into layer, but this       
   // pipeline might have some odd ways of deciding color and symbols,  
   // so assemble those here, and write to layer                        
   for (uint32_t y = 0; y < layer->mImage.GetView().mHeight; ++y) {
      for (uint32_t x = 0; x < layer->mImage.GetView().mWidth; ++x) {
         // mBufferXScale x mBufferYScale pixels -> 1 layer pixel       
         auto to = layer->mImage.GetPixel(x, y);

         if (mBufferScale == 1) {
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

/// Clip a triangle, if one of its points is inside the clip space            
///   @param in - the vertex which is inside the clip space                   
///   @param out1 - first vertex, that will be clipped                        
///   @param out2 - second vertex, that will be clipped                       
///   @param rasterizer - the rasterizer function to invoke with clipped      
///      vertex positions                                                     
LANGULUS(INLINED)
void ASCIIPipeline::Clip1(
   const Vec4& in, Vec4 out1, Vec4 out2, auto&& rasterizer
) const {
   const auto t1 = (in.z + in.w) / ((in.z + in.w) - (out1.z + out1.w));
   out1 = t1 * out1 + in;

   const auto t2 = (in.z + in.w) / ((in.z + in.w) - (out2.z + out2.w));
   out2 = t2 * out2 + in;

   rasterizer(Triangle4 {in, out1, out2});
}
 
/// Clip a triangle, if two of its points are inside clip space               
///   @param in1 - the first vertex which is inside the clip space            
///   @param in2 - the second vertex which is inside the clip space           
///   @param out - vertex, that will be clipped                               
///   @param rasterizer - the rasterizer function to invoke with clipped      
///      vertex positions - will be invoked twice for each new triangle       
LANGULUS(INLINED)
void ASCIIPipeline::Clip2(
   const Vec4& in1, const Vec4& in2, Vec4 out, auto&& rasterizer
) const {
   // First new vertex:                                                 
   const auto t1 = (in1.z + in1.w) / ((in1.z + in1.w) - (out.z + out.w));
   const auto out1 = t1 * out + in1;

   // Second new vertex:                                                
   const auto t2 = (in2.z + in2.w) / ((in2.z + in2.w) - (out.z + out.w));
   const auto out2 = t2 * out + in2;

   // Split into 2 triangles:                                           
   rasterizer(Triangle4 {in1, out1, in2});
   rasterizer(Triangle4 {in1, in2, out2});
}

/// Clip a triangle, depending on how many vertices are in clips-pace         
///   @param MVP - model*view*projection matrix                               
///   @param triangle - the triangle to clip                                  
///   @param rasterizer - rasterizer to use                                   
void ASCIIPipeline::ClipTriangle(
   const Mat4& MVP, const ASCIIGeometry::Vertex* triangle, auto&& rasterizer
) const {
   // Transform to eye space                                            
   const Vec4 pt0 = MVP * triangle[0].mPos;
   const Vec4 pt1 = MVP * triangle[1].mPos;
   const Vec4 pt2 = MVP * triangle[2].mPos;

   if (pt0.x < -pt0.w or pt0.x > pt0.w
   or  pt0.y < -pt0.w or pt0.y > pt0.w
   /*or  pt0.z < -pt0.w or pt0.z > pt0.w*/) {
      // First point is outside clip space                              
      if (pt1.x < -pt1.w or pt1.x > pt1.w
      or  pt1.y < -pt1.w or pt1.y > pt1.w
      /*or  pt1.z < -pt1.w or pt1.z > pt1.w*/) {
         // First & second point is outside clip space                  
         if (pt2.x < -pt2.w or pt2.x > pt2.w
         or  pt2.y < -pt2.w or pt2.y > pt2.w
         /*or  pt2.z < -pt2.w or pt2.z > pt2.w*/) {
            // All points are outside clip space, don't rasterize at all
            return;
         }
         else {
            // Only the third point is inside                           
            Clip1(pt2, pt0, pt1, rasterizer);
         }
      }
      else
      if (pt2.x < -pt2.w or pt2.x > pt2.w
      or  pt2.y < -pt2.w or pt2.y > pt2.w
      /*or  pt2.z < -pt2.w or pt2.z > pt2.w*/) {
         // First & third point is outside clip space, second is in     
         Clip1(pt1, pt0, pt2, rasterizer);
      }
      else {
         // Only the first point is outside                             
         Clip2(pt1, pt2, pt0, rasterizer);
      }
   }
   else
   if (pt1.x < -pt1.w or pt1.x > pt1.w
   or  pt1.y < -pt1.w or pt1.y > pt1.w
   /*or  pt1.z < -pt1.w or pt1.z > pt1.w*/) {
      // First point is inside clip space                               
      // Second point is outside clip space                             
      if (pt2.x < -pt2.w or pt2.x > pt2.w
      or  pt2.y < -pt2.w or pt2.y > pt2.w
      /*or  pt2.z < -pt2.w or pt2.z > pt2.w*/) {
         // First is in, second is out, third is out                    
         Clip1(pt0, pt1, pt2, rasterizer);
      }
      else {
         // First and third points are in, second is out                
         Clip2(pt0, pt2, pt1, rasterizer);
      }
   }
   else
   if (pt2.x < -pt2.w or pt2.x > pt2.w
   or  pt2.y < -pt2.w or pt2.y > pt2.w
   /*or  pt2.z < -pt2.w or pt2.z > pt2.w*/) {
      // First and second points are inside clip space                  
      // Only the third point is outside clip space                     
      Clip2(pt0, pt1, pt2, rasterizer);
   }
   else {
      // All points are inside clip space                               
      rasterizer(Triangle4 {pt0, pt1, pt2});
   }
}

/// Rasterize a single triangle                                               
///   @tparam LIT - whether or not to calculate lights and speculars          
///   @tparam DEPTH - whether or not to perform depth test and write depth    
///   @tparam SMOOTH - interpolate normals/colors inside trianlges            
///   @param ps - the pipeline state                                          
///   @param M - precomputed model orientation matrix for rotating normals    
///   @param triangle - pointer to the first vertex of three consecutive ones 
template<bool LIT, bool DEPTH, bool SMOOTH>
void ASCIIPipeline::RasterizeTriangle(
   const PipelineState& ps, const Mat3& M,
   const ASCIIGeometry::Vertex* triangle,
   const Triangle4& clipped
) const {
   const Vec3 p0 = clipped[0].xyz() / clipped[0].w;
   const Vec3 p1 = clipped[1].xyz() / clipped[1].w;
   const Vec3 p2 = clipped[2].xyz() / clipped[2].w;

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

   const Vec2i minp
      = Math::Max(Math::Floor(Math::Min(p0, p1, p2) * 0.5 + 0.5)
      * ps.mResolution, 0);
   const Vec2i maxp
      = Math::Min(Math::Ceil (Math::Max(p0, p1, p2) * 0.5 + 0.5)
      * ps.mResolution, ps.mResolution);

   // Clip                                                              
   if (maxp.x < 0 or maxp.y < 0
   or  minp.x >= ps.mResolution.x
   or  minp.y >= ps.mResolution.y) {
      // Triangle is fully outside view                                 
      return;
   }

   Normal n {0, 0, 1};

   if constexpr (not SMOOTH) {
      // Get an average normal for the triangle for flat rendering      
      n = M * Vec3(triangle[0].mNor + triangle[1].mNor + triangle[2].mNor)
              .Normalize();
   }

   // Iterate all pixels in the area of interest                        
   for (int y = minp.y; y < maxp.y; ++y) {
      bool row_started = false;

      for (int x = minp.x; x < maxp.x; ++x) {
         const Vec2 screenuv = (Vec2(x, y) * 2 - ps.mResolution + 0.5)
            / ps.mResolution;
         const auto s = term_a * (term_s1
            + term_s2 *  screenuv.x
            + term_s3 * -screenuv.y);
         const auto t = term_a * (term_t1
            + term_t2 *  screenuv.x
            + term_t3 * -screenuv.y);
         const auto d = 1 - s - t;

         if (s < 0 or t < 0 or d < 0) {
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
         row_started = true;

         if constexpr (DEPTH) {
            // Interpolate depth at the current pixel                   
            const auto z = p1.z * s + p2.z * t + p0.z * d;
            auto& global_depth = ps.mLayer->mDepth.Get(x / mBufferScale.x, y / mBufferScale.y);

            // Do depth test                                            
            if (z >= global_depth or z <= 0 or z >= 1)
               continue;

            global_depth = mDepth.Get(x, y) = z;
         }

         //                                                             
         // If reached, pixel is overwritten                            
         RGBA& pixel = mBuffer.Get(x, y);

         // Interpolate the color                                       
         //TODO fix color multiplication with normalization, see todo.md
         /*pixel = triangle[1].mCol * s
               + triangle[2].mCol * t
               + triangle[0].mCol * d;*/

         if constexpr (LIT) {
            if constexpr (SMOOTH) {
               // Interpolate and transform the normal                  
               n = M * Vec3( triangle[0].mNor * d
                           + triangle[1].mNor * s
                           + triangle[2].mNor * t);

               //TODO apply light sources
               pixel = ps.mSubscriber.color
                     * n.Dot(Normal(1, 1, 0).Normalize());
            }
            else {
               // Flat triangles                                        
               pixel = ps.mSubscriber.color
                     * n.Dot(Normal(1, 1, 0).Normalize());
            }
         }
         else {
            // Just blend vertex color with the one provided from the   
            // instance                                                 
            pixel *= ps.mSubscriber.color;
         }
      }
   }
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

      if (mLit) {
         // Do a lighting pass, too                                     
         if (mDepthTest) {
            // Do a depth test and write                                
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3) {
               ClipTriangle(MVP, vertices + i, [&](const Triangle4& t) {
                  RasterizeTriangle<true, true, false>(ps, M, vertices + i, t);
               });
            }
         }
         else {
            // No depth testing/writing                                 
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3) {
               ClipTriangle(MVP, vertices + i, [&](const Triangle4& t) {
                  RasterizeTriangle<true, false, false>(ps, M, vertices + i, t);
               });
            }
         }
      }
      else {
         // No lighting                                                 
         if (mDepthTest) {
            // Do a depth test and write                                
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3) {
               ClipTriangle(MVP, vertices + i, [&](const Triangle4& t) {
                  RasterizeTriangle<false, true, false>(ps, M, vertices + i, t);
               });
            }
         }
         else {
            // No depth testing/writing                                 
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3) {
               ClipTriangle(MVP, vertices + i, [&](const Triangle4& t) {
                  RasterizeTriangle<false, false, false>(ps, M, vertices + i, t);
               });
            }
         }
      }
   }
   else TODO();
}
