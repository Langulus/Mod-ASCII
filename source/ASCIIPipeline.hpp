///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Common.hpp"
#include "inner/ASCIITexture.hpp"
#include "inner/ASCIIGeometry.hpp"
#include <Math/Normal.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/IO.hpp>


struct PipeSubscriber {
   RGBA color;
   Mat4 transform;
   const ASCIIGeometry* mesh;
   const ASCIITexture*  texture;
};

/// Defines how pixels are mapped onto symbols                                
enum class ASCIIStyle {
   Text = 0,      // Just for drawing text as it is                     

   Fullblocks,    // ' ', '░', '▒', '▓', '█'                            

   Halfblocks,    // ▖,	▗,	▘,	▙,	▚,	▛,	▜,	▝,	▞,	▟,                      

   Braille        //  , ⠁, ⠂, ⠃, ⠄, ⠅, ⠆, ⠇, ⠈, ⠉, ⠊, ⠋, ⠌, ⠍, ⠎, ⠏,    
                  // ⠐, ⠑, ⠒, ⠓, ⠔, ⠕, ⠖, ⠗, ⠘, ⠙, ⠚, ⠛, ⠜, ⠝, ⠞, ⠟,    
                  // ⠠, ⠡, ⠢, ⠣, ⠤, ⠥, ⠦, ⠧, ⠨, ⠩, ⠪, ⠫, ⠬, ⠭, ⠮, ⠯,    
                  // ⠰, ⠱, ⠲, ⠳, ⠴, ⠵, ⠶, ⠷, ⠸, ⠹, ⠺, ⠻, ⠼, ⠽, ⠾, ⠿     
                  // ⡀, ⡁, ⡂, ⡃, ⡄, ⡅, ⡆, ⡇, ⡈, ⡉, ⡊, ⡋, ⡌, ⡍, ⡎, ⡏     
                  // ⡐, ⡑, ⡒, ⡓, ⡔, ⡕, ⡖, ⡗, ⡘, ⡙, ⡚, ⡛, ⡜, ⡝, ⡞, ⡟,    
                  // ⡠, ⡡, ⡢, ⡣, ⡤, ⡥, ⡦, ⡧, ⡨, ⡩, ⡪, ⡫, ⡬, ⡭, ⡮, ⡯,    
                  // ⡰, ⡱, ⡲, ⡳, ⡴, ⡵, ⡶, ⡷, ⡸, ⡹, ⡺, ⡻, ⡼, ⡽, ⡾, ⡿,    
                  // ⢀, ⢁, ⢂, ⢃, ⢄, ⢅, ⢆, ⢇, ⢈, ⢉, ⢊, ⢋, ⢌, ⢍, ⢎, ⢏,    
                  // ⢐, ⢑, ⢒, ⢓, ⢔, ⢕, ⢖, ⢗, ⢘, ⢙, ⢚, ⢛, ⢜, ⢝, ⢞, ⢟,    
                  // ⢠, ⢡, ⢢, ⢣, ⢤, ⢥, ⢦, ⢧, ⢨, ⢩, ⢪, ⢫, ⢬, ⢭, ⢮, ⢯,    
                  // ⢰, ⢱, ⢲, ⢳, ⢴, ⢵, ⢶, ⢷, ⢸, ⢹, ⢺, ⢻, ⢼, ⢽, ⢾, ⢿,    
                  // ⣀, ⣁, ⣂, ⣃, ⣄, ⣅, ⣆, ⣇, ⣈, ⣉, ⣊, ⣋, ⣌, ⣍, ⣎, ⣏,    
                  // ⣐, ⣑, ⣒, ⣓, ⣔, ⣕, ⣖, ⣗, ⣘, ⣙, ⣚, ⣛, ⣜, ⣝, ⣞, ⣟,    
                  // ⣠, ⣡, ⣢, ⣣, ⣤, ⣥, ⣦, ⣧, ⣨, ⣩, ⣪, ⣫, ⣬, ⣭, ⣮, ⣯,    
                  // ⣰, ⣱, ⣲, ⣳, ⣴, ⣵, ⣶, ⣷, ⣸, ⣹, ⣺, ⣻, ⣼, ⣽, ⣾, ⣿     
};


///                                                                           
///   ASCII pipeline                                                          
///                                                                           
/// Rasterizes vector graphics into the backbuffer of an ASCIILayer           
///                                                                           
struct ASCIIPipeline : A::Graphics, ProducedFrom<ASCIIRenderer> {
   LANGULUS(ABSTRACT) false;
   LANGULUS_BASES(A::Graphics);

private:
   // Toggle depth testing and writing                                  
   bool mDepthTest = true;
   // Toggle light calculation                                          
   bool mLit = true;

   // Toggle culling                                                    
   enum Cull {
      NoCulling, CullBack, CullFront
   } mCull = NoCulling;
   
   // Rendering style                                                   
   ASCIIStyle mStyle = ASCIIStyle::Fullblocks;

   // Some styles involve more pixels per character                     
   // Halfblocks are 2x2 pixels per symbol, while Braille is 2x4        
   Scale2i mBufferScale;

   // An intermediate render buffer, used only by the pipeline          
   // This buffer is then compiled into an image inside ASCIILayer      
   mutable ASCIIBuffer<RGBA>  mBuffer;

   // Intermediate (may be sub-pixel) depth buffer, that also acts as   
   // a stencil buffer (pixel is valid if depth is not at max)          
   mutable ASCIIBuffer<float> mDepth;

public:
   ASCIIPipeline(ASCIIRenderer*, const Neat&);

   void Clear(RGBA, float);
   void Resize(int x, int y);
   void Render(const ASCIILayer*, const Mat4&, const PipeSubscriber&) const;
   void Assemble(const ASCIILayer*) const;

private:
   struct PipelineState {
      const ASCIILayer* mLayer;
      const Scale2 mResolution;
      const Mat4& mProjectedView;
      const PipeSubscriber& mSubscriber;
   };

   void RasterizeMesh(const PipelineState&) const;

   template<bool LIT, bool DEPTH, bool SMOOTH>
   void RasterizeTriangle(
      const PipelineState&,
      const Mat3&, const Mat4&,
      const ASCIIGeometry::Vertex*
   ) const;
};
