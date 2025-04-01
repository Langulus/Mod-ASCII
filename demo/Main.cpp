///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/Platform.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/Physical.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/Input.hpp>
#include <Langulus/Flow/Time.hpp>
#include <Langulus/Profiler.hpp>
#include <thread>

using namespace Langulus;

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)


int main(int, char**) {
   LANGULUS(PROFILE);

   // Suppress any logging messages, so that we don't interfere with    
   // the ASCII renderer in the console. Instead, redirect all logging  
   // to an external HTML file.                                         
   Logger::ToHTML logFile {"ascii-demo.htm"};
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
   player->Run("? create Anticipator(MouseMove,          {thing? move (Yaw(?.x * 0.05), Pitch(?.y * 0.05))})");
   player->Run("? create Anticipator(Keys::W,            {thing? move (Axes::Forward  * 4, relative)})");
   player->Run("? create Anticipator(Keys::S,            {thing? move (Axes::Backward * 4, relative)})");
   player->Run("? create Anticipator(Keys::A,            {thing? move (Axes::Left     * 4, relative)})");
   player->Run("? create Anticipator(Keys::D,            {thing? move (Axes::Right    * 4, relative)})");
   player->Run("? create Anticipator(Keys::Space,        {thing? move (Axes::Up       * 4, relative)})");
   player->Run("? create Anticipator(Keys::LeftControl,  {thing? move (Axes::Down     * 4, relative)})");

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
