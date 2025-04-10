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
#include <Langulus/Math/Normal.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/IO.hpp>


/// Compiled renderable                                                       
struct PipeSubscriber {
   // Overall color                                                     
   RGBAf color;
   // Instance transformation                                           
   Mat4 transform;
   // Mesh                                                              
   const ASCIIGeometry* mesh;
   // Texture                                                           
   const ASCIITexture*  texture;
};

/// A compiled light                                                          
struct LightSubscriber {
   // Light color premultiplied by intensity                            
   RGBAf color;
   // Light MVP                                                         
   Mat4 transform;
   // Light position in world space                                     
   Vec3 position;
   // Light direction for directional/spotlights                        
   Vec3 direction;
   // Type of the light                                                 
   A::Light::Type type;
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
   // Toggle smooth shading                                             
   bool mSmooth = false;
   // Toggle fog calculation                                            
   bool mFog = true;
   RGBAf mFogColor = {0.30f, 0, 0, 1.0f};
   Range1 mFogRange = {5, 25};
   // Toggle vertex colors                                              
   bool mColorize = false;
   // Toggle shadows                                                    
   bool mShadows = true;

   // Toggle culling                                                    
   enum Cull {
      NoCulling, CullBack, CullFront
   } mCull = CullFront;
   
   // Rendering style                                                   
   ASCIIStyle mStyle = ASCIIStyle::Fullblocks;

   // Some styles involve more pixels per character                     
   // Halfblocks are 2x2 pixels per symbol, while Braille is 2x4        
   Scale2i mBufferScale;

   // An intermediate render buffer, used only by the pipeline          
   // This buffer is then compiled into an image inside ASCIILayer      
   mutable ASCIIBuffer<RGBAf> mBuffer;

   // Intermediate (may be sub-pixel) depth buffer, that also acts as   
   // a stencil buffer (pixel is valid if depth is not at max)          
   mutable ASCIIBuffer<float> mDepth;
   
   // Shadowmaps generated by lights                                    
   mutable TMany<ASCIIBuffer<float>> mShadowmaps;

public:
   ASCIIPipeline(ASCIIRenderer*, const Many&);

   void Clear(const RGBAf&, float);
   void Resize(int x, int y);
   void Render(const ASCIILayer*, const Mat4&, const PipeSubscriber&, const TMany<LightSubscriber>&) const;
   void Assemble(const ASCIILayer*) const;

private:
   struct PipelineState {
      const ASCIILayer* mLayer;
      const Scale2 mResolution;
      const Mat4& mProjectedView;
      const PipeSubscriber& mSubscriber;
      const TMany<LightSubscriber>& mLights;
   };

   void RasterizeMesh(const PipelineState&) const;

   template<bool LIT, bool DEPTH, bool SMOOTH, bool FOG, bool COLORIZE, bool SHADOWED>
   void RasterizeTriangle(
      const PipelineState&,
      const Mat4&,
      const ASCIIGeometry::Vertex*,
      const Triangle4&
   ) const;

   void ClipTriangle(const Mat4&, const ASCIIGeometry::Vertex*, auto&&) const;
};
