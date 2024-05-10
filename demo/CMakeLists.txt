file(GLOB_RECURSE
	LANGULUS_MOD_ASCII_DEMO_SOURCES
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp
)

add_executable(LangulusModASCIIDemo ${LANGULUS_MOD_ASCII_DEMO_SOURCES})

target_link_libraries(LangulusModASCIIDemo
	PRIVATE		Langulus
				$<$<NOT:$<BOOL:${WIN32}>>:pthread>
)

add_dependencies(LangulusModASCIIDemo
	LangulusModASCII
	LangulusModFTXUI
	LangulusModFileSystem
	LangulusModAssetsGeometry
	LangulusModPhysics
)
