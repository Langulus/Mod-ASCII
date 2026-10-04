///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/CppAPI/Platform.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/CppAPI/Physical.hpp>
#include <Langulus/CppAPI/Mesh.hpp>
#include <Langulus/CppAPI/Input.hpp>
#include <Langulus/Flow/Time.hpp>
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
      Things::Window,
      Things::Renderer,
      Things::Layer,
      A::World,
      Things::InputGatherer
   >();

   // Create a player entity with controllable camera                   
   auto player = root.CreateChild("Player");
   player->CreateUnits<Things::Camera, Things::InputListener>();
   player->CreateUnit<Things::Instance>(Traits::Place {0, 20, 20});
   player->Run("? create Anticipator(MouseMove,          {thing? move (Yaw(?.x * 0.05), Pitch(?.y * 0.05))})");
   player->Run("? create Anticipator(Keys::W,            {thing? move (Axes::Forward  * 4, relative)})");
   player->Run("? create Anticipator(Keys::S,            {thing? move (Axes::Backward * 4, relative)})");
   player->Run("? create Anticipator(Keys::A,            {thing? move (Axes::Left     * 4, relative)})");
   player->Run("? create Anticipator(Keys::D,            {thing? move (Axes::Right    * 4, relative)})");
   player->Run("? create Anticipator(Keys::Space,        {thing? move (Axes::Up       * 4, relative)})");
   player->Run("? create Anticipator(Keys::LeftControl,  {thing? move (Axes::Down     * 4, relative)})");

   // Create a castle                                                   
   auto castle = root.CreateChild("Castle");
   castle->CreateUnits<Things::Renderable>();
   castle->CreateUnit<Things::Instance>(Traits::Size {450}, Traits::Place {0, -5, 0});
   castle->CreateUnit<Things::Mesh>("castle.obj");

   // Create a directional light source                                 
   auto sun = root.CreateChild("Sun");
   sun->CreateUnits<Things::Light>();
   sun->CreateUnit<Things::Instance>(Traits::Aim {-1, -1, 0});
   sun->Run("? move^1 (Yaw(1), relative)");

   // Loop until quit                                                   
   while (root.Update(fps.GetDeltaTime()))
      fps.Tick();

   Logger::DettachRedirector(&logFile);
   return 0;
}
