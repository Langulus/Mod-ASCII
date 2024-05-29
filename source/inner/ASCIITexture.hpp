///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "ASCIIBuffer.hpp"


///                                                                           
///   ASCII intermediate image container                                      
///                                                                           
/// Optimizes a image asset for cache friendly CPU bound rasterization        
///                                                                           
struct ASCIITexture : A::Graphics, ProducedFrom<ASCIIRenderer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

private:
   ASCIIImage mImage;

   void Upload(const A::Image&);

public:
   ASCIITexture(ASCIIRenderer*, const Neat&);

   NOD() auto GetImage() const noexcept -> const ASCIIImage&;
};
