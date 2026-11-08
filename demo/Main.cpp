///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/CppAPI/Platform.hpp>
#include <Langulus/CppAPI/Graphics.hpp>
#include <Langulus/CppAPI/Physical.hpp>
#include <Langulus/CppAPI/Mesh.hpp>
#include <Langulus/CppAPI/Input.hpp>
#include <Langulus/Time.hpp>
#include <thread>

using namespace Langulus;


int main(int, char**) {
   LglsProfile();

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
   root.CreateParts<
      Things::Window,
      Things::Renderer,
      Things::Layer,
      Things::World,
      Things::InputGatherer
   >();

   // Create a player entity with controllable camera                   
   auto player = root.CreateChild("Player");
   player->CreateParts<Things::Camera, Things::InputListener>();
   player->CreatePart<Things::Instance>(Tags::Place {0, 20, 20});
   player->Run("? create Anticipator(MouseMove,          {thing? move (Yaw(?.x * 0.05), Pitch(?.y * 0.05))})");
   player->Run("? create Anticipator(Keys::W,            {thing? move (Axes::Forward  * 4, relative)})");
   player->Run("? create Anticipator(Keys::S,            {thing? move (Axes::Backward * 4, relative)})");
   player->Run("? create Anticipator(Keys::A,            {thing? move (Axes::Left     * 4, relative)})");
   player->Run("? create Anticipator(Keys::D,            {thing? move (Axes::Right    * 4, relative)})");
   player->Run("? create Anticipator(Keys::Space,        {thing? move (Axes::Up       * 4, relative)})");
   player->Run("? create Anticipator(Keys::LeftControl,  {thing? move (Axes::Down     * 4, relative)})");

   // Create a castle                                                   
   auto castle = root.CreateChild("Castle");
   castle->CreateParts<Things::Renderable>();
   castle->CreatePart<Things::Instance>(Tags::Size {450}, Tags::Place {0, -5, 0});
   castle->CreatePart<Things::Mesh>("castle.obj");

   // Create a directional light source                                 
   auto sun = root.CreateChild("Sun");
   sun->CreateParts<Things::Light>();
   sun->CreatePart<Things::Instance>(Tags::Aim {-1, -1, 0});
   sun->Run("? move^1 (Yaw(1), relative)");

   // Loop until quit                                                   
   while (root.Update(fps.GetDeltaTime()))
      fps.Tick();

   Logger::DettachRedirector(&logFile);
   return 0;
}
