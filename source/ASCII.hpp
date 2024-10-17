///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
   ASCII(Runtime*, const Many&);

   bool Update(Time);
   void Create(Verb&);
   void Teardown();
};
