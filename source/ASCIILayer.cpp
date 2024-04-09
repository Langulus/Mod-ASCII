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
   , mLights {this}
   , mFallbackCamera {this}
   , mImage {producer} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Layer destruction                                                         
ASCIILayer::~ASCIILayer() {
   Detach();
}

/// Detach layer from hierarchy                                               
void ASCIILayer::Detach() {
   //mBatchSequence.Reset();
   //mHierarchicalSequence.Reset();

   mCameras.Reset();
   mRenderables.Reset();
   mLights.Reset();
   ProducedFrom::Detach();
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
void ASCIILayer::Generate() {
   mBatchSequence.Clear();
   mHierarchicalSequence.Clear();

   CompileCameras();
   CompileLevels();
}

/// Compile the camera transformations                                        
void ASCIILayer::CompileCameras() {
   for (auto& camera : mCameras)
      camera.Compile();
}

/// Compile all levels and their instances                                    
///   @return the number of relevant cameras                                  
void ASCIILayer::CompileLevels() {
   if (not mCameras) {
      // No camera, so just render default level on the whole screen    
      if (mStyle & Style::Hierarchical)
         CompileLevelHierarchical(mFallbackCamera, Level::Default);
      else
         CompileLevelBatched(mFallbackCamera, Level::Default);
   }
   else for (const auto& cam : mCameras) {
      if (mStyle & Style::Multilevel) {
         // Multilevel style - tests all camera-visible levels          
         for (auto level  = cam.mObservableRange.mMax;
                   level >= cam.mObservableRange.mMin; --level) {
            if (mStyle & Style::Hierarchical)
               CompileLevelHierarchical(cam, level);
            else
               CompileLevelBatched(cam, level);
         }
      }
      else if (cam.mObservableRange.Inside(Level::Default)) {
         // Default level style - checks only if camera sees default    
         if (mStyle & Style::Hierarchical)
            CompileLevelHierarchical(cam, Level::Default);
         else
            CompileLevelBatched(cam, Level::Default);
      }
   }
}

/// Compile a single level's instances hierarchical style                     
///   @param cam - the camera to compile                                      
///   @param level - the level to compile                                     
void ASCIILayer::CompileLevelHierarchical(const ASCIICamera& cam, Level level) {
   // Construct view and frustum for culling                            
   LOD lod {level, cam.GetViewTransform(level), cam.mProjection};

   // Nest-iterate all children of the layer owner                      
   for (const auto& owner : GetOwners())
      CompileThing(owner, lod, cam);
}

/// Compile a single level's instances batched style                          
///   @param cam - the camera to compile                                      
///   @param level - the level to compile                                     
void ASCIILayer::CompileLevelBatched(const ASCIICamera& cam, Level level) {
   // Construct view and frustum for culling                            
   LOD lod {level, cam.GetViewTransform(level), cam.mProjection};

   // Iterate all renderables                                           
   for (const auto& renderable : mRenderables) {
      if (not renderable.mInstances)
         CompileInstance(&renderable, nullptr, lod, cam);
      else for (auto instance : renderable.mInstances)
         CompileInstance(&renderable, instance, lod, cam);
   }
}

/// Compile an entity and all of its children entities                        
/// Used only for hierarchical styled layers                                  
///   @param thing - entity to compile                                        
///   @param lod - the lod state to use                                       
///   @param cam - the camera to compile                                      
void ASCIILayer::CompileThing(const Thing* thing, LOD& lod, const ASCIICamera& cam) {
   // Iterate all renderables of the entity, which are part of this     
   // layer - disregard all others layers                               
   auto renderables = thing->GatherUnits<ASCIIRenderable, Seek::Here>();

   // Compile the instances associated with these renderables           
   for (auto renderable : renderables) {
      if (not mRenderables.Owns(renderable))
         continue;

      if (not renderable->mInstances)
         CompileInstance(renderable, nullptr, lod, cam);
      else for (auto instance : renderable->mInstances)
         CompileInstance(renderable, instance, lod, cam);
   }

   // Nest to children                                                  
   for (auto child : thing->GetChildren())
      CompileThing(child, lod, cam);
}

/// Compile a single renderable instance, culling it if able                  
/// This will create or reuse a pipeline, capable of rendering it             
///   @param renderable - the renderable to compile                           
///   @param instance - the instance to compile                               
///   @param lod - the lod state to use                                       
///   @param cam - the camera to compile                                      
void ASCIILayer::CompileInstance(
   const ASCIIRenderable* renderable,
   const A::Instance* instance,
   LOD& lod, const ASCIICamera& cam
) {
   if (not instance) {
      // No instances, so culling based only on default level           
      if (lod.mLevel != Level::Default)
         return;
      lod.Transform();
   }
   else {
      // Instance available, so cull                                    
      if (instance->Cull(lod))
         return;
      lod.Transform(instance->GetModelTransform(lod));
   }

   // Get relevant pipeline                                             
   const auto* pipeline = renderable->GetOrCreatePipeline(lod, this);

   // Cache the instance in the appropriate sequence                    
   if (mStyle & Style::Hierarchical) {
      auto cachedCam = mHierarchicalSequence.FindIt(&cam);
      if (not cachedCam) {
         mHierarchicalSequence.Insert(&cam);
         cachedCam = mHierarchicalSequence.FindIt(&cam);
      }

      auto cachedLvl = cachedCam.mValue->FindIt(-lod.mLevel);
      if (not cachedLvl) {
         cachedCam.mValue->Insert(-lod.mLevel);
         cachedLvl = cachedCam.mValue->FindIt(-lod.mLevel);
      }

      auto& cachedPipes = *cachedLvl.mValue;
      cachedPipes << TPair { pipeline, PipeSubscriber {
         renderable->GetColor(),
         lod.mModel,
         renderable->GetGeometry(lod),
         renderable->GetTexture(lod)
      }};
   }
   else {
      auto cachedCam = mBatchSequence.FindIt(&cam);
      if (not cachedCam) {
         mBatchSequence.Insert(&cam);
         cachedCam = mBatchSequence.FindIt(&cam);
      }

      auto cachedLvl = cachedCam.mValue->FindIt(-lod.mLevel);
      if (not cachedLvl) {
         cachedCam.mValue->Insert(-lod.mLevel);
         cachedLvl = cachedCam.mValue->FindIt(-lod.mLevel);
      }

      auto cachedPipe = cachedLvl.mValue->FindIt(pipeline);
      if (not cachedPipe) {
         cachedLvl.mValue->Insert(pipeline);
         cachedPipe = cachedLvl.mValue->FindIt(pipeline);
      }

      auto& cachedRends = *cachedPipe.mValue;
      cachedRends << PipeSubscriber {
         renderable->GetColor(),
         lod.mModel,
         renderable->GetGeometry(lod),
         renderable->GetTexture(lod)
      };
   }
}

/// Render the layer to a specific command buffer and framebuffer             
///   @param config - where to render to                                      
void ASCIILayer::Render(const RenderConfig& config) const {
   if (mStyle & Style::Hierarchical)
      RenderHierarchical(config);
   else
      RenderBatched(config);
}

/// Render all instanced renderables in the order with least overhead         
/// This is used only for Batched style layers                                
///   @param config - where to render to                                      
void ASCIILayer::RenderBatched(const RenderConfig& cfg) const {
   // Rendering from each custom camera's point of view                 
   for (const auto camera : mBatchSequence) {
      // Draw all relevant levels from the camera's POV                 
      for (auto level : KeepIterator(camera.mValue)) {
         const auto projectedView = camera.mKey->mProjection
            * camera.mKey->GetViewTransform(*level.mKey);

         // Involve all relevant pipelines for that level               
         for (const auto pipeline : *level.mValue) {
            // Draw all renderables that use that pipeline in their     
            // current LOD state, from that particular level & POV      
            for (const auto& instance : pipeline.mValue) {
               pipeline.mKey->Render(this, projectedView, instance);
            }

            //TODO bake the intermediate pipeline buffer to layer's image
         }

         //TODO light calculations before depth is erased

         // Clear depth after rendering each level                      
         mDepth.Fill(cfg.mClearDepth);
      }
   }
}

/// Render all instanced renderables in the order they appear in the scene    
/// This is used only for Hierarchical style layers                           
///   @param config - where to render to                                      
void ASCIILayer::RenderHierarchical(const RenderConfig& cfg) const {
   // Rendering from each custom camera's point of view                 
   for (const auto camera : mHierarchicalSequence) {
      // Draw all relevant levels from the camera's POV                 
      for (auto level : KeepIterator(camera.mValue)) {
         const auto projectedView = camera.mKey->mProjection
            * camera.mKey->GetViewTransform(*level.mKey);

         // Involve all relevant pipe-renderable pairs for that level   
         for (const auto& instance : *level.mValue) {
            instance.mKey->Render(this, projectedView, instance.mValue);

            //TODO bake the intermediate pipeline buffer after each draw
         }

         //TODO light calculations before depth is erased

         // Clear depth after rendering each level                      
         mDepth.Fill(cfg.mClearDepth);
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
