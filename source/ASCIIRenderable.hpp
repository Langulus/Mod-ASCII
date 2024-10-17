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
///   ASCII renderable element                                                
///                                                                           
/// Gives things the ability to be drawn to screen. The unit gathers relevant 
/// graphical resources from the context, and generates a graphical pipeline  
/// capable of visualizing them                                               
///                                                                           
struct ASCIIRenderable final : A::Renderable, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCIILayer;
   LANGULUS_BASES(A::Renderable);

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   RTTI::Tag<Pin<RGBA>, Traits::Color> mColor = Colors::White;
   TMany<const A::Instance*> mInstances;
   TRange<Level> mLevelRange;
   Ref<A::Mesh>  mGeometryContent;
   Ref<A::Image> mTextureContent;
   mutable Ref<ASCIIPipeline> mPredefinedPipeline;

   // Precompiled content, updated on Refresh()                         
   mutable struct {
      Ref<ASCIIGeometry> mGeometry;
      Ref<ASCIITexture>  mTexture;
      Ref<ASCIIPipeline> mPipeline;
   } mLOD[LOD::IndexCount];

public:
   ASCIIRenderable(ASCIILayer*, const Many&);

   NOD() auto GetRenderer() const noexcept -> ASCIIRenderer*;
   NOD() auto GetGeometry(const LOD&) const -> const ASCIIGeometry*;
   NOD() auto GetTexture(const LOD&) const -> const ASCIITexture*;
   NOD() auto GetColor() const -> RGBA;
   NOD() auto GetOrCreatePipeline(const LOD&, const ASCIILayer*) const -> ASCIIPipeline*;

   void Refresh();
   void Teardown();
};
