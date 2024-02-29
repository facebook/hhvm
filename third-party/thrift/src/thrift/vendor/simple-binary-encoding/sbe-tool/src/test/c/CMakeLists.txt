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

function(sbe_test name)
    add_executable("C${name}" "${name}.cpp")
    target_include_directories("C${name}"
        PRIVATE ${GTEST_SOURCE_DIR}/googletest/include
        PRIVATE ${C_CODEC_TARGET_DIR}
    )
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:AppleClang>:-Werror>)
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Werror>)
    target_compile_options("C${name}" PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Werror -Wno-maybe-uninitialized>)
    target_link_libraries("C${name}" ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
    add_test(NAME C${name} COMMAND C${name} WORKING_DIRECTORY ${C_CODEC_TARGET_DIR})
    add_dependencies(C${name} gtest)
    if (${ARGC} GREATER 1)
        add_dependencies(C${name} ${ARGV1})
    endif ()
endfunction()

find_package(Java REQUIRED)

set(CODE_GENERATION_SCHEMA ${CODEC_SCHEMA_DIR}/code-generation-schema.xml)
set(GROUP_WITH_DATA_SCHEMA ${CODEC_SCHEMA_DIR}/group-with-data-schema.xml)
set(ISSUE889_SCHEMA ${CODEC_SCHEMA_DIR}/issue889.xml)

set(GENERATED_CODECS
    ${C_CODEC_TARGET_DIR}
)

add_custom_command(
    OUTPUT ${GENERATED_CODECS}
    DEPENDS sbe-jar ${SBE_JAR} ${CODE_GENERATION_SCHEMA} ${GROUP_WITH_DATA_SCHEMA} ${ISSUE889_SCHEMA}
    COMMAND
        ${Java_JAVA_EXECUTABLE}
            -Dsbe.output.dir=${C_CODEC_TARGET_DIR}
            -Dsbe.target.language="C"
            -jar ${SBE_JAR}
            ${CODE_GENERATION_SCHEMA}
            ${GROUP_WITH_DATA_SCHEMA}
            ${ISSUE889_SCHEMA}
)

add_custom_target(c_codecs DEPENDS ${GENERATED_CODECS})

# codec tests
sbe_test(BoundsCheckTest c_codecs)
sbe_test(CodeGenTest c_codecs)
sbe_test(GroupWithDataTest c_codecs)
sbe_test(Issue889Test c_codecs)

# Compile a dummy C source to test C compliance of generated headers.
add_executable(CComplianceTest CComplianceTest.c)

target_include_directories(CComplianceTest
    PRIVATE ${C_CODEC_TARGET_DIR}
)

add_dependencies(CComplianceTest c_codecs)
