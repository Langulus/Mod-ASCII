///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Export.hpp"


///                                                                           
///   Light source unit                                                       
///                                                                           
struct ASCIILight final : Things::Light, ProducedFrom<ASCIILayer> {
   using CTTI_Abstract = No;
   using CTTI_Producer = ASCIILayer;
   LANGULUS_BASES(Things::Light);

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   Pin<RGBA, Tags::Color> mColor = Colors::White;
   TMany<const Things::Instance*> mInstances;
   TRange<Level> mLevelRange;
   Scale2 mShadowmapSize = {64, 64};
   Degrees mSpotlightSize = 90;

public:
   ASCIILight(ASCIILayer*, Many const&);

   auto GetColor() const -> RGBA;
   auto GetProjection(Range1 depth) const -> Mat4;

   void Refresh();
   void Teardown();
};
