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
#include <Langulus/Physical.hpp>


/// Descriptor constructor                                                    
///   @param producer - the camera producer                                   
///   @param descriptor - the camera descriptor                               
ASCIILayer::ASCIILayer(ASCIIRenderer* producer, const Neat& descriptor)
   : Resolvable {this}
   , ProducedFrom {producer, descriptor}
   , mCameras {this}
   , mRenderables {this}
   , mLights {this} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

ASCIILayer::~ASCIILayer() {
   Detach();
}

void ASCIILayer::Detach() {
   mSubscribers.Reset();
   mRelevantCameras.Reset();
   mRelevantLevels.Reset();
   mRelevantPipelines.Reset();

   mCameras.Reset();
   mRenderables.Reset();
   mLights.Reset();

   ProducedFrom<ASCIIRenderer>::Detach();
}

/// Create/destroy renderables, cameras, lights                               
///   @param verb - creation verb                                             
void ASCIILayer::Create(Verb& verb) {
   mCameras.Create(verb);
   mRenderables.Create(verb);
   mLights.Create(verb);
}

/// Generate the draw list for the layer                                      
///   @param pipelines - [out] a set of all used pipelines                    
///   @return true if anything renderable was generated                       
bool ASCIILayer::Generate(PipelineSet& pipelines) {
   CompileCameras();
   CompileLevels();
   return 0 != pipelines.InsertBlock(mRelevantPipelines);
}

/// Compile the camera transformations                                        
void ASCIILayer::CompileCameras() {
   for (auto& camera : mCameras)
      camera.Compile();
}

/// Compile a single renderable instance, culling it if able                  
/// This will create or reuse a pipeline, capable of rendering it             
///   @param renderable - the renderable to compile                           
///   @param instance - the instance to compile                               
///   @param lod - the lod state to use                                       
///   @return the pipeline if instance is relevant                            
ASCIIPipeline* ASCIILayer::CompileInstance(
   const ASCIIRenderable* renderable, const A::Instance* instance, LOD& lod
) {
   if (not instance) {
      // No instances, so culling based only on default level           
      if (lod.mLevel != Level::Default)
         return nullptr;
      lod.Transform();
   }
   else {
      // Instance available, so cull                                    
      if (instance->Cull(lod))
         return nullptr;
      lod.Transform(instance->GetModelTransform(lod));
   }

   // Get relevant pipeline                                             
   return renderable->GetOrCreatePipeline(lod, this);
}

/// Compile an entity and all of its children entities                        
/// Used only for hierarchical styled layers                                  
///   @param thing - entity to compile                                        
///   @param lod - the lod state to use                                       
///   @param pipesPerCamera - [out] pipelines used by the hierarchy           
///   @return 1 if something from the hierarchy was rendered                  
Count ASCIILayer::CompileThing(const Thing* thing, LOD& lod, PipelineSet& pipesPerCamera) {
   // Iterate all renderables of the entity, which are part of this     
   // layer - disregard all others                                      
   auto relevantRenderables = thing->GatherUnits<ASCIIRenderable, Seek::Here>();

   // Compile the instances associated with these renderables           
   Count renderedInstances = 0;
   for (auto renderable : relevantRenderables) {
      if (not renderable->mInstances) {
         // Imagine a default instance                                  
         auto pipeline = CompileInstance(renderable, nullptr, lod);
         if (pipeline) {
            pipesPerCamera << pipeline;
            mRelevantPipelines << pipeline;
            mSubscribers << LayerSubscriber {pipeline, renderable};

            ++mSubscriberCountPerLevel.Last();
            ++mSubscriberCountPerCamera.Last();
            ++renderedInstances;
         }
      }
      else for (auto instance : renderable->mInstances) {
         // Compile each available instance                             
         auto pipeline = CompileInstance(renderable, instance, lod);
         if (pipeline) {
            pipesPerCamera << pipeline;
            mRelevantPipelines << pipeline;
            mSubscribers << LayerSubscriber {pipeline, renderable};

            ++mSubscriberCountPerLevel.Last();
            ++mSubscriberCountPerCamera.Last();
            ++renderedInstances;
         }
      }
   }

   // Nest to children                                                  
   for (auto child : thing->GetChildren())
      renderedInstances += CompileThing(child, lod, pipesPerCamera);

   return renderedInstances > 0;
}

