///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "ASCII.hpp"


/// Descriptor constructor                                                    
///   @param producer - the pipeline producer                                 
///   @param descriptor - the pipeline descriptor                             
ASCIIPipeline::ASCIIPipeline(ASCIIRenderer* producer, const Neat& descriptor)
   : Resolvable {this}
   , ProducedFrom {producer, descriptor} {
   VERBOSE_ASCII("Initializing graphics pipeline from: ", descriptor);
   bool predefinedMaterial = false;
   descriptor.ForEach(
      [this](const ASCIILayer& layer) {
         // Add layer preferences                                       
         if (layer.GetStyle() & ASCIILayer::Hierarchical)
            mDepth = false;
         return Loop::NextLoop;
      },
      [this, &predefinedMaterial](const A::Material& material) {
         // Create from predefined material generator                   
         TODO();
         predefinedMaterial = true;
         return Loop::Break;
      }
   );

   if (not predefinedMaterial) {
      // We must generate the material ourselves                        
      Construct material;
      descriptor.ForEach(
         [&](const Text& text) {
            // Create from file                                         
            material = FromText(text);
            return Loop::Break;
         },
         [&](const A::Mesh& mesh) {
            // Adapt to a mesh                                          
            material = FromMesh(mesh);
            return Loop::Break;
         }
      );

      LANGULUS_ASSERT(material.GetDescriptor(), Graphics,
         "Couldn't generate material request for pipeline");

      material << Traits::Output {
         Traits::Color {Rate::Pixel, MetaOf<Text>()}
      };

      // Now create generator, and the pipeline from it                 
      VERBOSE_ASCII("Pipeline material will be generated from: ", material);
      Verbs::Create creator {Abandon(material)};
      producer->RunIn(creator)->ForEach(
         [this](const A::Material& generator) {
            TODO();
         }
      );
   }
}

/// Create a pipeline capable of rendering a mesh                             
///   @attention geometry will be generated, in order to access data types    
///   @param mesh - the mesh content generator                                
Construct ASCIIPipeline::FromMesh(const A::Mesh& mesh) {
   // We use the geometry's properties to define a material             
   // generator, which we later use to initialize this pipeline         
   auto request = Construct::From<A::Material>(
      Traits::Topology {mesh.GetTopology()}
   );

   Traits::Input inputs;
   const auto instances = mesh.GetData<Traits::Transform>();
   if (instances) {
      // Create geometry shader hardware instancing input               
      inputs << Traits::Transform {
         Rate::Primitive, instances->GetType()
      };
   }

   const auto positions = mesh.GetData<Traits::Place>();
   if (positions) {
      // Create a vertex position input and project it                  
      inputs << Traits::Place       {Rate::Vertex, positions->GetType()}
             << Traits::View        {Rate::Level,  positions->GetType()}
             << Traits::Projection  {Rate::Camera, positions->GetType()};

      // If not using hardware-instancing, use the Thing's instances    
      if (not instances) {
         inputs << Traits::Transform {
            Rate::Instance, positions->GetType()
         };
      }
   }
         
   const auto normals = mesh.GetData<Traits::Aim>();
   if (normals) {
      // Create a vertex normals input                                  
      inputs << Traits::Aim {
         Rate::Vertex, normals->GetType()
      };
   }
         
   const auto textureCoords = mesh.GetData<Traits::Sampler>();
   if (textureCoords) {
      // Create a vertex texture coordinates input                      
      inputs << Traits::Sampler {
         Rate::Vertex, textureCoords->GetType()
      };
   }
         
   const auto materialIds = mesh.GetData<Traits::Material>();
   if (materialIds) {
      // Create a vertex material ids input                             
      inputs << Traits::Material {
         Rate::Vertex, materialIds->GetType()
      };
   }

   if (inputs)
      request << Abandon(inputs);
   return Abandon(request);
}

/// Directly initialize the back buffer using an ASCII "screenshot"           
///   @param text - the formatted screenshot                                  
Construct ASCIIPipeline::FromText(const Text& text) {
   TODO();
   return {};
}

/// Draw all subscribers of a given level (used in batched rendering)         
///   @param offset - the subscriber to start from                            
///   @return the number of rendered subscribers                              
Count ASCIIPipeline::RenderLevel(Offset offset) const {
   // Get the initial state to check for interrupts                     
   const auto& initial = mSubscribers[offset];

   // And for each subscriber...                                        
   auto i = offset;
   for (; i < mSubscribers.GetCount() - 1; ++i)
      Render(*mSubscribers[i]);
   return i;
}

/// Draw a single renderable (used in hierarchical drawing)                   
///   @param sub - the subscriber to render                                   
void ASCIIPipeline::Render(const ASCIIRenderable& sub) const {
   TODO(); // this is where rasterization happens
}