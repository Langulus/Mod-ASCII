///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "../ASCII.hpp"
#include <Langulus/Math/Normal.hpp>
#include <Langulus/Math/Sampler.hpp>


/// Descriptor constructor                                                    
///   @param producer the producer of the part                                
///   @param descriptor the part descriptor                                   
ASCIIGeometry::ASCIIGeometry(ASCIIRenderer* producer, Many const& descriptor)
   : Downcastable {this}
   , ProducedFrom {producer, descriptor} {
   bool firstVertex = true;
   Range4 dataRange;

   // Scan the descriptor                                               
   descriptor.ForEachDeep([&](Things::Mesh const& mesh) {
      if (mesh.MadeOfTriangles()) {
         // Cache a triangle list                                       
         mesh.ForEachVertex(
            [&](const Tags::Place&   p,
                const Tags::Aim&     n,
                const Tags::Sampler& t,
                const Tags::Color&   c
            ) {
               Vertex output;
               LglsAssert(p, "No vertex position");

               if (p.IsSame<Vec3>())
                  output.mPos = Vec4(*p.GetRaw<Vec3>(), 1);
               else if (p.IsSame<Vec2>())
                  output.mPos = Vec4(*p.GetRaw<Vec2>(), 0, 1);
               else if (p.IsSame<Vec4>())
                  output.mPos = *p.GetRaw<Vec4>();
               else
                  LglsError("Unsupported place type");

               if (firstVertex) {
                  dataRange.mMin = dataRange.mMax = output.mPos;
                  firstVertex = false;
               }
               else dataRange.Embrace(output.mPos);

               if (n) {
                  if (n.IsSame<Vec3>())
                     output.mNor = *n.GetRaw<Vec3>();
                  else
                     LglsError("Unsupported normal type");
               }

               if (t) {
                  if (t and t.IsSame<Vec2>())
                     output.mTex = *t.GetRaw<Vec2>();
                  else
                     LglsError("Unsupported sampler type");
               }

               if (c)
                  output.mCol = c.AsCast<RGBA, false>();

               // Cache the vertex                                      
               mVertices << output;
            }
         );

         mView.mTopology = MetaDataOf<Math::Triangle>();
         mView.mPrimitiveCount = static_cast<uint32_t>(mVertices.GetCount() / 3);
         mView.mTextureMapping = Math::MapMode::Custom;
      }
      else TODO();
   });

   Logger::Verbose(this, ": Range is: ", dataRange);
}

/// Check if the cached geometry is made of triangles                         
///   @return true if made of triangle list, otherwise it is line list        
auto ASCIIGeometry::MadeOfTriangles() const noexcept -> bool {
   return mView.mTopology and mView.mTopology->IsSame<Math::Triangle>();
}

/// Get the vertex array                                                      
///   @return a reference to the vertex array                                 
auto ASCIIGeometry::GetVertices() const noexcept -> const TMany<Vertex>& {
   return mVertices;
}
