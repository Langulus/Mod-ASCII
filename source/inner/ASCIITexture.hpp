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
/// Optimizes a image art for cache friendly CPU bound rasterization          
struct ASCIITexture : Things::Graphics, ProducedFrom<ASCIIRenderer> {
   using CTTI_Abstract  = No;
   using CTTI_Bases     = Things::Graphics;

private:
   ASCIIImage mImage;

   void Upload(Things::Image const&);

public:
   ASCIITexture(ASCIIRenderer const*, Neat const&);

   auto GetImage() const noexcept -> ASCIIImage const&;
};
