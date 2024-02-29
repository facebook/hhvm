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
#include <iostream>

#include "gtest/gtest.h"
#include "composite_elements/MessageHeader.h"
#include "composite_elements/Msg.h"
#include "otf/IrDecoder.h"
#include "otf/OtfHeaderDecoder.h"
#include "otf/OtfMessageDecoder.h"

using namespace composite::elements;

enum EventNumber
{
    EN_beginOuter = 0,
    EN_enumOne,
    EN_zeroth,
    EN_setOne,
    EN_beginInner,
    EN_innerFirst,
    EN_innerSecond,
    EN_endInner,
    EN_endOuter
};

class CompositeElementsTest : public testing::Test, public OtfMessageDecoder::BasicTokenListener
{
public:
    char m_buffer[2048] = {};
    IrDecoder m_irDecoder;
    int m_eventNumber = 0;

    void SetUp() override
    {
        m_eventNumber = 0;
    }

    std::uint64_t encodeHdrAndMsg()
    {
        MessageHeader hdr;
        Msg msg;

        hdr.wrap(m_buffer, 0, 0, sizeof(m_buffer))
            .blockLength(Msg::sbeBlockLength())
            .templateId(Msg::sbeTemplateId())
            .schemaId(Msg::sbeSchemaId())
            .version(Msg::sbeSchemaVersion());

        msg.wrapForEncode(m_buffer, hdr.encodedLength(), sizeof(m_buffer));

        msg.structure()
            .enumOne(EnumOne::Value10)
            .zeroth(42)
            .setOne().clear().bit0(false).bit16(true).bit26(false);

        msg.structure()
            .inner()
                .first(101)
                .second(202);

        return hdr.encodedLength() + msg.encodedLength();
    }

    void onBeginComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex) override
    {
        switch (EventNumber(m_eventNumber++))
        {
            case EN_beginOuter:
                EXPECT_EQ(tokens.at(fromIndex).name(), "outer");
                break;

            case EN_beginInner:
                EXPECT_EQ(tokens.at(fromIndex).name(), "inner");
                break;

            default:
                FAIL() << "unknown beginComposite event number " << m_eventNumber - 1;
        }
    }

    void onEndComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex) override
    {
        switch (EventNumber(m_eventNumber++))
        {
            case EN_endInner:
                EXPECT_EQ(tokens.at(fromIndex).name(), "inner");
                break;

            case EN_endOuter:
                EXPECT_EQ(tokens.at(fromIndex).name(), "outer");
                break;

            default:
                FAIL() << "unknown endComposite event number " << m_eventNumber - 1;
        }
    }

    void onEncoding(
        Token &fieldToken,
        const char *buffer,
        Token &typeToken,
        std::uint64_t actingVersion) override
    {
        switch (EventNumber(m_eventNumber++))
        {
            case EN_zeroth:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 42u);
                break;
            }

            case EN_innerFirst:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 101l);
                break;
            }

            case EN_innerSecond:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 202l);
                break;
            }

            default:
                FAIL() << "unknown Encoding event number " << m_eventNumber - 1;
        }
    }

    void onBitSet(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion) override
    {
        switch (EventNumber(m_eventNumber++))
        {
            case EN_setOne:
            {
                const Token &typeToken = tokens.at(fromIndex + 1);
                const Encoding &encoding = typeToken.encoding();

                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT32);
                EXPECT_EQ(encoding.getAsUInt(buffer), 0x00010000u);

                EXPECT_EQ(tokens.at(fromIndex + 1).name(), "Bit0");
                EXPECT_EQ(tokens.at(fromIndex + 2).name(), "Bit16");
                EXPECT_EQ(tokens.at(fromIndex + 3).name(), "Bit26");
                EXPECT_EQ(toIndex - 1 - fromIndex, 3u);
                break;
            }

            default:
                FAIL() << "unknown BitSet event number " << m_eventNumber - 1;
        }
    }

    void onEnum(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion) override
    {
        switch (EventNumber(m_eventNumber++))
        {
            case EN_enumOne:
            {
                const Token &typeToken = tokens.at(fromIndex + 1);
                const Encoding &encoding = typeToken.encoding();

                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), 10u);

                EXPECT_EQ(tokens.at(fromIndex + 1).name(), "Value1");
                EXPECT_EQ(tokens.at(fromIndex + 2).name(), "Value10");
                EXPECT_EQ(toIndex - 1 - fromIndex, 2u);
                break;
            }

            default:
                FAIL() << "unknown Enum event number " << m_eventNumber - 1;
        }
    }
};

