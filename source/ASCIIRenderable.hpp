///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"


///                                                                           
///   ASCII renderable element                                                
///                                                                           
/// Gives things the ability to be drawn to screen. The unit gathers relevant 
/// graphical resources from the context, and generates a graphical pipeline  
/// capable of visualizing them                                               
///                                                                           
struct ASCIIRenderable final : A::Renderable, ProducedFrom<ASCIILayer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS(PRODUCER) ASCIILayer;
   LANGULUS_BASES(A::Renderable);

protected:
   friend struct ASCIILayer;

   // Precompiled instances and levels, updated on Refresh()            
   TAny<const A::Instance*> mInstances;
   TRange<Level> mLevelRange;
   Ref<A::Material> mMaterialContent;
   Ref<A::Mesh> mGeometryContent;
   Ref<A::Image> mTextureContent;
   mutable Ref<ASCIIPipeline> mPredefinedPipeline;

public:
   ASCIIRenderable(ASCIILayer*, const Neat&);
   ~ASCIIRenderable();

   NOD() ASCIIRenderer* GetRenderer() const noexcept;
   NOD() ASCIIPipeline* GetOrCreatePipeline(const LOD&, const ASCIILayer*) const;

   void Refresh();
   void Detach();
};
