cmake_minimum_required(VERSION 3.14)
project(netcdf_server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined,address")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined,address")

include_directories(/app/lib/include /app/include)
link_directories(/app/lib)

# Output executable to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY /app/bin)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "/app/lib")

# Add executable from source files
add_executable(main
    src/main.cpp
    src/netcdf_server.cpp
)

# Link required libraries
target_link_libraries(main PUBLIC tiff jpeg png matplot netcdf_c++4 netcdf z pthread m)