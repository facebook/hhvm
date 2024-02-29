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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "order_check/MultipleVarLength.h"
#include "order_check/GroupAndVarLength.h"
#include "order_check/VarLengthInsideGroup.h"
#include "order_check/NestedGroups.h"
#include "order_check/CompositeInsideGroup.h"
#include "order_check/AddPrimitiveV1.h"
#include "order_check/AddPrimitiveV0.h"
#include "order_check/AddPrimitiveBeforeGroupV1.h"
#include "order_check/AddPrimitiveBeforeGroupV0.h"
#include "order_check/AddPrimitiveBeforeVarDataV1.h"
#include "order_check/AddPrimitiveBeforeVarDataV0.h"
#include "order_check/AddPrimitiveInsideGroupV1.h"
#include "order_check/AddPrimitiveInsideGroupV0.h"
#include "order_check/AddGroupBeforeVarDataV1.h"
#include "order_check/AddGroupBeforeVarDataV0.h"
#include "order_check/AddEnumBeforeGroupV1.h"
#include "order_check/AddEnumBeforeGroupV0.h"
#include "order_check/AddCompositeBeforeGroupV1.h"
#include "order_check/AddCompositeBeforeGroupV0.h"
#include "order_check/AddArrayBeforeGroupV1.h"
#include "order_check/AddArrayBeforeGroupV0.h"
#include "order_check/AddBitSetBeforeGroupV1.h"
#include "order_check/AddBitSetBeforeGroupV0.h"
#include "order_check/EnumInsideGroup.h"
#include "order_check/BitSetInsideGroup.h"
#include "order_check/ArrayInsideGroup.h"
#include "order_check/MultipleGroups.h"
#include "order_check/AddVarDataV1.h"
#include "order_check/AddVarDataV0.h"
#include "order_check/AsciiInsideGroup.h"
#include "order_check/AddAsciiBeforeGroupV1.h"
#include "order_check/AddAsciiBeforeGroupV0.h"
#include "order_check/NoBlock.h"
#include "order_check/GroupWithNoBlock.h"
#include "order_check/NestedGroupWithVarLength.h"

using namespace order::check;
using ::testing::HasSubstr;

class FieldAccessOrderCheckTest : public testing::Test
{
public:
    static const std::size_t BUFFER_LEN = 2048u;
    static const std::size_t OFFSET = 0;

    char m_buffer[BUFFER_LEN] = {};

};

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingVariableLengthFieldsInSchemaDefinedOrder1)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingVariableLengthFieldsInSchemaDefinedOrder2)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    auto bLength = decoder.bLength();
    auto *b = decoder.b();
    const std::string bStr(b, bLength);
    EXPECT_EQ(bStr, "abc");
    auto cLength = decoder.cLength();
    auto *c = decoder.c();
    const std::string cStr(c, cLength);
    EXPECT_EQ(cStr, "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingVariableLengthFieldsInSchemaDefinedOrder3)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    auto *bIn = "abc";
    encoder.putB(bIn, static_cast<uint8_t>(strlen(bIn)));
    auto *cIn = "def";
    encoder.putC(cIn, static_cast<uint8_t>(strlen(cIn)));
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);

    const auto bLength = decoder.bLength();
    auto *bOut = new char[bLength];
    decoder.getB(bOut, bLength);
    EXPECT_EQ(std::strncmp(bOut, "abc", bLength), 0);
    delete[] bOut;

    auto cLength = decoder.cLength();
    auto *cOut = new char[cLength];
    decoder.getC(cOut, cLength);
    EXPECT_EQ(std::strncmp(cOut, "def", cLength), 0);
    delete[] cOut;
}

