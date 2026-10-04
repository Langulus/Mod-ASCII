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
///   ASCII renderable element                                                
///                                                                           
/// Gives things the ability to be drawn to screen. The unit gathers relevant 
/// graphical resources from the context, and generates a graphical pipeline  
/// capable of visualizing them                                               
///                                                                           
struct ASCIIRenderable final : Things::Renderable, ProducedFrom<ASCIILayer> {
   using CTTI_Abstract  = No;
   using CTTI_Producer  = ASCIILayer;
   using CTTI_Bases     = Things::Renderable;

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   Pin<RGBA, Tags::Color> mColor = Colors::White;
   TMany<const Things::Instance*> mInstances;
   TRange<Level> mLevelRange;
   Ref<Things::Mesh>  mGeometryContent;
   Ref<Things::Image> mTextureContent;
   mutable Ref<ASCIIPipeline> mPredefinedPipeline;

   // Precompiled content, updated on Refresh()                         
   mutable struct {
      Ref<ASCIIGeometry> mGeometry;
      Ref<ASCIITexture>  mTexture;
      Ref<ASCIIPipeline> mPipeline;
   } mLOD[LOD::IndexCount];

public:
   ASCIIRenderable(ASCIILayer*, Many const&);

   auto GetRenderer() const noexcept -> ASCIIRenderer*;
   auto GetGeometry(const LOD&) const -> const ASCIIGeometry*;
   auto GetTexture(const LOD&) const -> const ASCIITexture*;
   auto GetColor() const -> RGBA;
   auto GetOrCreatePipeline(const LOD&, const ASCIILayer*) const -> ASCIIPipeline*;

   void Refresh();
   void Teardown();
};
