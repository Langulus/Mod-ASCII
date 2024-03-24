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


///                                                                           
///   Light source unit                                                       
///                                                                           
struct ASCIILight : A::Graphics, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

   ASCIILight(ASCIILayer*, const Neat&);
};
