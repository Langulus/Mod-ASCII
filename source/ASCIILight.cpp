///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"


/// Descriptor constructor                                                    
///   @param producer - the light producer                                    
///   @param descriptor - the light descriptor                                
ASCIILight::ASCIILight(ASCIILayer* producer, const Many& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// First stage of destruction                                                
void ASCIILight::Teardown() {
   mInstances.Reset();
}

/// Get light color                                                           
///   @return the color                                                       
auto ASCIILight::GetColor() const -> RGBA {
   return *mColor;
}

/// The projection associated with the light. Depends on the type of light:   
///   - directional lights use an orthographic projection                     
///   - spot lights use a perspective projection with custom FOV              
///   - point lights use a 90 degree FOV projection that is applied to each   
///     side of a shadow cubemap                                              
///   - domain lights aren't projected - they're drawn into a volume          
///   @return the projection matrix                                           
auto ASCIILight::GetProjection(Range1 depth) const -> Mat4 {
   switch (mType) {
   case Type::Directional:
      return A::Matrix::Orthographic<Real>(mShadowmapSize.x, mShadowmapSize.y, depth.mMin, depth.mMax);
   case Type::Point:
      return A::Matrix::PerspectiveFOV<Real>(90_deg, 1, depth.mMin, depth.mMax);
   case Type::Spot:
      return A::Matrix::PerspectiveFOV<Real>(mSpotlightSize, 1, depth.mMin, depth.mMax);
   case Type::Domain:
      return {};
   }
}

/// Called on environment change                                              
void ASCIILight::Refresh() {
   Teardown();

   // Gather all instances for this renderable, and calculate levels    
   mInstances = GatherUnits<A::Instance, Seek::Here>();
   if (mInstances)
      mLevelRange = mInstances[0]->GetLevel();
   else
      mLevelRange = {};

   for (auto instance : mInstances)
      mLevelRange.Embrace(instance->GetLevel());
}
