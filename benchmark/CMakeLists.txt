# Aggregate source files
AUX_SOURCE_DIRECTORY(src SOURCE_FILES)

# Add benchmark executable
add_executable(benchmark ${SOURCE_FILES})

# Link (and add dependencies for) rtclm framework libraries
target_link_libraries(benchmark 
  icom
  pthread)

# Includes
target_include_directories(benchmark PUBLIC
  "${PROJECT_SOURCE_DIR}/benchmark/inc"
  "${PROJECT_SOURCE_DIR}/icom/inc"
)
