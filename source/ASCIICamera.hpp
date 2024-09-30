///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"
#include <Langulus/Physical.hpp>
#include <Math/Range.hpp>
 
using LevelRange = TRange<Level>;


///                                                                           
///   ASCII Camera unit                                                       
///                                                                           
/// Provides fine control over camera properties, like field of view,         
/// screen viewport, aspect ratio, etc.                                       
///                                                                           
struct ASCIICamera final : A::Camera, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCIILayer;
   LANGULUS_BASES(A::Camera);

protected:
   friend struct ASCIILayer;

   // Whether or not a perspective projection is used                   
   bool mPerspective = true;
   // The projection matrix                                             
   Mat4 mProjection;
   // Clipping range in all directions, including depth                 
   Range4 mViewport {{0, 0, 0.1, 0}, {640, 480, 1000, 0}};
   // Horizontal field of view, in radians                              
   Radians mFOV {90_deg};
   // Aspect ratio (width / height)                                     
   Real mAspectRatio {Real {720} / Real {480}};
   // Human retina is 32 milimeters (10^-3) across, which means that    
   // we can observe stuff slightly smaller than human level            
   LevelRange mObservableRange {Level::Default, Level::Max};
   // Camera instances, for different points of view                    
   TMany<const A::Instance*> mInstances;
   // Inverse of mProjection                                            
   Mat4 mProjectionInverted;
   // The screen resolution (can be bigger than the viewport)           
   Scale2u32 mResolution {640, 480};

public:
   ASCIICamera(ASCIILayer*, const Many& = {});

   void Refresh();
   void Compile();
   NOD() Mat4 GetViewTransform(const LOD&) const;
   NOD() Mat4 GetViewTransform(const Level& = {}) const;
};
