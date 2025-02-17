cmake_minimum_required(VERSION 3.28)
project(LangulusModASCII
    VERSION         1.0.0
    DESCRIPTION     "Langulus plug-in module for rendering real-time graphics using ASCII"
    HOMEPAGE_URL    https://langulus.com
)

file(GLOB_RECURSE
	LANGULUS_MOD_ASCII_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	source/*.cpp
)

# Build the module                                                              
add_langulus_mod(LangulusModASCII ${LANGULUS_MOD_ASCII_SOURCES})

if(LANGULUS_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()

add_subdirectory(demo)
