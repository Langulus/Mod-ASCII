///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Common.hpp"
#include <Langulus/Image.hpp>


///                                                                           
///   An ASCII image                                                          
///                                                                           
///   Used for a backbuffer by the ASCII renderer, as well as a target type   
/// to Verbs::Interpret as, in order to render ASCII graphics                 
///                                                                           
struct ASCIIImage final : A::Image {
   using Style = Logger::Emphasis;

private:
   TAny<Text>     mSymbols;   // Array of utf8 encoded symbols          
   TAny<RGB>      mBgColors;  // Array of foreground colors             
   TAny<RGB>      mFgColors;  // Array of background colors             
   TAny<Style>    mStyle;     // Array of styles for each pixel         

public:
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

   ASCIIImage();
   ~ASCIIImage();

   /// A single pixel from the image                                          
   struct Pixel {
      Text&     mSymbol;
      RGB&      mFgColor;
      RGB&      mBgColor;
      Style&    mStyle;
   };

   void  Resize(int x, int y);
   Pixel GetPixel(int x, int y);
   void  Fill(const Text&, RGB fg = Colors::White, RGB bg = Colors::Black, Style = {});

   Ref<Image> GetLOD(const Math::LOD&) const { return {}; }
   void* GetGPUHandle() const noexcept { return nullptr; }
   bool Generate(TMeta, Offset = 0) { return true; }
};
