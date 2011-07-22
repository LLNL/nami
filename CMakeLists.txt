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

# RPATH setup.  Default is to rpath everything.  Set the option to
# false if you don't want this behavior.
option(CMAKE_INSTALL_RPATH_USE_LINK_PATH "Add rpath for all dependencies." TRUE)

# Whether to build for regular MPI apps or with PMPI bindings (for tools).
option(NAMI_USE_PMPI "Build with PMPI bindings?" FALSE)

function(notify_package name)
  if (${name}_FOUND)
    message(STATUS "Found ${name} in ${${name}_DIR}.")
  else()
    message(STATUS "${name} package not found.")
  endif()
endfunction()

find_package(adept_utils REQUIRED)
notify_package(adept_utils)

# nami requires boost for matrices and random number generators.
find_package(Boost NO_MODULE QUIET)
if(NOT Boost_FOUND)
  find_package(Boost)
endif()

# Find the MPI library and set some definitions
# This line ensures that we skpi C++ headers altogether, avoiding unnecessary symbols in the .o files.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX")
find_package(MPI QUIET)
if (MPI_FOUND)
  set(NAMI_HAVE_MPI TRUE)
endif()

# Configure a header file with all the variables we found.
configure_file(
  ${PROJECT_SOURCE_DIR}/nami-config.h.in
  ${PROJECT_BINARY_DIR}/nami-config.h)
install(FILES ${PROJECT_BINARY_DIR}/nami-config.h DESTINATION include)

# Make nami findable
configure_file(
  ${PROJECT_SOURCE_DIR}/nami-config.cmake.in
  ${PROJECT_BINARY_DIR}/nami-config.cmake
  @ONLY)

install(FILES ${PROJECT_BINARY_DIR}/nami-config.cmake DESTINATION share/cmake/nami)
install(EXPORT nami-libs                              DESTINATION share/cmake/nami)

# Configure subdirectories here, after everything else, so that they 
# inherit variables set in the global scope.
add_subdirectory(src)
add_subdirectory(tests)


