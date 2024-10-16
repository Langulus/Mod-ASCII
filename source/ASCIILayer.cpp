///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"
#include <Langulus/Platform.hpp>
#include <Langulus/Physical.hpp>


/// Descriptor constructor                                                    
///   @param producer - the camera producer                                   
///   @param descriptor - the camera descriptor                               
ASCIILayer::ASCIILayer(ASCIIRenderer* producer, const Many& descriptor)
   : Resolvable      {this}
   , ProducedFrom    {producer, descriptor}
   , mFallbackCamera {this}
   , mImage          {producer} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Reference the layer, triggering teardown if no longer used                
auto ASCIILayer::Reference(int x) -> Count {
   if (A::Layer::Reference(x) == 1) {
      mImage.Reference(-1);
      mHierarchicalSequence.Reset();
      mBatchSequence.Reset();
      mLights.Teardown();
      mRenderables.Teardown();
      mCameras.Teardown();
      ProducedFrom::Teardown();
   }

   return GetReferences();
}

/// Create/destroy renderables, cameras, lights                               
///   @param verb - creation verb                                             
void ASCIILayer::Create(Verb& verb) {
   mCameras.Create(this, verb);
   mRenderables.Create(this, verb);
   mLights.Create(this, verb);
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
      mFallbackCamera.mPerspective = false;
      mFallbackCamera.Compile();

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
      // Instance available, so do frustum culling                      
      if (instance->Cull(lod))
         return;
      lod.Transform(instance->GetModelTransform(lod));
   }

   // Get relevant pipeline and geometry                                
   const auto* pipeline = renderable->GetOrCreatePipeline(lod, this);
   if (not pipeline)
      return;

   auto* geometry = renderable->GetGeometry(lod);
   if (not geometry)
      return;

   // Cache the instance in the appropriate sequence                    
   if (mStyle & Style::Hierarchical) {
      auto cachedCam = mHierarchicalSequence.FindIt(&cam);
      if (not cachedCam) {
         mHierarchicalSequence.Insert(&cam);
         cachedCam = mHierarchicalSequence.FindIt(&cam);
      }

      auto cachedLvl = cachedCam.GetValue().FindIt(-lod.mLevel);
      if (not cachedLvl) {
         cachedCam.GetValue().Insert(-lod.mLevel);
         cachedLvl = cachedCam.GetValue().FindIt(-lod.mLevel);
      }

      auto& cachedPipes = cachedLvl.GetValue();
      cachedPipes << TPair { pipeline, PipeSubscriber {
         instance
            ? renderable->GetColor() * instance->GetColor()
            : renderable->GetColor(),
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

      auto cachedLvl = cachedCam.GetValue().FindIt(-lod.mLevel);
      if (not cachedLvl) {
         cachedCam.GetValue().Insert(-lod.mLevel);
         cachedLvl = cachedCam.GetValue().FindIt(-lod.mLevel);
      }

      auto cachedPipe = cachedLvl.GetValue().FindIt(pipeline);
      if (not cachedPipe) {
         cachedLvl.GetValue().Insert(pipeline);
         cachedPipe = cachedLvl.GetValue().FindIt(pipeline);
      }

      auto& cachedRends = cachedPipe.GetValue();
      cachedRends << PipeSubscriber {
         instance
            ? renderable->GetColor() * instance->GetColor()
            : renderable->GetColor(),
         lod.mModel,
         renderable->GetGeometry(lod),
         renderable->GetTexture(lod)
      };
   }
}

/// Render the layer to a specific command buffer and framebuffer             
///   @param config - where to render to                                      
void ASCIILayer::Render(const RenderConfig& config) const {
   const int sizex = static_cast<int>(GetWindow()->GetSize().x);
   const int sizey = static_cast<int>(GetWindow()->GetSize().y);

   mImage.Resize(sizex, sizey);
   mDepth.Resize(sizex, sizey);

   mImage.Fill(' ', Colors::White, Colors::Red);
   mDepth.Fill(config.mClearDepth);

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
            * camera.mKey->GetViewTransform(level.GetKey()).Invert();

         // Involve all relevant pipelines for that level               
         for (const auto pipeline : level.GetValue()) {
            // Draw all renderables that use that pipeline in their     
            // current LOD state, from that particular level & POV      
            for (const auto& instance : pipeline.mValue)
               pipeline.mKey->Render(this, projectedView, instance);

            // Assemble after everything has been drawn                 
            pipeline.mKey->Assemble(this);
         }

         // Clear global depth after rendering each level               
         mDepth.Fill(cfg.mClearDepth);
      }
   }
}

/// Render all instanced renderables in the order they appear in the scene    
/// This is used only for Hierarchical style layers                           
///   @param cfg - where to render to                                         
void ASCIILayer::RenderHierarchical(const RenderConfig& cfg) const {
   // Rendering from each custom camera's point of view                 
   for (const auto camera : mHierarchicalSequence) {
      // Draw all relevant levels from the camera's POV                 
      for (auto level : KeepIterator(camera.mValue)) {
         const auto projectedView = camera.mKey->mProjection
            * camera.mKey->GetViewTransform(level.GetKey());

         // Render all relevant pipe-renderable pairs for that level    
         for (const auto& instance : level.GetValue()) {
            instance.mKey->Render(this, projectedView, instance.mValue);

            // Assemble after each draw in order to keep hierarchy      
            instance.mKey->Assemble(this);
         }

         // Clear depth after rendering each level                      
         mDepth.Fill(cfg.mClearDepth);
      }
   }
}

/// Get the style of a layer                                                  
///   @return the layer style                                                 
auto ASCIILayer::GetStyle() const noexcept -> Style {
   return mStyle;
}

/// Get the window of a layer                                                 
///   @return the window interface                                            
auto ASCIILayer::GetWindow() const noexcept -> const A::Window* {
   return mProducer->GetWindow();
}
