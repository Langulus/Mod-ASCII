///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "../ASCII.hpp"


/// Texture constructor                                                       
///   @param producer - the texture producer                                  
///   @param descriptor - the texture descriptor                              
ASCIITexture::ASCIITexture(ASCIIRenderer* producer, Many const& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor}
   , mImage       {producer} {
   descriptor.ForEachDeep([&](const Things::Image& content) {
      Upload(content);
   });
}

/// Initialize from the provided content                                      
///   @param content - the abstract texture content interface                 
void ASCIITexture::Upload(const Things::Image&) {
   TODO(); //compile into intermediate format
}

/// Get the image                                                             
///   @return a reference to the image                                        
auto ASCIITexture::GetImage() const noexcept -> const ASCIIImage& {
   return mImage;
}
