///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Langulus.hpp>
#include <Math/Color.hpp>
#include <Langulus/Material.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/Platform.hpp>

LANGULUS_EXCEPTION(Graphics);

using namespace Langulus;
using namespace Math;

struct ASCII;
struct ASCIIRenderer;
struct ASCIILayer;
struct ASCIICamera;
struct ASCIILight;
struct ASCIIRenderable;
struct ASCIIPipeline;
struct ASCIIImage;

#if 1
   #define VERBOSE_ASCII_ENABLED() 1
   #define VERBOSE_ASCII(...)      Logger::Verbose(Self(), __VA_ARGS__)
   #define VERBOSE_ASCII_TAB(...)  const auto tab = Logger::Verbose(Self(), __VA_ARGS__, Logger::Tabs {})
#else
   #define VERBOSE_ASCII_ENABLED() 0
   #define VERBOSE_ASCII(...)      
   #define VERBOSE_ASCII_TAB(...)  
#endif