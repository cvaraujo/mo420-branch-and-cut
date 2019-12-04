cmake_minimum_required(VERSION 3.5)
project(MO420_Branch_and_Cut)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIL_STD")
set (CMAKE_CXX_STANDARD 11)

include_directories("/opt/ibm/ILOG/CPLEX_Studio129/cplex/include")
include_directories("/opt/ibm/ILOG/CPLEX_Studio129/concert/include")

link_directories("/opt/ibm/ILOG/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic")
link_directories("/opt/ibm/ILOG/CPLEX_Studio129/concert/lib/x86-64_linux/static_pic")

add_executable(MO420_Branch_and_Cut main.cpp src/graph.cpp inc/graph.h inc/include.h src/model.cpp inc/model.h)

target_link_libraries(MO420_Branch_and_Cut concert ilocplex cplex pthread dl)
#lconcert -lilocplex -lcplex -lpthread -ldl