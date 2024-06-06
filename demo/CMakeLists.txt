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
    LangulusModInputSDL
)

# Make the data dir                                                     
add_custom_command(
    TARGET LangulusModASCIIDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/maxwell"
    COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PROPERTY:LangulusModAssetsGeometry,SOURCE_DIR>/test/data/maxwell.obj"
		"$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/maxwell/maxwell.obj"
    COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PROPERTY:LangulusModAssetsGeometry,SOURCE_DIR>/test/data/maxwell.mtl"
		"$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/maxwell/maxwell.mtl"
    COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PROPERTY:LangulusModAssetsGeometry,SOURCE_DIR>/test/data/dingus_nowhiskers.png"
		"$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/maxwell/dingus_nowhiskers.png"
    COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PROPERTY:LangulusModAssetsGeometry,SOURCE_DIR>/test/data/dingus_whiskers.png"
		"$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/maxwell/dingus_whiskers.png"
    COMMAND ${CMAKE_COMMAND} -E copy
		"$<TARGET_PROPERTY:LangulusModASCIIDemo,SOURCE_DIR>/data/castle.obj"
		"$<TARGET_FILE_DIR:LangulusModASCIIDemo>/data/assets/meshes/castle.obj"
)