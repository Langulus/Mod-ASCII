///                                                                           
/// Langulus::Module::ASCII                                                   
/// Copyright (c) 2024 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/Flow/Time.hpp>
#include <Langulus/Platform.hpp>
#include <Langulus/Graphics.hpp>
#include <Langulus/Physical.hpp>
#include <Langulus/Mesh.hpp>
#include <Langulus/Image.hpp>
#include <Langulus/Verbs/Interpret.hpp>
#include <Langulus/Verbs/Compare.hpp>
#include <Langulus/Testing.hpp>


SCENARIO("Renderer creation inside a window", "[renderer]") {
   static Allocator::State memoryState;

   for (int repeat = 0; repeat != 10; ++repeat) {
      GIVEN(std::string("Init and shutdown cycle #") + std::to_string(repeat)) {
         // Create root entity                                          
         auto root = Thing::Root<false>("FTXUI", "ASCII");
         
         WHEN("A renderer is created via abstractions") {
            auto window = root.CreateUnit<A::Window>();
            auto renderer = root.CreateUnit<A::Renderer>();
            root.DumpHierarchy();
               
            REQUIRE(window);
            REQUIRE(window.IsSparse());
            REQUIRE(window.CastsTo<A::Window>());

            REQUIRE(renderer);
            REQUIRE(renderer.IsSparse());
            REQUIRE(renderer.CastsTo<A::Renderer>());

            REQUIRE(root.GetUnits().GetCount() == 2);
         }

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         WHEN("A renderer is created via tokens") {
            auto window = root.CreateUnitToken("A::Window");
            auto renderer = root.CreateUnitToken("Renderer");
            root.DumpHierarchy();
               
            REQUIRE(window);
            REQUIRE(window.IsSparse());
            REQUIRE(window.CastsTo<A::Window>());

            REQUIRE(renderer);
            REQUIRE(renderer.IsSparse());
            REQUIRE(renderer.CastsTo<A::Renderer>());

            REQUIRE(root.GetUnits().GetCount() == 2);
         }
      #endif

         // Check for memory leaks after each cycle                     
         REQUIRE(memoryState.Assert());
      }
   }
}

SCENARIO("Drawing an empty window", "[renderer]") {
   static Allocator::State memoryState;

   GIVEN("A window with a renderer") {
      // Create the scene                                               
      auto root = Thing::Root<false>("FTXUI", "ASCII");
      root.CreateUnits<A::Window, A::Renderer>();

      static Allocator::State memoryState2;

      for (int repeat = 0; repeat != 10; ++repeat) {
         WHEN(std::string("Update cycle #") + std::to_string(repeat)) {
            // Update the scene                                         
            root.Update(16ms);

            // And interpret the scene as an image, i.e. taking a       
            // screenshot                                               
            Verbs::InterpretAs<A::Image*> interpret;
            root.Run(interpret);

            REQUIRE(root.GetUnits().GetCount() == 2);
            REQUIRE_FALSE(root.HasUnits<A::Image>());
            REQUIRE(interpret.IsDone());
            REQUIRE(interpret->GetCount() == 1);
            REQUIRE(interpret->IsSparse());
            REQUIRE(interpret->template CastsTo<A::Image>());

            Verbs::Compare compare {Colors::Red};
            interpret.Then(compare);

            REQUIRE(compare.IsDone());
            REQUIRE(compare->GetCount() == 1);
            REQUIRE(compare->IsDense());
            REQUIRE(compare.GetOutput() == Compared::Equal);

            root.DumpHierarchy();

            // Check for memory leaks after each update cycle           
            REQUIRE(memoryState2.Assert());
         }
      }
   }

   // Check for memory leaks after each initialization cycle            
   REQUIRE(memoryState.Assert());
}

SCENARIO("Drawing solid polygons", "[renderer]") {
   static Allocator::State memoryState;

   GIVEN("A window with a renderer") {
      // Create the scene                                               
      auto root = Thing::Root<false>(
         "FTXUI",
         "ASCII",
         "FileSystem",
         "AssetsGeometry",
         "Physics"
      );
      root.CreateUnits<A::Window, A::Renderer, A::Layer, A::World>();

      auto rect = root.CreateChild(Traits::Size {10, 5}, "Rectangles");
      rect->CreateUnit<A::Renderable>();
      rect->CreateUnit<A::Mesh>(Math::Box2 {});
      rect->CreateUnit<A::Instance>(Traits::Place(10, 10), Colors::Black);
      rect->CreateUnit<A::Instance>(Traits::Place(50, 10), Colors::Green);
      rect->CreateUnit<A::Instance>(Traits::Place(10, 30), Colors::Blue);
      rect->CreateUnit<A::Instance>(Traits::Place(50, 30), Colors::White);
      root.DumpHierarchy();

      //static Allocator::State memoryState2;

      for (int repeat = 0; repeat != 10; ++repeat) {
         WHEN(std::string("Update cycle #") + std::to_string(repeat)) {
            // Update the scene                                         
            root.Update(16ms);

            // And interpret the scene as an image, i.e. taking a       
            // screenshot                                               
            Verbs::InterpretAs<A::Image*> interpret;
            root.Run(interpret);

            REQUIRE(root.GetUnits().GetCount() == 4);
            REQUIRE(rect->GetUnits().GetCount() == 6);
            REQUIRE(root.GetChildren().GetCount() == 1);
            REQUIRE_FALSE(root.HasUnits<A::Image>());
            REQUIRE(interpret.IsDone());
            REQUIRE(interpret->GetCount() == 1);
            REQUIRE(interpret->IsSparse());
            REQUIRE(interpret->template CastsTo<A::Image>());

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

   // Check for memory leaks after each initialization cycle            
   REQUIRE(memoryState.Assert());
}

