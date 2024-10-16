///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "ASCII.hpp"
#include <Langulus/Physical.hpp>


/// Descriptor constructor                                                    
///   @param producer - the renderable producer                               
///   @param descriptor - the renderable descriptor                           
ASCIIRenderable::ASCIIRenderable(ASCIILayer* producer, const Many& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

/// Reset the renderable, releasing all used content and pipelines            
void ASCIIRenderable::Reset() {
   for (auto& lod : mLOD) {
      lod.mGeometry.Reset();
      lod.mTexture.Reset();
      lod.mPipeline.Reset();
   }
   mPredefinedPipeline.Reset();
   mTextureContent.Reset();
   mGeometryContent.Reset();
   mInstances.Reset();
}

/// Reference the renderable, triggering teardown if no longer used           
auto ASCIIRenderable::Reference(int x) -> Count {
   if (A::Renderable::Reference(x) == 1) {
      Reset();
      ProducedFrom::Teardown();
   }

   return GetReferences();
}

/// Get the renderer                                                          
///   @return a pointer to the renderer                                       
auto ASCIIRenderable::GetRenderer() const noexcept -> ASCIIRenderer* {
   return GetProducer()->GetProducer();
}

/// Get VRAM geometry corresponding to an octave of this renderable           
/// This is the point where content might be generated upon request           
///   @param lod - information used to extract the best LOD                   
///   @return the VRAM geometry or nullptr if content is not available        
auto ASCIIRenderable::GetGeometry(const LOD& lod) const -> const ASCIIGeometry* {
   const auto i = lod.GetAbsoluteIndex();
   if (not mLOD[i].mGeometry and mGeometryContent) {
      // Cache geometry to a more cache-friendly format                 
      Verbs::Create creator {
         Construct::From<ASCIIGeometry>(mGeometryContent->GetLOD(lod).Get())
      };
      GetRenderer()->Create(creator);
      mLOD[i].mGeometry = creator->template As<ASCIIGeometry*>();
   }

   return mLOD[i].mGeometry;
}

/// Get VRAM texture corresponding to an octave of this renderable            
/// This is the point where content might be generated upon request           
///   @param lod - information used to extract the best LOD                   
///   @return the VRAM texture or nullptr if content is not available         
auto ASCIIRenderable::GetTexture(const LOD& lod) const -> const ASCIITexture* {
   const auto i = lod.GetAbsoluteIndex();
   if (not mLOD[i].mTexture and mTextureContent) {
      // Cache texture to a more cache-friendly format                  
      Verbs::Create creator {
         Construct::From<ASCIITexture>(mTextureContent->GetLOD(lod).Get())
      };
      GetRenderer()->Create(creator);
      mLOD[i].mTexture = creator->template As<ASCIITexture*>();
   }

   return mLOD[i].mTexture;
}

/// Get uniform color                                                         
///   @return the color                                                       
auto ASCIIRenderable::GetColor() const -> RGBA {
   return *mColor;
}

/// Create GPU pipeline able to utilize geometry, textures and shaders        
///   @param lod - information used to extract the best LOD                   
///   @param layer - additional settings might be provided by the used layer  
///   @return the pipeline                                                    
auto ASCIIRenderable::GetOrCreatePipeline(
   const LOD& lod, const ASCIILayer* layer
) const -> ASCIIPipeline* {
   // Always return the predefined pipeline if available                
   if (mPredefinedPipeline)
      return mPredefinedPipeline;

   // Always return the cached pipeline if available                    
   const auto i = lod.GetAbsoluteIndex();
   if (mLOD[i].mPipeline)
      return mLOD[i].mPipeline;

   // Construct a pipeline                                              
   bool usingGlobalPipeline = false;
   auto construct = Construct::From<ASCIIPipeline>();
   auto color = SeekTrait<Traits::Color>();
   if (color)
      construct << color;
   if (layer)
      construct << layer;

   // Get, or create the pipeline                                       
   Verbs::Create creator {Abandon(construct)};
   GetRenderer()->Create(creator);

   creator->ForEachDeep([&](ASCIIPipeline& p) {
      if (usingGlobalPipeline)
         mPredefinedPipeline = &p;
      else
         mLOD[i].mPipeline = &p;
   });

   if (mPredefinedPipeline)
      return mPredefinedPipeline;
   else
      return mLOD[i].mPipeline;
}

/// Called on environment change                                              
void ASCIIRenderable::Refresh() {
   Reset();

   // Gather all instances for this renderable, and calculate levels    
   mInstances = GatherUnits<A::Instance, Seek::Here>();
   if (mInstances)
      mLevelRange = mInstances[0]->GetLevel();
   else
      mLevelRange = {};

   for (auto instance : mInstances)
      mLevelRange.Embrace(instance->GetLevel());

   // Attempt extracting pipeline/material/geometry/textures from owners
   const auto pipeline = SeekUnit<ASCIIPipeline, Seek::Here>();
   if (pipeline) {
      mPredefinedPipeline = pipeline;
      return;
   }

   mGeometryContent = SeekUnit<A::Mesh,  Seek::Here>();
   mTextureContent  = SeekUnit<A::Image, Seek::Here>();
}
