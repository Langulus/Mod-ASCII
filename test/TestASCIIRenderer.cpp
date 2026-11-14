///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/Time.hpp>
#include <Langulus/CppAPI/Platform.hpp>
#include <Langulus/CppAPI/Graphics.hpp>
#include <Langulus/CppAPI/Physical.hpp>
#include <Langulus/CppAPI/Mesh.hpp>
#include <Langulus/CppAPI/Image.hpp>
#include <Langulus/Verbs/Interpret.hpp>
#include <Langulus/Verbs/Compare.hpp>
#include <Langulus/Testing.hpp>
#include <Langulus/Allocator.hpp>
#include <Langulus/Primitives/TBox.hpp>

using namespace Langulus;
using Things::Thing;


SCENARIO("Renderer creation inside a window") {
   static MemoryState memoryState;

   GIVEN("Init and shutdown cycles") {
      for (int repeat = 0; repeat != 10; ++repeat) {
         // Create root entity                                          
         auto root = Thing::Root<false>("FTXUI", "ASCII");
         
         WHEN("A renderer is created via abstractions") {
            auto window = root.CreatePart<Things::Window>();
            auto renderer = root.CreatePart<Things::Renderer>();
            root.DumpHierarchy();
               
            REQUIRE(window);
            REQUIRE(window.IsSparse());
            REQUIRE(window.CastsTo<Things::Window>());

            REQUIRE(renderer);
            REQUIRE(renderer.IsSparse());
            REQUIRE(renderer.CastsTo<Things::Renderer>());

            REQUIRE(root.GetParts().GetCount() == 2);
         }

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         WHEN("A renderer is created via tokens") {
            auto window = root.CreatePartToken("Things::Window");
            auto renderer = root.CreatePartToken("Renderer");
            root.DumpHierarchy();
               
            REQUIRE(window);
            REQUIRE(window.IsSparse());
            REQUIRE(window.CastsTo<Things::Window>());

            REQUIRE(renderer);
            REQUIRE(renderer.IsSparse());
            REQUIRE(renderer.CastsTo<Things::Renderer>());

            REQUIRE(root.GetParts().GetCount() == 2);
         }
      #endif

         // Check for memory leaks after each cycle                     
         REQUIRE(memoryState.Assert());
      }
   }

   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}

SCENARIO("Drawing an empty window") {
   static MemoryState memoryState;

   GIVEN("A window with a renderer") {
      // Create the scene                                               
      auto root = Thing::Root<false>("FTXUI", "ASCII");
      root.CreateParts<Things::Window, Things::Renderer>();

      static MemoryState memoryState2;

      WHEN("Update cycles") {
         for (int repeat = 0; repeat != 10; ++repeat) {
            // Update the scene                                         
            root.Update(16ms);

            // And interpret the scene as an image, i.e. taking a       
            // screenshot                                               
            Verbs::Interpret interpret(MetaDataOf<Things::Image*>());
            interpret.In(&root).Run();

            REQUIRE(root.GetParts().GetCount() == 2);
            REQUIRE_FALSE(root.HasParts<Things::Image>());
            REQUIRE(interpret.IsDone());
            REQUIRE(interpret->GetCount() == 1);
            REQUIRE(interpret->IsSparse());
            REQUIRE(interpret->template CastsTo<Things::Image>());

            Verbs::Compare compare {Colors::Red};
            interpret.Then(compare);

            REQUIRE(compare.IsDone());
            REQUIRE(compare->GetCount() == 1);
            REQUIRE(not compare->IsSparse());
            REQUIRE(compare.GetOutput() == Compared::Equal);

            root.DumpHierarchy();

            // Check for memory leaks after each update cycle           
            REQUIRE(memoryState2.Assert());
         }
      }
   }

   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}

SCENARIO("Drawing solid polygons") {
   static MemoryState memoryState;

   GIVEN("A window with a renderer") {
      // Create the scene                                               
      auto root = Thing::Root<false>(
         "FTXUI",
         "ASCII",
         "FileSystem",
         "AssetsGeometry",
         "Physics"
      );
      root.CreateParts<Things::Window, Things::Renderer, Things::Layer, Things::World>();

      auto rect = root.CreateChild(Tags::Size {10, 5}, "Rectangles");
      rect->CreatePart<Things::Renderable>();
      rect->CreatePart<Things::Mesh>(Math::Box2 {});
      rect->CreatePart<Things::Instance>(Tags::Place(10, 10), Colors::Black);
      rect->CreatePart<Things::Instance>(Tags::Place(50, 10), Colors::Green);
      rect->CreatePart<Things::Instance>(Tags::Place(10, 30), Colors::Blue);
      rect->CreatePart<Things::Instance>(Tags::Place(50, 30), Colors::White);
      root.DumpHierarchy();

      WHEN("Update cycles") {
         for (int repeat = 0; repeat != 10; ++repeat) {
            // Update the scene                                         
            root.Update(16ms);

            // And interpret the scene as an image, i.e. taking a       
            // screenshot                                               
            Verbs::Interpret interpret(MetaDataOf<Things::Image*>());
            interpret.In(&root).Run();

            REQUIRE(root.GetParts().GetCount() == 4);
            REQUIRE(rect->GetParts().GetCount() == 6);
            REQUIRE(root.GetChildren().GetCount() == 1);
            REQUIRE_FALSE(root.HasParts<Things::Image>());
            REQUIRE(interpret.IsDone());
            REQUIRE(interpret->GetCount() == 1);
            REQUIRE(interpret->IsSparse());
            REQUIRE(interpret->template CastsTo<Things::Image>());

            /*Verbs::Compare compare {"polygons.png"};
            interpret.Then(compare);

            REQUIRE(compare.IsDone());
            REQUIRE(compare->GetCount() == 1);
            REQUIRE(compare->IsDense());
            REQUIRE(compare.GetOutput() == Compared::Equal);*/

            root.DumpHierarchy();

            // Check for memory leaks after each update cycle           
            //REQUIRE(memoryState2.Assert());
         }
      }
   }

   REQUIRE(memoryState.Assert());
   REQUIRE_FALSE(Allocator::CollectGarbage());
}

