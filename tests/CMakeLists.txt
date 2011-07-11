# Simple function to add a Nami test executable 
function(add_test test_name src_name)
  add_executable(${test_name} ${src_name})
  target_link_libraries(${test_name} nami adept_utils adept_timing)
endfunction()

# Function to add MPI tests, but only if MPI is enabled.
function(add_mpi_test test_name src_name)
  if (NAMI_HAVE_MPI)
    add_test(${test_name} ${src_name})
    target_link_libraries(${test_name} ${MPI_LIBRARIES} adept_utils adept_timing)
  endif()
endfunction()

add_test(compress-matfile    compress_matfile.cpp)
add_test(vary-passes         vary_passes.cpp)
add_test(insert-bits-test    insert_bits_test.cpp)
add_test(ezwtest             ezwtest.cpp)
add_test(seqtest             seqtest.cpp)
add_test(vltest              vltest.cpp)
add_test(generictest         generictest.cpp)
add_test(two-test            two_test.cpp)

add_mpi_test(parezwtest      parezwtest.cpp)
add_mpi_test(parspeedbench   parspeedbench.cpp)
add_mpi_test(partest         partest.cpp)

include_directories(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/src
  ${adept_utils_INCLUDE_PATH}
  ${Boost_INCLUDE_DIR}
  ${MPI_INCLUDE_PATH})
