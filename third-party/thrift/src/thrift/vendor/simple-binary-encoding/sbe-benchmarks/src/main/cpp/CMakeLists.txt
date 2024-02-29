#
# Copyright 2013-2024 Real Logic Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

find_package(Java REQUIRED)

CHECK_LIBRARY_EXISTS(rt clock_gettime "time.h" HAVE_CLOCK_GETTIME_RT)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Build type(${CMAKE_BUILD_TYPE}): perf tests turning OFF bounds checks")
    add_definitions(-DNDEBUG)
    add_definitions(-DSBE_NO_BOUNDS_CHECK)
else ()
    message("Build type(${CMAKE_BUILD_TYPE}): perf tests turning ON bounds checks")
endif ()

set(SRCS_BENCHLET_MAIN benchlet-main.cpp)

set(GENERATED_CODECS
    ${CXX_CODEC_TARGET_DIR}/uk_co_real_logic_sbe_examples_car
    ${CXX_CODEC_TARGET_DIR}/uk_co_real_logic_sbe_samples_fix
)

set(SBE_CAR_SCHEMA ${CODEC_PERF_SCHEMA_DIR}/car.xml)
set(SBE_MD_SCHEMA ${CODEC_PERF_SCHEMA_DIR}/fix-message-samples.xml)

add_custom_command(
    OUTPUT ${GENERATED_CODECS}
    DEPENDS ${SBE_CAR_SCHEMA} ${SBE_MD_SCHEMA} sbe-jar ${SBE_JAR}
    COMMAND ${Java_JAVA_EXECUTABLE} -Dsbe.output.dir=${CXX_CODEC_TARGET_DIR} -Dsbe.target.language="cpp" -jar ${SBE_JAR} ${SBE_CAR_SCHEMA} ${SBE_MD_SCHEMA}
)
add_custom_target(perf_codecs DEPENDS ${GENERATED_CODECS})

add_executable(benchlet-sbe-car-runner ${SRCS_BENCHLET_MAIN} CarBench.cpp)
target_include_directories(benchlet-sbe-car-runner PRIVATE ${CXX_CODEC_TARGET_DIR})
target_link_libraries(benchlet-sbe-car-runner sbe)
add_executable(benchlet-sbe-md-runner ${SRCS_BENCHLET_MAIN} MarketDataBench.cpp)
target_include_directories(benchlet-sbe-md-runner PRIVATE ${CXX_CODEC_TARGET_DIR})
target_link_libraries(benchlet-sbe-md-runner sbe)
add_dependencies(benchlet-sbe-md-runner perf_codecs)
add_dependencies(benchlet-sbe-car-runner perf_codecs)

if (HAVE_CLOCK_GETTIME_RT)
    target_link_libraries(benchlet-sbe-md-runner rt)
    target_link_libraries(benchlet-sbe-car-runner rt)
endif (HAVE_CLOCK_GETTIME_RT)
