///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/CppAPI/Material.hpp>
#include <Langulus/CppAPI/Graphics.hpp>
#include <Langulus/CppAPI/Platform.hpp>
#include <Langulus/Math/Range.hpp>
#include <Langulus/Math/Color.hpp>

using namespace Langulus;
using namespace Math;

using Things::Thing;

struct ASCII;
struct ASCIIRenderer;
struct ASCIILayer;
struct ASCIICamera;
struct ASCIILight;
struct ASCIIRenderable;
struct ASCIIPipeline;
struct ASCIIImage;

struct ASCIIGeometry;
struct ASCIITexture;

using LevelRange = TRange<TVector<Level, 1>>;

#if 1
   #define VERBOSE_ASCII_ENABLED()  1
   #define VERBOSE_ASCII(...)       Logger::Verbose(this, __VA_ARGS__)
   #define VERBOSE_ASCII_TAB(...)   const auto tab = Logger::VerboseTab(this, __VA_ARGS__)
#else
   #define VERBOSE_ASCII_ENABLED()  0
   #define VERBOSE_ASCII(...)       LANGULUS(NOOP)
   #define VERBOSE_ASCII_TAB(...)   LANGULUS(NOOP)
#endif