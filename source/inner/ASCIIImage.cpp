///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "../ASCII.hpp"


/// Default constructor                                                       
ASCIIImage::ASCIIImage()
   : Resolvable {this} {
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

void ASCIIImage::Detach() {
   mSymbols.Reset();
   mFgColors.Reset();
   mBgColors.Reset();
   mStyle.Reset();
   A::Image::Detach();
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
ASCIIImage::Pixel ASCIIImage::GetPixel(int x, int y) {
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
