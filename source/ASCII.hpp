///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "ASCIIRenderer.hpp"


///                                                                           
///   ASCII graphics module                                                   
///                                                                           
/// Manages and produces ASCII renderers                                      
///                                                                           
struct ASCII final : A::GraphicsModule {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::GraphicsModule);
   LANGULUS_VERBS(Verbs::Create);

protected:
   friend struct ASCIIRenderer;

   // List of renderer components                                       
   TFactory<ASCIIRenderer> mRenderers;

public:
   ASCII(Runtime*, const Neat&);

   bool Update(Time);
   void Create(Verb&);
};
