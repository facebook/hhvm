/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _OTF_HEADERDECODER_H
#define _OTF_HEADERDECODER_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

#include "Token.h"

namespace sbe {
namespace otf {

class OtfHeaderDecoder
{
public:
    explicit OtfHeaderDecoder(const std::shared_ptr<std::vector<Token>> &tokens)
    {
        m_encodedLength = tokens->at(0).encodedLength();

        Token *blockLengthToken = nullptr;
        Token *templateIdToken = nullptr;
        Token *schemaIdToken = nullptr;
        Token *versionToken = nullptr;

        std::for_each(tokens->begin(), tokens->end(), [&](Token &token)
        {
            const std::string &name = token.name();

            if (name == "blockLength")
            {
                blockLengthToken = &token;
            }
            else if (name == "templateId")
            {
                templateIdToken = &token;
            }
            else if (name == "schemaId")
            {
                schemaIdToken = &token;
            }
            else if (name == "version")
            {
                versionToken = &token;
            }
        });

        if (nullptr == blockLengthToken)
        {
            throw std::runtime_error("blockLength token not found");
        }

        m_blockLengthOffset = blockLengthToken->offset();
        m_blockLengthType = blockLengthToken->encoding().primitiveType();
        m_blockLengthByteOrder = blockLengthToken->encoding().byteOrder();

        if (nullptr == templateIdToken)
        {
            throw std::runtime_error("templateId token not found");
        }

        m_templateIdOffset = templateIdToken->offset();
        m_templateIdType = templateIdToken->encoding().primitiveType();
        m_templateIdByteOrder = templateIdToken->encoding().byteOrder();

        if (nullptr == schemaIdToken)
        {
            throw std::runtime_error("schemaId token not found");
        }

        m_schemaIdOffset = schemaIdToken->offset();
        m_schemaIdType = schemaIdToken->encoding().primitiveType();
        m_schemaIdByteOrder = schemaIdToken->encoding().byteOrder();

        if (nullptr == versionToken)
        {
            throw std::runtime_error("version token not found");
        }

        m_schemaVersionOffset = versionToken->offset();
        m_schemaVersionType = versionToken->encoding().primitiveType();
        m_schemaVersionByteOrder = versionToken->encoding().byteOrder();
    }

    inline std::uint32_t encodedLength() const
    {
        return static_cast<std::uint32_t>(m_encodedLength);
    }

    /*
     * All elements must be unsigned integers according to Specification
     */

    std::uint64_t getTemplateId(const char *headerBuffer) const
    {
        return Encoding::getUInt(m_templateIdType, m_templateIdByteOrder, headerBuffer + m_templateIdOffset);
    }

    std::uint64_t getSchemaId(const char *headerBuffer) const
    {
        return Encoding::getUInt(m_schemaIdType, m_schemaIdByteOrder, headerBuffer + m_schemaIdOffset);
    }

    std::uint64_t getSchemaVersion(const char *headerBuffer) const
    {
        return Encoding::getUInt(m_schemaVersionType, m_schemaVersionByteOrder, headerBuffer + m_schemaVersionOffset);
    }

    std::uint64_t getBlockLength(const char *headerBuffer) const
    {
        return Encoding::getUInt(m_blockLengthType, m_blockLengthByteOrder, headerBuffer + m_blockLengthOffset);
    }

private:
    std::int32_t m_encodedLength;
    std::int32_t m_blockLengthOffset;
    std::int32_t m_templateIdOffset;
    std::int32_t m_schemaIdOffset;
    std::int32_t m_schemaVersionOffset;
    PrimitiveType m_blockLengthType;
    PrimitiveType m_templateIdType;
    PrimitiveType m_schemaIdType;
    PrimitiveType m_schemaVersionType;
    ByteOrder m_blockLengthByteOrder;
    ByteOrder m_templateIdByteOrder;
    ByteOrder m_schemaIdByteOrder;
    ByteOrder m_schemaVersionByteOrder;
};

}}

#endif
