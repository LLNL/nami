# Simple function to add a Nami test executable 
function(add_test test_name src_name)
  add_executable(${test_name} ${src_name})
  target_link_libraries(${test_name} nami)
endfunction()

# Function to add MPI tests, but only if MPI is enabled.
function(add_mpi_test test_name src_name)
  if (NAMI_HAVE_MPI)
    add_test(${test_name} ${src_name})
    target_link_libraries(${test_name} ${MPI_LIBRARIES})
  endif()
endfunction()

add_test(compress-matfile    compress_matfile.C)
add_test(vary-passes         vary_passes.C)
add_test(insert-bits-test    insert_bits_test.C)
add_test(ezwtest             ezwtest.C)
add_test(seqtest             seqtest.C)
add_test(vltest              vltest.C)
add_test(generictest         generictest.C)

add_mpi_test(parezwtest      parezwtest.C)
add_mpi_test(parspeedbench   parspeedbench.C)
add_mpi_test(partest         partest.C)

include_directories(
  ${PROJECT_BINARY_DIR}
  ${Boost_INCLUDE_DIR}
  ${MPI_INCLUDE_PATH})
