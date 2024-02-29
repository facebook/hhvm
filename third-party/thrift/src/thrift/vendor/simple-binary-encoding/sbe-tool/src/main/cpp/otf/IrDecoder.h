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
#ifndef _OTF_IRDECODER_H
#define _OTF_IRDECODER_H

#if defined(WIN32) || defined(_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#define fileno _fileno
#define read _read
#define stat _stat64
#else
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#endif /* WIN32 */

#include <memory>
#include <exception>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

#include "uk_co_real_logic_sbe_ir_generated/TokenCodec.h"
#include "uk_co_real_logic_sbe_ir_generated/FrameCodec.h"
#include "Token.h"

using namespace sbe::otf;

namespace sbe { namespace otf {

class IrDecoder
{
public:
    IrDecoder() = default;

    int decode(char *irBuffer, std::uint64_t length)
    {
        m_length = length;
        if (m_length == 0)
        {
            return -1;
        }
        std::unique_ptr<char[]> buffer(new char[m_length]);
        m_buffer = std::move(buffer);
        std::memcpy(m_buffer.get(), irBuffer, m_length);

        return decodeIr();
    }

    int decode(const char *filename)
    {
        long long fileSize = getFileSize(filename);

        if (fileSize < 0)
        {
            return -1;
        }

        m_length = static_cast<std::uint64_t>(fileSize);
        if (m_length == 0)
        {
            return -1;
        }
        std::unique_ptr<char[]> buffer(new char[m_length]);
        m_buffer = std::move(buffer);

        if (readFileIntoBuffer(m_buffer.get(), filename, m_length) < 0)
        {
            return -1;
        }

        return decodeIr();
    }

    std::shared_ptr<std::vector<Token>> header()
    {
        return m_headerTokens;
    }

    std::vector<std::shared_ptr<std::vector<Token>>> messages()
    {
        return m_messages;
    }

    std::shared_ptr<std::vector<Token>> message(int id, int version)
    {
        std::shared_ptr<std::vector<Token>> result;

        std::for_each(m_messages.begin(), m_messages.end(),
            [&](const std::shared_ptr<std::vector<Token>> &tokens)
            {
                Token &token = tokens->at(0);

                if (token.signal() == Signal::BEGIN_MESSAGE &&
                    token.fieldId() == id &&
                    token.tokenVersion() <= version)
                {
                    result = tokens;
                }
            });

        return result;
    }

    std::shared_ptr<std::vector<Token>> message(int id)
    {
        std::shared_ptr<std::vector<Token>> result;

        std::for_each(m_messages.begin(), m_messages.end(),
            [&](const std::shared_ptr<std::vector<Token>> &tokens)
            {
                Token &token = tokens->at(0);

                if (token.signal() == Signal::BEGIN_MESSAGE && token.fieldId() == id)
                {
                    result = tokens;
                }
            });

        return result;
    }

protected:
    // OS specifics
    static long long getFileSize(const char *filename)
    {
        struct stat fileStat{};

        if (::stat(filename, &fileStat) != 0)
        {
            return -1;
        }

        return fileStat.st_size;
    }

    static int readFileIntoBuffer(char *buffer, const char *filename, std::uint64_t length)
    {
        FILE *fptr = ::fopen(filename, "rb");
        std::uint64_t remaining = length;

        if (nullptr == fptr)
        {
            return -1;
        }

        int fd = fileno(fptr);
        while (remaining > 0)
        {
            auto bytes = static_cast<unsigned int>(4098 < remaining ? 4098 : remaining);
            long long sz = ::read(fd, buffer + (length - remaining), bytes);
            remaining -= sz;
            if (sz < 0)
            {
                break;
            }
        }

        fclose(fptr);

        return remaining == 0 ? 0 : -1;
    }

private:
    std::shared_ptr<std::vector<Token>> m_headerTokens;
    std::vector<std::shared_ptr<std::vector<Token>>> m_messages;
    std::unique_ptr<char[]> m_buffer;
    std::uint64_t m_length = 0;
    int m_id = 0;

