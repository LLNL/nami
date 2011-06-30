###############################################################
# Nami                                         /|
# Parallel Wavelet Compression Library      __/  \  .__
# by Todd Gamblin, tgamblin@llnl.gov              \/
###############################################################
project(nami)
cmake_minimum_required(VERSION 2.8)

# Version information comes from here.  Incrememnt these when new stuff happens.
set(NAMI_MAJOR_VERSION 1)
set(NAMI_MINOR_VERSION 0)
set(NAMI_PATCH_VERSION 0)
set(NAMI_VERSION "${NAMI_MAJOR_VERSION}.${NAMI_MINOR_VERSION}.${NAMI_PATCH_VERSION}")

# Build shared libraries by default.
get_property(DEFAULT_SHARED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
option(BUILD_SHARED_LIBS "Build shared libraries?" ${DEFAULT_SHARED})

# Whether to build for regular MPI apps or with PMPI bindings (for tools).
option(NAMI_USE_PMPI "Build with PMPI bindings?" FALSE)

# Muster requires boost for matrices and random number generators.
find_package(Boost REQUIRED)

# Find the MPI library and set some definitions
# This line ensures that we skpi C++ headers altogether, avoiding unnecessary symbols in the .o files.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX")
find_package(MPI QUIET)
if (MPI_FOUND)
  set(NAMI_HAVE_MPI TRUE)
endif()

# Check for various timing functions, so we can support highest-resolution timers available.
include(CheckFunctionExists)

set(CMAKE_REQUIRED_INCLUDES  time.h sys/time.h)
set(CMAKE_REQUIRED_LIBRARIES rt)
check_function_exists(clock_gettime NAMI_HAVE_CLOCK_GETTIME)

unset(CMAKE_REQUIRED_LIBRARIES)
check_function_exists(gettimeofday NAMI_HAVE_GETTIMEOFDAY)

# Configure a CMake file and set it to be installed.  This file tells other projects
# how to use Adept Utils.
foreach (nami)
  get_target_property(${lib}_LIBRARY_PATH ${lib} LOCATION)
  get_filename_component(${lib}_LIBRARY ${${lib}_LIBRARY_PATH} NAME)
endforeach()

# Configure a header file with all the variables we found.
configure_file(
  ${PROJECT_SOURCE_DIR}/nami-config.h.in
  ${PROJECT_BINARY_DIR}/nami-config.h)
install(FILES ${PROJECT_BINARY_DIR}/nami-config.h DESTINATION include)


# Make nami findable
foreach (lib nami)
  get_target_property(${lib}_LIBRARY_PATH ${lib} LOCATION)
  get_filename_component(${lib}_LIBRARY ${${lib}_LIBRARY_PATH} NAME)
endforeach()
configure_file(
  ${PROJECT_SOURCE_DIR}/nami-config.cmake.in
  ${PROJECT_BINARY_DIR}/nami-config.cmake
  @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/nami-config.cmake DESTINATION lib/cmake/nami)
install(EXPORT nami-libs                             DESTINATION lib/cmake/nami)


# Configure subdirectories here, after everything else, so that they 
# inherit variables set in the global scope.
add_subdirectory(src)
add_subdirectory(tests)


