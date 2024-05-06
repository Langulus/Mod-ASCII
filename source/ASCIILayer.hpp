///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "ASCIICamera.hpp"
#include "ASCIIRenderable.hpp"
#include "ASCIILight.hpp"
#include <Anyness/TSet.hpp>


struct RenderConfig {
   RGB   mClearColor;
   float mClearDepth;
};


/// For each enabled camera, there exist N levels sorted in a descending      
/// order. Each level contains something renderable.                          
/// For each of these levels, there's a set of relevant pipelines             
/// And each of these pipelines draws a list of collapsed renderables         
using BatchSequence = 
   TUnorderedMap<const ASCIICamera*,
      TOrderedMap<Level,
         TUnorderedMap<const ASCIIPipeline*, TMany<PipeSubscriber>>>>;


/// For each enabled camera, there exist N levels sorted in a descending      
/// order. Each level contains something renderable.                          
/// For each of these levels, there's a list of pipe-renderable pairs that    
/// have to be drawn in the order they appear                                 
using HierarchicalSequence = 
   TUnorderedMap<const ASCIICamera*,
      TOrderedMap<Level,
         TMany<TPair<const ASCIIPipeline*, PipeSubscriber>>>>;


///                                                                           
///   Graphics layer unit                                                     
///                                                                           
///   A logical group of cameras, renderables, and lights, isolated from      
/// other layers. Useful for capsulating a GUI, for example. Layers can blend 
/// with each other, but never interact in any other way.                     
///                                                                           
struct ASCIILayer : A::Layer, ProducedFrom<ASCIIRenderer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCIIRenderer;
   LANGULUS_BASES(A::Layer);
   LANGULUS_VERBS(Verbs::Create);

protected:
   friend struct ASCIICamera;
   friend struct ASCIIRenderable;
   friend struct ASCIILight;
   friend struct ASCIIPipeline;
   friend struct ASCIIRenderer;

   // List of cameras                                                   
   TFactory<ASCIICamera> mCameras;
   // Fallback camera, for when no custom ones exist                    
   ASCIICamera mFallbackCamera;

   // List of rendererables                                             
   TFactory<ASCIIRenderable> mRenderables;
   // List of lights                                                    
   TFactory<ASCIILight> mLights;

   // The compiled batch render sequence                                
   BatchSequence mBatchSequence;
   // The compiled hierarchical render sequence                         
   HierarchicalSequence mHierarchicalSequence;

   // Depth buffer                                                      
   mutable ASCIIBuffer<float> mDepth;
   // Normals buffer                                                    
   mutable ASCIIBuffer<Vec3> mNormals;

   // The final, combined rendered layer image, after all pipelines,    
   // texturization and illumination. All layer's images are later      
   // blended together into the final ASCIIRenderer's backbuffer        
   mutable ASCIIImage mImage;


   /// The layer style determines how the scene will be compiled              
   /// Combine these flags to configure the layer to your needs               
   enum Style {
      // Batched layers are compiled for optimal performance, by        
      // grouping all similar renderables and drawing them at once.     
      // This is the opposite of hierarchical rendering, because it     
      // destroys the order in which renderables appear. It is best     
      // suited for non-blended, depth-tested scenes.                   
      Batched = 0,

      // Hierarchical layers preserve the order in which elements occur 
      // It is the opposite of batched rendering, because structure is  
      // preserved. This style is a bit less efficient, but is mandatory
      // for rendering UI, for example                                  
      Hierarchical = 1,

      // A multilevel layer supports instances that are not in the      
      // default human level. It is useful for rendering objects of the 
      // size of the universe, or of the size of atoms, depending on    
      // the camera configuration. Works by rendering the biggest       
      // levels first, working down to the camera's level range,        
      // clearing the depth after each successive level. This way one   
      // can seamlessly compose infinitely complex scenes. Needless to  
      // say, this incurs some performance penalty, despite being as    
      // optimized as possible.                                         
      Multilevel = 2,

      // If enabled will sort instances by distance to camera (depth),  
      // before committing them for rendering                           
      Sorted = 4,

      // The default visual layer style                                 
      Default = Batched | Multilevel
   };

   Style mStyle = Style::Default;

public:
   ASCIILayer(ASCIIRenderer*, const Neat&);
   ~ASCIILayer();

   void Create(Verb&);
   void Generate();
   void Render(const RenderConfig&) const;
   void Detach();

   NOD() Style GetStyle() const noexcept;
   NOD() const A::Window* GetWindow() const noexcept;

private:
   void CompileCameras();

   void CompileLevels();
   void CompileLevelBatched(const ASCIICamera&, Level);
   void CompileLevelHierarchical(const ASCIICamera&, Level);

   void CompileThing(const Thing*, LOD&, const ASCIICamera&);
   void CompileInstance(const ASCIIRenderable*, const A::Instance*, LOD&, const ASCIICamera&);

   void RenderBatched(const RenderConfig&) const;
   void RenderHierarchical(const RenderConfig&) const;
};
