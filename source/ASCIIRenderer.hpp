///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "ASCIIPipeline.hpp"
#include "ASCIILayer.hpp"
#include "inner/ASCIITexture.hpp"
#include "inner/ASCIIGeometry.hpp"
#include <Flow/Verbs/Create.hpp>
#include <Flow/Verbs/Interpret.hpp>
#include <Math/Gradient.hpp>
#include <Entity/Pin.hpp>


///                                                                           
///   Vulkan renderer                                                         
///                                                                           
/// Binds with a window and renders to it. Manages framebuffers, VRAM         
/// contents, and layers                                                      
///                                                                           
struct ASCIIRenderer : A::Renderer, ProducedFrom<ASCII> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCII;
   LANGULUS_BASES(A::Renderer);
   LANGULUS_VERBS(Verbs::Create, Verbs::Interpret);

protected:
   friend struct ASCIIPipeline;
   friend struct ASCIICamera;
   friend struct ASCIILayer;

   //                                                                   
   // Runtime updatable variables                                       
   //                                                                   
   
   // The platform window, where the renderer is created                
   Ref<const A::Window> mWindow;
   // The time gradient, used for animations                            
   Ref<TGradient<Time>> mTime;
   // Mouse position, can be passed to shaders                          
   Ref<Grad2v2> mMousePosition;
   // Mouse scroll, can be passed to shaders                            
   Ref<Grad2v2> mMouseScroll;

   // Layers                                                            
   TFactory<ASCIILayer> mLayers;
   // Pipelines                                                         
   TFactoryUnique<ASCIIPipeline> mPipelines;

   // Geometry content mirror                                           
   TFactoryUnique<ASCIIGeometry> mGeometries;
   // Texture content mirror                                            
   TFactoryUnique<ASCIITexture> mTextures;

   // Backbuffer                                                        
   ASCIIImage mBackbuffer;

public:
   ASCIIRenderer(ASCII*, const Many&);

   auto Reference(int) -> Count;
   void Refresh() override;
   void Draw();

   void Create(Verb&);
   void Interpret(Verb&);

   NOD() auto GetWindow() const noexcept -> const A::Window*;
   NOD() auto GetResolution() const noexcept -> Scale2;
};
