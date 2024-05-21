///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "../ASCII.hpp"


/// Texture constructor                                                       
///   @param producer - the texture producer                                  
///   @param descriptor - the texture descriptor                              
ASCIITexture::ASCIITexture(ASCIIRenderer* producer, const Neat& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor}
   , mImage       {producer} {
   descriptor.ForEachDeep([&](const A::Image& content) {
      Upload(content);
   });
}

/// Initialize from the provided content                                      
///   @param content - the abstract texture content interface                 
void ASCIITexture::Upload(const A::Image& content) {
   TODO(); //compile into intermediate format
}

/// Get the image                                                             
///   @return a reference to the image                                        
auto ASCIITexture::GetImage() const noexcept -> const ASCIIImage& {
   return mImage;
}
