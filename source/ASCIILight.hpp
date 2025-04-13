///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"


///                                                                           
///   Light source unit                                                       
///                                                                           
struct ASCIILight final : A::Light, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCIILayer;
   LANGULUS_BASES(A::Light);

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   RTTI::Tag<Pin<RGBA>, Traits::Color> mColor = Colors::White;
   TMany<const A::Instance*> mInstances;
   TRange<Level> mLevelRange;
   Scale2 mShadowmapSize = {64, 64};
   Degrees mSpotlightSize = 90;

public:
   ASCIILight(ASCIILayer*, const Many&);

   auto GetColor() const -> RGBA;
   auto GetProjection(Range1 depth) const -> Mat4;

   void Refresh();
   void Teardown();
};
