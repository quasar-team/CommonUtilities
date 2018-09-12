include_directories(
	include	
)

file(GLOB COMMON_UTILITIES_SRCS src/*.cpp include/*.h)

add_library (CommonUtilities OBJECT ${COMMON_UTILITIES_SRCS})

enable_testing()
add_subdirectory( test )