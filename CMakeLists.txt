cmake_minimum_required(VERSION 3.1)

if (NOT DEFINED CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER g++-13)
else()
  message(STATUS "using ${CMAKE_CXX_COMPILER}")
endif()

project(stlext LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-O3")
if (CMAKE_COMPILER_IS_GNUCXX)
  message(STATUS "detected GNU compiler")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  add_definitions(-D_GLIBCXX_PARALLEL)
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Intel")
elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  message(STATUS "detected clang compiler")
  if (UNIX AND NOT APPLE)
    # this doesn't seem to be supported on mac
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
  endif()
  add_definitions(-D_GLIBCXX_PARALLEL)
endif()

set(external_libraries)
# TBB
find_package(TBB)
if(TBB_FOUND)
        message(STATUS "found TBB")
        set(external_libraries ${external_libraries} TBB::tbb)
        add_definitions(-DHAVE_TBB=1)
else()
        message(WARNING "did not find TBB")
        add_definitions(-DHAVE_TBB=0)
endif()

find_package(OpenMP)
if (OpenMP_CXX_FOUND)
        message(STATUS "found OpenMP")
        set(external_libraries ${external_libraries} OpenMP::OpenMP_CXX)
else()
        message(STATUS "did not find OpenMP")
endif()
# intentionally turning off openmp for testing
add_definitions(-DHAVE_OMP=0)

# sort test
add_executable(test_sort test_sort.cpp)
set_property(TARGET test_sort PROPERTY CXX_STANDARD 17)
target_link_libraries(test_sort ${external_libraries})

# loop test
add_executable(test_loop test_loop.cpp)
set_property(TARGET test_sort PROPERTY CXX_STANDARD 17)
target_link_libraries(test_sort ${external_libraries})

# hash test
add_executable(test_hash test_hash.cpp)
set_property(TARGET test_hash PROPERTY CXX_STANDARD 17)
target_link_libraries(test_hash ${external_libraries})

add_custom_target(testall COMMAND ./test_hash && ./test_loop && ./test_sort 100000000 1 && ./test_sort 100000000 2)
