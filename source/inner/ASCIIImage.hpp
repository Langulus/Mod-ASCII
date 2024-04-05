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
#include <Flow/Verbs/Compare.hpp>


///                                                                           
///   An ASCII image                                                          
///                                                                           
///   Used for a backbuffer by the ASCII renderer, as well as a target type   
/// to Verbs::Interpret as, in order to render ASCII graphics                 
///                                                                           
struct ASCIIImage final : A::Image {
   using Style = Logger::Emphasis;

private:
   mutable TAny<Text>     mSymbols;   // Array of utf8 encoded symbols  
   mutable TAny<RGB>      mBgColors;  // Array of foreground colors     
   mutable TAny<RGB>      mFgColors;  // Array of background colors     
   mutable TAny<Style>    mStyle;     // Array of styles for each pixel 

   ASCIIRenderer* mRenderer;

   bool CompareInner(const A::Image&) const;

public:
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Image);
   LANGULUS_VERBS(Verbs::Compare);

   ASCIIImage() = delete;
   ASCIIImage(ASCIIRenderer*);

   /// A single pixel from the image                                          
   struct Pixel {
      Text&     mSymbol;
      RGB&      mFgColor;
      RGB&      mBgColor;
      Style&    mStyle;

      bool operator == (const RGBA&) const noexcept;
   };

   void  Resize(int x, int y);
   Pixel GetPixel(int x, int y) const;
   void  Fill(const Text&, RGB fg = Colors::White, RGB bg = Colors::Black, Style = {});
   void  Compare(Verb&) const;
   auto  ForEachPixel(auto&&) const;

   Ref<Image> GetLOD(const Math::LOD&) const { return {}; }
   void* GetGPUHandle() const noexcept { return nullptr; }
   bool Generate(TMeta, Offset = 0) { return true; }
};