/// Compile a single level's instances hierarchical style                     
///   @param view - the camera view matrix                                    
///   @param projection - the camera projection matrix                        
///   @param level - the level to compile                                     
///   @param pipesPerCamera - [out] pipeline set for the current level only   
///   @return the number of compiled entities                                 
Count ASCIILayer::CompileLevelHierarchical(
   const Mat4& view, const Mat4& projection, 
   Level level, PipelineSet& pipesPerCamera
) {
   // Construct view and frustum for culling                            
   LOD lod {level, view, projection};

   // Nest-iterate all children of the layer owner                      
   Count renderedEntities {};
   for (const auto& owner : GetOwners())
      renderedEntities += CompileThing(owner, lod, pipesPerCamera);
   
   if (renderedEntities) {
      mSubscriberCountPerLevel.New();

      // Store the negative level in the set, so they're always in      
      // a descending order                                             
      mRelevantLevels << -level;
   }

   return renderedEntities;
}

/// Compile a single level's instances batched style                          
///   @param view - the camera view matrix                                    
///   @param projection - the camera projection matrix                        
///   @param level - the level to compile                                     
///   @param pipesPerCamera - [out] pipeline set for the current level only   
///   @return 1 if anything was rendered, zero otherwise                      
Count ASCIILayer::CompileLevelBatched(
   const Mat4& view, const Mat4& projection, 
   Level level, PipelineSet& pipesPerCamera
) {
   // Construct view and frustum   for culling                          
   LOD lod {level, view, projection};

   // Iterate all renderables                                           
   Count renderedInstances = 0;
   for (const auto& renderable : mRenderables) {
      if (not renderable.mInstances) {
         auto pipeline = CompileInstance(&renderable, nullptr, lod);
         if (pipeline) {
            pipesPerCamera << pipeline;
            mRelevantPipelines << pipeline;
            ++renderedInstances;
         }
      }
      else for (auto instance : renderable.mInstances) {
         auto pipeline = CompileInstance(&renderable, instance, lod);
         if (pipeline) {
            pipesPerCamera << pipeline;
            mRelevantPipelines << pipeline;
            ++renderedInstances;
         }
      }
   }

   if (renderedInstances) {
      for (auto pipeline : pipesPerCamera) {
         // Store the negative level in the set, so they're always in   
         // a descending order                                          
         mRelevantLevels << -level;
      }

      return 1;
   }

   return 0;
}

/// Compile all levels and their instances                                    
///   @return the number of relevant cameras                                  
Count ASCIILayer::CompileLevels() {
   Count renderedCameras = 0;
   mRelevantLevels.Clear();
   mRelevantPipelines.Clear();

   if (mStyle & Style::Hierarchical) {
      mSubscribers.Clear();
      mSubscribers.New(1);
      mSubscriberCountPerLevel.Clear();
      mSubscriberCountPerLevel.New(1);
      mSubscriberCountPerCamera.Clear();
      mSubscriberCountPerCamera.New(1);
   }

   if (not mCameras) {
      // No camera, so just render default level on the whole screen    
      PipelineSet pipesPerCamera;
      if (mStyle & Style::Hierarchical)
         CompileLevelHierarchical({}, {}, {}, pipesPerCamera);
      else
         CompileLevelBatched({}, {}, {}, pipesPerCamera);

      if (pipesPerCamera) {
         if (mStyle & Style::Hierarchical)
            mSubscriberCountPerCamera.New();
         ++renderedCameras;
      }
   }
   else for (const auto& camera : mCameras) {
      // Iterate all levels per camera                                  
      PipelineSet pipesPerCamera;
      if (mStyle & Style::Multilevel) {
         // Multilevel style - tests all camera-visible levels          
         for (auto level = camera.mObservableRange.mMax; level >= camera.mObservableRange.mMin; --level) {
            const auto view = camera.GetViewTransform(level);
            if (mStyle & Style::Hierarchical)
               CompileLevelHierarchical(view, camera.mProjection, level, pipesPerCamera);
            else
               CompileLevelBatched(view, camera.mProjection, level, pipesPerCamera);
         }
      }
      else if (camera.mObservableRange.Inside(Level::Default)) {
         // Default level style - checks only if camera sees default    
         const auto view = camera.GetViewTransform();
         if (mStyle & Style::Hierarchical)
            CompileLevelHierarchical(view, camera.mProjection, {}, pipesPerCamera);
         else
            CompileLevelBatched(view, camera.mProjection, {}, pipesPerCamera);
      }
      else continue;

      if (pipesPerCamera) {
         if (mStyle & Style::Hierarchical)
            mSubscriberCountPerCamera.New(1);
         mRelevantCameras << &camera;
         ++renderedCameras;
      }
   }

   return renderedCameras;
}