    int decodeIr()
    {
        using namespace uk::co::real_logic::sbe::ir::generated;

        FrameCodec frame;
        std::uint64_t offset = 0;
        char tmp[256] = {};

        frame.wrapForDecode(
            m_buffer.get(),
            offset,
            FrameCodec::sbeBlockLength(),
            FrameCodec::sbeSchemaVersion(),
            m_length);

        frame.getPackageName(tmp, sizeof(tmp));

        if (frame.irVersion() != 0)
        {
            return -1;
        }

        frame.getNamespaceName(tmp, sizeof(tmp));
        frame.getSemanticVersion(tmp, sizeof(tmp));

        offset += frame.encodedLength();

        m_headerTokens.reset(new std::vector<Token>());

        std::uint64_t headerLength = readHeader(offset);

        m_id = frame.irId();

        offset += headerLength;

        while (offset < m_length)
        {
            offset += readMessage(offset);
        }

        return 0;
    }

    std::uint64_t decodeAndAddToken(std::shared_ptr<std::vector<Token>> &tokens, std::uint64_t offset)
    {
        using namespace uk::co::real_logic::sbe::ir::generated;

        TokenCodec tokenCodec;
        tokenCodec.wrapForDecode(
            m_buffer.get(),
            offset,
            TokenCodec::sbeBlockLength(),
            TokenCodec::sbeSchemaVersion(),
            m_length);

        auto signal = static_cast<Signal>(tokenCodec.signal());
        auto type = static_cast<PrimitiveType>(tokenCodec.primitiveType());
        auto presence = static_cast<Presence>(tokenCodec.presence());
        auto byteOrder = static_cast<ByteOrder>(tokenCodec.byteOrder());
        std::int32_t tokenOffset = tokenCodec.tokenOffset();
        std::int32_t tokenSize = tokenCodec.tokenSize();
        std::int32_t id = tokenCodec.fieldId();
        std::int32_t version = tokenCodec.tokenVersion();
        std::int32_t componentTokenCount = tokenCodec.componentTokenCount();
        char tmpBuffer[256] = {};
        std::uint64_t tmpLen = 0;

        tmpLen = tokenCodec.getName(tmpBuffer, sizeof(tmpBuffer));
        std::string name(tmpBuffer, static_cast<std::size_t>(tmpLen));

        tmpLen = tokenCodec.getConstValue(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue constValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.getMinValue(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue minValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.getMaxValue(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue maxValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.getNullValue(tmpBuffer, sizeof(tmpBuffer));
        PrimitiveValue nullValue(type, tmpLen, tmpBuffer);

        tmpLen = tokenCodec.getCharacterEncoding(tmpBuffer, sizeof(tmpBuffer));
        std::string characterEncoding(tmpBuffer, tmpLen);

        tmpLen = tokenCodec.getEpoch(tmpBuffer, sizeof(tmpBuffer));
        std::string epoch(tmpBuffer, tmpLen);

        tmpLen = tokenCodec.getTimeUnit(tmpBuffer, sizeof(tmpBuffer));
        std::string timeUnit(tmpBuffer, tmpLen);

        tmpLen = tokenCodec.getSemanticType(tmpBuffer, sizeof(tmpBuffer));
        std::string semanticType(tmpBuffer, tmpLen);

        tmpLen = tokenCodec.getDescription(tmpBuffer, sizeof(tmpBuffer));
        std::string description(tmpBuffer, tmpLen);

        tmpLen = tokenCodec.getReferencedName(tmpBuffer, sizeof(tmpBuffer));
        std::string referencedName(tmpBuffer, tmpLen);

        Encoding encoding(
            type,
            presence,
            byteOrder,
            minValue,
            maxValue,
            nullValue,
            constValue,
            characterEncoding,
            epoch,
            timeUnit,
            semanticType);

        Token token(tokenOffset, id, version, tokenSize, componentTokenCount, signal, name, description, encoding);

        tokens->push_back(token);

        return tokenCodec.encodedLength();
    }

    std::uint64_t readHeader(std::uint64_t offset)
    {
        std::uint64_t size = 0;

        while (offset + size < m_length)
        {
            size += decodeAndAddToken(m_headerTokens, offset + size);

            Token &token = m_headerTokens->back();

            if (token.signal() == Signal::END_COMPOSITE)
            {
                break;
            }
        }

        return size;
    }

    std::uint64_t readMessage(std::uint64_t offset)
    {
        std::uint64_t size = 0;

        std::shared_ptr<std::vector<Token>> tokensForMessage(new std::vector<Token>());

        while (offset + size < m_length)
        {
            size += decodeAndAddToken(tokensForMessage, offset + size);

            Token &token = tokensForMessage->back();

            if (token.signal() == Signal::END_MESSAGE)
            {
                break;
            }
        }

        m_messages.push_back(tokensForMessage);

        return size;
    }
};

}}

#endif
