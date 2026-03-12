cmake_minimum_required(VERSION 3.28)
project(dungeoncrawler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

function(setup_target_warnings_and_sanitizers target)
  target_compile_options(${target} PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wshadow -fdiagnostics-color=always>
    $<$<C_COMPILER_ID:MSVC>:/W4>
  )
  set(SANITIZER_EXPR $<$<AND:$<C_COMPILER_ID:GNU,Clang>,$<NOT:$<PLATFORM_ID:Windows>>,$<CONFIG:Debug>>:-fsanitize=address,undefined,leak>)
  target_compile_options(${target} PRIVATE ${SANITIZER_EXPR})
  target_link_options(${target} PRIVATE ${SANITIZER_EXPR})
endfunction()

# fetch an compile raylib
include(FetchContent)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  raygui
  GIT_REPOSITORY https://github.com/raysan5/raygui.git
  GIT_TAG 4.0
)

FetchContent_MakeAvailable(raygui)

add_library(raygui INTERFACE)
target_include_directories(raygui INTERFACE ${raygui_SOURCE_DIR}/src)

# find resources for embedding, gen c src with xxd, and configure API .in templates.
# provides: GEN_DIR, GEN_C_FILES
include(resources.cmake)

add_executable(dungeoncrawler
  src/main.c
  src/map.c
  src/types.c
  src/raygui_impl.c
  src/player.c
  ${GEN_C_FILES}
)

target_include_directories(dungeoncrawler PRIVATE ${GEN_DIR})
target_link_libraries(dungeoncrawler PRIVATE raylib raygui)
setup_target_warnings_and_sanitizers(dungeoncrawler)

# copy compile_commands.json from the build dir to the source dir
add_custom_target(copy_compile_commands ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  COMMENT "Copying compile_commands.json to source root"
)
