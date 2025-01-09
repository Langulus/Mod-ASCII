///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../Common.hpp"
#include <Langulus/Image.hpp>
#include <Flow/Verbs/Compare.hpp>


///                                                                           
///   An ASCII buffer                                                         
///                                                                           
///   Used as an intermediate screen buffer, per-pipeline. At the end of      
/// rendering, all these pipeline buffers are combined into a single ASCII    
/// image, which is then displayed on the screen.                             
///   This intermediate image is required, because depending on the pipeline's
/// style, a different set of symbols are used, each requiring a different    
/// resolution and pixel->symbol mapping.                                     
///                                                                           
template<class T>
struct ASCIIBuffer final : A::Image {
private:
   // Data for the buffer                                               
   mutable TMany<T> mData;

public:
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Image);

   ASCIIBuffer() : Resolvable {this} {}

   void Resize(int x, int y) {
      LANGULUS_ASSUME(DevAssumes, x and y, "Invalid resize dimensions");
      if (x == static_cast<int>(mView.mWidth)
      and y == static_cast<int>(mView.mHeight))
         return;

      const auto count = x * y;
      mData.Clear();
      mData.New(count);
      mView.mWidth = static_cast<uint32_t>(x);
      mView.mHeight = static_cast<uint32_t>(y);
   }

   T& Get(int x, int y) {
      LANGULUS_ASSUME(DevAssumes,
         x < static_cast<int>(mView.mWidth) and x >= 0,
         "Pixel out of horizontal limits");
      LANGULUS_ASSUME(DevAssumes,
         y < static_cast<int>(mView.mHeight) and y >= 0,
         "Pixel out of vertical limits");
      return mData[y * static_cast<int>(mView.mWidth) + x];
   }

   void Fill(const T& v) {
      mData.Fill(v);
   }

   auto ForEachPixel(auto&& call) const {
      using F = Deref<decltype(call)>;
      using A = ArgumentOf<F>;
      using R = ReturnOf<F>;
      static_assert(CT::Similar<A, T>,
         "Pixel iterator must be CT::Similar to T");

      [[maybe_unused]] Count counter = 0;
      for (uint32_t y = 0; y < mView.mHeight; ++y) {
         for (uint32_t x = 0; x < mView.mWidth; ++x) {
            if constexpr (CT::Bool<R>) {
               if (not call(Get(x, y)))
                  return counter;
               ++counter;
            }
            else call(Get(x, y));
         }
      }

      if constexpr (CT::Bool<R>)
         return counter;
   }

   void Reset() {
      mData.Reset();
      mView = {};
      mDataListMap.Reset();
   }
};


///                                                                           
///   An ASCII image                                                          
///                                                                           
///   Used for a backbuffer by the ASCII renderer, as well as a target type   
/// to Verbs::Interpret as, in order to render ASCII graphics                 
///                                                                           
struct ASCIIImage final : A::Image {
   using Style = Logger::Emphasis;

private:
   mutable TMany<Text>  mSymbols;   // Array of utf8 encoded symbols    
   mutable TMany<RGB>   mBgColors;  // Array of foreground colors       
   mutable TMany<RGB>   mFgColors;  // Array of background colors       
   mutable TMany<Style> mStyle;     // Array of styles for each pixel   

   // Required only in case we're comparing against other images,       
   // provided by a filename                                            
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
      Text&  mSymbol;
      RGB&   mFgColor;
      RGB&   mBgColor;
      Style& mStyle;

      bool operator == (const RGBA&) const noexcept;
   };

   void Resize(int x, int y);
   auto GetPixel(int x, int y) const -> Pixel;
   void Fill(const Text&, RGB fg = Colors::White, RGB bg = Colors::Black, Style = {});
   void Compare(Verb&) const;
   void Copy(const ASCIIImage&);
   auto ForEachPixel(auto&&) const;
   void Reset();
};
