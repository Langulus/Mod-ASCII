///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include <Langulus.hpp>
#include <Langulus/Platform.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/Physical.hpp>
#include <Langulus/Mesh.hpp>
#include <Flow/Time.hpp>
#include <thread>

using namespace Langulus;
using namespace std::chrono;
using dsec = duration<double>;
constexpr int FPS = 60;

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)


int main(int, char**) {
   auto invFpsLimit = round<system_clock::duration>(dsec {1. / FPS});
   auto m_BeginFrame = system_clock::now();
   auto m_EndFrame = m_BeginFrame + invFpsLimit;
   auto prev_time_in_seconds = time_point_cast<seconds>(m_BeginFrame);

   // Create root entity                                                
   Thing root;
   root.SetName("ROOT");
   root.CreateRuntime();
   root.CreateFlow();
   root.LoadMod("FTXUI");
   root.LoadMod("ASCII");
   root.LoadMod("FileSystem");
   root.LoadMod("AssetsGeometry");
   root.LoadMod("Physics");

   root.CreateUnit<A::Window>();
   root.CreateUnit<A::Renderer>();
   root.CreateUnit<A::Layer>();
   root.CreateUnit<A::Camera>();
   root.CreateUnit<A::World>();

   auto maxwell = root.CreateChild({Traits::Name {"Maxwell"}});
   maxwell->CreateUnit<A::Renderable>();
   maxwell->CreateUnit<A::Mesh>("maxwell/maxwell.obj");
   maxwell->CreateUnit<A::Instance>(Traits::Place {0, -9, -25.0});
   maxwell->Run("Move^1 Yaw(-20)");

   while (true) {
      // Update until quit                                              
      if (not root.Update(16ms))
         break;

      // This part is just measuring if we're keeping the frame rate    
      const auto time_in_seconds = time_point_cast<seconds>(system_clock::now());
      if (time_in_seconds > prev_time_in_seconds)
         prev_time_in_seconds = time_in_seconds;

      // This part keeps the frame rate                                 
      std::this_thread::sleep_until(m_EndFrame);
      m_BeginFrame = m_EndFrame;
      m_EndFrame = m_BeginFrame + invFpsLimit;
   }

   return 0;
}