TEST_F(FieldAccessOrderCheckTest, allowsDecodingVariableLengthFieldsAfterRewind)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");

    decoder.sbeRewind();

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsDecodingToSkipVariableLengthFields)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.skipB(), 3ul);
    EXPECT_EQ(decoder.getCAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelPrimitiveFields)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.a(43);
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 43);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingEncodingOfVariableLengthField1)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    EXPECT_THROW(
        {
            try
            {
                encoder.putC("def");
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingEncodingOfVariableLengthField2)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    EXPECT_THROW(
        {
            try
            {
                auto *cIn = "cIn";
                encoder.putC(cIn, static_cast<uint8_t>(std::strlen(cIn)));
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingEarlierVariableLengthFields)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    EXPECT_THROW(
        {
            try
            {
                encoder.putB("ghi");
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b\" in state: V0_C_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingLatestVariableLengthField)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    EXPECT_THROW(
        {
            try
            {
                encoder.putC("ghi");
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_C_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingDecodingOfVariableLengthField1)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);

    EXPECT_THROW(
        {
            try
            {
                decoder.getCAsString();
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingDecodingOfVariableLengthField2)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);

    EXPECT_THROW(
        {
            try
            {
                decoder.cLength();
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot decode length of var data \"c\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingDecodingOfVariableLengthField3)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);

    EXPECT_THROW(
        {
            try
            {
                char cOut[3];
                decoder.getC(cOut, 3);
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingEarlierVariableLengthField)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");

    EXPECT_THROW(
        {
            try
            {
                decoder.getBAsString();
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b\" in state: V0_C_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingLatestVariableLengthField)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.putC("def");
    encoder.checkEncodingIsComplete();

    MultipleVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleVarLength::sbeBlockLength(),
        MultipleVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
    EXPECT_EQ(decoder.getCAsString(), "def");

    EXPECT_THROW(
        {
            try
            {
                decoder.getCAsString();
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"c\" in state: V0_C_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingGroupAndVariableLengthFieldsInSchemaDefinedOrder)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &b = encoder.bCount(2);
    b.next().c(1);
    b.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsDecodingGroupAndVariableLengthFieldsAfterRewind)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &b = encoder.bCount(2);
    b.next().c(1);
    b.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(decoder.getDAsString(), "abc");

    decoder.sbeRewind();

    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder2 = decoder.b();
    EXPECT_EQ(bDecoder2.count(), 2u);
    EXPECT_EQ(bDecoder2.next().c(), 1);
    EXPECT_EQ(bDecoder2.next().c(), 2);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsDecodingToSkipMessage)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &b = encoder.bCount(2);
    b.next().c(1);
    b.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    const std::uint64_t nextEncodeOffset = encoder.sbePosition();
    encoder.wrapForEncode(m_buffer, nextEncodeOffset, BUFFER_LEN);
    encoder.a(43);
    GroupAndVarLength::B &b2 = encoder.bCount(2);
    b2.next().c(3);
    b2.next().c(4);
    encoder.putD("def");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    decoder.skip();
    const std::uint64_t nextDecodeOffset = decoder.sbePosition();
    EXPECT_EQ(nextDecodeOffset, nextEncodeOffset);

    decoder.wrapForDecode(
        m_buffer,
        nextDecodeOffset,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 43);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 3);
    EXPECT_EQ(bDecoder.next().c(), 4);
    EXPECT_EQ(decoder.getDAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsDecodingToDetermineMessageLengthBeforeReadingFields)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(43);
    GroupAndVarLength::B &b = encoder.bCount(2);
    b.next().c(3);
    b.next().c(4);
    encoder.putD("def");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );

    EXPECT_EQ(decoder.decodeLength(), 18u);
    EXPECT_EQ(decoder.a(), 43);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 3);
    EXPECT_EQ(bDecoder.next().c(), 4);
    EXPECT_EQ(decoder.getDAsString(), "def");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingEmptyGroupAndVariableLengthFieldsInSchemaDefinedOrder)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.bCount(0);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 0u);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncoderToResetZeroGroupLengthToNonZero)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.bCount(0).resetCountToIndex();
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 0u);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncoderToResetNonZeroGroupLengthToNonZeroBeforeCallingNext)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.bCount(2).resetCountToIndex();
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 0u);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncoderToResetNonZeroGroupLengthToNonZero)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2).next();
    bEncoder.c(43);
    bEncoder.resetCountToIndex();
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 43);
    EXPECT_EQ(decoder.getDAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncoderToResetGroupLengthMidGroupElement)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    NestedGroups::B &bEncoder = encoder.bCount(2).next();
    bEncoder.c(43);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.resetCountToIndex();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot reset count of repeating group \"b\" in state: V0_B_N_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}


TEST_F(FieldAccessOrderCheckTest, disallowsEncodingGroupElementBeforeCallingNext)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.c(1);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingGroupElementBeforeCallingNext)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);

    EXPECT_THROW(
        {
            try
            {
                bDecoder.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsSkippingEncodingOfGroup)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    EXPECT_THROW(
        {
            try
            {
                encoder.putD("abc");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"d\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingVariableLengthFieldAfterGroup)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    EXPECT_THROW(
        {
            try
            {
                encoder.putD("def");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"d\" in state: V0_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingGroupCount)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    EXPECT_THROW(
        {
            try
            {
                encoder.bCount(1);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot encode count of repeating group \"b\" in state: V0_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfGroupBeforeVariableLengthField)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    EXPECT_THROW(
        {
            try
            {
                decoder.getDAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"d\" in state: V0_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingVariableLengthFieldAfterGroup)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(decoder.getDAsString(), "abc");
    EXPECT_THROW(
        {
            try
            {
                decoder.getDAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"d\" in state: V0_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingGroupAfterVariableLengthField)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1);
    bEncoder.next().c(2);
    encoder.putD("abc");
    encoder.checkEncodingIsComplete();

    GroupAndVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupAndVarLength::sbeBlockLength(),
        GroupAndVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    GroupAndVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(decoder.getDAsString(), "abc");
    EXPECT_THROW(
        {
            try
            {
                decoder.b();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot decode count of repeating group \"b\" in state: V0_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingVariableLengthFieldInsideGroupInSchemaDefinedOrder)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1).putD("abc");
    bEncoder.next().c(2).putD("def");
    encoder.putE("ghi");
    encoder.checkEncodingIsComplete();

    VarLengthInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        VarLengthInsideGroup::sbeBlockLength(),
        VarLengthInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    VarLengthInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.getDAsString(), "abc");
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(bDecoder.getDAsString(), "def");
    EXPECT_EQ(decoder.getEAsString(), "ghi");
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedGroupElementVariableLengthFieldToEncodeAtTopLevel)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next().c(1);
    EXPECT_THROW(
        {
            try
            {
                encoder.putE("abc");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"e\" in state: V0_B_1_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedGroupElementVariableLengthFieldToEncodeNextElement)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next();
    EXPECT_THROW(
        {
            try
            {
                bEncoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedGroupElementEncoding)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1).putD("abc");
    EXPECT_THROW(
        {
            try
            {
                encoder.putE("abc");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"e\" in state: V0_B_N_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingGroupElementVariableLengthField)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next().c(1).putD("abc");
    encoder.putE("def");
    EXPECT_THROW(
        {
            try
            {
                bEncoder.putD("ghi");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.d\" in state: V0_E_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingGroupElementVariableLengthField)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1).putD("abc");
    bEncoder.next().c(2).putD("def");
    encoder.putE("ghi");
    encoder.checkEncodingIsComplete();

    VarLengthInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        VarLengthInsideGroup::sbeBlockLength(),
        VarLengthInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    VarLengthInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_EQ(bDecoder.getDAsString(), "abc");
    EXPECT_THROW(
        {
            try
            {
                bDecoder.getDAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.d\" in state: V0_B_N_D_DONE")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfGroupElementVariableLengthFieldToNextElement)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1).putD("abc");
    bEncoder.next().c(2).putD("def");
    encoder.putE("ghi");
    encoder.checkEncodingIsComplete();

    VarLengthInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        VarLengthInsideGroup::sbeBlockLength(),
        VarLengthInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    VarLengthInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_THROW(
        {
            try
            {
                bDecoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfGroupElementVariableLengthFieldToTopLevel)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next().c(1).putD("abc");
    encoder.putE("ghi");
    encoder.checkEncodingIsComplete();

    VarLengthInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        VarLengthInsideGroup::sbeBlockLength(),
        VarLengthInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    VarLengthInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_THROW(
        {
            try
            {
                decoder.getEAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"e\" in state: V0_B_1_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfGroupElement)
{
    VarLengthInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    VarLengthInsideGroup::B &bEncoder = encoder.bCount(2);
    bEncoder.next().c(1).putD("abc");
    bEncoder.next().c(2).putD("def");
    encoder.putE("ghi");
    encoder.checkEncodingIsComplete();

    VarLengthInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        VarLengthInsideGroup::sbeBlockLength(),
        VarLengthInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    VarLengthInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 1);
    EXPECT_THROW(
        {
            try
            {
                decoder.getEAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"e\" in state: V0_B_N_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingNestedGroupsInSchemaDefinedOrder)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    NestedGroups::B &bEncoder = encoder.bCount(2);
    bEncoder.next();
    bEncoder.c(1);
    NestedGroups::B::D &b0dEncoder = bEncoder.dCount(2);
    b0dEncoder.next().e(2);
    b0dEncoder.next().e(3);
    NestedGroups::B::F &b0fEncoder = bEncoder.fCount(1);
    b0fEncoder.next().g(4);
    bEncoder.next();
    bEncoder.c(5);
    NestedGroups::B::D &b1dEncoder = bEncoder.dCount(1);
    b1dEncoder.next().e(6);
    NestedGroups::B::F &b1fEncoder = bEncoder.fCount(1);
    b1fEncoder.next().g(7);
    NestedGroups::H &hEncoder = encoder.hCount(1);
    hEncoder.next().i(8);
    encoder.checkEncodingIsComplete();

    NestedGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroups::sbeBlockLength(),
        NestedGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    NestedGroups::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    NestedGroups::B &b0Decoder = bDecoder.next();
    EXPECT_EQ(b0Decoder.c(), 1);
    NestedGroups::B::D &b0dDecoder = b0Decoder.d();
    EXPECT_EQ(b0dDecoder.count(), 2u);
    EXPECT_EQ(b0dDecoder.next().e(), 2);
    EXPECT_EQ(b0dDecoder.next().e(), 3);
    NestedGroups::B::F &b0fDecoder = b0Decoder.f();
    EXPECT_EQ(b0fDecoder.count(), 1u);
    EXPECT_EQ(b0fDecoder.next().g(), 4);
    NestedGroups::B &b1Decoder = bDecoder.next();
    EXPECT_EQ(b1Decoder.c(), 5);
    NestedGroups::B::D &b1dDecoder = b1Decoder.d();
    EXPECT_EQ(b1dDecoder.count(), 1u);
    EXPECT_EQ(b1dDecoder.next().e(), 6);
    NestedGroups::B::F &b1fDecoder = b1Decoder.f();
    EXPECT_EQ(b1fDecoder.count(), 1u);
    EXPECT_EQ(b1fDecoder.next().g(), 7);
    NestedGroups::H &hDecoder = decoder.h();
    EXPECT_EQ(hDecoder.count(), 1u);
    EXPECT_EQ(hDecoder.next().i(), 8);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingEmptyNestedGroupsInSchemaDefinedOrder)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.bCount(0);
    encoder.hCount(0);
    encoder.checkEncodingIsComplete();

    NestedGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroups::sbeBlockLength(),
        NestedGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    NestedGroups::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 0u);
    NestedGroups::H &hDecoder = decoder.h();
    EXPECT_EQ(hDecoder.count(), 0u);
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedEncodingOfNestedGroup)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    NestedGroups::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c(1);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.fCount(1);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot encode count of repeating group \"b.f\" in state: V0_B_1_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingCompositeInsideGroupInSchemaDefinedOrder)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    Point &cEncoder = bEncoder.c();
    cEncoder.x(3);
    cEncoder.y(4);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingCompositeInsideGroupBeforeCallingNext)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingCompositeInsideGroupBeforeCallingNext)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    Point &cEncoder = bEncoder.c();
    cEncoder.x(3);
    cEncoder.y(4);

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                bDecoder.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelCompositeViaReWrap)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    Point &cEncoder = bEncoder.c();
    cEncoder.x(3);
    cEncoder.y(4);
    encoder.a().x(5);
    encoder.a().y(6);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 5);
    EXPECT_EQ(aDecoder.y(), 6);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelCompositeViaEncoderReference)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    Point &cEncoder = bEncoder.c();
    cEncoder.x(3);
    cEncoder.y(4);
    aEncoder.x(5);
    aEncoder.y(6);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 5);
    EXPECT_EQ(aDecoder.y(), 6);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingGroupElementCompositeViaReWrap)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c().x(3).y(4);
    bEncoder.c().x(5).y(6);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 5);
    EXPECT_EQ(cDecoder.y(), 6);
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingGroupElementCompositeViaEncoderReference)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    Point &cEncoder = bEncoder.c();
    cEncoder.x(3);
    cEncoder.y(4);
    cEncoder.x(5);
    cEncoder.y(6);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 5);
    EXPECT_EQ(cDecoder.y(), 6);
}

TEST_F(FieldAccessOrderCheckTest, allowsReDecodingTopLevelCompositeViaReWrap)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c().x(3).y(4);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &a1Decoder = decoder.a();
    EXPECT_EQ(a1Decoder.x(), 1);
    EXPECT_EQ(a1Decoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
    Point &a2Decoder = decoder.a();
    EXPECT_EQ(a2Decoder.x(), 1);
    EXPECT_EQ(a2Decoder.y(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsReDecodingTopLevelCompositeViaEncoderReference)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c().x(3).y(4);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsReDecodingGroupElementCompositeViaReWrap)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c().x(3).y(4);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &c1Decoder = bDecoder.next().c();
    EXPECT_EQ(c1Decoder.x(), 3);
    EXPECT_EQ(c1Decoder.y(), 4);
    Point &c2Decoder = bDecoder.c();
    EXPECT_EQ(c2Decoder.x(), 3);
    EXPECT_EQ(c2Decoder.y(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsReDecodingGroupElementCompositeViaEncoderReference)
{
    CompositeInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Point &aEncoder = encoder.a();
    aEncoder.x(1);
    aEncoder.y(2);
    CompositeInsideGroup::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c().x(3).y(4);
    encoder.checkEncodingIsComplete();

    CompositeInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        CompositeInsideGroup::sbeBlockLength(),
        CompositeInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Point &aDecoder = decoder.a();
    EXPECT_EQ(aDecoder.x(), 1);
    EXPECT_EQ(aDecoder.y(), 2);
    CompositeInsideGroup::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    Point &cDecoder = bDecoder.next().c();
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
    EXPECT_EQ(cDecoder.x(), 3);
    EXPECT_EQ(cDecoder.y(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedPrimitiveField)
{
    AddPrimitiveV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.b(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveV1::sbeBlockLength(),
        AddPrimitiveV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.b(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingPrimitiveFieldAsNullValue)
{
    AddPrimitiveV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.checkEncodingIsComplete();

    AddPrimitiveV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.b(), AddPrimitiveV1::bNullValue());
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedPrimitiveFieldBeforeGroup)
{
    AddPrimitiveBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(3);
    AddPrimitiveBeforeGroupV1::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeGroupV1::sbeBlockLength(),
        AddPrimitiveBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(), 3);
    AddPrimitiveBeforeGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingPrimitiveFieldBeforeGroupAsNullValue)
{
    AddPrimitiveBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    AddPrimitiveBeforeGroupV0::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(), AddPrimitiveBeforeGroupV1::dNullValue());
    AddPrimitiveBeforeGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeGroup)
{
    AddPrimitiveBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(3);
    AddPrimitiveBeforeGroupV1::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeGroupV1::sbeBlockLength(),
        AddPrimitiveBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveBeforeGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedPrimitiveFieldBeforeGroup)
{
    AddPrimitiveBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(3);
    AddPrimitiveBeforeGroupV1::B &bEncoder = encoder.bCount(1).next();
    bEncoder.c(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveBeforeGroupV0::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedPrimitiveFieldBeforeVarData)
{
    AddPrimitiveBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.c(3);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeVarDataV1::sbeBlockLength(),
        AddPrimitiveBeforeVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.c(), 3);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingPrimitiveFieldBeforeVarDataAsNullValue)
{
    AddPrimitiveBeforeVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeVarDataV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.c(), AddPrimitiveBeforeVarDataV1::cNullValue());
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeVarData)
{
    AddPrimitiveBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.c(3);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeVarDataV1::sbeBlockLength(),
        AddPrimitiveBeforeVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedPrimitiveFieldBeforeVarData)
{
    AddPrimitiveBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.c(3);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddPrimitiveBeforeVarDataV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveBeforeVarDataV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedPrimitiveFieldInsideGroup)
{
    AddPrimitiveInsideGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2).d(3);
    encoder.checkEncodingIsComplete();

    AddPrimitiveInsideGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveInsideGroupV1::sbeBlockLength(),
        AddPrimitiveInsideGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveInsideGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(bDecoder.d(), 3);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingPrimitiveFieldInsideGroupAsNullValue)
{
    AddPrimitiveInsideGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddPrimitiveInsideGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveInsideGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveInsideGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(bDecoder.d(), AddPrimitiveInsideGroupV1::B::dNullValue());
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedPrimitiveFieldInsideGroup)
{
    AddPrimitiveInsideGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(2).next().c(2).d(3).next().c(4).d(5);
    encoder.checkEncodingIsComplete();

    AddPrimitiveInsideGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveInsideGroupV1::sbeBlockLength(),
        AddPrimitiveInsideGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveInsideGroupV1::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(bDecoder.next().c(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedPrimitiveFieldInsideGroup)
{
    AddPrimitiveInsideGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(2).next().c(2).d(3).next().c(4).d(5);
    encoder.checkEncodingIsComplete();

    AddPrimitiveInsideGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddPrimitiveInsideGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddPrimitiveInsideGroupV0::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    EXPECT_EQ(bDecoder.next().c(), 4);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedGroupBeforeVarData)
{
    AddGroupBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.cCount(1).next().d(2);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddGroupBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddGroupBeforeVarDataV1::sbeBlockLength(),
        AddGroupBeforeVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddGroupBeforeVarDataV1::C &cDecoder = decoder.c();
    EXPECT_EQ(cDecoder.count(), 1u);
    EXPECT_EQ(cDecoder.next().d(), 2);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingGroupBeforeVarDataAsNullValue)
{
    AddGroupBeforeVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddGroupBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddGroupBeforeVarDataV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddGroupBeforeVarDataV1::C &cDecoder = decoder.c();
    EXPECT_EQ(cDecoder.count(), 0u);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipMissingGroupBeforeVarData)
{
    AddGroupBeforeVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddGroupBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddGroupBeforeVarDataV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, disallowsNewDecoderToSkipPresentButAddedGroupBeforeVarData)
{
    AddGroupBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.cCount(1).next().d(2);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddGroupBeforeVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddGroupBeforeVarDataV1::sbeBlockLength(),
        AddGroupBeforeVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_THROW(
        {
            try
            {
                decoder.b();
            }
            catch (const std::exception &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b\" in state: V1_BLOCK")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedGroupBeforeVarData)
{
    AddGroupBeforeVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.cCount(1).next().d(2);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddGroupBeforeVarDataV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddGroupBeforeVarDataV1::sbeBlockLength(),
        AddGroupBeforeVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);

    // In reality, we would use a numGroups field in the message header to skip over unknown groups
    // rather than the hardcoded knowledge of one extra group below:

    GroupSizeEncoding groupSizeEncodingDecoder;
    groupSizeEncodingDecoder.wrap(
        m_buffer,
        decoder.sbePosition(),
        GroupSizeEncoding::sbeSchemaVersion(),
        BUFFER_LEN
    );
    const std::uint64_t bytesToSkip = GroupSizeEncoding::encodedLength() +
                                      groupSizeEncodingDecoder.blockLength() * groupSizeEncodingDecoder.numInGroup();
    decoder.sbePosition(decoder.sbePosition() + bytesToSkip);

    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedEnumFieldBeforeGroup)
{
    AddEnumBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(Direction::BUY);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddEnumBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddEnumBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(), Direction::BUY);
    AddEnumBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingEnumFieldBeforeGroupAsNullValue)
{
    AddEnumBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddEnumBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddEnumBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(), Direction::NULL_VALUE);
    AddEnumBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedEnumFieldBeforeGroup)
{
    AddEnumBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(Direction::SELL);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddEnumBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddEnumBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddEnumBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedEnumFieldBeforeGroup)
{
    AddEnumBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.d(Direction::BUY);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddEnumBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddEnumBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddEnumBeforeGroupV0::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedCompositeFieldBeforeGroup)
{
    AddCompositeBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    Point &d = encoder.d();
    d.x(-1);
    d.y(-2);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddCompositeBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddCompositeBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    Point &d2 = decoder.d();
    EXPECT_EQ(d2.x(), -1);
    EXPECT_EQ(d2.y(), -2);
    AddCompositeBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedCompositeFieldBeforeGroup)
{
    AddCompositeBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    Point &d = encoder.d();
    d.x(-1);
    d.y(-2);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddCompositeBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddCompositeBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddCompositeBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedCompositeFieldBeforeGroup)
{
    AddCompositeBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    Point &d = encoder.d();
    d.x(-1);
    d.y(-2);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddCompositeBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddCompositeBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddCompositeBeforeGroupV0::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedArrayFieldBeforeGroup)
{
    AddArrayBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD(1, 2, 3, 4);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(0), 1);
    EXPECT_EQ(decoder.d(1), 2);
    EXPECT_EQ(decoder.d(2), 3);
    EXPECT_EQ(decoder.d(3), 4);
    AddArrayBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue1)
{
    AddArrayBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.d(0), AddArrayBeforeGroupV1::dNullValue());
    EXPECT_EQ(decoder.d(1), AddArrayBeforeGroupV1::dNullValue());
    EXPECT_EQ(decoder.d(2), AddArrayBeforeGroupV1::dNullValue());
    EXPECT_EQ(decoder.d(3), AddArrayBeforeGroupV1::dNullValue());
    AddArrayBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue2)
{
    AddArrayBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    char *d = decoder.d();
    EXPECT_EQ(d, nullptr);
    AddArrayBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue3)
{
    AddArrayBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    const AddArrayBeforeGroupV1 &constDecoder = decoder;
    const char *d = constDecoder.d();
    EXPECT_EQ(d, nullptr);
    AddArrayBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedArrayFieldBeforeGroup)
{
    AddArrayBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD(1, 2, 3, 4);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddArrayBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedArrayFieldBeforeGroup)
{
    AddArrayBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD(1, 2, 3, 4);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddArrayBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddArrayBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddArrayBeforeGroupV0::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedBitSetFieldBeforeGroup)
{
    AddBitSetBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddBitSetBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddBitSetBeforeGroupV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    Flags &d = decoder.d();
    EXPECT_EQ(d.guacamole(), true);
    EXPECT_EQ(d.cheese(), true);
    EXPECT_EQ(d.sourCream(), false);
    AddBitSetBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingBitSetFieldBeforeGroupAsNullValue)
{
    AddBitSetBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddBitSetBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddBitSetBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.dInActingVersion(), false);
    AddBitSetBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedBitSetFieldBeforeGroup)
{
    AddBitSetBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddBitSetBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddBitSetBeforeGroupV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddBitSetBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedBitSetFieldBeforeGroup)
{
    AddBitSetBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
    encoder.bCount(1).next().c(2);
    encoder.checkEncodingIsComplete();

    AddBitSetBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddBitSetBeforeGroupV1::sbeBlockLength(),
        1,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddBitSetBeforeGroupV0::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingEnumInsideGroupInSchemaDefinedOrder)
{
    EnumInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(Direction::BUY)
        .bCount(1)
        .next()
        .c(Direction::SELL);
    encoder.checkEncodingIsComplete();

    EnumInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        EnumInsideGroup::sbeBlockLength(),
        EnumInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), Direction::BUY);
    EnumInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), Direction::SELL);
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingEnumInsideGroupBeforeCallingNext)
{
    EnumInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(Direction::BUY);
    EnumInsideGroup::B &bEncoder = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.c(Direction::SELL);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingEnumInsideGroupBeforeCallingNext)
{
    EnumInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(Direction::BUY)
        .bCount(1)
        .next()
        .c(Direction::SELL);
    encoder.checkEncodingIsComplete();

    EnumInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        EnumInsideGroup::sbeBlockLength(),
        EnumInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), Direction::BUY);
    EnumInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                b.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelEnum)
{
    EnumInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(Direction::BUY)
        .bCount(1)
        .next()
        .c(Direction::SELL);

    encoder.a(Direction::SELL);
    encoder.checkEncodingIsComplete();

    EnumInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        EnumInsideGroup::sbeBlockLength(),
        EnumInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), Direction::SELL);
    EnumInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), Direction::SELL);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingBitSetInsideGroupInSchemaDefinedOrder)
{
    BitSetInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Flags &a = encoder.a();
    a.cheese(true).guacamole(true).sourCream(false);
    BitSetInsideGroup::B &b = encoder.bCount(1)
        .next();
    Flags &c = b.c();
    c.cheese(false).guacamole(false).sourCream(true);
    encoder.checkEncodingIsComplete();

    BitSetInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        BitSetInsideGroup::sbeBlockLength(),
        BitSetInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Flags &aFlags = decoder.a();
    EXPECT_EQ(aFlags.guacamole(), true);
    EXPECT_EQ(aFlags.cheese(), true);
    EXPECT_EQ(aFlags.sourCream(), false);
    BitSetInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    Flags &cFlags = bGroup.next().c();
    EXPECT_EQ(cFlags.guacamole(), false);
    EXPECT_EQ(cFlags.cheese(), false);
    EXPECT_EQ(cFlags.sourCream(), true);
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingBitSetInsideGroupBeforeCallingNext)
{
    BitSetInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Flags &a = encoder.a();
    a.cheese(true).guacamole(true).sourCream(false);
    BitSetInsideGroup::B &b = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                b.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingBitSetInsideGroupBeforeCallingNext)
{
    BitSetInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Flags &a = encoder.a();
    a.cheese(true).guacamole(true).sourCream(false);
    BitSetInsideGroup::B &b = encoder.bCount(1)
        .next();
    Flags &c = b.c();
    c.cheese(false).guacamole(false).sourCream(true);
    encoder.checkEncodingIsComplete();

    BitSetInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        BitSetInsideGroup::sbeBlockLength(),
        BitSetInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Flags &aFlags = decoder.a();
    EXPECT_EQ(aFlags.guacamole(), true);
    EXPECT_EQ(aFlags.cheese(), true);
    EXPECT_EQ(aFlags.sourCream(), false);
    BitSetInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                bGroup.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelBitSetViaReWrap)
{
    BitSetInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    Flags &a = encoder.a();
    a.cheese(true).guacamole(true).sourCream(false);
    BitSetInsideGroup::B &b = encoder.bCount(1)
        .next();
    Flags &c = b.c();
    c.cheese(false).guacamole(false).sourCream(true);

    Flags &aPrime = encoder.a();
    aPrime.sourCream(true);
    encoder.checkEncodingIsComplete();

    BitSetInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        BitSetInsideGroup::sbeBlockLength(),
        BitSetInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    Flags &aFlags = decoder.a();
    EXPECT_EQ(aFlags.guacamole(), true);
    EXPECT_EQ(aFlags.cheese(), true);
    EXPECT_EQ(aFlags.sourCream(), true);
    BitSetInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    Flags &cFlags = bGroup.next().c();
    EXPECT_EQ(cFlags.guacamole(), false);
    EXPECT_EQ(cFlags.cheese(), false);
    EXPECT_EQ(cFlags.sourCream(), true);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingArrayInsideGroupInSchemaDefinedOrder)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &b = encoder.bCount(1)
        .next();
    b.putC(5, 6, 7, 8);
    encoder.checkEncodingIsComplete();

    ArrayInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        ArrayInsideGroup::sbeBlockLength(),
        ArrayInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(0), 1);
    EXPECT_EQ(decoder.a(1), 2);
    EXPECT_EQ(decoder.a(2), 3);
    EXPECT_EQ(decoder.a(3), 4);
    ArrayInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    bGroup.next();
    EXPECT_EQ(bGroup.c(0), 5);
    EXPECT_EQ(bGroup.c(1), 6);
    EXPECT_EQ(bGroup.c(2), 7);
    EXPECT_EQ(bGroup.c(3), 8);
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingArrayInsideGroupBeforeCallingNext1)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bEncoder.putC(5, 6, 7, 8);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_B_N"));
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingArrayInsideGroupBeforeCallingNext2)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    const char c[4] = {5, 6, 7, 8};
    const char *cPtr = c;
    EXPECT_THROW(
        {
            try
            {
                bEncoder.putC(cPtr);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_B_N"));
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingArrayInsideGroupBeforeCallingNext1)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.putC(5, 6, 7, 8);
    encoder.checkEncodingIsComplete();

    ArrayInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        ArrayInsideGroup::sbeBlockLength(),
        ArrayInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(0), 1);
    EXPECT_EQ(decoder.a(1), 2);
    EXPECT_EQ(decoder.a(2), 3);
    EXPECT_EQ(decoder.a(3), 4);
    ArrayInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                bGroup.c(0);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_B_N"));
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingArrayInsideGroupBeforeCallingNext2)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.putC(5, 6, 7, 8);
    encoder.checkEncodingIsComplete();

    ArrayInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        ArrayInsideGroup::sbeBlockLength(),
        ArrayInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(0), 1);
    EXPECT_EQ(decoder.a(1), 2);
    EXPECT_EQ(decoder.a(2), 3);
    EXPECT_EQ(decoder.a(3), 4);
    ArrayInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                const std::uint64_t charCount = 4;
                char c[charCount];
                bGroup.getC(c, charCount);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_B_N"));
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingArrayInsideGroupBeforeCallingNext3)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.putC(5, 6, 7, 8);
    encoder.checkEncodingIsComplete();

    ArrayInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        ArrayInsideGroup::sbeBlockLength(),
        ArrayInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(0), 1);
    EXPECT_EQ(decoder.a(1), 2);
    EXPECT_EQ(decoder.a(2), 3);
    EXPECT_EQ(decoder.a(3), 4);
    ArrayInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                bGroup.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_B_N"));
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelArray)
{
    ArrayInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA(1, 2, 3, 4);
    ArrayInsideGroup::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.putC(5, 6, 7, 8);

    encoder.putA(9, 10, 11, 12);
    encoder.checkEncodingIsComplete();

    ArrayInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        ArrayInsideGroup::sbeBlockLength(),
        ArrayInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(0), 9);
    EXPECT_EQ(decoder.a(1), 10);
    EXPECT_EQ(decoder.a(2), 11);
    EXPECT_EQ(decoder.a(3), 12);
    ArrayInsideGroup::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    bGroup.next();
    EXPECT_EQ(bGroup.c(0), 5);
    EXPECT_EQ(bGroup.c(1), 6);
    EXPECT_EQ(bGroup.c(2), 7);
    EXPECT_EQ(bGroup.c(3), 8);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder1)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.bCount(0);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    MultipleGroups::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 0u);
    MultipleGroups::D &dGroup = decoder.d();
    EXPECT_EQ(dGroup.count(), 1u);
    dGroup.next();
    EXPECT_EQ(dGroup.e(), 43);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder2)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 41);
    MultipleGroups::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    bGroup.next();
    EXPECT_EQ(bGroup.c(), 42);
    MultipleGroups::D &dGroup = decoder.d();
    EXPECT_EQ(dGroup.count(), 1u);
    dGroup.next();
    EXPECT_EQ(dGroup.e(), 43);
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelPrimitiveFieldsAfterGroups)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.a(44);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 44);
    MultipleGroups::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    bGroup.next();
    EXPECT_EQ(bGroup.c(), 42);
    MultipleGroups::D &dGroup = decoder.d();
    EXPECT_EQ(dGroup.count(), 1u);
    dGroup.next();
    EXPECT_EQ(dGroup.e(), 43);
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedEncodingOfGroupField)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    EXPECT_THROW(
        {
            try
            {
                encoder.dCount(0);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot encode count of repeating group \"d\" in state: V0_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingEarlierGroupFields)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    EXPECT_THROW(
        {
            try
            {
                encoder.bCount(1);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot encode count of repeating group \"b\" in state: V0_D_1_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, disallowsReEncodingLatestGroupField)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    EXPECT_THROW(
        {
            try
            {
                encoder.dCount(1);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot encode count of repeating group \"d\" in state: V0_D_1_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfGroupField)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 41);
    EXPECT_THROW(
        {
            try
            {
                decoder.d();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot decode count of repeating group \"d\" in state: V0_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingEarlierGroupField)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 41);
    MultipleGroups::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_EQ(bGroup.next().c(), 42);
    MultipleGroups::D &dGroup = decoder.d();
    EXPECT_EQ(dGroup.count(), 1u);
    EXPECT_EQ(dGroup.next().e(), 43);
    EXPECT_THROW(
        {
            try
            {
                decoder.b();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot decode count of repeating group \"b\" in state: V0_D_1_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, disallowsReDecodingLatestGroupField)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(41);
    MultipleGroups::B &bEncoder = encoder.bCount(1);
    bEncoder.next();
    bEncoder.c(42);
    MultipleGroups::D &dEncoder = encoder.dCount(1);
    dEncoder.next();
    dEncoder.e(43);
    encoder.checkEncodingIsComplete();

    MultipleGroups decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        MultipleGroups::sbeBlockLength(),
        MultipleGroups::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 41);
    MultipleGroups::B &bGroup = decoder.b();
    EXPECT_EQ(bGroup.count(), 1u);
    EXPECT_EQ(bGroup.next().c(), 42);
    MultipleGroups::D &dGroup = decoder.d();
    EXPECT_EQ(dGroup.count(), 1u);
    EXPECT_EQ(dGroup.next().e(), 43);
    EXPECT_THROW(
        {
            try
            {
                decoder.d();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot decode count of repeating group \"d\" in state: V0_D_1_BLOCK")
                );
                throw;
            }
        }, std::logic_error);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedVarData)
{
    AddVarDataV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.putB("abc");
    encoder.checkEncodingIsComplete();

    AddVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddVarDataV1::sbeBlockLength(),
        AddVarDataV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue1)
{
    AddVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.checkEncodingIsComplete();

    AddVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddVarDataV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.b(), nullptr);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue2)
{
    AddVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.checkEncodingIsComplete();

    AddVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddVarDataV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsString(), "");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue3)
{
    AddVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.checkEncodingIsComplete();

    AddVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddVarDataV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    EXPECT_EQ(decoder.getBAsJsonEscapedString(), "");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue4)
{
    AddVarDataV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    encoder.checkEncodingIsComplete();

    AddVarDataV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddVarDataV1::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 42);
    char b[16];
    EXPECT_EQ(decoder.getB(b, 16), 0u);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder1)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().getCAsString(), "EURUSD");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder2)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    std::string a("GBPUSD");
    encoder.putA(a);
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next()
        .c(0, 'E')
        .c(1, 'U')
        .c(2, 'R')
        .c(3, 'U')
        .c(4, 'S')
        .c(5, 'D');
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    char aOut[6];
    decoder.getA(aOut, 6);
    EXPECT_STREQ(aOut, "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(0), 'E');
    EXPECT_EQ(b.c(1), 'U');
    EXPECT_EQ(b.c(2), 'R');
    EXPECT_EQ(b.c(3), 'U');
    EXPECT_EQ(b.c(4), 'S');
    EXPECT_EQ(b.c(5), 'D');
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingAsciiInsideGroupBeforeCallingNext1)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bGroup.putC("EURUSD");
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingAsciiInsideGroupBeforeCallingNext2)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                const std::string c("EURUSD");
                bGroup.putC(c);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingAsciiInsideGroupBeforeCallingNext3)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    EXPECT_THROW(
        {
            try
            {
                bGroup.c(0, 'E')
                    .c(1, 'U')
                    .c(2, 'R')
                    .c(3, 'U')
                    .c(4, 'S')
                    .c(5, 'D');
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingAsciiInsideGroupBeforeCallingNext1)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                b.getCAsString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingAsciiInsideGroupBeforeCallingNext2)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                b.c();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingAsciiInsideGroupBeforeCallingNext3)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                char c[6];
                b.getC(c, 6);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingAsciiInsideGroupBeforeCallingNext4)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                b.c(0);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsDecodingAsciiInsideGroupBeforeCallingNext5)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "GBPUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_THROW(
        {
            try
            {
                b.getCAsJsonEscapedString();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(
                    e.what(),
                    HasSubstr("Cannot access field \"b.c\" in state: V0_B_N")
                );
                throw;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsReEncodingTopLevelAscii)
{
    AsciiInsideGroup encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("GBPUSD");
    AsciiInsideGroup::B &bGroup = encoder.bCount(1);
    bGroup.next().putC("EURUSD");

    encoder.putA("CADUSD");
    encoder.checkEncodingIsComplete();

    AsciiInsideGroup decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AsciiInsideGroup::sbeBlockLength(),
        AsciiInsideGroup::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "CADUSD");
    AsciiInsideGroup::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    b.next();
    EXPECT_EQ(b.getCAsString(), "EURUSD");
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup1)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    EXPECT_EQ(decoder.getDAsString(), "EURUSD");
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup2)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    const char *d = decoder.d();
    const std::string dAsString(d, 6);
    ASSERT_EQ(dAsString, "EURUSD");
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup3)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    const AddAsciiBeforeGroupV1 &constDecoder = decoder;
    EXPECT_EQ(constDecoder.a(), 1);
    const char *d = constDecoder.d();
    const std::string dAsString(d, 6);
    ASSERT_EQ(dAsString, "EURUSD");
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup4)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    char d[6];
    decoder.getD(d, 6);
    const std::string dAsString(d, 6);
    ASSERT_EQ(dAsString, "EURUSD");
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup5)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    ASSERT_EQ(decoder.d(0), 'E');
    ASSERT_EQ(decoder.d(1), 'U');
    ASSERT_EQ(decoder.d(2), 'R');
    ASSERT_EQ(decoder.d(3), 'U');
    ASSERT_EQ(decoder.d(4), 'S');
    ASSERT_EQ(decoder.d(5), 'D');
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup6)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    ASSERT_EQ(decoder.dLength(), 6u);
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue1)
{
    AddAsciiBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    AddAsciiBeforeGroupV0::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    ASSERT_EQ(decoder.d(0), AddAsciiBeforeGroupV1::dNullValue());
    ASSERT_EQ(decoder.d(1), AddAsciiBeforeGroupV1::dNullValue());
    ASSERT_EQ(decoder.d(2), AddAsciiBeforeGroupV1::dNullValue());
    ASSERT_EQ(decoder.d(3), AddAsciiBeforeGroupV1::dNullValue());
    ASSERT_EQ(decoder.d(4), AddAsciiBeforeGroupV1::dNullValue());
    ASSERT_EQ(decoder.d(5), AddAsciiBeforeGroupV1::dNullValue());
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue2)
{
    AddAsciiBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    AddAsciiBeforeGroupV0::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    char d[6];
    ASSERT_EQ(decoder.getD(d, 6), 0u);
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue3)
{
    AddAsciiBeforeGroupV0 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    AddAsciiBeforeGroupV0::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV0::sbeBlockLength(),
        0,
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    ASSERT_EQ(decoder.getDAsString(), "");
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsNewDecoderToSkipPresentButAddedAsciiFieldBeforeGroup)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV1 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddAsciiBeforeGroupV1::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsOldDecoderToSkipAddedAsciiFieldBeforeGroup)
{
    AddAsciiBeforeGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    encoder.putD("EURUSD");
    AddAsciiBeforeGroupV1::B &bGroup = encoder.bCount(1);
    bGroup.next().c(2);
    encoder.checkEncodingIsComplete();

    AddAsciiBeforeGroupV0 decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        AddAsciiBeforeGroupV1::sbeBlockLength(),
        AddAsciiBeforeGroupV1::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    AddAsciiBeforeGroupV0::B &b = decoder.b();
    EXPECT_EQ(b.count(), 1u);
    EXPECT_EQ(b.next().c(), 2);
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodeAndDecodeOfMessagesWithNoBlock)
{
    NoBlock encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.putA("abc");
    encoder.checkEncodingIsComplete();

    NoBlock decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NoBlock::sbeBlockLength(),
        NoBlock::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.getAAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodeAndDecodeOfGroupsWithNoBlock)
{
    GroupWithNoBlock encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    GroupWithNoBlock::A &aGroup = encoder.aCount(1);
    aGroup.next().putB("abc");
    encoder.checkEncodingIsComplete();

    GroupWithNoBlock decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        GroupWithNoBlock::sbeBlockLength(),
        GroupWithNoBlock::sbeSchemaVersion(),
        BUFFER_LEN
    );
    GroupWithNoBlock::A &a = decoder.a();
    EXPECT_EQ(a.count(), 1u);
    EXPECT_EQ(a.next().getBAsString(), "abc");
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingElementOfEmptyGroup1)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    MultipleGroups::B &bGroup = encoder.bCount(0);
    MultipleGroups::D &dGroup = encoder.dCount(1);
    dGroup.next().e(43);

    EXPECT_THROW(
        {
            try
            {
                bGroup.c(44);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_D_1_BLOCK"));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingElementOfEmptyGroup2)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    NestedGroups::B &bGroup = encoder.bCount(1);
    bGroup.next();
    bGroup.c(43);
    NestedGroups::B::D &dGroup = bGroup.dCount(0);
    bGroup.fCount(0);
    encoder.hCount(0);

    EXPECT_THROW(
        {
            try
            {
                dGroup.e(44);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.d.e\" in state: V0_H_DONE"));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingElementOfEmptyGroup3)
{
    NestedGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    NestedGroups::B &bGroup = encoder.bCount(1);
    bGroup.next();
    bGroup.c(43);
    NestedGroups::B::D &dGroup = bGroup.dCount(0);
    bGroup.fCount(0);

    EXPECT_THROW(
        {
            try
            {
                dGroup.e(44);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.d.e\" in state: V0_B_1_F_DONE"));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingElementOfEmptyGroup4)
{
    AddPrimitiveInsideGroupV1 encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    AddPrimitiveInsideGroupV1::B &bGroup = encoder.bCount(0);

    EXPECT_THROW(
        {
            try
            {
                bGroup.c(43);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V1_B_DONE"));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsEncodingElementOfEmptyGroup5)
{
    GroupAndVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(42);
    GroupAndVarLength::B &bGroup = encoder.bCount(0);
    encoder.putD("abc");

    EXPECT_THROW(
        {
            try
            {
                bGroup.c(43);
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr("Cannot access field \"b.c\" in state: V0_D_DONE"));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, allowsEncodingAndDecodingNestedGroupWithVarDataInSchemaDefinedOrder)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(3);
    bGroup.next().c(2).dCount(0);
    bGroup.next().c(3).dCount(1).next().e(4).putF("abc");
    bGroup.next().c(5).dCount(2).next().e(6).putF("def").next().e(7).putF("ghi");
    encoder.checkEncodingIsComplete();

    NestedGroupWithVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroupWithVarLength::sbeBlockLength(),
        NestedGroupWithVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    NestedGroupWithVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 3u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    NestedGroupWithVarLength::B::D &dDecoder1 = bDecoder.d();
    EXPECT_EQ(dDecoder1.count(), 0u);
    EXPECT_EQ(bDecoder.next().c(), 3);
    NestedGroupWithVarLength::B::D &dDecoder2 = bDecoder.d();
    EXPECT_EQ(dDecoder2.count(), 1u);
    EXPECT_EQ(dDecoder2.next().e(), 4);
    EXPECT_EQ(dDecoder2.getFAsString(), "abc");
    EXPECT_EQ(bDecoder.next().c(), 5);
    NestedGroupWithVarLength::B::D &dDecoder3 = bDecoder.d();
    EXPECT_EQ(dDecoder3.count(), 2u);
    EXPECT_EQ(dDecoder3.next().e(), 6);
    EXPECT_EQ(dDecoder3.getFAsString(), "def");
    EXPECT_EQ(dDecoder3.next().e(), 7);
    EXPECT_EQ(dDecoder3.getFAsString(), "ghi");
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedEncodingOfVarLengthFieldInNestedGroupToNextInnerElement1)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(1);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(5).dCount(2);
    dGroup.next().e(7);

    EXPECT_THROW(
        {
            try
            {
                dGroup.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b.d\" in state: V0_B_1_D_N_BLOCK"));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedEncodingOfVarLengthFieldInNestedGroupToNextInnerElement2)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(2);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(5).dCount(2);
    dGroup.next().e(7);

    EXPECT_THROW(
        {
            try
            {
                dGroup.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b.d\" in state: V0_B_N_D_N_BLOCK"));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement1)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(1);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(2).dCount(2);
    dGroup.next().e(3).putF("abc");
    dGroup.next().e(4).putF("def");
    encoder.checkEncodingIsComplete();

    NestedGroupWithVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroupWithVarLength::sbeBlockLength(),
        NestedGroupWithVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    NestedGroupWithVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 1u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    NestedGroupWithVarLength::B::D &dDecoder = bDecoder.d();
    EXPECT_EQ(dDecoder.count(), 2u);
    EXPECT_EQ(dDecoder.next().e(), 3);

    EXPECT_THROW(
        {
            try
            {
                dDecoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b.d\" in state: V0_B_1_D_N_BLOCK."));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement2)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(2);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(2).dCount(2);
    dGroup.next().e(3).putF("abc");
    dGroup.next().e(4).putF("def");
    bGroup.next().c(5).dCount(0);
    encoder.checkEncodingIsComplete();

    NestedGroupWithVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroupWithVarLength::sbeBlockLength(),
        NestedGroupWithVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    NestedGroupWithVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    NestedGroupWithVarLength::B::D &dDecoder = bDecoder.d();
    EXPECT_EQ(dDecoder.count(), 2u);
    EXPECT_EQ(dDecoder.next().e(), 3);

    EXPECT_THROW(
        {
            try
            {
                dDecoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b.d\" in state: V0_B_N_D_N_BLOCK."));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement1)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(2);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(2).dCount(2);
    dGroup.next().e(3).putF("abc");
    dGroup.next().e(4).putF("def");
    bGroup.next().c(5).dCount(0);
    encoder.checkEncodingIsComplete();

    NestedGroupWithVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroupWithVarLength::sbeBlockLength(),
        NestedGroupWithVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    NestedGroupWithVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    NestedGroupWithVarLength::B::D &dDecoder = bDecoder.d();
    EXPECT_EQ(dDecoder.count(), 2u);
    EXPECT_EQ(dDecoder.next().e(), 3);

    EXPECT_THROW(
        {
            try
            {
                bDecoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b\" in state: V0_B_N_D_N_BLOCK."));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement2)
{
    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);
    NestedGroupWithVarLength::B &bGroup = encoder.bCount(2);
    NestedGroupWithVarLength::B::D &dGroup = bGroup.next().c(2).dCount(1);
    dGroup.next().e(3).putF("abc");
    bGroup.next().c(5).dCount(0);
    encoder.checkEncodingIsComplete();

    NestedGroupWithVarLength decoder;
    decoder.wrapForDecode(
        m_buffer,
        OFFSET,
        NestedGroupWithVarLength::sbeBlockLength(),
        NestedGroupWithVarLength::sbeSchemaVersion(),
        BUFFER_LEN
    );
    EXPECT_EQ(decoder.a(), 1);
    NestedGroupWithVarLength::B &bDecoder = decoder.b();
    EXPECT_EQ(bDecoder.count(), 2u);
    EXPECT_EQ(bDecoder.next().c(), 2);
    NestedGroupWithVarLength::B::D &dDecoder = bDecoder.d();
    EXPECT_EQ(dDecoder.count(), 1u);
    EXPECT_EQ(dDecoder.next().e(), 3);

    EXPECT_THROW(
        {
            try
            {
                bDecoder.next();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Cannot access next element in repeating group \"b\" in state: V0_B_N_D_1_BLOCK."));
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsIncompleteMessagesDueToMissingVarLengthField1)
{
    MultipleVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).putB("abc");

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                std::string("Not fully encoded, current state: V0_B_DONE, ") +
                                "allowed transitions: \"cLength()\", \"c(?)\""));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsIncompleteMessagesDueToMissingVarLengthField2)
{
    NoBlock encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(),
                            testing::HasSubstr(
                                std::string("Not fully encoded, current state: V0_BLOCK, ") +
                                "allowed transitions: \"aLength()\", \"a(?)\""));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsIncompleteMessagesDueToMissingTopLevelGroup1)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(0);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: V0_B_DONE, allowed transitions:"
                    " \"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\""));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsIncompleteMessagesDueToMissingTopLevelGroup2)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(1).next().c(2);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: V0_B_1_BLOCK, allowed transitions: "
                    "\"b.c(?)\", \"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\""));
                throw e;
            }
        },
        std::logic_error
    );
}

