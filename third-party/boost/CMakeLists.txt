cmake_minimum_required(VERSION 2.8.0)                                           
include(ExternalProject)                                                        
message(STATUS "Boost CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")
ExternalProject_Add(                                                            
  boostBuild
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/boost/                                 
  CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/boost/bootstrap.sh --without-libraries=python --prefix=${CMAKE_CURRENT_SOURCE_DIR}/build
  BUILD_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/boost/b2 link=static variant=release threading=multi runtime-link=static cxxflags=-std=gnu++14 --prefix=${CMAKE_CURRENT_SOURCE_DIR}/build install
  PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/build                                                                  
  BUILD_IN_SOURCE true                                                          
  INSTALL_COMMAND ""                                                            
)                                                                               
