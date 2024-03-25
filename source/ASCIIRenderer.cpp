///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "ASCII.hpp"
#include <Langulus/Platform.hpp>
#include <set>


/// Descriptor constructor                                                    
///   @param producer - the renderer producer                                 
///   @param descriptor - the renderer descriptor                             
ASCIIRenderer::ASCIIRenderer(ASCII* producer, const Neat& descriptor)
   : Renderer {MetaOf<ASCIIRenderer>()}
   , ProducedFrom {producer, descriptor}
   , mLayers {this}
   , mPipelines {this} {
   VERBOSE_ASCII("Initializing...");

   // Retrieve relevant traits from the environment                     
   mWindow = SeekUnitAux<A::Window>(descriptor);
   LANGULUS_ASSERT(mWindow, Construct,
      "No window available for renderer - did you create a window component "
      "_before_ creating the renderer?");

   if (not SeekValueAux<Traits::Size>(descriptor, mResolution))
      mResolution = mWindow->GetSize();

   SeekValueAux<Traits::Time>(descriptor, mTime);
   SeekValueAux<Traits::MousePosition>(descriptor, mMousePosition);
   SeekValueAux<Traits::MouseScroll>(descriptor, mMouseScroll);
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Destroy anything created                                                  
void ASCIIRenderer::Detach() {
   mLayers.Reset();
   mWindow.Reset();
   ProducedFrom<ASCII>::Detach();
}

/// Renderer destruction                                                      
ASCIIRenderer::~ASCIIRenderer() {
   Detach();
}

/// React to changes in environment                                           
void ASCIIRenderer::Refresh() {
   // Resize swapchain from the environment                             
   const Scale2 previousResolution = *mResolution;
   if (not SeekValue<Traits::Size>(mResolution))
      mResolution = mWindow->GetSize();

   if (*mResolution != previousResolution) {
      TODO();
   }

   // Refresh time and mouse properties                                 
   SeekValue<Traits::Time>(mTime);
   SeekValue<Traits::MousePosition>(mMousePosition);
   SeekValue<Traits::MouseScroll>(mMouseScroll);
}

/// Introduce renderables, cameras, lights, shaders, textures, geometry       
/// Also initialized the renderer if a window is provided                     
///   @param verb - creation verb                                             
void ASCIIRenderer::Create(Verb& verb) {
   mLayers.Create(verb);
   mPipelines.Create(verb);
}

/// Interpret the renderer as Text, i.e. take a "screenshot"                  
///   @param verb - interpret verb                                            
void ASCIIRenderer::Interpret(Verb& verb) {
   verb.ForEach([&](DMeta meta) {
      if (meta->template CastsTo<Text>())
         TODO();
   });
}

/// Render an object, along with all of its children                          
/// Rendering pipeline depends on each entity's components                    
void ASCIIRenderer::Draw() {
   if (mWindow->IsMinimized())
      return;

   // Generate the draw lists for all layers                            
   // This will populate uniform buffers for all relevant pipelines     
   PipelineSet relevantPipes;
   for (auto& layer : mLayers)
      layer.Generate(relevantPipes);

   RenderConfig config { " ", 1_real };

   if (mLayers) {
      // Render all layers                                              
      for (const auto& layer : mLayers)
         layer.Render(config);
   }
   else {
      // No layers available, so just clear screen                      
      TODO();
   }
}

/// Get the window interface                                                  
///   @return the window interface                                            
const A::Window* ASCIIRenderer::GetWindow() const noexcept {
   return mWindow;
}

/// Get the current resolution                                                
///   @return the resolution                                                  
const Scale2& ASCIIRenderer::GetResolution() const noexcept {
   return *mResolution;
}