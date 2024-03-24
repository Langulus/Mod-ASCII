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
///   @param producer - the light producer                                    
///   @param descriptor - the light descriptor                                
ASCIILight::ASCIILight(ASCIILayer* producer, const Neat& descriptor)
   : Graphics {MetaOf<ASCIILight>()}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}
