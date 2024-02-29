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
#include "message_block_length_test/MessageHeader.h"
#include "message_block_length_test/MsgName.h"
#include "otf/IrDecoder.h"
#include "otf/OtfHeaderDecoder.h"
#include "otf/OtfMessageDecoder.h"

using namespace message::block::length::test;

class MessageBlockLengthIrTest : public testing::Test, public OtfMessageDecoder::BasicTokenListener
{
public:
    char m_buffer[2048] = {};
    IrDecoder m_irDecoder = {};
    int m_eventNumber = 0;

    void SetUp() override
    {
        m_eventNumber = 0;
    }

    std::uint64_t encodeHdrAndMsg()
    {
        MessageHeader hdr;
        MsgName msg;

        hdr.wrap(m_buffer, 0, 0, sizeof(m_buffer))
            .blockLength(MsgName::sbeBlockLength())
            .templateId(MsgName::sbeTemplateId())
            .schemaId(MsgName::sbeSchemaId())
            .version(MsgName::sbeSchemaVersion());

        msg.wrapForEncode(m_buffer, hdr.encodedLength(), sizeof(m_buffer));

        msg.field1(187);
        msg.field2().clear()
            .choice1(true);

        MsgName::GrName &grp = msg.grNameCount(2);

        grp.next()
            .grField1(10)
            .grField2(20);

        grp.next()
            .grField1(30)
            .grField2(40);

        return hdr.encodedLength() + msg.encodedLength();
    }

    void onEncoding(
        Token &fieldToken,
        const char *buffer,
        Token &typeToken,
        std::uint64_t actingVersion) override
    {
        switch (m_eventNumber++)
        {
            case 0:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 187u);
                break;
            }

            case 3:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 10u);
                break;
            }

            case 4:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 20);
                break;
            }

            case 5:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 30u);
                break;
            }

            case 6:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 40);
                break;
            }

            default:
                FAIL() << "unknown event number " << m_eventNumber;
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
        switch (m_eventNumber++)
        {
            case 1:
            {
                const Token &typeToken = tokens.at(fromIndex + 1);
                const Encoding &encoding = typeToken.encoding();

                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), 0x2u);
                break;
            }

            default:
                FAIL() << "unknown event number " << m_eventNumber;
        }
    }

    void onGroupHeader(
        Token &token,
        std::uint64_t numInGroup) override
    {
        switch (m_eventNumber++)
        {
            case 2:
            {
                EXPECT_EQ(numInGroup, 2u);
                break;
            }

            default:
                FAIL() << "unknown event number " << m_eventNumber;
        }
    }
};

TEST_F(MessageBlockLengthIrTest, shouldHandleAllEventsCorrectlyInOrder)
{
    std::uint64_t sz = encodeHdrAndMsg();
    const char *bufferPtr = m_buffer;

    ASSERT_EQ(sz, 54u);
    EXPECT_EQ(*((::uint16_t *)bufferPtr), MsgName::sbeBlockLength());
    EXPECT_EQ(*((::uint16_t *)(bufferPtr + 2)), MsgName::sbeTemplateId());
    EXPECT_EQ(*((::uint16_t *)(bufferPtr + 4)), MsgName::sbeSchemaId());
    EXPECT_EQ(*((::uint16_t *)(bufferPtr + 6)), MsgName::sbeSchemaVersion());

    EXPECT_EQ(*((::uint64_t *)(bufferPtr + 8)), 187u); // field 1
    EXPECT_EQ(*((::uint8_t *)(bufferPtr + 16)), 0x2u); // field 2

    EXPECT_EQ(*((::uint16_t *)(bufferPtr + 19)), 16u); // groupSizeEncoding blockLength
    EXPECT_EQ(*((::uint8_t *)(bufferPtr + 21)), 2u);   // groupSizeEncoding numInGroup

    ASSERT_GE(m_irDecoder.decode("message-block-length-test.sbeir"), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();
    std::shared_ptr<std::vector<Token>> messageTokens = m_irDecoder.message(
        MsgName::sbeTemplateId(), MsgName::sbeSchemaVersion());

    ASSERT_TRUE(headerTokens != nullptr);
    ASSERT_TRUE(messageTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    const char *messageBuffer = m_buffer + headerDecoder.encodedLength();
    std::size_t length = 54 - headerDecoder.encodedLength();
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(m_buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(m_buffer);

    const std::size_t result = OtfMessageDecoder::decode(
        messageBuffer, length, actingVersion, blockLength, messageTokens, *this);
    EXPECT_EQ(result, static_cast<std::size_t>(54 - MessageHeader::encodedLength()));

    EXPECT_EQ(m_eventNumber, 7);
}
