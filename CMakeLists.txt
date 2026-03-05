cmake_minimum_required(VERSION 3.16)
project(vivid_drums)

set(VIVID_DRUM_OPS
  drum_kick
  drum_snare
  drum_hihat
  drum_clap
  drum_cymbal
)

foreach(name IN LISTS VIVID_DRUM_OPS)
  add_library(${name} MODULE src/${name}.cpp)
  target_include_directories(${name} PRIVATE ${VIVID_SRC_DIR}/src)
  target_compile_features(${name} PRIVATE cxx_std_17)
  set_target_properties(${name} PROPERTIES
    PREFIX ""
    SUFFIX "${VIVID_PLUGIN_SUFFIX}"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endforeach()

add_custom_target(operators DEPENDS ${VIVID_DRUM_OPS})

add_executable(test_package_manifest tests/test_package_manifest.cpp)
target_compile_features(test_package_manifest PRIVATE cxx_std_17)
