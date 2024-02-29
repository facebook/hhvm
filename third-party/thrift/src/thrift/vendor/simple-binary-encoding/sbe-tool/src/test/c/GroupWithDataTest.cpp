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
#include <string>

#include <gtest/gtest.h>

#include "group_with_data/testMessage1.h"
#include "group_with_data/testMessage2.h"
#include "group_with_data/testMessage3.h"
#include "group_with_data/testMessage4.h"

#define GWD(name) group_with_data_##name

static const std::uint32_t TAG_1 = 32;
static const std::uint64_t ENTRIES_COUNT = 2;

static const char TAG_GROUP_1_IDX_0[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '0' };
static const char TAG_GROUP_1_IDX_1[] = { 'T', 'a', 'g', 'G', 'r', 'o', 'u', 'p', '1' };
static const std::uint64_t TAG_GROUP_1_IDX_0_LENGTH = sizeof(TAG_GROUP_1_IDX_0);
static const std::uint64_t TAG_GROUP_1_IDX_1_LENGTH = sizeof(TAG_GROUP_1_IDX_1);

static const std::int64_t TAG_GROUP_2_IDX_0 = -120;
static const std::int64_t TAG_GROUP_2_IDX_1 = 120;
static const std::int64_t TAG_GROUP_2_IDX_2 = 75;

static const std::int64_t TAG_GROUP_2_IDX_3 = 76;
static const std::int64_t TAG_GROUP_2_IDX_4 = 77;
static const std::int64_t TAG_GROUP_2_IDX_5 = 78;

static const char *VAR_DATA_FIELD_IDX_0 = "neg idx 0";
static const std::uint64_t VAR_DATA_FIELD_IDX_0_LENGTH = 9;
static const char *VAR_DATA_FIELD_IDX_1 = "idx 1 positive";
static const std::uint64_t VAR_DATA_FIELD_IDX_1_LENGTH = 14;

static const std::uint64_t NESTED_ENTRIES_COUNT = 3;

static const char *VAR_DATA_FIELD_NESTED_IDX_0 = "zero";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_0_LENGTH = 4;
static const char *VAR_DATA_FIELD_NESTED_IDX_1 = "one";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_1_LENGTH = 3;
static const char *VAR_DATA_FIELD_NESTED_IDX_2 = "two";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_2_LENGTH = 3;

static const char *VAR_DATA_FIELD_NESTED_IDX_3 = "three";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_3_LENGTH = 5;
static const char *VAR_DATA_FIELD_NESTED_IDX_4 = "four";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_4_LENGTH = 4;
static const char *VAR_DATA_FIELD_NESTED_IDX_5 = "five";
static const std::uint64_t VAR_DATA_FIELD_NESTED_IDX_5_LENGTH = 4;

static const char *VAR_DATA_FIELD_1_IDX_0 = "neg idx 0";
static const std::uint64_t VAR_DATA_FIELD_1_IDX_0_LENGTH = 9;
static const char *VAR_DATA_FIELD_1_IDX_1 = "idx 1 positive";
static const std::uint64_t VAR_DATA_FIELD_1_IDX_1_LENGTH = 14;

static const char *VAR_DATA_FIELD_2_IDX_0 = "negative index 0";
static const std::uint64_t VAR_DATA_FIELD_2_IDX_0_LENGTH = 16;
static const char *VAR_DATA_FIELD_2_IDX_1 = "index 1 pos";
static const std::uint64_t VAR_DATA_FIELD_2_IDX_1_LENGTH = 11;

static const std::uint64_t expectedTestMessage1Size = 78;
static const std::uint64_t expectedTestMessage2Size = 107;
static const std::uint64_t expectedTestMessage3Size = 145;
static const std::uint64_t expectedTestMessage4Size = 73;

class GroupWithDataTest : public testing::Test
{
public:

    std::uint64_t encodeTestMessage1(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(testMessage1_wrap_for_encode)(&m_msg1, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage1_set_tag1)(&m_msg1, TAG_1);

        GWD(testMessage1_entries) entries;
        if (!GWD(testMessage1_entries_set_count)(&m_msg1, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage1_entries_next)(&entries);
        GWD(testMessage1_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_0);
        GWD(testMessage1_entries_set_tagGroup2)(&entries, TAG_GROUP_2_IDX_0);

        GWD(testMessage1_entries_put_varDataField)(&entries, VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        GWD(testMessage1_entries_next)(&entries);
        GWD(testMessage1_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_1);
        GWD(testMessage1_entries_set_tagGroup2)(&entries, TAG_GROUP_2_IDX_1);

        GWD(testMessage1_entries_put_varDataField)(&entries, VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);


        return GWD(testMessage1_encoded_length)(&m_msg1);
    }

    std::uint64_t encodeTestMessage2(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(testMessage2_wrap_for_encode)(&m_msg2, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage2_set_tag1)(&m_msg2, TAG_1);

        GWD(testMessage2_entries) entries;
        if (!GWD(testMessage2_entries_set_count)(&m_msg2, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage2_entries_next)(&entries);
        GWD(testMessage2_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_0);
        GWD(testMessage2_entries_set_tagGroup2)(&entries, TAG_GROUP_2_IDX_0);

        GWD(testMessage2_entries_put_varDataField1)(&entries, VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        GWD(testMessage2_entries_put_varDataField2)(&entries, VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        GWD(testMessage2_entries_next)(&entries);
        GWD(testMessage2_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_1);
        GWD(testMessage2_entries_set_tagGroup2)(&entries, TAG_GROUP_2_IDX_1);

        GWD(testMessage2_entries_put_varDataField1)(&entries, VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        GWD(testMessage2_entries_put_varDataField2)(&entries, VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return GWD(testMessage2_encoded_length)(&m_msg2);
    }

    std::uint64_t encodeTestMessage3(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(testMessage3_wrap_for_encode)(&m_msg3, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage3_set_tag1)(&m_msg3, TAG_1);

        GWD(testMessage3_entries) entries;
        if (!GWD(testMessage3_entries_set_count)(&m_msg3, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage3_entries_next)(&entries);
        GWD(testMessage3_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_0);

        GWD(testMessage3_entries_nestedEntries) nestedEntries0;
        if (!GWD(testMessage3_entries_nestedEntries_set_count)(&entries, &nestedEntries0, NESTED_ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries0, TAG_GROUP_2_IDX_0);

        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_0,
            VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);

        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries0, TAG_GROUP_2_IDX_1);

        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_1,
            VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);

        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries0, TAG_GROUP_2_IDX_2);

        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries0, VAR_DATA_FIELD_NESTED_IDX_2,
            VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);

        GWD(testMessage3_entries_put_varDataField)(&entries, VAR_DATA_FIELD_IDX_0, VAR_DATA_FIELD_IDX_0_LENGTH);

        GWD(testMessage3_entries_next)(&entries);
        GWD(testMessage3_entries_put_tagGroup1)(&entries, TAG_GROUP_1_IDX_1);

        GWD(testMessage3_entries_nestedEntries) nestedEntries1;
        if (!GWD(testMessage3_entries_nestedEntries_set_count)(&entries, &nestedEntries1, NESTED_ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries1, TAG_GROUP_2_IDX_3);

        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_3,
            VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);

        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries1, TAG_GROUP_2_IDX_4);


        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_4,
            VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);

        GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);
        GWD(testMessage3_entries_nestedEntries_set_tagGroup2)(&nestedEntries1, TAG_GROUP_2_IDX_5);

        GWD(testMessage3_entries_nestedEntries_put_varDataFieldNested)(&nestedEntries1, VAR_DATA_FIELD_NESTED_IDX_5,
            VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);

        GWD(testMessage3_entries_put_varDataField)(&entries, VAR_DATA_FIELD_IDX_1, VAR_DATA_FIELD_IDX_1_LENGTH);

        return GWD(testMessage3_encoded_length)(&m_msg3);
    }

    std::uint64_t encodeTestMessage4(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!GWD(testMessage4_wrap_for_encode)(&m_msg4, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage4_set_tag1)(&m_msg4, TAG_1);

        GWD(testMessage4_entries) entries;
        if (!GWD(testMessage4_entries_set_count)(&m_msg4, &entries, ENTRIES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        GWD(testMessage4_entries_next)(&entries);

        GWD(testMessage4_entries_put_varDataField1)(&entries, VAR_DATA_FIELD_1_IDX_0, VAR_DATA_FIELD_1_IDX_0_LENGTH);
        GWD(testMessage4_entries_put_varDataField2)(&entries, VAR_DATA_FIELD_2_IDX_0, VAR_DATA_FIELD_2_IDX_0_LENGTH);

        GWD(testMessage4_entries_next)(&entries);

        GWD(testMessage4_entries_put_varDataField1)(&entries, VAR_DATA_FIELD_1_IDX_1, VAR_DATA_FIELD_1_IDX_1_LENGTH);
        GWD(testMessage4_entries_put_varDataField2)(&entries, VAR_DATA_FIELD_2_IDX_1, VAR_DATA_FIELD_2_IDX_1_LENGTH);

        return GWD(testMessage4_encoded_length)(&m_msg4);
    }

    GWD(testMessage1) m_msg1 = {};
    GWD(testMessage2) m_msg2 = {};
    GWD(testMessage3) m_msg3 = {};
    GWD(testMessage4) m_msg4 = {};
};

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage1Correctly)
{
    char buffer[2048] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage1(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(testMessage1_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH + sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);
    offset += VAR_DATA_FIELD_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH),
        std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);
    offset += VAR_DATA_FIELD_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage1Correctly)
{
    char buffer[2048] = {};
    std::uint64_t sz = encodeTestMessage1(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage1Size);

    GWD(testMessage1) msg1Decoder;
    if (!GWD(testMessage1_reset)(
        &msg1Decoder,
        buffer,
        0,
        sizeof(buffer),
        GWD(testMessage1_sbe_block_length)(),
        GWD(testMessage1_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(testMessage1_tag1)(&msg1Decoder), TAG_1);

    GWD(testMessage1_entries) entries;
    if (!GWD(testMessage1_get_entries)(&msg1Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage1_entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage1_entries_has_next)(&entries));
    GWD(testMessage1_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage1_entries_tagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(
        std::string(GWD(testMessage1_entries_tagGroup1_buffer)(&entries), GWD(testMessage1_entries_tagGroup1_length)()),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(GWD(testMessage1_entries_tagGroup2)(&entries), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(testMessage1_entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage1_entries_varDataField)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH),
        VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(GWD(testMessage1_entries_has_next)(&entries));
    GWD(testMessage1_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage1_entries_tagGroup1_length)(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(
        std::string(GWD(testMessage1_entries_tagGroup1_buffer)(&entries), GWD(testMessage1_entries_tagGroup1_length)()),
        std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    EXPECT_EQ(GWD(testMessage1_entries_tagGroup2)(&entries), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(testMessage1_entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage1_entries_varDataField)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH),
        VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(GWD(testMessage1_encoded_length)(&msg1Decoder), expectedTestMessage1Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage2Correctly)
{
    char buffer[2048] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage2(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(testMessage2_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH + sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    offset += VAR_DATA_FIELD_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);
    offset += VAR_DATA_FIELD_2_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH),
        std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    offset += VAR_DATA_FIELD_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);
    offset += VAR_DATA_FIELD_2_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage2Correctly)
{
    char buffer[2048] = {};
    std::uint64_t sz = encodeTestMessage2(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage2Size);

    GWD(testMessage2) msg2Decoder;
    if (!GWD(testMessage2_reset)(&msg2Decoder, buffer, 0, sizeof(buffer), GWD(testMessage2_sbe_block_length)(),
        GWD(testMessage2_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(testMessage2_tag1)(&msg2Decoder), TAG_1);

    GWD(testMessage2_entries) entries;
    if (!GWD(testMessage2_get_entries)(&msg2Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage2_entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage2_entries_has_next)(&entries));
    GWD(testMessage2_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage2_entries_tagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(
        std::string(GWD(testMessage2_entries_tagGroup1_buffer)(&entries), GWD(testMessage2_entries_tagGroup1_length)()),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    EXPECT_EQ(GWD(testMessage2_entries_tagGroup2)(&entries), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(testMessage2_entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage2_entries_varDataField1)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH),
        VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(GWD(testMessage2_entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage2_entries_varDataField2)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH),
        VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(GWD(testMessage2_entries_has_next)(&entries));
    GWD(testMessage2_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage2_entries_tagGroup1_length)(), TAG_GROUP_1_IDX_1_LENGTH);
    EXPECT_EQ(
        std::string(GWD(testMessage2_entries_tagGroup1_buffer)(&entries), GWD(testMessage2_entries_tagGroup1_length)()),
        std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));

    EXPECT_EQ(GWD(testMessage2_entries_tagGroup2)(&entries), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(testMessage2_entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage2_entries_varDataField1)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH),
        VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(GWD(testMessage2_entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage2_entries_varDataField2)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH),
        VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(GWD(testMessage2_encoded_length)(&msg2Decoder), expectedTestMessage2Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage3Correctly)
{
    char buffer[2048] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage3(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(testMessage1_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), TAG_GROUP_1_IDX_0_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_0_LENGTH),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));
    offset += TAG_GROUP_1_IDX_0_LENGTH;

    // nested entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), NESTED_ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_0);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_0_LENGTH), VAR_DATA_FIELD_NESTED_IDX_0);
    offset += VAR_DATA_FIELD_NESTED_IDX_0_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_1);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_1_LENGTH), VAR_DATA_FIELD_NESTED_IDX_1);
    offset += VAR_DATA_FIELD_NESTED_IDX_1_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_2);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_2_LENGTH), VAR_DATA_FIELD_NESTED_IDX_2);
    offset += VAR_DATA_FIELD_NESTED_IDX_2_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_0_LENGTH), VAR_DATA_FIELD_IDX_0);
    offset += VAR_DATA_FIELD_IDX_0_LENGTH;