TEST_F(FieldAccessOrderCheckTest, disallowsIncompleteMessagesDueToMissingTopLevelGroup3)
{
    MultipleGroups encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: V0_BLOCK, allowed transitions: "
                    "\"a(?)\", \"bCount(0)\", \"bCount(>0)\""));
                throw e;
            }
        },
        std::logic_error
    );
}

class DisallowsIncompleteMessagesPart1Test : public FieldAccessOrderCheckTest,
                                             public testing::WithParamInterface<std::tuple<int, std::string>>
{
};

TEST_P(DisallowsIncompleteMessagesPart1Test, disallowsIncompleteMessagesDueToMissingNestedGroup1)
{
    const auto bCount = std::get<0>(GetParam());
    const auto expectedState = std::get<1>(GetParam());

    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(bCount).next().c(2);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: " + expectedState));
                throw e;
            }
        },
        std::logic_error
    );
}

INSTANTIATE_TEST_SUITE_P(
    FieldAccessOrderCheckTest,
    DisallowsIncompleteMessagesPart1Test,
    testing::Values(
        std::make_tuple(1, "V0_B_1_BLOCK"),
        std::make_tuple(2, "V0_B_N_BLOCK")
    )
);

class DisallowsIncompleteMessagesPart2Test : public FieldAccessOrderCheckTest,
                                             public testing::WithParamInterface<std::tuple<int, int, std::string>>
{
};

