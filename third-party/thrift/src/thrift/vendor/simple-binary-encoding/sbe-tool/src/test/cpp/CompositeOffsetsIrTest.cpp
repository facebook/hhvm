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
#include "composite_offsets_test/MessageHeader.h"
#include "composite_offsets_test/TestMessage1.h"
#include "otf/OtfHeaderDecoder.h"
#include "otf/OtfMessageDecoder.h"
#include "otf/IrDecoder.h"

using namespace composite::offsets::test;

static const char *SCHEMA_FILENAME = "composite-offsets-schema.sbeir";

class CompositeOffsetsIrTest : public testing::Test, public OtfMessageDecoder::BasicTokenListener
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
        TestMessage1 msg;

        hdr.wrap(m_buffer, 0, 0, sizeof(m_buffer))
            .blockLength(TestMessage1::sbeBlockLength())
            .templateId(TestMessage1::sbeTemplateId())
            .schemaId(TestMessage1::sbeSchemaId())
            .version(TestMessage1::sbeSchemaVersion());

        msg.wrapForEncode(m_buffer, hdr.encodedLength(), sizeof(m_buffer));

        TestMessage1::Entries &entries = msg.entriesCount(2);

        entries.next()
            .tagGroup1(10)
            .tagGroup2(20);

        entries.next()
            .tagGroup1(30)
            .tagGroup2(40);

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
            case 1:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 10u);
                break;
            }
            case 2:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 20);
                break;
            }
            case 3:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(typeToken.encoding().getAsUInt(buffer), 30u);
                break;
            }
            case 4:
            {
                EXPECT_EQ(typeToken.encoding().primitiveType(), PrimitiveType::INT64);
                EXPECT_EQ(typeToken.encoding().getAsInt(buffer), 40);
                break;
            }
            default:
                FAIL() << "unknown event number " << m_eventNumber;
        }

    }

    void onGroupHeader(Token &token, std::uint64_t numInGroup) override
    {
        switch (m_eventNumber++)
        {
            case 0:
            {
                EXPECT_EQ(numInGroup, 2u);
                break;
            }
            default:
                FAIL() << "unknown event number " << m_eventNumber;
        }
    }
};

TEST_F(CompositeOffsetsIrTest, shouldHandleDecodingOfMessageHeaderCorrectly)
{
    ASSERT_EQ(encodeHdrAndMsg(), 52u);

    ASSERT_GE(m_irDecoder.decode(SCHEMA_FILENAME), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();

    ASSERT_TRUE(headerTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    EXPECT_EQ(headerDecoder.getTemplateId(m_buffer), TestMessage1::sbeTemplateId());
    EXPECT_EQ(headerDecoder.getBlockLength(m_buffer), TestMessage1::sbeBlockLength());
    EXPECT_EQ(headerDecoder.getSchemaId(m_buffer), TestMessage1::sbeSchemaId());
    EXPECT_EQ(headerDecoder.getSchemaVersion(m_buffer), TestMessage1::sbeSchemaVersion());
}

TEST_F(CompositeOffsetsIrTest, shouldHandleAllEventsCorrectlyInOrder)
{
    ASSERT_EQ(encodeHdrAndMsg(), 52u);

    ASSERT_GE(m_irDecoder.decode(SCHEMA_FILENAME), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();
    std::shared_ptr<std::vector<Token>> messageTokens = m_irDecoder.message(
        TestMessage1::sbeTemplateId(), TestMessage1::sbeSchemaVersion());

    ASSERT_TRUE(headerTokens != nullptr);
    ASSERT_TRUE(messageTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    const char *messageBuffer = m_buffer + headerDecoder.encodedLength();
    std::size_t length = 52 - headerDecoder.encodedLength();
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(m_buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(m_buffer);

    const std::size_t result = OtfMessageDecoder::decode(
        messageBuffer, length, actingVersion, blockLength, messageTokens, *this);
    EXPECT_EQ(result, static_cast<std::size_t>(52 - MessageHeader::encodedLength()));

    EXPECT_EQ(m_eventNumber, 5);
}
