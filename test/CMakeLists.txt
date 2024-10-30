file(GLOB_RECURSE
	LANGULUS_MOD_ASCII_TEST_SOURCES 
	LIST_DIRECTORIES FALSE CONFIGURE_DEPENDS
	*.cpp
)

add_langulus_test(LangulusModASCIITest
	SOURCES			${LANGULUS_MOD_ASCII_TEST_SOURCES}
	LIBRARIES		Langulus
					$<$<NOT:$<BOOL:${WIN32}>>:pthread>
	DEPENDENCIES    LangulusModASCII
					LangulusModFTXUI
					LangulusModFileSystem
					LangulusModAssetsGeometry
					LangulusModPhysics
)