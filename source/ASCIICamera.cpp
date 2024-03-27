
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

   if (mResolution[0] <= 1)
      mResolution[0] = 1;
   if (mResolution[1] <= 1)
      mResolution[1] = 1;

   mAspectRatio = static_cast<Real>(mResolution[0])
                / static_cast<Real>(mResolution[1]);
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
         mFOV, mAspectRatio, mViewport.mMin[2], mViewport.mMax[2]
      );
   }
   else {
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
      mProjection = A::Matrix::Orthographic(
         mViewport.mMax[0], mViewport.mMax[1], 
         mViewport.mMin[2], mViewport.mMax[2]
      );
   }

   mProjectionInverted = mProjection.Invert();

   const auto viewport = mViewport.Length();
   const auto offset = mViewport.mMin;
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
