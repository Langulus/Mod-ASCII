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
   player->CreateUnit<A::Instance>(Traits::Place {0, 20, 20});
   player->Run("Create Anticipator(MouseMove, {thing? Move*-1 (Yaw(?.x * 0.05), Pitch(?.y * 0.05))})");
   //player->Run("Create Anticipator(Keys::W,   [thing? Move    (Axes::Forward * Derive(?.Time))])");
   //player->Run("Move Pitch(-45)");

   // Create a castle                                                   
   auto castle = root.CreateChild("Castle");
   castle->CreateUnits<A::Renderable>();
   castle->CreateUnit<A::Instance>(Traits::Size {450}, Traits::Place {0, -5, 0});
   castle->CreateUnit<A::Mesh>("castle.obj");

   // Loop until quit                                                   
   while (root.Update(fps.GetDeltaTime()))
      fps.Tick();

   Logger::DettachRedirector(&logFile);
   return 0;
}
