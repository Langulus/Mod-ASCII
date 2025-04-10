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
