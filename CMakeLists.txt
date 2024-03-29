cmake_minimum_required(VERSION 3.12)

project(MVECS C CXX)


set(CMAKE_CXX_FLAGS "-std=c++17 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++17 -g3 -O0 -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++17 -O2 -s -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-std=c++17 -g3 -Og -pg")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-std=c++17 -Os -s -DNDEBUG -march=native")

include_directories(
   "./include"
)

#file(GLOB_RECURSE HDRS include/*.h*)
file(GLOB_RECURSE SRCS src/*.c*)

file(GLOB_RECURSE HDRS include/*.h*)
file(GLOB_RECURSE SRCS src/*.c*)

add_library(
   mvecs STATIC
   ${SRCS}
)

target_link_libraries(mvecs
   pthread
)

install(TARGETS mvecs ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)
install(FILES ${HDRS} DESTINATION include/MVECS)

