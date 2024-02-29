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

using namespace composite::offsets::test;

class CompositeOffsetsCodeGenTest : public testing::Test
{
public:

    std::uint64_t encodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_hdr.wrap(buffer, offset, 0, bufferLength)
            .blockLength(TestMessage1::sbeBlockLength())
            .templateId(TestMessage1::sbeTemplateId())
            .schemaId(TestMessage1::sbeSchemaId())
            .version(TestMessage1::sbeSchemaVersion());

        return m_hdr.encodedLength();
    }

    std::uint64_t encodeMsg(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_msg.wrapForEncode(buffer, offset, bufferLength);

        TestMessage1::Entries &entries = m_msg.entriesCount(2);

        entries.next()
            .tagGroup1(10)
            .tagGroup2(20);

        entries.next()
            .tagGroup1(30)
            .tagGroup2(40);

        return m_msg.encodedLength();
    }

    MessageHeader m_hdr = {};
    MessageHeader m_hdrDecoder = {};
    TestMessage1 m_msg = {};
    TestMessage1 m_msgDecoder = {};
};

TEST_F(CompositeOffsetsCodeGenTest, shouldReturnCorrectValuesForMessageHeaderStaticFields)
{
    EXPECT_EQ(MessageHeader::encodedLength(), 12u);
    // only checking the block length field
    EXPECT_EQ(MessageHeader::blockLengthNullValue(), 65535);
    EXPECT_EQ(MessageHeader::blockLengthMinValue(), 0);
    EXPECT_EQ(MessageHeader::blockLengthMaxValue(), 65534);
}

TEST_F(CompositeOffsetsCodeGenTest, shouldReturnCorrectValuesForTestMessage1StaticFields)
{
    EXPECT_EQ(TestMessage1::sbeBlockLength(), 0);
    EXPECT_EQ(TestMessage1::sbeTemplateId(), 1);
    EXPECT_EQ(TestMessage1::sbeSchemaId(), 15);
    EXPECT_EQ(TestMessage1::sbeSchemaVersion(), 0);
    EXPECT_EQ(std::string(TestMessage1::sbeSemanticType()), std::string(""));
    EXPECT_EQ(TestMessage1::Entries::sbeBlockLength(), 16u);
    EXPECT_EQ(TestMessage1::Entries::sbeHeaderSize(), 8u);
}

TEST_F(CompositeOffsetsCodeGenTest, shouldBeAbleToEncodeMessageHeaderCorrectly)
{
    char buffer[2048] = {};
    const char *bp = buffer;

    std::uint64_t sz = encodeHdr(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, 12u);
    EXPECT_EQ(*((::uint16_t *)bp), TestMessage1::sbeBlockLength());
    EXPECT_EQ(*((::uint16_t *)(bp + 4)), TestMessage1::sbeTemplateId());
    EXPECT_EQ(*((::uint16_t *)(bp + 8)), TestMessage1::sbeSchemaId());
    EXPECT_EQ(*((::uint16_t *)(bp + 10)), TestMessage1::sbeSchemaVersion());
}

TEST_F(CompositeOffsetsCodeGenTest, shouldBeAbleToEncodeAndDecodeMessageHeaderCorrectly)
{
    char buffer[2048] = {};

    encodeHdr(buffer, 0, sizeof(buffer));

    m_hdrDecoder.wrap(buffer, 0, 0, sizeof(buffer));
    EXPECT_EQ(m_hdrDecoder.blockLength(), TestMessage1::sbeBlockLength());
    EXPECT_EQ(m_hdrDecoder.templateId(), TestMessage1::sbeTemplateId());
    EXPECT_EQ(m_hdrDecoder.schemaId(), TestMessage1::sbeSchemaId());
    EXPECT_EQ(m_hdrDecoder.version(), TestMessage1::sbeSchemaVersion());
}

TEST_F(CompositeOffsetsCodeGenTest, shouldBeAbleToEncodeMessageCorrectly)
{
    char buffer[2048] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeMsg(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, 40u);

    EXPECT_EQ(*(::uint16_t *)bp, TestMessage1::Entries::sbeBlockLength());
    EXPECT_EQ(*(::uint8_t *)(bp + 7), 2u);
    EXPECT_EQ(*(::uint64_t *)(bp + 8), 10u);
    EXPECT_EQ(*(::int64_t *)(bp + 16), 20u);
    EXPECT_EQ(*(::uint64_t *)(bp + 24), 30u);
    EXPECT_EQ(*(::int64_t *)(bp + 32), 40u);
}

TEST_F(CompositeOffsetsCodeGenTest, shouldBeAbleToDecodeHeaderAndMsgCorrectly)
{
    char buffer[2048] = {};
    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t sz = encodeMsg(buffer, hdrSz, sizeof(buffer));

    EXPECT_EQ(hdrSz, 12u);
    EXPECT_EQ(sz, 40u);

    m_hdrDecoder.wrap(buffer, 0, 0, hdrSz + sz);

    EXPECT_EQ(m_hdrDecoder.blockLength(), TestMessage1::sbeBlockLength());
    EXPECT_EQ(m_hdrDecoder.templateId(), TestMessage1::sbeTemplateId());
    EXPECT_EQ(m_hdrDecoder.schemaId(), TestMessage1::sbeSchemaId());
    EXPECT_EQ(m_hdrDecoder.version(), TestMessage1::sbeSchemaVersion());

    m_msgDecoder.wrapForDecode(
        buffer, hdrSz, TestMessage1::sbeBlockLength(), TestMessage1::sbeSchemaVersion(), hdrSz + sz);

    TestMessage1::Entries &entries = m_msgDecoder.entries();
    EXPECT_EQ(entries.count(), 2u);

    ASSERT_TRUE(entries.hasNext());
    entries.next();
    EXPECT_EQ(entries.tagGroup1(), 10u);
    EXPECT_EQ(entries.tagGroup2(), 20u);

    ASSERT_TRUE(entries.hasNext());
    entries.next();
    EXPECT_EQ(entries.tagGroup1(), 30u);
    EXPECT_EQ(entries.tagGroup2(), 40u);

    EXPECT_EQ(m_msgDecoder.encodedLength(), 40u);
}
