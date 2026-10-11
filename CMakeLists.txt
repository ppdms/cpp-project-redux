cmake_minimum_required(VERSION 3.15)
project(Game)

# ---------------------------------------------------------------------------
# Emscripten / WebAssembly build.
# Configure with: emcmake cmake -B build-web && cmake --build build-web
# Then serve the project root (e.g. `python3 -m http.server`) and open
# http://localhost:8000/web/index.html
# ---------------------------------------------------------------------------
if (EMSCRIPTEN)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    file(GLOB GAME_SOURCES CONFIGURE_DEPENDS src/*.cpp)
    file(GLOB SGG_SOURCES CONFIGURE_DEPENDS third_party/sgg/*.cpp)
    add_executable(game_web ${GAME_SOURCES} ${SGG_SOURCES})

    # <sgg/...> includes resolve to third_party/sgg, <glm/...> to third_party/glm.
    target_include_directories(game_web PRIVATE third_party src)

    # Port flags are needed both when compiling (headers) and when linking.
    target_compile_options(game_web PRIVATE
        "-O3"
        "-flto"
        "-sUSE_SDL=2"
        "-sUSE_SDL_MIXER=2"
        "-sUSE_FREETYPE=1"
    )
    target_link_options(game_web PRIVATE
        "-O3"
        "-flto"
        "--closure=1"
        "-sUSE_SDL=2"
        "-sUSE_SDL_MIXER=2"
        "-sUSE_FREETYPE=1"
        "-sMAX_WEBGL_VERSION=2"
        "-sMIN_WEBGL_VERSION=2"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sENVIRONMENT=web"
        "--preload-file=${CMAKE_SOURCE_DIR}/assets@/assets"
        # Desktop WAVs are not shipped: the web build plays the converted OGGs.
        "--exclude-file=*.wav"
        # Level/end music streams on first use instead of blocking page load.
        # (Patterns match against absolute source paths, hence the leading *.)
        "--exclude-file=*castle_music.*"
        "--exclude-file=*ending_theme.*"
        # Persist the preloaded bundle in IndexedDB so repeat visits load
        # without re-downloading.
        "--use-preload-cache"
        "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
    )

    # Deferred music is served as plain files next to the page; AudioManager
    # fetches each track into MEMFS on first playMusic call.
    add_custom_command(TARGET game_web POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/web/assets
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/assets/castle_music.ogg
            ${CMAKE_SOURCE_DIR}/assets/ending_theme.ogg
            ${CMAKE_SOURCE_DIR}/web/assets/
    )

    set_target_properties(game_web PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/web
        OUTPUT_NAME index
    )
    return()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_COMPILER "/Library/Developer/CommandLineTools/usr/bin/cc")
    set(CMAKE_CXX_COMPILER "/Library/Developer/CommandLineTools/usr/bin/c++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wl,-ld_classic")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
add_executable(game ${SOURCE_FILES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/lib/linux/include)
    target_link_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/lib/linux/lib)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)
    find_library(SDL2_LIBRARY NAME SDL2)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    target_include_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/lib/macosx/include)
    target_include_directories(game PRIVATE ${OPENGL_INCLUDE_DIR})
    target_include_directories(game PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_directories(game PRIVATE ${CMAKE_SOURCE_DIR}/lib/macosx/lib)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(game sgg GL GLEW SDL2 SDL2_mixer freetype)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(game sgg OpenGL::GL GLEW::GLEW SDL2::SDL2 /opt/homebrew/lib/libSDL2_mixer.dylib /opt/homebrew/lib/libfreetype.dylib)
endif()

set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
