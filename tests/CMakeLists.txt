# Aggregate source files
AUX_SOURCE_DIRECTORY(src SOURCE_FILES)

# Add google-tests executable
add_executable(tests ${SOURCE_FILES})

# Link (and add dependencies for) rtclm framework libraries
target_link_libraries(tests 
  icom
  pthread
  gtest)

# Includes
target_include_directories(tests PUBLIC
  "${PROJECT_SOURCE_DIR}/tests/inc"
  "${PROJECT_SOURCE_DIR}/icom/inc"
)
