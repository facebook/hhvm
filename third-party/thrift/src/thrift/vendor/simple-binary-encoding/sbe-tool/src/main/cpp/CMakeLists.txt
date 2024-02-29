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

set(CODECS
    uk_co_real_logic_sbe_ir_generated/ByteOrderCodec.h
    uk_co_real_logic_sbe_ir_generated/FrameCodec.h
    uk_co_real_logic_sbe_ir_generated/MessageHeader.h
    uk_co_real_logic_sbe_ir_generated/PresenceCodec.h
    uk_co_real_logic_sbe_ir_generated/PrimitiveTypeCodec.h
    uk_co_real_logic_sbe_ir_generated/SignalCodec.h
    uk_co_real_logic_sbe_ir_generated/TokenCodec.h
    uk_co_real_logic_sbe_ir_generated/VarDataEncoding.h)

find_package(Java REQUIRED)

add_custom_command(OUTPUT ${CODECS}
    COMMAND
        ${Java_JAVA_EXECUTABLE}
            "-Dsbe.output.dir=${CMAKE_CURRENT_SOURCE_DIR}"
            "-Dsbe.target.language=CPP"
            "-jar" "${SBE_JAR}"
            "../resources/sbe-ir.xml"
            DEPENDS ${SBE_JAR} sbe-jar "../resources/sbe-ir.xml"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generate IR codecs"
)

add_custom_target(
    ir_codecs
    DEPENDS ${CODECS}
)

set(HEADERS
    otf/IrDecoder.h
    otf/Token.h
    otf/Encoding.h
    otf/OtfMessageDecoder.h
    otf/OtfHeaderDecoder.h)

add_library(sbe INTERFACE)
target_include_directories(sbe INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_dependencies(sbe ir_codecs)
