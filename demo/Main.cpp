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

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)


int main(int, char**) {
   // Suppress any logging messages, so that we don't interfere with    
   // the ASCII renderer in the console. Instead, redirect all logging  
   // to an external HTML file.                                         
   Logger::ToHTML logFile {"logfile.htm"};
   Logger::AttachRedirector(&logFile);

   // Create root entity                                                
   Framerate<60> fps;
   auto root = Thing::Root(
      "FTXUI",
      "ASCII",
      "FileSystem",
      "AssetsGeometry",
      "Physics",
      "InputSDL"
   );
   root.CreateUnits<
      A::Window,
      A::Renderer,
      A::Layer,
      A::World,
      A::InputGatherer
   >();

   // Create a player entity with controllable camera                   
   auto player = root.CreateChild("Player");
   player->CreateUnits<A::Camera, A::InputListener>();
   player->CreateUnit<A::Instance>(Traits::Place {0, 9, 25.0});
   player->Run("Create("
      "Anticipator(MouseMoveHorizontal, [Move Yaw  (1)]), "
      "Anticipator(MouseMoveVertical,   [Move Pitch(1)]))"
   );

   // Create a rotating dingus                                          
   auto maxwell = root.CreateChild("Maxwell");
   maxwell->CreateUnits<A::Renderable, A::Instance>();
   maxwell->CreateUnit<A::Mesh>("maxwell/maxwell.obj");
   maxwell->Run("Move^1 Yaw(-1)");

   // Loop until quit                                                   
   while (root.Update(fps.GetDeltaTime()))
      fps.Tick();

   Logger::DettachRedirector(&logFile);
   return 0;
}
