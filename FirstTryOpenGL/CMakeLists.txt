cmake_minimum_required(VERSION 3.16)
project(SPH_FluidSimulation CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Try finding system GLFW3; fallback to FetchContent if not found locally
find_package(glfw3 3.3 QUIET)

if(NOT glfw3_FOUND)
    message(STATUS "GLFW3 not found locally. Fetching GLFW 3.4 automatically via FetchContent...")
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        3.4
    )
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
endif()

# Project Sources and Headers
set(SOURCES
    main.cpp
    SphKernels.cpp
    SpatialGrid.cpp
    FluidSimulation.cpp
    Renderer.cpp
)

set(HEADERS
    Config.hpp
    SphKernels.hpp
    SpatialGrid.hpp
    FluidSimulation.hpp
    Renderer.hpp
)

# Add Executable Target
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include Directory
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link Dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    OpenMP::OpenMP_CXX
)

if(TARGET glfw)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
endif()
