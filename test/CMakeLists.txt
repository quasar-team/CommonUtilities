cmake_minimum_required(VERSION 2.8)
find_package(Threads REQUIRED)

file(GLOB COMMON_UTILITIES_TEST_SRCS src/*.cpp include/*.h)
include_directories(include)

add_executable(CommonUtilitiesTest 
	$<TARGET_OBJECTS:CommonUtilities>
	$<TARGET_OBJECTS:LogIt>
	${COMMON_UTILITIES_TEST_SRCS}
)

SET (CMAKE_EXE_LINKER_FLAGS -v)

target_link_libraries (CommonUtilitiesTest
	gtest_main
	${CMAKE_THREAD_LIBS_INIT}
	${BOOST_LIBS}
	${OPCUA_TOOLKIT_LIBS}
	ws2_32.lib
)

add_dependencies (CommonUtilitiesTest gtest_main CommonUtilities LogIt)
