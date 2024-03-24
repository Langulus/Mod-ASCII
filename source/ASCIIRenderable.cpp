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
   : A::Renderable {MetaOf<ASCIIRenderable>()}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing...");
   Couple(descriptor);
   VERBOSE_ASCII("Initialized");
}

ASCIIRenderable::~ASCIIRenderable() {
   Detach();
}

/// Reset the renderable, releasing all used content and pipelines            
void ASCIIRenderable::Detach() {
   mMaterialContent.Reset();
   mGeometryContent.Reset();
   mTextureContent.Reset();
   mInstances.Reset();
   mPredefinedPipeline.Reset();
   ProducedFrom<ASCIILayer>::Detach();
}

/// Get the renderer                                                          
///   @return a pointer to the renderer                                       
ASCIIRenderer* ASCIIRenderable::GetRenderer() const noexcept {
   return GetProducer()->GetProducer();
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
   if (mMaterialContent) {
      construct << mMaterialContent;
      usingGlobalPipeline = true;
   }
   else {
      if (mGeometryContent)
         construct << mGeometryContent->GetLOD(lod);
      if (mTextureContent)
         construct << mTextureContent;
   }

   // Add shaders if any such trait exists in unit environment          
   auto shader = SeekTrait<Traits::Shader>();
   if (shader)
      construct << shader;

   // Add colorization if available                                     
   auto color = SeekTrait<Traits::Color>();
   if (color)
      construct << color;

   // If at this point the construct is empty, then nothing to draw     
   if (not construct.GetDescriptor()) {
      Logger::Warning(Self(), "No contents available for generating pipeline");
      return nullptr;
   }
   
   // Add the layer, too, if available                                  
   // This is intentionally added after the above check                 
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

/// Called when owner changes components/traits                               
void ASCIIRenderable::Refresh() {
   // Just reset - new resources will be regenerated or reused upon     
   // request if they need be                                           
   Detach();

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

   const auto material = SeekUnit<A::Material, Seek::Here>();
   if (material) {
      mMaterialContent = material;
      return;
   }

   const auto geometry = SeekUnit<A::Mesh, Seek::Here>();
   if (geometry)
      mGeometryContent = geometry;

   const auto texture = SeekUnit<A::Image, Seek::Here>();
   if (texture)
      mTextureContent = texture;
}
