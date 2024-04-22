///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"
#include <Langulus/Physical.hpp>
#include <Math/Range.hpp>
 
using LevelRange = TRange<Level>;


///                                                                           
///   Camera unit                                                             
///                                                                           
struct ASCIICamera final : A::Graphics, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

protected:
   friend struct ASCIILayer;

   // Whether or not a perspective projection is used                   
   bool mPerspective {true};
   // The projection matrix                                             
   Mat4 mProjection;
   // Clipping range in all directions, including depth                 
   Range4 mViewport {{0, 0, 0.1, 0}, {720, 480, 1000, 0}};
   // Horizontal field of view, in radians                              
   Radians mFOV {90_deg};
   // Aspect ratio (width / height)                                     
   Real mAspectRatio {Real {720} / Real {480}};
   // Human retina is 32 milimeters (10^-3) across, which means that    
   // we can observe stuff slightly smaller than human octave           
   LevelRange mObservableRange {Level::Default, Level::Max};
   // Eye separation. Stereo if more/less than zero                     
   Real mEyeSeparation {};

   TMany<const A::Instance*> mInstances;
   Mat4 mProjectionInverted;
   Scale2u32 mResolution {640, 480};

public:
   ASCIICamera(ASCIILayer*, const Neat& = {});

   void Refresh();
   void Compile();
   NOD() Mat4 GetViewTransform(const LOD&) const;
   NOD() Mat4 GetViewTransform(const Level& = {}) const;
};
