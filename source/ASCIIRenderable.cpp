///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "ASCII.hpp"
#include <Langulus/Physical.hpp>


/// Descriptor constructor                                                    
///   @param producer - the renderable producer                               
///   @param descriptor - the renderable descriptor                           
ASCIIRenderable::ASCIIRenderable(ASCIILayer* producer, const Neat& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

ASCIIRenderable::~ASCIIRenderable() {
   Detach();
}

/// Reset the renderable, releasing all used content and pipelines            
void ASCIIRenderable::Reset() {
   for (auto& lod : mLOD) {
      lod.mGeometry.Reset();
      lod.mTexture.Reset();
      lod.mPipeline.Reset();
   }

   mGeometryContent.Reset();
   mTextureContent.Reset();
   mInstances.Reset();
   mPredefinedPipeline.Reset();
}

/// Detach the renderable from the hierarchy                                  
void ASCIIRenderable::Detach() {
   Reset();
   ProducedFrom::Detach();
}

/// Get the renderer                                                          
///   @return a pointer to the renderer                                       
ASCIIRenderer* ASCIIRenderable::GetRenderer() const noexcept {
   return GetProducer()->GetProducer();
}

/// Get VRAM geometry corresponding to an octave of this renderable           
/// This is the point where content might be generated upon request           
///   @param lod - information used to extract the best LOD                   
///   @return the VRAM geometry or nullptr if content is not available        
A::Mesh* ASCIIRenderable::GetGeometry(const LOD& lod) const {
   const auto i = lod.GetAbsoluteIndex();
   if (not mLOD[i].mGeometry and mGeometryContent)
      mLOD[i].mGeometry = mGeometryContent->GetLOD(lod);
   return mLOD[i].mGeometry;
}

/// Get VRAM texture corresponding to an octave of this renderable            
/// This is the point where content might be generated upon request           
///   @param lod - information used to extract the best LOD                   
///   @return the VRAM texture or nullptr if content is not available         
A::Image* ASCIIRenderable::GetTexture(const LOD& lod) const {
   const auto i = lod.GetAbsoluteIndex();
   if (not mLOD[i].mTexture and mTextureContent)
      mLOD[i].mTexture = mTextureContent->GetLOD(lod);
   return mLOD[i].mTexture;
}

/// Get uniform color                                                         
///   @return the color                                                       
RGBA ASCIIRenderable::GetColor() const {
   return *mColor;
}

/// Create GPU pipeline able to utilize geometry, textures and shaders        
///   @param lod - information used to extract the best LOD                   
///   @param layer - additional settings might be provided by the used layer  
///   @return the pipeline                                                    
ASCIIPipeline* ASCIIRenderable::GetOrCreatePipeline(
   const LOD& lod, const ASCIILayer* layer
) const {
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
   Verbs::Create creator {&construct};
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

   mGeometryContent = SeekUnit<A::Mesh, Seek::Here>();
   mTextureContent = SeekUnit<A::Image, Seek::Here>();
}
