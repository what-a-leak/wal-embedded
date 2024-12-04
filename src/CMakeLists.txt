# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

# Glob all source files in the src directory
FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# Register the component and add esp-dsp as a dependency
idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS "." # Include current directory for headers
                       REQUIRES esp-dsp) # Add esp-dsp as a required component
