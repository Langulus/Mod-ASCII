///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "../ASCII.hpp"
#include <Math/Normal.hpp>
#include <Math/Sampler.hpp>


/// Descriptor constructor                                                    
///   @param producer - the producer of the unit                              
///   @param descriptor - the unit descriptor                                 
ASCIIGeometry::ASCIIGeometry(ASCIIRenderer* producer, const Neat& descriptor)
   : Resolvable   {this}
   , ProducedFrom {producer, descriptor} {
   // Scan the descriptor                                               
   descriptor.ForEachDeep([&](const A::Mesh& mesh) {
      if (mesh.MadeOfTriangles()) {
         // Cache a triangle list                                       
         mesh.ForEachVertex(
            [&](const Traits::Place&   p,
                const Traits::Aim&     n,
                const Traits::Sampler& t,
                const Traits::Color&   c
            ) {
               Vertex output;
               LANGULUS_ASSERT(p, Access, "No vertex position");

               if (p.IsSimilar<Vec3>())
                  output.mPos = Vec4(*p.GetRaw<Vec3>(), 1);
               else if (p.IsSimilar<Vec2>())
                  output.mPos = Vec4(*p.GetRaw<Vec2>(), 0, 1);
               else if (p.IsSimilar<Vec4>())
                  output.mPos = *p.GetRaw<Vec4>();
               else
                  LANGULUS_OOPS(Access, "Unsupported place type");

               if (n) {
                  if (n.IsSimilar<Vec3>())
                     output.mNor = *n.GetRaw<Vec3>();
                  else
                     LANGULUS_OOPS(Access, "Unsupported normal type");
               }

               if (t) {
                  if (t and t.IsSimilar<Vec2>())
                     output.mTex = *t.GetRaw<Vec2>();
                  else
                     LANGULUS_OOPS(Access, "Unsupported sampler type");
               }

               if (c)
                  output.mCol = c.AsCast<RGBA, false>();

               // Cache the vertex                                      
               mVertices << output;
            }
         );

         mView.mTopology = MetaDataOf<A::Triangle>();
         mView.mPrimitiveCount = static_cast<uint32_t>(mVertices.GetCount() / 3);
         mView.mTextureMapping = Math::MapMode::Custom;
      }
      else TODO();
   });
}

/// Check if the cached geometry is made of triangles                         
///   @return true if made of triangle list, otherwise it is line list        
auto ASCIIGeometry::MadeOfTriangles() const noexcept -> bool {
   return mView.mTopology and mView.mTopology->IsSimilar<A::Triangle>();
}

/// Get the vertex array                                                      
///   @return a reference to the vertex array                                 
auto ASCIIGeometry::GetVertices() const noexcept -> const TMany<Vertex>& {
   return mVertices;
}