TEST_F(CompositeElementsTest, shouldEncodeMsgCorrectly)
{
    std::uint64_t sz = encodeHdrAndMsg();
    const char *bufferPtr = m_buffer;
    std::uint64_t offset = 0;

    ASSERT_EQ(sz, 8u + 22u);
    ASSERT_EQ(Msg::isConstLength(), true);
    EXPECT_EQ(sz, Msg::sbeBlockAndHeaderLength());

    EXPECT_EQ(*((std::uint16_t *)(bufferPtr + offset)), Msg::sbeBlockLength());
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*((std::uint16_t *)(bufferPtr + offset)), Msg::sbeTemplateId());
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*((std::uint16_t *)(bufferPtr + offset)), Msg::sbeSchemaId());
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*((std::uint16_t *)(bufferPtr + offset)), Msg::sbeSchemaVersion());
    offset += sizeof(std::uint16_t);

    EXPECT_EQ(*((std::uint8_t *)(bufferPtr + offset)), 10u);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*((std::uint8_t *)(bufferPtr + offset)), 42u);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*((std::uint32_t *)(bufferPtr + offset)), 0x00010000u);
    offset += sizeof(std::uint32_t);

    EXPECT_EQ(*((std::int64_t *)(bufferPtr + offset)), 101l);
    offset += sizeof(std::int64_t);

    EXPECT_EQ(*((std::int64_t *)(bufferPtr + offset)), 202l);
    offset += sizeof(std::int64_t);

    EXPECT_EQ(offset, sz);
}

TEST_F(CompositeElementsTest, shouldEncodeAndDecodeMsgCorrectly)
{
    std::uint64_t sz = encodeHdrAndMsg();

    ASSERT_EQ(sz, MessageHeader::encodedLength() + Msg::sbeBlockLength());

    MessageHeader hdr;
    Msg msg;

    hdr.wrap(m_buffer, 0, Msg::sbeSchemaVersion(), sizeof(m_buffer));

    EXPECT_EQ(hdr.blockLength(), Msg::sbeBlockLength());
    EXPECT_EQ(hdr.templateId(), Msg::sbeTemplateId());
    EXPECT_EQ(hdr.schemaId(), Msg::sbeSchemaId());
    EXPECT_EQ(hdr.version(), Msg::sbeSchemaVersion());

    msg.wrapForDecode(m_buffer, MessageHeader::encodedLength(), hdr.blockLength(), hdr.version(), sizeof(m_buffer));

    EXPECT_EQ(msg.structure().enumOne(), EnumOne::Value::Value10);
    EXPECT_EQ(msg.structure().zeroth(), 42u);
    EXPECT_EQ(msg.structure().setOne().bit0(), false);
    EXPECT_EQ(msg.structure().setOne().bit16(), true);
    EXPECT_EQ(msg.structure().setOne().bit26(), false);
    EXPECT_EQ(msg.structure().inner().first(), 101l);
    EXPECT_EQ(msg.structure().inner().second(), 202l);

    EXPECT_EQ(msg.encodedLength(), sz - MessageHeader::encodedLength());
}

TEST_F(CompositeElementsTest, shouldHandleAllEventsCorrectlyInOrder)
{
    std::uint64_t sz = encodeHdrAndMsg();

    ASSERT_EQ(sz, MessageHeader::encodedLength() + Msg::sbeBlockLength());

    ASSERT_GE(m_irDecoder.decode("composite-elements-schema.sbeir"), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();
    std::shared_ptr<std::vector<Token>> messageTokens = m_irDecoder.message(
        Msg::sbeTemplateId(), Msg::sbeSchemaVersion());

    ASSERT_TRUE(headerTokens != nullptr);
    ASSERT_TRUE(messageTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    const char *messageBuffer = m_buffer + headerDecoder.encodedLength();
    std::size_t length = sz - headerDecoder.encodedLength();
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(m_buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(m_buffer);

    const std::size_t result = OtfMessageDecoder::decode(
        messageBuffer, length, actingVersion, blockLength, messageTokens, *this);
    EXPECT_EQ(result, static_cast<std::size_t>(Msg::sbeBlockLength()));

    EXPECT_EQ(m_eventNumber, 9);
}
