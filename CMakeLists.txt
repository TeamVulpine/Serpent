cmake_minimum_required(VERSION 3.10)
project(Serpent LANGUAGES CXX)

option(SERPENT_BUILD_SHARED "Build Serpent as a shared library" OFF)

file(GLOB_RECURSE SERPENT_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
)

if(SERPENT_BUILD_SHARED)
    add_library(serpent SHARED ${SERPENT_SOURCES})
    target_compile_definitions(serpent PUBLIC SERPENT_SHARED)
else()
    add_library(serpent STATIC ${SERPENT_SOURCES})
endif()

target_include_directories(serpent PUBLIC include)
target_compile_features(serpent PUBLIC cxx_std_23)

file(GLOB_RECURSE SERPENT_TEST_SOURCES
    CONFIGURE_DEPENDS
    test/*.cpp
)

option(SERPENT_BUILD_TESTS "Build Serpent test executable" ON)

if(SERPENT_TEST_SOURCES AND SERPENT_BUILD_TESTS)
    add_executable(serpent_tests ${SERPENT_TEST_SOURCES})
    target_link_libraries(serpent_tests PRIVATE serpent)
    target_compile_features(serpent_tests PRIVATE cxx_std_23)
endif()
