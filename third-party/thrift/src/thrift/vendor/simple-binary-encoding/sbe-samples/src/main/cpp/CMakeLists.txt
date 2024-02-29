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

set(GENERATED_CODECS
    ${CXX_CODEC_TARGET_DIR}/baseline)

set(EXAMPLES_SCHEMA ${CODEC_EXAMPLES_SCHEMA_DIR}/example-schema.xml)

add_custom_command(
    OUTPUT ${GENERATED_CODECS}
    DEPENDS ${EXAMPLES_SCHEMA} sbe-jar ${SBE_JAR}
    COMMAND ${Java_JAVA_EXECUTABLE} -Dsbe.output.dir=${CXX_CODEC_TARGET_DIR} -Dsbe.generate.ir=true -Dsbe.target.language=cpp -Dsbe.xinclude.aware=true -jar ${SBE_JAR} ${EXAMPLES_SCHEMA}
)
add_custom_target(examples_codecs DEPENDS ${GENERATED_CODECS})

add_executable(GeneratedStubExample GeneratedStubExample.cpp)
target_include_directories(GeneratedStubExample PRIVATE ${CXX_CODEC_TARGET_DIR})
target_link_libraries(GeneratedStubExample sbe)
add_executable(OtfExample OtfExample.cpp)
target_include_directories(OtfExample PRIVATE ${CXX_CODEC_TARGET_DIR})
target_link_libraries(OtfExample sbe)

add_dependencies(GeneratedStubExample examples_codecs)
add_dependencies(OtfExample examples_codecs)
