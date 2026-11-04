///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Export.hpp"
#include <Langulus/CppAPI/Physical.hpp>


///                                                                           
///   Light source part                                                       
struct ASCIILight final : Things::Light, ProducedFrom<ASCIILayer> {
   using CTTI_Abstract  = No;
   using CTTI_Producer  = ASCIILayer;
   using CTTI_Bases     = Things::Light;

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   Pin<RGBA, Tags::Color> mColor = Colors::White;
   TMany<Things::Instance const*> mInstances;
   LevelRange mLevelRange;
   Scale2 mShadowmapSize = {64, 64};
   Degrees mSpotlightSize = 90;

public:
   ASCIILight(ASCIILayer const*, Neat const&);

   auto GetColor() const -> RGBA;
   auto GetProjection(Range1 depth) const -> Mat4;

   void Refresh();
   void Teardown();
};