TEST_P(DisallowsIncompleteMessagesPart2Test, disallowsIncompleteMessagesDueToMissingNestedGroup2)
{
    const auto bCount = std::get<0>(GetParam());
    const auto dCount = std::get<1>(GetParam());
    const auto expectedState = std::get<2>(GetParam());

    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(bCount).next().c(2).dCount(dCount);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: " + expectedState));
                throw e;
            }
        },
        std::logic_error
    );
}

INSTANTIATE_TEST_SUITE_P(
    FieldAccessOrderCheckTest,
    DisallowsIncompleteMessagesPart2Test,
    testing::Values(
        std::make_tuple(1, 1, "V0_B_1_D_N"),
        std::make_tuple(1, 2, "V0_B_1_D_N"),
        std::make_tuple(2, 0, "V0_B_N_D_DONE"),
        std::make_tuple(2, 1, "V0_B_N_D_N"),
        std::make_tuple(2, 2, "V0_B_N_D_N")
    )
);

class DisallowsIncompleteMessagesPart3Test : public FieldAccessOrderCheckTest,
                                             public testing::WithParamInterface<std::tuple<int, int, std::string>>
{
};

TEST_P(DisallowsIncompleteMessagesPart3Test, disallowsIncompleteMessagesDueToMissingVarDataInNestedGroup)
{
    const auto bCount = std::get<0>(GetParam());
    const auto dCount = std::get<1>(GetParam());
    const auto expectedState = std::get<2>(GetParam());

    NestedGroupWithVarLength encoder;
    encoder.wrapForEncode(m_buffer, OFFSET, BUFFER_LEN);
    encoder.a(1).bCount(bCount).next().c(2).dCount(dCount).next().e(10);

    EXPECT_THROW(
        {
            try
            {
                encoder.checkEncodingIsComplete();
            }
            catch (const std::logic_error &e)
            {
                EXPECT_THAT(e.what(), testing::HasSubstr(
                    "Not fully encoded, current state: " + expectedState));
                throw e;
            }
        },
        std::logic_error
    );
}

INSTANTIATE_TEST_SUITE_P(
    FieldAccessOrderCheckTest,
    DisallowsIncompleteMessagesPart3Test,
    testing::Values(
        std::make_tuple(1, 1, "V0_B_1_D_1_BLOCK"),
        std::make_tuple(1, 2, "V0_B_1_D_N_BLOCK"),
        std::make_tuple(2, 1, "V0_B_N_D_1_BLOCK"),
        std::make_tuple(2, 2, "V0_B_N_D_N_BLOCK")
    )
);
