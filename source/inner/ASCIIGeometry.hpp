///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Common.hpp"
#include <Math/Normal.hpp>
#include <Langulus/Mesh.hpp>


///                                                                           
///   ASCII intermediate geometry container                                   
///                                                                           
/// Optimizes a geometry asset for cache friendly CPU bound rasterization     
///                                                                           
struct ASCIIGeometry : A::Graphics, ProducedFrom<ASCIIRenderer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

   // An interleaved cache-friendly vertex format                       
   // We can't really do that much detail with an ASCII renderer, so    
   // this general purpose vertex format should be sufficient for       
   // nearly 99% of the use cases.                                      
   struct Vertex {
      Vec4    mPos {0, 0, 0, 1};    // Vertex position                  
      Normal3 mNor {0, 0, 1};       // Vertex normal                    
      Point2  mTex;                 // Vertex texture coordinates       
      RGBA    mCol = Colors::White; // Vertex color                     
   };

private:
   // Mesh info                                                         
   MeshView mView;

   // The vertex buffer                                                 
   TMany<Vertex> mVertices;

public:
   ASCIIGeometry(ASCIIRenderer*, const Neat&);

   NOD() auto MadeOfTriangles() const noexcept -> bool;
   NOD() auto GetVertices() const noexcept -> const TMany<Vertex>&;
};
