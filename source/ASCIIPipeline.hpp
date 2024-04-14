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
#include "inner/ASCIIImage.hpp"
#include <Math/Normal.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/IO.hpp>


struct PipeSubscriber {
   RGBA color;
   Mat4 transform;
   A::Mesh* mesh;
   A::Image* texture;
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
   bool mDepth = true;
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
   int mBufferXScale = 1, mBufferYScale = 1;

   // An intermediate render buffer, used only by the pipeline          
   // This buffer is then compiled into an image inside ASCIILayer      
   // Note: alpha channel acts as a stencil, to mark painted pixels     
   mutable ASCIIBuffer<RGBA> mBuffer;

public:
   ASCIIPipeline(ASCIIRenderer*, const Neat&);

   void Resize(int x, int y);
   void Render(const ASCIILayer*, const Mat4&, const PipeSubscriber&) const;

private:
   struct CameraState {
      const ASCIILayer* mLayer;
      const Scale2 mResolution;
      const Mat4& mProjectedView;
      const Mat4& mTransform;
   };

   void RasterizeMesh(const CameraState&, const A::Mesh&) const;
   void RasterizeTriangle(const CameraState&, const Triangle&, const Vec3&, const RGBA&) const;
};
