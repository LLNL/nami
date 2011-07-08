set(NAMI_SOURCES
	cdf97.cpp 
	wt_1d.cpp 
	wt_2d.cpp 
	wt_lift.cpp 
	wt_direct.cpp 
	wt_1d_lift.cpp 
	wt_1d_direct.cpp 
	wt_utils.cpp 
	io_utils.cpp
	matrix_utils.cpp
	filter_bank.cpp
	ezw.cpp
	ezw_encoder.cpp
	ezw_decoder.cpp
	obitstream.cpp
	ibitstream.cpp
	vector_obitstream.cpp
	vector_ibitstream.cpp
	timing.cpp
	Timer.cpp
	rle.cpp
	huffman.cpp)

if (NAMI_HAVE_MPI)
  list(APPEND NAMI_SOURCES
    par_wt.cpp	
    par_ezw_encoder.cpp)
endif()

add_library(nami ${NAMI_SOURCES})
if (NAMI_HAVE_MPI)
  target_link_libraries(nami ${MPI_LIBRARIES})
endif()
install(TARGETS nami DESTINATION lib EXPORT nami-libs)

include_directories(
  ${Boost_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}
  ${MPI_INCLUDE_PATH})

set(NAMI_HEADERS 
	cdf97.h
  io_utils.h
	ezw.h
	ezw_encoder.h
	ezw_decoder.h
	filter_bank.h
	ibitstream.h
	obitstream.h
	vector_ibitstream.h
	vector_obitstream.h
	nami_matrix.h
	wt_1d.h
	wt_2d.h
	wt_direct.h
	wt_1d_lift.h
	wt_1d_direct.h
	wt_lift.h)

install(FILES ${NAMI_HEADERS} DESTINATION include)
