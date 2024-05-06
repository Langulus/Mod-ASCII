
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
///   @param producer - the camera producer                                   
///   @param descriptor - the camera descriptor                               
ASCIICamera::ASCIICamera(ASCIILayer* producer, const Neat& descriptor)
   : Resolvable {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Compile the camera                                                        
void ASCIICamera::Compile() {
   mResolution = mProducer->mProducer->mWindow->GetSize();

   if (mResolution.x <= 1)
      mResolution.x = 1;
   if (mResolution.y <= 1)
      mResolution.y = 1;

   mAspectRatio = static_cast<Real>(mResolution.x)
                / static_cast<Real>(mResolution.y);
   mViewport.mMax.xy() = mResolution;

   if (mPerspective) {
      // Perspective is enabled, so use FOV, aspect ratio, and viewport 
      // The final projection coordinates should look like that:        
      //                                                                
      //                  +Aspect*Y                                     
      //                      ^    ^ looking at +Z (towards the screen) 
      //                      |   /                                     
      //               -X+Y   |  /      +X+Y                            
      //                      | /                                       
      //                      |/                                        
      //   -1X <--------------+--------------> +1X                      
      //                screen center                                   
      //                      |                                         
      //               -X-Y   |         +X-Y                            
      //                      v                                         
      //                  -Aspect*Y                                     
      //                                                                
      mProjection = A::Matrix::PerspectiveFOV(
         mFOV, mAspectRatio, mViewport.mMin.z, mViewport.mMax.z
      );
   }
   else {
      mViewport.mMin.z = -100;
      mViewport.mMax.z = +100;

      // Orthographic is enabled, so use only viewport                  
      // Origin shall be at the top-left, x/y increasing bottom-right   
      // The final projection coordinates should look like that:        
      //                                                                
      //   top-left screen corner                                       
      //     +--------------> +X                                        
      //     |                      looking at +Z (towards the screen)  
      //     |         +X+Y                                             
      //     v                                                          
      //   +Aspect*Y                                                    
      //                                                                

      mProjection = mProjection.Null();
      const auto range = mViewport.mMax.z - mViewport.mMin.z;
      mProjection.mArray[0]  =  2.0f / mResolution.x;
      mProjection.mArray[5]  = -2.0f / mResolution.y;
      mProjection.mArray[10] = -2.0f / range;
      mProjection.mArray[12] = -1.f;
      mProjection.mArray[13] =  1.f;
      mProjection.mArray[14] =  1.0f / range;
      mProjection.mArray[15] =  1.0f;

      /*mProjection = A::Matrix::Orthographic(
         mViewport.mMax[0], mViewport.mMax[1], 
         mViewport.mMin[2], mViewport.mMax[2]
      );*/
   }

   mProjectionInverted = mProjection.Invert();

   //const auto viewport = mViewport.Length();
   //const auto offset   = mViewport.mMin;
}

/// Recompile the camera                                                      
void ASCIICamera::Refresh() {
   mInstances = GatherUnits<A::Instance, Seek::Here>();
}

/// Get view transformation for a given LOD state                             
///   @param lod - the level-of-detail state                                  
///   @return the view transformation for the camera                          
Mat4 ASCIICamera::GetViewTransform(const LOD& lod) const {
   if (not mInstances)
      return {};

   return mInstances[0]->GetViewTransform(lod);
}

/// Get view transformation for a given level                                 
///   @param level - the level                                                
///   @return the view transformation for the camera                          
Mat4 ASCIICamera::GetViewTransform(const Level& level) const {
   if (not mInstances)
      return {};

   return mInstances[0]->GetViewTransform(level);
}
