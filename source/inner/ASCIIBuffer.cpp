///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "../ASCII.hpp"


/// Default constructor                                                       
ASCIIImage::ASCIIImage(ASCIIRenderer* renderer)
   : Resolvable {this}
   , mRenderer  {renderer} {
   VERBOSE_ASCII("Initializing...");
   // Member arrays are commited as references to reduce boilerplate    
   // but beware of descriptor-content disparity if this Image class    
   // is produced from factories at some point                          
   Commit(&mSymbols);
   Commit<Traits::Color>(&mFgColors);
   Commit<Traits::Color>(&mBgColors);
   Commit(&mStyle);
   VERBOSE_ASCII("Initialized");
}

/// Reset the image                                                           
void ASCIIImage::Reset() {
   mView = {};
   mDataListMap.Reset();
   mSymbols.Reset();
   mBgColors.Reset();
   mFgColors.Reset();
   mStyle.Reset();
}

/// Resize the image                                                          
///   @param x - new width                                                    
///   @param y - new height                                                   
void ASCIIImage::Resize(int x, int y) {
   LANGULUS_ASSUME(DevAssumes, x and y, "Invalid resize dimensions");
   if (x == static_cast<int>(mView.mWidth)
   and y == static_cast<int>(mView.mHeight))
      return;

   const auto count = x * y;
   mSymbols.Clear();
   mSymbols.New(count, ' ');

   mFgColors.Clear();
   mFgColors.New(count, Colors::White);

   mBgColors.Clear();
   mBgColors.New(count, Colors::Black);

   mStyle.Clear();
   mStyle.New(count);

   mView.mWidth  = static_cast<uint32_t>(x);
   mView.mHeight = static_cast<uint32_t>(y);
}

/// Get a pixel at coordinates x, y                                           
///   @param x - the x coordinate                                             
///   @param y - the y coordinate                                             
///   @return the pixel interface                                             
ASCIIImage::Pixel ASCIIImage::GetPixel(int x, int y) const {
   LANGULUS_ASSUME(DevAssumes, x < static_cast<int>(mView.mWidth) and x >= 0,
      "Pixel out of horizontal limits");
   LANGULUS_ASSUME(DevAssumes, y < static_cast<int>(mView.mHeight) and y >= 0,
      "Pixel out of vertical limits");

   const auto index = y * static_cast<int>(mView.mWidth) + x;
   return {
      mSymbols[index],
      mFgColors[index],
      mBgColors[index],
      mStyle[index]
   };
}

/// Fill the image with a single symbol and style                             
///   @param s - the utf8 encoded symbol that will be displayed everywhere    
///   @param fg - the color that will be used for the background              
///   @param bg - the color that will be used for the foreground              
///   @param f - the emphasis that will be used                               
void ASCIIImage::Fill(const Text& s, RGB fg, RGB bg, Style f) {
   mSymbols.Fill(s);
   mFgColors.Fill(fg);
   mBgColors.Fill(bg);
   mStyle.Fill(f);
}

/// Iterate all pixels using the local Pixel representation                   
///   @tparam F - the function signature (deducible)                          
///   @param call - the function to execute for each pixel                    
///   @return the number of pixels that were iterated                         
auto ASCIIImage::ForEachPixel(auto&& call) const {
   using F = Deref<decltype(call)>;
   using A = ArgumentOf<F>;
   using R = ReturnOf<F>;

   static_assert(CT::Exact<A, const Pixel&>,
      "Pixel iterator must be constant ASCIIImage::Pixel reference");

   UNUSED() Count counter = 0;
   for (uint32_t y = 0; y < mView.mHeight; ++y) {
      for (uint32_t x = 0; x < mView.mWidth; ++x) {
         if constexpr (CT::Bool<R>) {
            if (not call(GetPixel(x, y)))
               return counter;
            ++counter;
         }
         else call(GetPixel(x, y));
      }
   }

   if constexpr (CT::Bool<R>)
      return counter;
}

/// Now, since this is an ASCII image, pixel-color comparisons are a bit weird
/// - we must either compare against fullblock “█” (U+2588) symbol with the   
/// same foreground color, or a space " " symbol with the same background     
/// color (unless inverted)                                                   
bool ASCIIImage::Pixel::operator == (const RGBA& color) const noexcept {
   if (mStyle == Style::Default) {
      return (mSymbol == "█" and mFgColor == color)
          or (mSymbol == " " and mBgColor == color);
   }
   else if (mStyle == Style::Reverse) {
      return (mSymbol == "█" and mBgColor == color)
          or (mSymbol == " " and mFgColor == color);
   }
   else return false;
}

/// Compare image to another image/uniform color, etc.                        
///   @param verb - the comparison verb                                       
void ASCIIImage::Compare(Verb& verb) const {
   if (verb.CastsTo<A::Color>()) {
      // Compare against colors                                         
      if (verb.GetCount() == 1) {
         // Check if texture is filled with a uniform color             
         const auto cast = verb.AsCast<RGBA>();
         const auto color = mView.mReverseFormat
            ? RGBA (cast[2], cast[1], cast[0], cast[3])
            : cast;

         const auto matches = ForEachPixel(
            [&color](const Pixel& pixel) noexcept -> bool {
               return pixel == color;
            });

         verb << (matches == mView.GetPixelCount()
            ? Compared::Equal
            : Compared::Unequal);
      }
   }
   else if (verb.CastsTo<A::Image>()) {
      // Compare against other images                                   
      verb << (CompareInner(verb->As<Image>())
         ? Compared::Equal
         : Compared::Unequal);
   }
   else if (verb.CastsTo<A::Text>()) {
      // Compare against other image files (need to be loaded by an     
      // asset module)                                                  
      Verbs::Create rhs {Construct::From<Image>(verb.GetArgument())};
      verb << (CompareInner(mRenderer->RunIn(rhs)->As<Image>())
         ? Compared::Equal
         : Compared::Unequal);
   }
}

/// Inner pixel-by-pixel comparison                                           
/// Accounts for inversed pixel formats                                       
///   @param rhs - the image to compare against                               
///   @return true if both images match exactly                               
bool ASCIIImage::CompareInner(const A::Image& rhs) const {
   if (rhs.GetView() == GetView()
   and dynamic_cast<const ASCIIImage*>(&rhs)) {
      // We can batch-compare - both images are ASCII                   
      TODO(); //this will actually fail because we keep pointers in the data map - i think it would be best if we add ASCII image support directly in the image module
      return GetDataListMap() == rhs.GetDataListMap();
   }
   else if (rhs.GetView().mHeight == GetView().mHeight
   and rhs.GetView().mWidth  == GetView().mWidth) {
      // We have to compare pixel-by-pixel, because formats differ      
      auto rit = rhs.begin();
      return mView.GetPixelCount() == ForEachPixel(
         [&rit](const Pixel& pixel) noexcept -> bool {
            return pixel == (rit++).As<RGBA>();
         });
   }

   return false;
}

/// Copy another image, skip symbols that are space - they are considered     
/// 'transparent'                                                             
///   @param other - the image to copy                                        
void ASCIIImage::Copy(const ASCIIImage& other) {
   for (uint32_t y = 0; y < GetView().mHeight; ++y) {
      for (uint32_t x = 0; x < GetView().mWidth; ++x) {
         auto to = GetPixel(x, y);
         auto from = other.GetPixel(x, y);

         if (from.mSymbol != " ") {
            to.mSymbol = from.mSymbol;
            to.mBgColor = from.mBgColor;
            to.mFgColor = from.mFgColor;
            to.mStyle = from.mStyle;
         }
      }
   }
}
