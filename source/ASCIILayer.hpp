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

struct LayerSubscriber {
   const ASCIIPipeline* pipeline {};
   const ASCIIRenderable* sub {};
};

using LevelSet = TOrderedSet<Level>;
using CameraSet = TUnorderedSet<const ASCIICamera*>;
using PipelineSet = TUnorderedSet<ASCIIPipeline*>;

struct RenderConfig {
   Text mClearColor;
   Real mClearDepth;
};


///                                                                           
///   Graphics layer unit                                                     
///                                                                           
/// A logical group of cameras, renderables, and lights, isolated from other  
/// layers. Useful for capsulating a GUI, for example.                        
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

   // List of cameras                                                   
   TFactory<ASCIICamera> mCameras;
   // List of rendererables                                             
   TFactory<ASCIIRenderable> mRenderables;
   // List of lights                                                    
   TFactory<ASCIILight> mLights;
   // A cache of relevant pipelines                                     
   PipelineSet mRelevantPipelines;
   // A cache of relevant levels                                        
   LevelSet mRelevantLevels;
   // A cache of relevant cameras                                       
   CameraSet mRelevantCameras;

   // Subscribers, used only for hierarchical styled layers             
   // Otherwise, ASCIIPipeline::Subscriber is used                      
   TAny<LayerSubscriber> mSubscribers;
   TAny<Count> mSubscriberCountPerLevel;
   TAny<Count> mSubscriberCountPerCamera;

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

      // If enabled, will separate light computation on a different pass
      // Significantly improves performance on scenes with complex      
      // lighting and shadowing, but does incur some memory costs.      
      DeferredLights = 4,

      // If enabled will sort instances by distance to camera (depth),  
      // before committing them for rendering                           
      Sorted = 8,

      // The default visual layer style                                 
      Default = Batched | Multilevel | DeferredLights
   };

   Style mStyle = Style::Default;

public:
   ASCIILayer(ASCIIRenderer*, const Neat&);
   ~ASCIILayer();

   void Create(Verb&);
   bool Generate(PipelineSet&);
   void Render(const RenderConfig&) const;
   void Detach();

   NOD() Style GetStyle() const noexcept;
   NOD() const A::Window* GetWindow() const noexcept;

private:
   void CompileCameras();

   Count CompileLevelBatched(const Mat4&, const Mat4&, Level, PipelineSet&);
   Count CompileLevelHierarchical(const Mat4&, const Mat4&, Level, PipelineSet&);
   Count CompileThing(const Thing*, LOD&, PipelineSet&);
   NOD() ASCIIPipeline* CompileInstance(const ASCIIRenderable*, const A::Instance*, LOD&);
   Count CompileLevels();

   void RenderBatched(const RenderConfig&) const;
   void RenderHierarchical(const RenderConfig&) const;
};
