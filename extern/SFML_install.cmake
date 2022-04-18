FetchContent_Declare(
    SFML
    URL ${SFML_URL}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/SFML/
    )
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/extern/SFML)
message(STATUS "Downloading SFML")
FetchContent_MakeAvailable(SFML)
include_directories(${CMAKE_SOURCE_DIR}/extern/SFML/include)

if(UNIX)
	set(PREFIX_SFML "lib")
	set(SUFFIX_SFML "so.2.5")
    set(SUFFIX_SFML_Find "so.2.5*")
elseif(MSVC)
	set(PREFIX_SFML "")
	set(SUFFIX_SFML "lib")
    set(SUFFIX_SFML_Find "lib")
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} PRE_BUILD
						COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_SOURCE_DIR}/extern/SFML/bin/ $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
					  )
endif()

set(SFML_clean FALSE CACHE BOOL "")
mark_as_advanced(SFML_clean)

if(NOT ${SFML_clean})
    file(GLOB SFML_includes "${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/[SWGCswgc]*.hpp")
    file(GLOB SFML_includes_graphics "${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/Graphics/*")
    file(GLOB SFML_includes_window "${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/Window/*")
    file(GLOB SFML_includes_system "${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/System/*")
    file(GLOB SFML_includes_bin "${CMAKE_SOURCE_DIR}/extern/SFML/bin/[swg]*[smw]-2.dll")
    file(GLOB SFML_libs "${CMAKE_SOURCE_DIR}/extern/SFML/lib/${PREFIX_SFML}sfml-[swg]*[ceo]?.${SUFFIX_SFML_Find}")
    file(GLOB_RECURSE SFML_files_to_delete "${CMAKE_SOURCE_DIR}/extern/SFML/*")
    list(APPEND SFML_includes ${SFML_includes_bin} ${SFML_includes_graphics} ${SFML_includes_system} 
                ${SFML_includes_window} ${SFML_includes_bin} ${SFML_libs})  
    list(APPEND SFML_files_to_delete ${CMAKE_SOURCE_DIR}/extern/SFML/examples
                ${CMAKE_SOURCE_DIR}/extern/SFML/doc
                ${CMAKE_SOURCE_DIR}/extern/SFML/lib/cmake
                ${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/Audio
                ${CMAKE_SOURCE_DIR}/extern/SFML/include/SFML/Network
                ${CMAKE_SOURCE_DIR}/extern/SFML/share
        )
    list(REMOVE_ITEM SFML_files_to_delete ${SFML_includes})
    file(REMOVE_RECURSE ${SFML_files_to_delete})
    set(SFML_clean TRUE CACHE BOOL "" FORCE)
endif()
find_library(sfml-graphics
	NAMES ${PREFIX_SFML}sfml-graphics.${SUFFIX_SFML} 
	PATHS ${CMAKE_SOURCE_DIR}/extern/SFML/lib/
	)
find_library(sfml-window
	NAMES ${PREFIX_SFML}sfml-window.${SUFFIX_SFML} 
	PATHS ${CMAKE_SOURCE_DIR}/extern/SFML/lib/
	)
find_library(sfml-system
	NAMES ${PREFIX_SFML}sfml-system.${SUFFIX_SFML}
	PATHS ${CMAKE_SOURCE_DIR}/extern/SFML/lib/
	)
#foreach(files IN LISTS SFML_includes)
#	message(STATUS ${files})
#endforeach()

target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC ${sfml-graphics} ${sfml-window} ${sfml-system})
