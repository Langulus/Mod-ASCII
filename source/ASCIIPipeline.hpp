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
#include <Math/Blend.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/Image.hpp>
#include <Langulus/IO.hpp>


///                                                                           
///   ASCII pipeline that rasterizes vector graphics into ASCII symbols       
///                                                                           
struct ASCIIPipeline : A::Graphics, ProducedFrom<ASCIIRenderer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

private:
   // Blending mode (participates in hash)                              
   Math::BlendMode mBlendMode = Math::BlendMode::Alpha;
   // Toggle depth testing and writing                                  
   bool mDepth = true;
   // Subscribers                                                       
   TAny<const ASCIIRenderable*> mSubscribers;

   static Construct FromFile(const A::File&);
   static Construct FromMesh(const A::Mesh&);
   static Construct FromText(const Text&);

public:
   ASCIIPipeline(ASCIIRenderer*, const Neat&);

   NOD() Count RenderLevel(Offset) const;
   void Render(const ASCIIRenderable&) const;
};