/// Render the layer to a specific command buffer and framebuffer             
///   @param config - where to render to                                      
void ASCIILayer::Render(const RenderConfig& config) const {
   if (mStyle & Style::Hierarchical)
      RenderHierarchical(config);
   else
      RenderBatched(config);
}

/// Render the layer to a specific command buffer and framebuffer             
/// This is used only for Batched style layers, and relies on previously      
/// compiled set of pipelines                                                 
///   @param config - where to render to                                      
void ASCIILayer::RenderBatched(const RenderConfig&) const {
   // Iterate all valid cameras                                         
   TUnorderedMap<const ASCIIPipeline*, Count> done;

   if (not mRelevantCameras) {
      // Rendering using a fallback camera                              
      // Iterate all relevant levels                                    
      for (const auto& level : mRelevantLevels) {
         // Draw all subscribers to the pipeline for the current level  
         for (auto pipeline : mRelevantPipelines)
            done[pipeline] = pipeline->RenderLevel(done[pipeline]);

         if (level != *mRelevantLevels.last()) {
            // Clear depth after rendering this level (if not last)     
            TODO();
         }
      }
   }
   else for (const auto& camera : mRelevantCameras) {
      // Rendering from each custom camera's point of view              
      // Iterate all relevant levels                                    
      for (const auto& level : mRelevantLevels) {
         // Draw all subscribers to the pipeline for the current level  
         for (auto pipeline : mRelevantPipelines)
            done[pipeline] = pipeline->RenderLevel(done[pipeline]);

         if (level != *mRelevantLevels.last()) {
            // Clear depth after rendering this level (if not last)     
            TODO();
         }
      }
   }
}

/// Render the layer to a specific command buffer and framebuffer             
/// This is used only for Hierarchical style layers, and relies on locally    
/// compiled subscribers, rendering them in their respective order            
///   @param config - where to render to                                      
void ASCIILayer::RenderHierarchical(const RenderConfig&) const {
   Count subscribersDone = 0;
   auto subscriberCountPerLevel = &mSubscriberCountPerLevel[0];

   if (not mRelevantCameras) {
      // Rendering using a fallback camera                              
      // Iterate all relevant levels                                    
      for (const auto& level : mRelevantLevels) {
         // Draw all subscribers to the pipeline for the current level  
         for (Count s = 0; s < *subscriberCountPerLevel; ++s) {
            auto& subscriber = mSubscribers[subscribersDone + s];
            subscriber.pipeline->Render(*subscriber.sub);
         }

         if (level != *mRelevantLevels.last()) {
            // Clear depth after rendering this level (if not last)     
            TODO();
         }

         subscribersDone += *subscriberCountPerLevel;
         ++subscriberCountPerLevel;
      }
   }
   else for (const auto& camera : mRelevantCameras) {
      // Iterate all relevant levels                                    
      for (const auto& level : mRelevantLevels) {
         // Draw all subscribers to the pipeline for the current level  
         // and camera                                                  
         for (Count s = 0; s < *subscriberCountPerLevel; ++s) {
            auto& subscriber = mSubscribers[subscribersDone + s];
            subscriber.pipeline->Render(*subscriber.sub);
         }

         if (level != *mRelevantLevels.last()) {
            // Clear depth after rendering this level (if not last)     
            TODO();
         }

         subscribersDone += *subscriberCountPerLevel;
         ++subscriberCountPerLevel;
      }
   }
}

/// Get the style of a layer                                                  
///   @return the layer style                                                 
ASCIILayer::Style ASCIILayer::GetStyle() const noexcept {
   return mStyle;
}

/// Get the window of a layer                                                 
///   @return the window interface                                            
const A::Window* ASCIILayer::GetWindow() const noexcept {
   return mProducer->GetWindow();
}
