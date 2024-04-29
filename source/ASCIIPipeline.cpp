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
   : Resolvable {this}
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
void ASCIIPipeline::Resize(int x, int y) {
   mBuffer.Resize(x * mBufferXScale, y * mBufferYScale);
   mBuffer.Fill({0, 0, 0, 0});
}

/// Draw a single renderable (used in hierarchical drawing)                   
///   @param pv - the projection-view matrix                                  
///   @param sub - prepared renderable instance LOD to draw                   
void ASCIIPipeline::Render(
   const ASCIILayer* layer, const Mat4& pv, const PipeSubscriber& sub
) const {
   if (not sub.mesh)
      return;

   CameraState cam {layer, mBuffer.GetView().GetScale(), pv, sub.transform};
   RasterizeMesh(cam, *sub.mesh);
}

/// Rasterize a single triangle                                               
///   @param camera - [in/out] reusable camera state                          
///   @param triangle - triangle to rasterize                                 
///   @param normal - the normal for the triangle (not smooth)                
void ASCIIPipeline::RasterizeTriangle(
   const CameraState& cam,
   const Triangle& triangle,
   const Vec3& normal,
   const RGBA& color
) const {
   if (color.a == 0) {
      // Whole triangle discarded by alpha test                         
      return;
   }

   // Transform to eye space                                            
   const Vec4f pt0 = cam.mProjectedView * Vec4f(triangle[0], 1);
   const Vec4f pt1 = cam.mProjectedView * Vec4f(triangle[1], 1);
   const Vec4f pt2 = cam.mProjectedView * Vec4f(triangle[2], 1);

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
   case CullFront:
      if (front)
         return;
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

   const auto minp = Math::Floor(Math::Min(p0, p1, p2) * cam.mResolution * 0.5f);
   const auto maxp = Math::Ceil (Math::Max(p0, p1, p2) * cam.mResolution * 0.5f);

   // Clip                                                              
   if (maxp.x < 0.0 or maxp.y < 0.0
   or minp.x >= cam.mResolution.x or minp.y >= cam.mResolution.y) {
      // Triangle is fully outside view                                 
      return;
   }

   // Iterate all pixels in the area of interest                        
   for (int y = static_cast<int>(minp.y);
            y < static_cast<int>(maxp.y); ++y) {
      for (int x = static_cast<int>(minp.x);
               x < static_cast<int>(maxp.x); ++x) {
         const Vec2f screenuv = (Vec2f(x, y) * 2.0f - cam.mResolution)
            / cam.mResolution.x;

         const float s = term_a * (term_s1
            + term_s2 *  screenuv.x
            + term_s3 * -screenuv.y);
         const float t = term_a * (term_t1
            + term_t2 *  screenuv.x
            + term_t3 * -screenuv.y);

         if (s <= 0 or t <= 0 or 1 - s - t <= 0) {
            // Pixel discarded (not inside the triangle)                
            continue;
         }

         // Depth test                                                  
         Vec4f test {1, s, t, 1 - s - t};
         float denominator = 1.0f / (test.y / pt1.w + test.z / pt2.w + test.w / pt0.w);
         float z = (
               (pt1.z * test.y) / pt1.w +
               (pt2.z * test.z) / pt2.w +
               (pt0.z * test.w) / pt0.w
            ) * denominator;


         auto& depth = cam.mLayer->mDepth.Get(x / mBufferXScale, y / mBufferYScale);
         if (z >= depth or z <= 0) {
            // Pixel discarded by depth test                            
            continue;
         }

         // If reached, pixel is overwritten                            
         mBuffer.Get(x, y) = color;

         // Only last pixel of a group is allowed to write depth and    
         // normal, otherwise subtle bugs might occur                   
         if ((x % mBufferXScale) == mBufferXScale - 1
         and (y % mBufferYScale) == mBufferYScale - 1) {
            depth = z;
            cam.mLayer->mNormals.Get(x / mBufferXScale, y / mBufferYScale) = normal;
         }
      }
   }
}

/// Rasterize all primitives inside a mesh                                    
///   @param camera - the camera state                                        
///   @param mesh - the mesh we want to rasterize                             
void ASCIIPipeline::RasterizeMesh(const CameraState& camera, const A::Mesh& mesh) const {
   // Create a buffer for each relevant data trait                      
   // Each data request will generate that data, if it hasn't yet       
   const auto posData = mesh.GetData<Traits::Place>();
   if (not posData)
      return; // Nothing to draw...                                     

   /*const auto norData = mesh.GetData<Traits::Aim>();
   const auto texData = mesh.GetData<Traits::Sampler>();
   const auto colData = mesh.GetData<Traits::Color>();
   const auto idxData = mesh.GetData<Traits::Index>();

   if (idxData) {
      // Draw indexed                                                   
   }
   else {

   }*/
}
