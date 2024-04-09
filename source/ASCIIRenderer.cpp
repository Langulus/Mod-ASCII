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
   : Resolvable {this}
   , ProducedFrom {producer, descriptor}
   , mLayers {this}
   , mPipelines {this}
   , mBackbuffer {this} {
   VERBOSE_ASCII("Initializing...");

   // Retrieve relevant traits from the environment                     
   mWindow = SeekUnitAux<A::Window>(descriptor);
   LANGULUS_ASSERT(mWindow, Construct,
      "No window available for renderer - did you create a window component "
      "_before_ creating the renderer?");

   SeekValueAux<Traits::Time>(descriptor, mTime);
   SeekValueAux<Traits::MousePosition>(descriptor, mMousePosition);
   SeekValueAux<Traits::MouseScroll>(descriptor, mMouseScroll);

   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Destroy anything created                                                  
void ASCIIRenderer::Detach() {
   mBackbuffer.Detach();
   mLayers.Reset();
   mPipelines.Reset();
   mWindow.Reset();
   ProducedFrom::Detach();
}

/// Renderer destruction                                                      
ASCIIRenderer::~ASCIIRenderer() {
   Detach();
}

/// React to changes in environment                                           
void ASCIIRenderer::Refresh() {
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

/// Interpret the renderer as an image, i.e. take an ascii "screenshot"       
///   @param verb - interpret verb                                            
void ASCIIRenderer::Interpret(Verb& verb) {
   verb.ForEach([&](DMeta meta) {
      if (meta->template CastsTo<A::Image>())
         verb << &mBackbuffer;
   });
}

/// Render an object, along with all of its children                          
/// Rendering pipeline depends on each entity's components                    
void ASCIIRenderer::Draw() {
   if (mWindow->IsMinimized())
      return;

   // Generate the draw lists for all layers                            
   // This will populate uniform buffers for all relevant pipelines     
   for (auto& layer : mLayers)
      layer.Generate();

   // Prepare the backbuffer                                            
   const int sizex = static_cast<int>(mWindow->GetSize().x);
   const int sizey = static_cast<int>(mWindow->GetSize().y);

   mBackbuffer.Resize(sizex, sizey);
   mBackbuffer.Fill(' ', Colors::White, Colors::Red);

   // Resize and clear the pipelines                                    
   for (auto& pipe : mPipelines)
      pipe.Resize(sizex, sizey);

   // Render all layers (if any)                                        
   RenderConfig config {" ", 1_real};
   for (const auto& layer : mLayers)
      layer.Render(config);

   // Send the rendered backbuffer to the window                        
   (void) mWindow->Draw(&mBackbuffer);
}

/// Get the window interface                                                  
///   @return the window interface                                            
const A::Window* ASCIIRenderer::GetWindow() const noexcept {
   return mWindow;
}

/// Get the current resolution                                                
///   @return the resolution                                                  
Scale2 ASCIIRenderer::GetResolution() const noexcept {
   return {
      mBackbuffer.GetView().mWidth,
      mBackbuffer.GetView().mHeight
   };
}
