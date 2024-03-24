///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
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
ASCII::ASCII(Runtime* runtime, const Neat&)
   : A::GraphicsModule {MetaOf<ASCII>(), runtime}
   , mRenderers {this} {
   VERBOSE_ASCII("Initializing...");
   VERBOSE_ASCII("Initialized");
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
   mRenderers.Create(verb);
}
