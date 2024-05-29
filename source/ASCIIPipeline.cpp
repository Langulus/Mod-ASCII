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
ASCIIPipeline::ASCIIPipeline(ASCIIRenderer* producer, const Neat& descriptor)
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

/// Rasterize a single triangle                                               
///   @param ps - the pipeline state                                          
///   @param MVP - precomputed model*view*projection matrix                   
///   @param triangle - pointer to the first vertex of three consecutive ones 
template<bool LIT, bool DEPTH>
void ASCIIPipeline::RasterizeTriangle(
   const PipelineState& ps, const Mat4& M, const Mat4& MVP,
   const ASCIIGeometry::Vertex* triangle
) const {
   // Transform to eye space                                            
   const Vec4 pt0 = MVP * triangle[0].mPos;
   const Vec4 pt1 = MVP * triangle[1].mPos;
   const Vec4 pt2 = MVP * triangle[2].mPos;

   const Vec2 p0 = pt0.xy() / pt0.w;
   const Vec2 p1 = pt1.xy() / pt1.w;
   const Vec2 p2 = pt2.xy() / pt2.w;

   const auto a = 0.5_real * (
      -p1.y * p2.x + 
       p0.y * (-p1.x + p2.x) + 
       p0.x * ( p1.y - p2.y) + 
       p1.x * p2.y
   );
   const bool front = a <= 0;

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
   const auto term_a  = 1.0_real / (2.0_real * a);
   const auto term_s1 = p0.y * p2.x - p0.x * p2.y;
   const auto term_s2 = p2.y - p0.y;
   const auto term_s3 = p0.x - p2.x;
   const auto term_t1 = p0.x * p1.y - p0.y * p1.x;
   const auto term_t2 = p0.y - p1.y;
   const auto term_t3 = p1.x - p0.x;

   const Vec2i minp
      = Math::Floor(Math::Min(p0, p1, p2) * 0.5 + 0.5)
      * ps.mResolution;
   const Vec2i maxp
      = Math::Ceil (Math::Max(p0, p1, p2) * 0.5 + 0.5)
      * ps.mResolution;

   // Clip                                                              
   if (maxp.x < 0 or maxp.y < 0
   or minp.x >= ps.mResolution.x or minp.y >= ps.mResolution.y) {
      // Triangle is fully outside view                                 
      return;
   }

   // Iterate all pixels in the area of interest                        
   for (auto y = minp.y; y < maxp.y; ++y) {
      bool row_started = false;

      for (auto x = minp.x; x < maxp.x; ++x) {
         const Vec2 screenuv = (Vec2(x, y) * 2 - ps.mResolution + 0.5)
            / ps.mResolution;
         const auto s = term_a * (term_s1
            + term_s2 *  screenuv.x
            + term_s3 * -screenuv.y);
         const auto t = term_a * (term_t1
            + term_t2 *  screenuv.x
            + term_t3 * -screenuv.y);
         const auto d = 1 - s - t;

         if (s <= 0 or t <= 0 or d <= 0) {
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

         if constexpr (DEPTH) {
            Vec4 test {1, s, t, d};
            const auto denominator = 1.0_real
               / (test.y / pt1.w + test.z / pt2.w + test.w / pt0.w);
            const auto z = (
               (pt1.z * test.y) / pt1.w +
               (pt2.z * test.z) / pt2.w +
               (pt0.z * test.w) / pt0.w
               ) * denominator;

            // Do depth test                                            
            auto& global_depth = ps.mLayer->mDepth.Get(x / mBufferScale.x, y / mBufferScale.y);
            if (z > global_depth or z <= 0)
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
            // Interpolate and transform the normal                     
            const Normal n = M * Vec4( triangle[0].mNor * d
                                     + triangle[1].mNor * s
                                     + triangle[2].mNor * t, 0);

            //TODO apply light sources
            pixel = ps.mSubscriber.color
                  * n.Dot(Normal(1, 1, 0).Normalize());
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
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3)
               RasterizeTriangle<true, true>(ps, M, MVP, vertices + i);
         }
         else {
            // No depth testing/writing                                 
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3)
               RasterizeTriangle<true, false>(ps, M, MVP, vertices + i);
         }
      }
      else {
         // No lighting                                                 
         if (mDepthTest) {
            // Do a depth test and write                                
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3)
               RasterizeTriangle<false, true>(ps, M, MVP, vertices + i);
         }
         else {
            // No depth testing/writing                                 
            for (Offset i = 0; i < mesh->GetVertices().GetCount(); i += 3)
               RasterizeTriangle<false, false>(ps, M, MVP, vertices + i);
         }
      }
   }
   else TODO();
}
