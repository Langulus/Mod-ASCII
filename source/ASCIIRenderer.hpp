///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "ASCIIPipeline.hpp"
#include "ASCIILayer.hpp"
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
   // Previous resolution (for detecting change)                        
   Pin<Scale2> mResolution;

   // Layers                                                            
   TFactory<ASCIILayer> mLayers;
   // Pipelines                                                         
   TFactoryUnique<ASCIIPipeline> mPipelines;

   // Backbuffer                                                        
   Text mBackbuffer;

public:
   ASCIIRenderer(ASCII*, const Neat&);
   ~ASCIIRenderer();

   void Detach();
   void Refresh() override;
   void Draw();

   void Create(Verb&);
   void Interpret(Verb&);

   NOD() const A::Window* GetWindow() const noexcept;
   NOD() const Scale2& GetResolution() const noexcept;
};