    EXPECT_EQ(std::string(bp + offset, TAG_GROUP_1_IDX_1_LENGTH),
        std::string(TAG_GROUP_1_IDX_1, TAG_GROUP_1_IDX_1_LENGTH));
    offset += TAG_GROUP_1_IDX_1_LENGTH;

    // nested entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), sizeof(std::int64_t));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), NESTED_ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_3);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_3_LENGTH), VAR_DATA_FIELD_NESTED_IDX_3);
    offset += VAR_DATA_FIELD_NESTED_IDX_3_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_4);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_4_LENGTH), VAR_DATA_FIELD_NESTED_IDX_4);
    offset += VAR_DATA_FIELD_NESTED_IDX_4_LENGTH;

    EXPECT_EQ(*(std::int64_t *)(bp + offset), TAG_GROUP_2_IDX_5);
    offset += sizeof(std::int64_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_NESTED_IDX_5_LENGTH), VAR_DATA_FIELD_NESTED_IDX_5);
    offset += VAR_DATA_FIELD_NESTED_IDX_5_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_IDX_1_LENGTH), VAR_DATA_FIELD_IDX_1);
    offset += VAR_DATA_FIELD_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage3Correctly)
{
    char buffer[2048] = {};
    std::uint64_t sz = encodeTestMessage3(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage3Size);

    GWD(testMessage3) msg3Decoder;
    if (!GWD(testMessage3_reset)(&msg3Decoder, buffer, 0, sizeof(buffer), GWD(testMessage3_sbe_block_length)(),
        GWD(testMessage3_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(testMessage3_tag1)(&msg3Decoder), TAG_1);

    GWD(testMessage3_entries) entries;
    if (!GWD(testMessage3_get_entries)(&msg3Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage3_entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage3_entries_has_next)(&entries));
    GWD(testMessage3_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage3_entries_tagGroup1_length)(), TAG_GROUP_1_IDX_0_LENGTH);
    EXPECT_EQ(
        std::string(GWD(testMessage3_entries_tagGroup1_buffer)(&entries), GWD(testMessage3_entries_tagGroup1_length)()),
        std::string(TAG_GROUP_1_IDX_0, TAG_GROUP_1_IDX_0_LENGTH));

    GWD(testMessage3_entries_nestedEntries) nestedEntries0;
    if (!GWD(testMessage3_entries_get_nestedEntries)(&entries, &nestedEntries0))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_count)(&nestedEntries0), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries0));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_0);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_0_LENGTH), VAR_DATA_FIELD_NESTED_IDX_0);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries0));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_1);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_1_LENGTH), VAR_DATA_FIELD_NESTED_IDX_1);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries0));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries0);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries0), TAG_GROUP_2_IDX_2);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_2_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries0),
        VAR_DATA_FIELD_NESTED_IDX_2_LENGTH), VAR_DATA_FIELD_NESTED_IDX_2);

    EXPECT_EQ(GWD(testMessage3_entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_varDataField)(&entries), VAR_DATA_FIELD_IDX_0_LENGTH),
        VAR_DATA_FIELD_IDX_0);

    ASSERT_TRUE(GWD(testMessage3_entries_has_next)(&entries));
    GWD(testMessage3_entries_next)(&entries);

    GWD(testMessage3_entries_nestedEntries) nestedEntries1;
    if (!GWD(testMessage3_entries_get_nestedEntries)(&entries, &nestedEntries1))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_count)(&nestedEntries1), NESTED_ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries1));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_3);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_3_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_3_LENGTH), VAR_DATA_FIELD_NESTED_IDX_3);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries1));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_4);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_4_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_4_LENGTH), VAR_DATA_FIELD_NESTED_IDX_4);

    ASSERT_TRUE(GWD(testMessage3_entries_nestedEntries_has_next)(&nestedEntries1));
    GWD(testMessage3_entries_nestedEntries_next)(&nestedEntries1);

    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_tagGroup2)(&nestedEntries1), TAG_GROUP_2_IDX_5);
    EXPECT_EQ(GWD(testMessage3_entries_nestedEntries_varDataFieldNested_length)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_5_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_nestedEntries_varDataFieldNested)(&nestedEntries1),
        VAR_DATA_FIELD_NESTED_IDX_5_LENGTH), VAR_DATA_FIELD_NESTED_IDX_5);

    EXPECT_EQ(GWD(testMessage3_entries_varDataField_length)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage3_entries_varDataField)(&entries), VAR_DATA_FIELD_IDX_1_LENGTH),
        VAR_DATA_FIELD_IDX_1);

    EXPECT_EQ(GWD(testMessage3_encoded_length)(&msg3Decoder), expectedTestMessage3Size);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeTestMessage4Correctly)
{
    char buffer[2048] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeTestMessage4(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), TAG_1);
    EXPECT_EQ(GWD(testMessage4_sbe_block_length)(), 16);
    offset += 16;  // root blockLength of 16

    // entries
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 0);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), ENTRIES_COUNT);
    offset += sizeof(std::uint8_t);

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_0_LENGTH), VAR_DATA_FIELD_1_IDX_0);
    offset += VAR_DATA_FIELD_1_IDX_0_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_0_LENGTH), VAR_DATA_FIELD_2_IDX_0);
    offset += VAR_DATA_FIELD_2_IDX_0_LENGTH;

    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_1_IDX_1_LENGTH), VAR_DATA_FIELD_1_IDX_1);
    offset += VAR_DATA_FIELD_1_IDX_1_LENGTH;
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, VAR_DATA_FIELD_2_IDX_1_LENGTH), VAR_DATA_FIELD_2_IDX_1);
    offset += VAR_DATA_FIELD_2_IDX_1_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(GroupWithDataTest, shouldBeAbleToEncodeAndDecodeTestMessage4Correctly)
{
    char buffer[2048] = {};
    std::uint64_t sz = encodeTestMessage4(buffer, 0, sizeof(buffer));

    EXPECT_EQ(sz, expectedTestMessage4Size);

    GWD(testMessage4) msg4Decoder;
    if (!GWD(testMessage4_reset)(&msg4Decoder, buffer, 0, sizeof(buffer), GWD(testMessage4_sbe_block_length)(),
        GWD(testMessage4_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(GWD(testMessage4_tag1)(&msg4Decoder), TAG_1);

    GWD(testMessage4_entries) entries;
    if (!GWD(testMessage4_get_entries)(&msg4Decoder, &entries))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(GWD(testMessage4_entries_count)(&entries), ENTRIES_COUNT);

    ASSERT_TRUE(GWD(testMessage4_entries_has_next)(&entries));
    GWD(testMessage4_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage4_entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage4_entries_varDataField1)(&entries), VAR_DATA_FIELD_1_IDX_0_LENGTH),
        VAR_DATA_FIELD_1_IDX_0);
    EXPECT_EQ(GWD(testMessage4_entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage4_entries_varDataField2)(&entries), VAR_DATA_FIELD_2_IDX_0_LENGTH),
        VAR_DATA_FIELD_2_IDX_0);

    ASSERT_TRUE(GWD(testMessage4_entries_has_next)(&entries));
    GWD(testMessage4_entries_next)(&entries);

    EXPECT_EQ(GWD(testMessage4_entries_varDataField1_length)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage4_entries_varDataField1)(&entries), VAR_DATA_FIELD_1_IDX_1_LENGTH),
        VAR_DATA_FIELD_1_IDX_1);
    EXPECT_EQ(GWD(testMessage4_entries_varDataField2_length)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH);
    EXPECT_EQ(std::string(GWD(testMessage4_entries_varDataField2)(&entries), VAR_DATA_FIELD_2_IDX_1_LENGTH),
        VAR_DATA_FIELD_2_IDX_1);

    EXPECT_EQ(GWD(testMessage4_encoded_length)(&msg4Decoder), expectedTestMessage4Size);
}
