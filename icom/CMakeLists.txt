# Aggregate source files
AUX_SOURCE_DIRECTORY(src SOURCE_FILES)

# Compile static/shared libraries using given sources
add_library(icom OBJECT ${SOURCE_FILES})
add_library(icom_static STATIC $<TARGET_OBJECTS:icom>)
add_library(icom_shared SHARED $<TARGET_OBJECTS:icom>)

# Properties
set_property(TARGET icom PROPERTY POSITION_INDEPENDENT_CODE 1)
set_property(TARGET icom_static PROPERTY OUTPUT_NAME icom)
set_property(TARGET icom_shared PROPERTY OUTPUT_NAME icom)

# Installation rules
install(TARGETS icom_static DESTINATION lib/edi/)
install(TARGETS icom_shared DESTINATION lib/edi/)
install(
  FILES 
    inc/icom.h
    inc/icom_type.h
    inc/icom_flags.h
    inc/icom_status.h
    inc/icom_macro.h
  DESTINATION
    include/edi)


# Include routines for generating packageConfig file
include(CMakePackageConfigHelpers)

# Generate compageConfig.cmake file, later used by the find_package() routine
configure_package_config_file(
  CMakeConfig.cmake.in
  "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/icomConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/${PROJECT_NAME}"
)

# Install generated compageConfig.cmake file
install(
  FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/icomConfig.cmake"
  DESTINATION
    lib/cmake/icom
  COMPONENT
    dev)


# Add library
target_link_libraries(icom
  pthread)


# Includes
target_include_directories(icom PUBLIC
  "${PROJECT_SOURCE_DIR}/icom/inc"
)


# Build configurations
set(CMAKE_CONFIGURATION_TYPES Release Debug)

# Default build configuration
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

