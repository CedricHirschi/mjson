cmake_minimum_required(VERSION 3.14)

project(mjson 
    VERSION 1.2.7
    DESCRIPTION "JSON parser, emitter, and JSON-RPC engine"
    LANGUAGES C
)

# Options for enabling/disabling features
option(MJSON_ENABLE_PRINT "Enable printing functionality" ON)
option(MJSON_ENABLE_RPC "Enable RPC functionality" ON)
option(MJSON_BUILD_TESTS "Build tests" OFF)
option(MJSON_BUILD_EXAMPLES "Build examples" OFF)

# Create the library
add_library(mjson STATIC
    src/mjson.c
    src/mjson.h
)

# Add an alias for namespaced usage
add_library(mjson::mjson ALIAS mjson)

# Set C standard
target_compile_features(mjson PUBLIC c_std_99)

# Set include directories
target_include_directories(mjson
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Configure compile definitions based on options
if(NOT MJSON_ENABLE_PRINT)
    target_compile_definitions(mjson PUBLIC MJSON_ENABLE_PRINT=0)
endif()

if(NOT MJSON_ENABLE_RPC)
    target_compile_definitions(mjson PUBLIC MJSON_ENABLE_RPC=0)
endif()

# Install rules
include(GNUInstallDirs)

install(TARGETS mjson
    EXPORT mjsonTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES src/mjson.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Export targets
install(EXPORT mjsonTargets
    FILE mjsonTargets.cmake
    NAMESPACE mjson::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mjson
)

# Create and install package config files
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mjsonConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mjsonConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mjsonConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mjson
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mjsonConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mjsonConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mjson
)

# Build tests if requested
if(MJSON_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

# Build examples if requested
if(MJSON_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
