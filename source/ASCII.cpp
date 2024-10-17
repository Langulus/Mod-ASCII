///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"

LANGULUS_DEFINE_MODULE(
   ASCII, 11, "ASCII",
   "ASCII graphics module", "",
   ASCII, ASCIIRenderer, ASCIILayer, ASCIICamera, ASCIIRenderable, ASCIILight
)


/// ASCII module construction                                                 
///   @param system - the system that owns the module instance                
///   @param handle - the library handle                                      
ASCII::ASCII(Runtime* runtime, const Many&)
   : Resolvable {this}
   , Module     {runtime} {
   VERBOSE_ASCII("Initializing...");
   VERBOSE_ASCII("Initialized");
}

/// First stage destruction                                                   
void ASCII::Teardown() {
   mRenderers.Teardown();
}

/// Module update routine                                                     
///   @param dt - time from last update                                       
bool ASCII::Update(Time) {
   for (auto& renderer : mRenderers)
      renderer.Draw();
   return true;
}

/// Create/destroy renderers                                                  
///   @param verb - the creation/destruction verb                             
void ASCII::Create(Verb& verb) {
   mRenderers.Create(this, verb);
}
