cmake_minimum_required(VERSION 3.20)
project(pinball3d LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---------------------------------------------------------------------------
# Dependencies (fetched from source — no manual setup required)
# ---------------------------------------------------------------------------
FetchContent_Declare(SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        release-2.28.5
  GIT_SHALLOW    TRUE)
set(SDL_TEST            OFF CACHE BOOL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "" FORCE)
set(SDL_SHARED          ON  CACHE BOOL "" FORCE)
set(SDL_STATIC          OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        1.0.1
  GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.3
  GIT_SHALLOW    TRUE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(json)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PINBALL_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(pinball ${PINBALL_SOURCES})
target_include_directories(pinball PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pinball PRIVATE
  SDL2::SDL2 SDL2::SDL2main
  glm::glm
  nlohmann_json::nlohmann_json
  opengl32)

if (MSVC)
  target_compile_options(pinball PRIVATE /W4 /permissive- /wd4201)
  target_compile_definitions(pinball PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# Copy the SDL2 runtime DLL next to the executable.
add_custom_command(TARGET pinball POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:SDL2::SDL2> $<TARGET_FILE_DIR:pinball>)

# Copy assets (shaders, levels) next to the executable.
add_custom_command(TARGET pinball POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:pinball>/assets)

# ---------------------------------------------------------------------------
# Headless self-test (domain + application + persistence; no SDL/OpenGL)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE PINBALL_CORE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/src/domain/*.cpp
  ${CMAKE_SOURCE_DIR}/src/application/*.cpp
  ${CMAKE_SOURCE_DIR}/src/infrastructure/persistence/*.cpp)

add_executable(pinball_selftest ${CMAKE_SOURCE_DIR}/tests/selftest.cpp ${PINBALL_CORE_SOURCES})
target_include_directories(pinball_selftest PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pinball_selftest PRIVATE glm::glm nlohmann_json::nlohmann_json)
if (MSVC)
  target_compile_options(pinball_selftest PRIVATE /permissive- /wd4201)
  target_compile_definitions(pinball_selftest PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()
