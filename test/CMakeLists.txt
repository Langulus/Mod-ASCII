file(GLOB_RECURSE
	LANGULUS_MOD_ASCII_TEST_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp
)

add_executable(LangulusModASCIITest ${LANGULUS_MOD_ASCII_TEST_SOURCES})

target_link_libraries(LangulusModASCIITest
	PRIVATE		Langulus
				Catch2
				$<$<NOT:$<BOOL:${WIN32}>>:pthread>
)

add_dependencies(LangulusModASCIITest
	LangulusModASCII
	LangulusModFTXUI
	LangulusModFileSystem
	LangulusModAssetsGeometry
	LangulusModPhysics
)

add_test(
	NAME		LangulusModASCIITest
	COMMAND		LangulusModASCIITest
	WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)