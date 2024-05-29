///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus.hpp>
#include <Langulus/Platform.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/Physical.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/Input.hpp>
#include <Flow/Time.hpp>
#include <thread>

using namespace Langulus;
using namespace std::chrono;
using dsec = duration<double>;
constexpr int FPS = 60;

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)


int main(int, char**) {
   // Suppress any logging messages, so that we don't interfere with    
   // the ASCII renderer in the console                                 
   Logger::ToHTML logFile {"logfile.htm"};
   Logger::AttachRedirector(&logFile);

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
   root.LoadMod("InputSDL");

   root.CreateUnit<A::Window>();
   root.CreateUnit<A::Renderer>();
   root.CreateUnit<A::Layer>();
   root.CreateUnit<A::World>();
   root.CreateUnit<A::InputGatherer>();

   // Create a player entity with controllable camera                   
   auto player = root.CreateChild({Traits::Name {"Player"}});
   player->CreateUnit<A::Camera>();
   player->CreateUnit<A::Instance>(Traits::Place {0, 9, 25.0});
   player->CreateUnit<A::InputListener>();
   player->Run("Create "
      "Anticipator(MouseMoveHorizontal, [Move Yaw(1)]), "
      "Anticipator(MouseMoveVertical,   [Move Pitch(1)])");

   // Create a rotating dingus                                          
   auto maxwell = root.CreateChild({Traits::Name {"Maxwell"}});
   maxwell->CreateUnit<A::Renderable>();
   maxwell->CreateUnit<A::Mesh>("maxwell/maxwell.obj");
   maxwell->CreateUnit<A::Instance>();
   maxwell->Run("Move^0.016 Yaw(-1)");

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

   Logger::DettachRedirector(&logFile);
   return 0;
}
