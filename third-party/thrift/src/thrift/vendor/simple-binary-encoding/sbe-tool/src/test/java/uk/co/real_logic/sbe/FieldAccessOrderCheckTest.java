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

package uk.co.real_logic.sbe;

import order.check.*;
import org.agrona.ExpandableArrayBuffer;
import org.agrona.MutableDirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.CsvSource;

import java.nio.charset.StandardCharsets;
import java.util.Random;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assumptions.assumeFalse;

public class FieldAccessOrderCheckTest
{
    private static final Class<IllegalStateException> INCORRECT_ORDER_EXCEPTION_CLASS = IllegalStateException.class;
    private static final int OFFSET = 0;
    private final MutableDirectBuffer buffer = new ExpandableArrayBuffer();
    private final MessageHeaderEncoder messageHeaderEncoder = new MessageHeaderEncoder();
    private final MessageHeaderDecoder messageHeaderDecoder = new MessageHeaderDecoder();
    private final Random random = new Random();

    @BeforeAll
    static void assumeDebugMode()
    {
        final boolean productionMode = Boolean.getBoolean("agrona.disable.bounds.checks");
        assumeFalse(productionMode);
    }

    @BeforeEach
    void setUp()
    {
        random.nextBytes(buffer.byteArray());
    }

    @Test
    void allowsEncodingAndDecodingVariableLengthFieldsInSchemaDefinedOrder()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        encoder.checkEncodingIsComplete();

        final MultipleVarLengthDecoder decoder = new MultipleVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
        assertThat(decoder.toString(), containsString("a=42|b='abc'|c='def'"));
    }

    @Test
    void allowsDecodingVariableLengthFieldsAfterRewind()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        encoder.checkEncodingIsComplete();

        final MultipleVarLengthDecoder decoder = new MultipleVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
        assertThat(decoder.toString(), containsString("a=42|b='abc'|c='def'"));

        decoder.sbeRewind();

        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
        assertThat(decoder.toString(), containsString("a=42|b='abc'|c='def'"));
    }

    @Test
    void allowsDecodingToSkipVariableLengthFields()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        encoder.checkEncodingIsComplete();

        final MultipleVarLengthDecoder decoder = new MultipleVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.skipB(), equalTo(3));
        assertThat(decoder.c(), equalTo("def"));
        assertThat(decoder.toString(), containsString("a=42|b='abc'|c='def'"));
    }

    @Test
    void allowsReEncodingTopLevelPrimitiveFields()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        encoder.a(43);
        encoder.checkEncodingIsComplete();

        final MultipleVarLengthDecoder decoder = new MultipleVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(43));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
    }

    @Test
    void disallowsSkippingEncodingOfVariableLengthField1()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.c("def"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingEncodingOfVariableLengthField2()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final CharSequence def = new StringBuilder("def");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.c(def));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingEncodingOfVariableLengthField3()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final byte[] value = "def".getBytes(StandardCharsets.US_ASCII);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.putC(value, 0, value.length));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingEncodingOfVariableLengthField4()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final byte[] value = "def".getBytes(StandardCharsets.US_ASCII);
        final UnsafeBuffer buffer = new UnsafeBuffer(value);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.putC(buffer, 0, buffer.capacity()));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsReEncodingEarlierVariableLengthFields()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.b("ghi"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b\" in state: V0_C_DONE"));
    }

    @Test
    void disallowsReEncodingLatestVariableLengthField()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.c("ghi"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_C_DONE"));
    }

    @Test
    void disallowsSkippingDecodingOfVariableLengthField1()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();

        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingDecodingOfVariableLengthField2()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> decoder.wrapC(new UnsafeBuffer()));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingDecodingOfVariableLengthField3()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> decoder.getC(new StringBuilder()));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingDecodingOfVariableLengthField4()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> decoder.getC(new byte[3], 0, 3));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsSkippingDecodingOfVariableLengthField5()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> decoder.getC(new ExpandableArrayBuffer(), 0, 3));
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_BLOCK"));
    }

    @Test
    void allowsRepeatedDecodingOfVariableLengthDataLength()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        assertThat(decoder.bLength(), equalTo(3));
        assertThat(decoder.bLength(), equalTo(3));
        assertThat(decoder.bLength(), equalTo(3));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.cLength(), equalTo(3));
        assertThat(decoder.cLength(), equalTo(3));
        assertThat(decoder.cLength(), equalTo(3));
    }

    @Test
    void disallowsReDecodingEarlierVariableLengthField()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::b);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b\" in state: V0_C_DONE"));
    }

    @Test
    void disallowsReDecodingLatestVariableLengthField()
    {
        final MultipleVarLengthDecoder decoder = decodeUntilVarLengthFields();
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.c(), equalTo("def"));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"c\" in state: V0_C_DONE"));
    }

    private MultipleVarLengthDecoder decodeUntilVarLengthFields()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.c("def");
        encoder.checkEncodingIsComplete();

        final MultipleVarLengthDecoder decoder = new MultipleVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        return decoder;
    }

    @Test
    void allowsEncodingAndDecodingGroupAndVariableLengthFieldsInSchemaDefinedOrder()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[(c=1),(c=2)]|d='abc'"));
    }

    @Test
    void allowsEncoderToResetZeroGroupLengthToZero()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(0).resetCountToIndex();
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(0));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[]|d='abc'"));
    }

    @Test
    void allowsEncoderToResetNonZeroGroupLengthToZeroBeforeCallingNext()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2).resetCountToIndex();
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(0));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[]|d='abc'"));
    }

    @Test
    void allowsEncoderToResetNonZeroGroupLengthToNonZero()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2).next().c(43).resetCountToIndex();
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(1));
        assertThat(bs.next().c(), equalTo(43));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[(c=43)]|d='abc'"));
    }

    @Test
    void disallowsEncoderToResetGroupLengthMidGroupElement()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final NestedGroupsEncoder.BEncoder bEncoder = encoder.bCount(2).next().c(43);
        final IllegalStateException exception = assertThrows(IllegalStateException.class, bEncoder::resetCountToIndex);
        assertThat(exception.getMessage(),
            containsString("Cannot reset count of repeating group \"b\" in state: V0_B_N_BLOCK"));
    }

    @Test
    void allowsDecodingGroupAndVariableLengthFieldsAfterRewind()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[(c=1),(c=2)]|d='abc'"));

        decoder.sbeRewind();

        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs2 = decoder.b();
        assertThat(bs2.count(), equalTo(2));
        assertThat(bs2.next().c(), equalTo(1));
        assertThat(bs2.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[(c=1),(c=2)]|d='abc'"));
    }

    @Test
    void allowsDecodingToSkipMessage()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final int nextEncodeOffset = encoder.limit();
        encoder.wrapAndApplyHeader(buffer, nextEncodeOffset, messageHeaderEncoder);
        encoder.a(43);
        encoder.bCount(2)
            .next()
            .c(3)
            .next()
            .c(4);
        encoder.d("def");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);

        decoder.sbeSkip();
        final int nextDecodeOffset = decoder.limit();
        assertThat(nextDecodeOffset, equalTo(nextEncodeOffset));

        decoder.wrapAndApplyHeader(buffer, nextDecodeOffset, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(43));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(3));
        assertThat(bs.next().c(), equalTo(4));
        assertThat(decoder.d(), equalTo("def"));
    }

    @Test
    void allowsDecodingToDetermineMessageLengthBeforeReadingFields()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(43);
        encoder.bCount(2)
            .next()
            .c(3)
            .next()
            .c(4);
        encoder.d("def");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);

        assertThat(decoder.sbeDecodedLength(), equalTo(18));
        assertThat(decoder.a(), equalTo(43));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(3));
        assertThat(bs.next().c(), equalTo(4));
        assertThat(decoder.d(), equalTo("def"));
    }

    @Test
    void allowsEncodingAndDecodingEmptyGroupAndVariableLengthFieldsInSchemaDefinedOrder()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(0);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(0));
        assertThat(decoder.d(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=42|b=[]|d='abc'"));
    }

    @Test
    @Disabled("Our access checks are too strict to allow the behaviour in this test.")
    void allowsReEncodingPrimitiveFieldInGroupElementAfterTopLevelVariableLengthField()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final GroupAndVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        bEncoder.c(3);
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(3));
        assertThat(decoder.d(), equalTo("abc"));
    }

    @Test
    @Disabled("Our access checks are too strict to allow the behaviour in this test.")
    void allowsReWrappingGroupDecoderAfterAccessingGroupCount()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final GroupAndVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b().count(), equalTo(2));
        final GroupAndVarLengthDecoder.BDecoder b = decoder.b();
        assertThat(b.next().c(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
    }

    @Test
    void disallowsEncodingGroupElementBeforeCallingNext()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final GroupAndVarLengthEncoder.BEncoder bEncoder = encoder.bCount(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(1));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingGroupElementBeforeCallingNext()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bs::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsSkippingEncodingOfGroup()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.d("abc"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"d\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsReEncodingVariableLengthFieldAfterGroup()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.d("def"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"d\" in state: V0_D_DONE"));
    }

    @Test
    void disallowsReEncodingGroupCount()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.bCount(1));
        assertThat(exception.getMessage(),
            containsString("Cannot encode count of repeating group \"b\" in state: V0_D_DONE"));
    }

    @Test
    @Disabled("Our access checks are too strict to allow the behaviour in this test.")
    void allowsReEncodingGroupElementBlockFieldAfterTopLevelVariableLengthField()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final GroupAndVarLengthEncoder.BEncoder b = encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        b.c(3);
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(3));
        assertThat(decoder.d(), equalTo("abc"));
    }

    @Test
    void disallowsMissedDecodingOfGroupBeforeVariableLengthField()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::d);
        assertThat(exception.getMessage(), containsString("Cannot access field \"d\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsReDecodingVariableLengthFieldAfterGroup()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::d);
        assertThat(exception.getMessage(), containsString("Cannot access field \"d\" in state: V0_D_DONE"));
    }

    @Test
    void disallowsReDecodingGroupAfterVariableLengthField()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .next()
            .c(2);
        encoder.d("abc");
        encoder.checkEncodingIsComplete();

        final GroupAndVarLengthDecoder decoder = new GroupAndVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final GroupAndVarLengthDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(decoder.d(), equalTo("abc"));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::b);
        assertThat(exception.getMessage(),
            containsString("Cannot decode count of repeating group \"b\" in state: V0_D_DONE"));
    }

    @Test
    void allowsEncodingAndDecodingVariableLengthFieldInsideGroupInSchemaDefinedOrder()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .d("abc")
            .next()
            .c(2)
            .d("def");
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.d(), equalTo("abc"));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(bs.d(), equalTo("def"));
        assertThat(decoder.e(), equalTo("ghi"));
        assertThat(decoder.toString(), containsString("a=42|b=[(c=1|d='abc'),(c=2|d='def')]|e='ghi'"));
    }

    @Test
    @Disabled("Our access checks are too strict to allow the behaviour in this test.")
    void allowsReEncodingGroupElementPrimitiveFieldAfterElementVariableLengthField()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final VarLengthInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1);
        bEncoder
            .next()
            .c(1)
            .d("abc");
        bEncoder.c(2);
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(1));
        assertThat(bs.next().c(), equalTo(2));
        assertThat(bs.d(), equalTo("abc"));
        assertThat(decoder.e(), equalTo("ghi"));
    }

    @Test
    void disallowsMissedGroupElementVariableLengthFieldToEncodeAtTopLevel()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(1).next().c(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.e("abc"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"e\" in state: V0_B_1_BLOCK"));
    }

    @Test
    void disallowsMissedGroupElementVariableLengthFieldToEncodeNextElement()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final VarLengthInsideGroupEncoder.BEncoder b = encoder.bCount(2)
            .next();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, b::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK"));
    }

    @Test
    void disallowsMissedGroupElementEncoding()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .d("abc");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.e("abc"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"e\" in state: V0_B_N_D_DONE"));
    }

    @Test
    void disallowsReEncodingGroupElementVariableLengthField()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final VarLengthInsideGroupEncoder.BEncoder b = encoder.bCount(1)
            .next()
            .c(1)
            .d("abc");
        encoder.e("def");
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.d("ghi"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.d\" in state: V0_E_DONE"));
    }

    @Test
    void disallowsReDecodingGroupElementVariableLengthField()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .d("abc")
            .next()
            .c(2)
            .d("def");
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        assertThat(bs.d(), equalTo("abc"));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bs::d);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.d\" in state: V0_B_N_D_DONE"));
    }

    @Test
    void disallowsMissedDecodingOfGroupElementVariableLengthFieldToNextElement()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .d("abc")
            .next()
            .c(2)
            .d("def");
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bs::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK"));
    }

    @Test
    void disallowsMissedDecodingOfGroupElementVariableLengthFieldToTopLevel()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(1)
            .next()
            .c(1)
            .d("abc");
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(1));
        assertThat(bs.next().c(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::e);
        assertThat(exception.getMessage(), containsString("Cannot access field \"e\" in state: V0_B_1_BLOCK"));
    }

    @Test
    void disallowsMissedDecodingOfGroupElement()
    {
        final VarLengthInsideGroupEncoder encoder = new VarLengthInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(2)
            .next()
            .c(1)
            .d("abc")
            .next()
            .c(2)
            .d("def");
        encoder.e("ghi");
        encoder.checkEncodingIsComplete();

        final VarLengthInsideGroupDecoder decoder = new VarLengthInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final VarLengthInsideGroupDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        assertThat(bs.next().c(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::e);
        assertThat(exception.getMessage(), containsString("Cannot access field \"e\" in state: V0_B_N_BLOCK"));
    }

    @Test
    void allowsEncodingNestedGroupsInSchemaDefinedOrder()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final NestedGroupsEncoder.BEncoder b = encoder.bCount(2)
            .next();
        b.c(1);
        b.dCount(2)
            .next()
            .e(2)
            .next()
            .e(3);
        b.fCount(1)
            .next()
            .g(4);
        b.next();
        b.c(5);
        b.dCount(1)
            .next()
            .e(6);
        b.fCount(1)
            .next()
            .g(7);
        encoder.hCount(1)
            .next()
            .i(8);
        encoder.checkEncodingIsComplete();

        final NestedGroupsDecoder decoder = new NestedGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final NestedGroupsDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(2));
        final NestedGroupsDecoder.BDecoder b0 = bs.next();
        assertThat(b0.c(), equalTo(1));
        final NestedGroupsDecoder.BDecoder.DDecoder b0ds = b0.d();
        assertThat(b0ds.count(), equalTo(2));
        assertThat(b0ds.next().e(), equalTo(2));
        assertThat(b0ds.next().e(), equalTo(3));
        final NestedGroupsDecoder.BDecoder.FDecoder b0fs = b0.f();
        assertThat(b0fs.count(), equalTo(1));
        assertThat(b0fs.next().g(), equalTo(4));
        final NestedGroupsDecoder.BDecoder b1 = bs.next();
        assertThat(b1.c(), equalTo(5));
        final NestedGroupsDecoder.BDecoder.DDecoder b1ds = b1.d();
        assertThat(b1ds.count(), equalTo(1));
        assertThat(b1ds.next().e(), equalTo(6));
        final NestedGroupsDecoder.BDecoder.FDecoder b1fs = b1.f();
        assertThat(b1fs.count(), equalTo(1));
        assertThat(b1fs.next().g(), equalTo(7));
        final NestedGroupsDecoder.HDecoder hs = decoder.h();
        assertThat(hs.count(), equalTo(1));
        assertThat(hs.next().i(), equalTo(8));
        assertThat(decoder.toString(),
            containsString("a=42|b=[(c=1|d=[(e=2),(e=3)]|f=[(g=4)]),(c=5|d=[(e=6)]|f=[(g=7)])]|h=[(i=8)]"));
    }

    @Test
    void allowsEncodingEmptyNestedGroupsInSchemaDefinedOrder()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(0);
        encoder.hCount(0);
        encoder.checkEncodingIsComplete();

        final NestedGroupsDecoder decoder = new NestedGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        final NestedGroupsDecoder.BDecoder bs = decoder.b();
        assertThat(bs.count(), equalTo(0));
        final NestedGroupsDecoder.HDecoder hs = decoder.h();
        assertThat(hs.count(), equalTo(0));
        assertThat(decoder.toString(), containsString("a=42|b=[]|h=[]"));
    }

    @Test
    void disallowsMissedEncodingOfNestedGroup()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final NestedGroupsEncoder.BEncoder b = encoder.bCount(1)
            .next()
            .c(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.fCount(1));
        assertThat(exception.getMessage(),
            containsString("Cannot encode count of repeating group \"b.f\" in state: V0_B_1_BLOCK"));
    }

    @Test
    void allowsEncodingAndDecodingCompositeInsideGroupInSchemaDefinedOrder()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
        assertThat(decoder.toString(), containsString("a=(x=1|y=2)|b=[(c=(x=3|y=4))]"));
    }

    @Test
    void disallowsEncodingCompositeInsideGroupBeforeCallingNext()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        final CompositeInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bEncoder::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingCompositeInsideGroupBeforeCallingNext()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, b::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void allowsReEncodingTopLevelCompositeViaReWrap()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.a().x(5).y(6);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(5));
        assertThat(a.y(), equalTo(6));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
    }

    @Test
    void allowsReEncodingTopLevelCompositeViaEncoderReference()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        final PointEncoder aEncoder = encoder.a();
        aEncoder.x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        aEncoder.x(5).y(6);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(5));
        assertThat(a.y(), equalTo(6));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
    }

    @Test
    void allowsReEncodingGroupElementCompositeViaReWrap()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        final CompositeInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1).next();
        bEncoder.c().x(3).y(4);
        bEncoder.c().x(5).y(6);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(5));
        assertThat(c.y(), equalTo(6));
    }

    @Test
    void allowsReEncodingGroupElementCompositeViaEncoderReference()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        final CompositeInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1).next();
        final PointEncoder cEncoder = bEncoder.c();
        cEncoder.x(3).y(4);
        cEncoder.x(5).y(6);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(5));
        assertThat(c.y(), equalTo(6));
    }

    @Test
    void allowsReDecodingTopLevelCompositeViaReWrap()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a1 = decoder.a();
        assertThat(a1.x(), equalTo(1));
        assertThat(a1.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
        final PointDecoder a2 = decoder.a();
        assertThat(a2.x(), equalTo(1));
        assertThat(a2.y(), equalTo(2));
    }

    @Test
    void allowsReDecodingTopLevelCompositeViaEncoderReference()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
    }

    @Test
    void allowsReDecodingGroupElementCompositeViaReWrap()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c1 = b.next().c();
        assertThat(c1.x(), equalTo(3));
        assertThat(c1.y(), equalTo(4));
        final PointDecoder c2 = b.c();
        assertThat(c2.x(), equalTo(3));
        assertThat(c2.y(), equalTo(4));
    }

    @Test
    void allowsReDecodingGroupElementCompositeViaEncoderReference()
    {
        final CompositeInsideGroupEncoder encoder = new CompositeInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().x(1).y(2);
        encoder.bCount(1)
            .next()
            .c().x(3).y(4);
        encoder.checkEncodingIsComplete();

        final CompositeInsideGroupDecoder decoder = new CompositeInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final PointDecoder a = decoder.a();
        assertThat(a.x(), equalTo(1));
        assertThat(a.y(), equalTo(2));
        final CompositeInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final PointDecoder c = b.next().c();
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
        assertThat(c.x(), equalTo(3));
        assertThat(c.y(), equalTo(4));
    }

    @Test
    void allowsNewDecoderToDecodeAddedPrimitiveField()
    {
        final AddPrimitiveV1Encoder encoder = new AddPrimitiveV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).b(2);
        encoder.checkEncodingIsComplete();

        final AddPrimitiveV1Decoder decoder = new AddPrimitiveV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.b(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingPrimitiveFieldAsNullValue()
    {
        final AddPrimitiveV0Encoder encoder = new AddPrimitiveV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddPrimitiveV1Decoder decoder = new AddPrimitiveV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.b(), equalTo(AddPrimitiveV1Decoder.bNullValue()));
    }

    @Test
    void allowsNewDecoderToDecodeAddedPrimitiveFieldBeforeGroup()
    {
        final AddPrimitiveBeforeGroupV1Encoder encoder = new AddPrimitiveBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(3).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddPrimitiveBeforeGroupV1Decoder decoder = new AddPrimitiveBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo(3));
        final AddPrimitiveBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingPrimitiveFieldBeforeGroupAsNullValue()
    {
        final AddPrimitiveBeforeGroupV0Encoder encoder = new AddPrimitiveBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddPrimitiveBeforeGroupV1Decoder decoder = new AddPrimitiveBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo(AddPrimitiveBeforeGroupV1Decoder.dNullValue()));
        final AddPrimitiveBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeGroup()
    {
        final AddPrimitiveBeforeGroupV1Encoder encoder = new AddPrimitiveBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(3).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddPrimitiveBeforeGroupV1Decoder decoder = new AddPrimitiveBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedPrimitiveFieldBeforeGroup()
    {
        final AddPrimitiveBeforeGroupV1Encoder encoder = new AddPrimitiveBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(3).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddPrimitiveBeforeGroupV0Decoder decoder = new AddPrimitiveBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeAddedPrimitiveFieldBeforeVarData()
    {
        final AddPrimitiveBeforeVarDataV1Encoder encoder = new AddPrimitiveBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).c(3).b("abc");
        encoder.checkEncodingIsComplete();

        final AddPrimitiveBeforeVarDataV1Decoder decoder = new AddPrimitiveBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.c(), equalTo(3));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsNewDecoderToDecodeMissingPrimitiveFieldBeforeVarDataAsNullValue()
    {
        final AddPrimitiveBeforeVarDataV0Encoder encoder = new AddPrimitiveBeforeVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).b("abc");
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddPrimitiveBeforeVarDataV1Decoder decoder = new AddPrimitiveBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.c(), equalTo(AddPrimitiveBeforeVarDataV1Decoder.cNullValue()));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeVarData()
    {
        final AddPrimitiveBeforeVarDataV1Encoder encoder = new AddPrimitiveBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).c(3).b("abc");
        encoder.checkEncodingIsComplete();

        final AddPrimitiveBeforeVarDataV1Decoder decoder = new AddPrimitiveBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsOldDecoderToSkipAddedPrimitiveFieldBeforeVarData()
    {
        final AddPrimitiveBeforeVarDataV1Encoder encoder = new AddPrimitiveBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).c(3).b("abc");
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddPrimitiveBeforeVarDataV0Decoder decoder = new AddPrimitiveBeforeVarDataV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsNewDecoderToDecodeAddedPrimitiveFieldInsideGroup()
    {
        final AddPrimitiveInsideGroupV1Encoder encoder = new AddPrimitiveInsideGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2).d(3);
        encoder.checkEncodingIsComplete();

        final AddPrimitiveInsideGroupV1Decoder decoder = new AddPrimitiveInsideGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveInsideGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
        assertThat(b.d(), equalTo(3));
    }

    @Test
    void allowsNewDecoderToDecodeMissingPrimitiveFieldInsideGroupAsNullValue()
    {
        final AddPrimitiveInsideGroupV0Encoder encoder = new AddPrimitiveInsideGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddPrimitiveInsideGroupV1Decoder decoder = new AddPrimitiveInsideGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveInsideGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
        assertThat(b.d(), equalTo(AddPrimitiveInsideGroupV1Decoder.BDecoder.dNullValue()));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedPrimitiveFieldInsideGroup()
    {
        final AddPrimitiveInsideGroupV1Encoder encoder = new AddPrimitiveInsideGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(2).next().c(2).d(3).next().c(4).d(5);
        encoder.checkEncodingIsComplete();

        final AddPrimitiveInsideGroupV1Decoder decoder = new AddPrimitiveInsideGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveInsideGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(2));
        assertThat(b.next().c(), equalTo(2));
        assertThat(b.next().c(), equalTo(4));
    }

    @Test
    void allowsOldDecoderToSkipAddedPrimitiveFieldInsideGroup()
    {
        final AddPrimitiveInsideGroupV1Encoder encoder = new AddPrimitiveInsideGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(2).next().c(2).d(3).next().c(4).d(5);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddPrimitiveInsideGroupV0Decoder decoder = new AddPrimitiveInsideGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddPrimitiveInsideGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(2));
        assertThat(b.next().c(), equalTo(2));
        assertThat(b.next().c(), equalTo(4));
    }

    @Test
    void allowsNewDecoderToDecodeAddedGroupBeforeVarData()
    {
        final AddGroupBeforeVarDataV1Encoder encoder = new AddGroupBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).cCount(1).next().d(2);
        encoder.b("abc");
        encoder.checkEncodingIsComplete();

        final AddGroupBeforeVarDataV1Decoder decoder = new AddGroupBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddGroupBeforeVarDataV1Decoder.CDecoder c = decoder.c();
        assertThat(c.count(), equalTo(1));
        assertThat(c.next().d(), equalTo(2));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsNewDecoderToDecodeMissingGroupBeforeVarDataAsNullValue()
    {
        final AddGroupBeforeVarDataV0Encoder encoder = new AddGroupBeforeVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).b("abc");
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddGroupBeforeVarDataV1Decoder decoder = new AddGroupBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddGroupBeforeVarDataV1Decoder.CDecoder c = decoder.c();
        assertThat(c.count(), equalTo(0));
        assertThat(decoder.b(), equalTo("abc"));
        assertThat(decoder.toString(), containsString("a=1|c=[]|b='abc'"));
    }

    @Test
    void allowsNewDecoderToSkipMissingGroupBeforeVarData()
    {
        final AddGroupBeforeVarDataV0Encoder encoder = new AddGroupBeforeVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).b("abc");
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddGroupBeforeVarDataV1Decoder decoder = new AddGroupBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void disallowsNewDecoderToSkipPresentButAddedGroupBeforeVarData()
    {
        final AddGroupBeforeVarDataV1Encoder encoder = new AddGroupBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).cCount(1).next().d(2);
        encoder.b("abc");
        encoder.checkEncodingIsComplete();

        final AddGroupBeforeVarDataV1Decoder decoder = new AddGroupBeforeVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::b);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b\" in state: V1_BLOCK"));
    }

    @Test
    void allowsOldDecoderToSkipAddedGroupBeforeVarData()
    {
        final AddGroupBeforeVarDataV1Encoder encoder = new AddGroupBeforeVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        messageHeaderEncoder.numGroups(1);
        encoder.a(1).cCount(1).next().d(2);
        encoder.b("abc");
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddGroupBeforeVarDataV0Decoder decoder = new AddGroupBeforeVarDataV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));

        for (int i = 0; i < messageHeaderDecoder.numGroups(); i++)
        {
            skipGroup(decoder);
        }

        assertThat(decoder.b(), equalTo("abc"));
    }

    private void skipGroup(final AddGroupBeforeVarDataV0Decoder decoder)
    {
        final GroupSizeEncodingDecoder groupSizeEncodingDecoder = new GroupSizeEncodingDecoder()
            .wrap(buffer, decoder.limit());
        final int bytesToSkip = groupSizeEncodingDecoder.encodedLength() +
            groupSizeEncodingDecoder.blockLength() * groupSizeEncodingDecoder.numInGroup();
        decoder.limit(decoder.limit() + bytesToSkip);
    }

    @Test
    void allowsNewDecoderToDecodeAddedEnumFieldBeforeGroup()
    {
        final AddEnumBeforeGroupV1Encoder encoder = new AddEnumBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(Direction.BUY).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddEnumBeforeGroupV1Decoder decoder = new AddEnumBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo(Direction.BUY));
        final AddEnumBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingEnumFieldBeforeGroupAsNullValue()
    {
        final AddEnumBeforeGroupV0Encoder encoder = new AddEnumBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddEnumBeforeGroupV1Decoder decoder = new AddEnumBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo(Direction.NULL_VAL));
        final AddEnumBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedEnumFieldBeforeGroup()
    {
        final AddEnumBeforeGroupV1Encoder encoder = new AddEnumBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(Direction.SELL).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddEnumBeforeGroupV1Decoder decoder = new AddEnumBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddEnumBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedEnumFieldBeforeGroup()
    {
        final AddEnumBeforeGroupV1Encoder encoder = new AddEnumBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d(Direction.BUY).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddEnumBeforeGroupV0Decoder decoder = new AddEnumBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddEnumBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeAddedCompositeFieldBeforeGroup()
    {
        final AddCompositeBeforeGroupV1Encoder encoder = new AddCompositeBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().x(-1).y(-2);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddCompositeBeforeGroupV1Decoder decoder = new AddCompositeBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final PointDecoder d = decoder.d();
        assertThat(d, notNullValue());
        assertThat(d.x(), equalTo(-1));
        assertThat(d.y(), equalTo(-2));
        final AddCompositeBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingCompositeFieldBeforeGroupAsNullValue()
    {
        final AddCompositeBeforeGroupV0Encoder encoder = new AddCompositeBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddCompositeBeforeGroupV1Decoder decoder = new AddCompositeBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), nullValue());
        final AddCompositeBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedCompositeFieldBeforeGroup()
    {
        final AddCompositeBeforeGroupV1Encoder encoder = new AddCompositeBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().x(-1).y(-2);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddCompositeBeforeGroupV1Decoder decoder = new AddCompositeBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddCompositeBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedCompositeFieldBeforeGroup()
    {
        final AddCompositeBeforeGroupV1Encoder encoder = new AddCompositeBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().x(-1).y(-2);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddCompositeBeforeGroupV0Decoder decoder = new AddCompositeBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddCompositeBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeAddedArrayFieldBeforeGroup()
    {
        final AddArrayBeforeGroupV1Encoder encoder = new AddArrayBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .putD((short)1, (short)2, (short)3, (short)4)
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(0), equalTo((short)1));
        assertThat(decoder.d(1), equalTo((short)2));
        assertThat(decoder.d(2), equalTo((short)3));
        assertThat(decoder.d(3), equalTo((short)4));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue1()
    {
        final AddArrayBeforeGroupV0Encoder encoder = new AddArrayBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(0), equalTo(AddArrayBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(1), equalTo(AddArrayBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(2), equalTo(AddArrayBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(3), equalTo(AddArrayBeforeGroupV1Decoder.dNullValue()));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue2()
    {
        final AddArrayBeforeGroupV0Encoder encoder = new AddArrayBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.getD(new byte[8], 0, 8), equalTo(0));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue3()
    {
        final AddArrayBeforeGroupV0Encoder encoder = new AddArrayBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.getD(new ExpandableArrayBuffer(), 0, 8), equalTo(0));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue4()
    {
        final AddArrayBeforeGroupV0Encoder encoder = new AddArrayBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final UnsafeBuffer buffer = new UnsafeBuffer();
        decoder.wrapD(buffer);
        assertThat(buffer.capacity(), equalTo(0));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedArrayFieldBeforeGroup()
    {
        final AddArrayBeforeGroupV1Encoder encoder = new AddArrayBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .putD((short)1, (short)2, (short)3, (short)4)
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddArrayBeforeGroupV1Decoder decoder = new AddArrayBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddArrayBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedArrayFieldBeforeGroup()
    {
        final AddArrayBeforeGroupV1Encoder encoder = new AddArrayBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .putD((short)1, (short)2, (short)3, (short)4)
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddArrayBeforeGroupV0Decoder decoder = new AddArrayBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddArrayBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeAddedBitSetFieldBeforeGroup()
    {
        final AddBitSetBeforeGroupV1Encoder encoder = new AddBitSetBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddBitSetBeforeGroupV1Decoder decoder = new AddBitSetBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final FlagsDecoder d = decoder.d();
        assertThat(d, notNullValue());
        assertThat(d.guacamole(), equalTo(true));
        assertThat(d.cheese(), equalTo(true));
        assertThat(d.sourCream(), equalTo(false));
        final AddBitSetBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingBitSetFieldBeforeGroupAsNullValue()
    {
        final AddBitSetBeforeGroupV0Encoder encoder = new AddBitSetBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddBitSetBeforeGroupV1Decoder decoder = new AddBitSetBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), nullValue());
        final AddBitSetBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedBitSetFieldBeforeGroup()
    {
        final AddBitSetBeforeGroupV1Encoder encoder = new AddBitSetBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        final AddBitSetBeforeGroupV1Decoder decoder = new AddBitSetBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddBitSetBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedBitSetFieldBeforeGroup()
    {
        final AddBitSetBeforeGroupV1Encoder encoder = new AddBitSetBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).d().guacamole(true).cheese(true).sourCream(false);
        encoder.bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddBitSetBeforeGroupV0Decoder decoder = new AddBitSetBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddBitSetBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsEncodingAndDecodingEnumInsideGroupInSchemaDefinedOrder()
    {
        final EnumInsideGroupEncoder encoder = new EnumInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(Direction.BUY)
            .bCount(1)
            .next()
            .c(Direction.SELL);
        encoder.checkEncodingIsComplete();

        final EnumInsideGroupDecoder decoder = new EnumInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(Direction.BUY));
        final EnumInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(Direction.SELL));
        assertThat(decoder.toString(), containsString("a=BUY|b=[(c=SELL)]"));
    }

    @Test
    void disallowsEncodingEnumInsideGroupBeforeCallingNext()
    {
        final EnumInsideGroupEncoder encoder = new EnumInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(Direction.BUY);
        final EnumInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(Direction.SELL));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingEnumInsideGroupBeforeCallingNext()
    {
        final EnumInsideGroupEncoder encoder = new EnumInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(Direction.BUY)
            .bCount(1)
            .next()
            .c(Direction.SELL);
        encoder.checkEncodingIsComplete();

        final EnumInsideGroupDecoder decoder = new EnumInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(Direction.BUY));
        final EnumInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, b::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void allowsReEncodingTopLevelEnum()
    {
        final EnumInsideGroupEncoder encoder = new EnumInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(Direction.BUY)
            .bCount(1)
            .next()
            .c(Direction.SELL);

        encoder.a(Direction.SELL);
        encoder.checkEncodingIsComplete();


        final EnumInsideGroupDecoder decoder = new EnumInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(Direction.SELL));
        final EnumInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(Direction.SELL));
    }

    @Test
    void allowsEncodingAndDecodingBitSetInsideGroupInSchemaDefinedOrder()
    {
        final BitSetInsideGroupEncoder encoder = new BitSetInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().cheese(true).guacamole(true).sourCream(false);
        encoder.bCount(1)
            .next()
            .c().cheese(false).guacamole(false).sourCream(true);
        encoder.checkEncodingIsComplete();

        final BitSetInsideGroupDecoder decoder = new BitSetInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final FlagsDecoder a = decoder.a();
        assertThat(a.guacamole(), equalTo(true));
        assertThat(a.cheese(), equalTo(true));
        assertThat(a.sourCream(), equalTo(false));
        final BitSetInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final FlagsDecoder c = b.next().c();
        assertThat(c.guacamole(), equalTo(false));
        assertThat(c.cheese(), equalTo(false));
        assertThat(c.sourCream(), equalTo(true));
        assertThat(decoder.toString(), containsString("a={guacamole,cheese}|b=[(c={sourCream})]"));
    }

    @Test
    void disallowsEncodingBitSetInsideGroupBeforeCallingNext()
    {
        final BitSetInsideGroupEncoder encoder = new BitSetInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().cheese(true).guacamole(true).sourCream(false);
        final BitSetInsideGroupEncoder.BEncoder bEncoder = encoder.bCount(1);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bEncoder::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingBitSetInsideGroupBeforeCallingNext()
    {
        final BitSetInsideGroupEncoder encoder = new BitSetInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().cheese(true).guacamole(true).sourCream(false);
        encoder.bCount(1)
            .next()
            .c().guacamole(false).cheese(false).sourCream(true);
        encoder.checkEncodingIsComplete();

        final BitSetInsideGroupDecoder decoder = new BitSetInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final FlagsDecoder a = decoder.a();
        assertThat(a.guacamole(), equalTo(true));
        assertThat(a.cheese(), equalTo(true));
        assertThat(a.sourCream(), equalTo(false));
        final BitSetInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, b::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void allowsReEncodingTopLevelBitSetViaReWrap()
    {
        final BitSetInsideGroupEncoder encoder = new BitSetInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a().cheese(true).guacamole(true).sourCream(false);
        encoder.bCount(1)
            .next()
            .c().cheese(false).guacamole(false).sourCream(true);

        encoder.a().sourCream(true);
        encoder.checkEncodingIsComplete();

        final BitSetInsideGroupDecoder decoder = new BitSetInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final FlagsDecoder a = decoder.a();
        assertThat(a.guacamole(), equalTo(true));
        assertThat(a.cheese(), equalTo(true));
        assertThat(a.sourCream(), equalTo(true));
        final BitSetInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        final FlagsDecoder c = b.next().c();
        assertThat(c.guacamole(), equalTo(false));
        assertThat(c.cheese(), equalTo(false));
        assertThat(c.sourCream(), equalTo(true));
    }

    @Test
    void allowsEncodingAndDecodingArrayInsideGroupInSchemaDefinedOrder()
    {
        final ArrayInsideGroupEncoder encoder = new ArrayInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.putA((short)1, (short)2, (short)3, (short)4);
        encoder.bCount(1)
            .next()
            .putC((short)5, (short)6, (short)7, (short)8);
        encoder.checkEncodingIsComplete();

        final ArrayInsideGroupDecoder decoder = new ArrayInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(0), equalTo((short)1));
        assertThat(decoder.a(1), equalTo((short)2));
        assertThat(decoder.a(2), equalTo((short)3));
        assertThat(decoder.a(3), equalTo((short)4));
        final ArrayInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        b.next();
        assertThat(b.c(0), equalTo((short)5));
        assertThat(b.c(1), equalTo((short)6));
        assertThat(b.c(2), equalTo((short)7));
        assertThat(b.c(3), equalTo((short)8));
        assertThat(decoder.toString(), containsString("a=[1,2,3,4]|b=[(c=[5,6,7,8])]"));
    }

    @Test
    void disallowsEncodingArrayInsideGroupBeforeCallingNext1()
    {
        final ArrayInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.putC((short)5, (short)6, (short)7, (short)8));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsEncodingArrayInsideGroupBeforeCallingNext2()
    {
        final ArrayInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(0, (short)5));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsEncodingArrayInsideGroupBeforeCallingNext3()
    {
        final ArrayInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithArrayInside();
        final byte[] bytes = {5, 6, 7, 8};
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.putC(bytes, 0, 4));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsEncodingArrayInsideGroupBeforeCallingNext4()
    {
        final ArrayInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithArrayInside();
        final UnsafeBuffer buffer = new UnsafeBuffer(new byte[8]);
        buffer.putByte(0, (byte)5);
        buffer.putByte(2, (byte)6);
        buffer.putByte(4, (byte)7);
        buffer.putByte(6, (byte)8);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.putC(buffer, 0, 4));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    private ArrayInsideGroupEncoder.BEncoder encodeUntilGroupWithArrayInside()
    {
        final ArrayInsideGroupEncoder encoder = new ArrayInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.putA((short)1, (short)2, (short)3, (short)4);
        return encoder.bCount(1);
    }

    @Test
    void disallowsDecodingArrayInsideGroupBeforeCallingNext1()
    {
        final ArrayInsideGroupDecoder.BDecoder b = decodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.c(0));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingArrayInsideGroupBeforeCallingNext2()
    {
        final ArrayInsideGroupDecoder.BDecoder b = decodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.getC(new byte[8], 0, 8));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingArrayInsideGroupBeforeCallingNext3()
    {
        final ArrayInsideGroupDecoder.BDecoder b = decodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.getC(new ExpandableArrayBuffer(), 0, 8));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingArrayInsideGroupBeforeCallingNext4()
    {
        final ArrayInsideGroupDecoder.BDecoder b = decodeUntilGroupWithArrayInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.wrapC(new UnsafeBuffer()));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    private ArrayInsideGroupDecoder.BDecoder decodeUntilGroupWithArrayInside()
    {
        final ArrayInsideGroupEncoder encoder = new ArrayInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.putA((short)1, (short)2, (short)3, (short)4);
        encoder.bCount(1)
            .next()
            .putC((short)5, (short)6, (short)7, (short)8);
        encoder.checkEncodingIsComplete();

        final ArrayInsideGroupDecoder decoder = new ArrayInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(0), equalTo((short)1));
        assertThat(decoder.a(1), equalTo((short)2));
        assertThat(decoder.a(2), equalTo((short)3));
        assertThat(decoder.a(3), equalTo((short)4));
        final ArrayInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        return b;
    }

    @Test
    void allowsReEncodingTopLevelArrayViaReWrap()
    {
        final ArrayInsideGroupEncoder encoder = new ArrayInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.putA((short)1, (short)2, (short)3, (short)4);
        encoder.bCount(1)
            .next()
            .putC((short)5, (short)6, (short)7, (short)8);

        encoder.putA((short)9, (short)10, (short)11, (short)12);
        encoder.checkEncodingIsComplete();

        final ArrayInsideGroupDecoder decoder = new ArrayInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(0), equalTo((short)9));
        assertThat(decoder.a(1), equalTo((short)10));
        assertThat(decoder.a(2), equalTo((short)11));
        assertThat(decoder.a(3), equalTo((short)12));
        final ArrayInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        b.next();
        assertThat(b.c(0), equalTo((short)5));
        assertThat(b.c(1), equalTo((short)6));
        assertThat(b.c(2), equalTo((short)7));
        assertThat(b.c(3), equalTo((short)8));
    }

    @Test
    void allowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder1()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.bCount(0);
        encoder.dCount(1).next().e(43);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b().count(), equalTo(0));
        final MultipleGroupsDecoder.DDecoder d = decoder.d();
        assertThat(d.count(), equalTo(1));
        assertThat(d.next().e(), equalTo(43));
        assertThat(decoder.toString(), containsString("a=42|b=[]|d=[(e=43)]"));
    }

    @Test
    void allowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder2()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(41));
        final MultipleGroupsDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(42));
        final MultipleGroupsDecoder.DDecoder d = decoder.d();
        assertThat(d.count(), equalTo(1));
        assertThat(d.next().e(), equalTo(43));
    }

    @Test
    void allowsReEncodingTopLevelPrimitiveFieldsAfterGroups()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        encoder.a(44);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(44));
        final MultipleGroupsDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(42));
        final MultipleGroupsDecoder.DDecoder d = decoder.d();
        assertThat(d.count(), equalTo(1));
        assertThat(d.next().e(), equalTo(43));
    }

    @Test
    void disallowsMissedEncodingOfGroupField()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.dCount(0));
        assertThat(exception.getMessage(),
            containsString("Cannot encode count of repeating group \"d\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsReEncodingEarlierGroupFields()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.bCount(1));
        assertThat(exception.getMessage(),
            containsString("Cannot encode count of repeating group \"b\" in state: V0_D_1_BLOCK"));
    }

    @Test
    void disallowsReEncodingLatestGroupField()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> encoder.dCount(1));
        assertThat(exception.getMessage(),
            containsString("Cannot encode count of repeating group \"d\" in state: V0_D_1_BLOCK"));
    }

    @Test
    void disallowsMissedDecodingOfGroupField()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(41));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::d);
        assertThat(exception.getMessage(),
            containsString("Cannot decode count of repeating group \"d\" in state: V0_BLOCK"));
    }

    @Test
    void disallowsReDecodingEarlierGroupField()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(41));
        final MultipleGroupsDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(42));
        final MultipleGroupsDecoder.DDecoder d = decoder.d();
        assertThat(d.count(), equalTo(1));
        assertThat(d.next().e(), equalTo(43));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::b);
        assertThat(exception.getMessage(),
            containsString("Cannot decode count of repeating group \"b\" in state: V0_D_1_BLOCK"));
    }

    @Test
    void disallowsReDecodingLatestGroupField()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(41);
        encoder.bCount(1).next().c(42);
        encoder.dCount(1).next().e(43);
        encoder.checkEncodingIsComplete();

        final MultipleGroupsDecoder decoder = new MultipleGroupsDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(41));
        final MultipleGroupsDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(42));
        final MultipleGroupsDecoder.DDecoder d = decoder.d();
        assertThat(d.count(), equalTo(1));
        assertThat(d.next().e(), equalTo(43));
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, decoder::d);
        assertThat(exception.getMessage(),
            containsString("Cannot decode count of repeating group \"d\" in state: V0_D_1_BLOCK"));
    }

    @Test
    void allowsNewDecoderToDecodeAddedVarData()
    {
        final AddVarDataV1Encoder encoder = new AddVarDataV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.b("abc");
        encoder.checkEncodingIsComplete();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b(), equalTo("abc"));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue1()
    {
        final AddVarDataV0Encoder encoder = new AddVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.b(), equalTo(""));
        assertThat(decoder.toString(), containsString("a=42|b=''"));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue2()
    {
        final AddVarDataV0Encoder encoder = new AddVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.getB(new StringBuilder()), equalTo(0));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue3()
    {
        final AddVarDataV0Encoder encoder = new AddVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.getB(new byte[3], 0, 3), equalTo(0));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue4()
    {
        final AddVarDataV0Encoder encoder = new AddVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.getB(new ExpandableArrayBuffer(), 0, 3), equalTo(0));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAddedVarDataAsNullValue5()
    {
        final AddVarDataV0Encoder encoder = new AddVarDataV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddVarDataV1Decoder decoder = new AddVarDataV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(42));
        assertThat(decoder.bLength(), equalTo(0));
    }

    @Test
    void allowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder1()
    {
        final AsciiInsideGroupEncoder encoder = new AsciiInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a("GBPUSD");
        encoder.bCount(1)
            .next()
            .c("EURUSD");
        encoder.checkEncodingIsComplete();

        final AsciiInsideGroupDecoder decoder = new AsciiInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo("GBPUSD"));
        final AsciiInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        b.next();
        assertThat(b.c(), equalTo("EURUSD"));
        assertThat(decoder.toString(), containsString("a=GBPUSD|b=[(c=EURUSD)]"));
    }

    @Test
    void allowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder2()
    {
        final AsciiInsideGroupEncoder encoder = new AsciiInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        final byte[] gbpUsdBytes = "GBPUSD".getBytes(StandardCharsets.US_ASCII);
        encoder.putA(gbpUsdBytes, 0);
        encoder.bCount(1)
            .next()
            .c(0, (byte)'E')
            .c(1, (byte)'U')
            .c(2, (byte)'R')
            .c(3, (byte)'U')
            .c(4, (byte)'S')
            .c(5, (byte)'D');
        encoder.checkEncodingIsComplete();

        final AsciiInsideGroupDecoder decoder = new AsciiInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final byte[] aBytes = new byte[6];
        decoder.getA(aBytes, 0);
        assertThat(aBytes, equalTo(gbpUsdBytes));
        final AsciiInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        b.next();
        assertThat(b.c(0), equalTo((byte)'E'));
        assertThat(b.c(1), equalTo((byte)'U'));
        assertThat(b.c(2), equalTo((byte)'R'));
        assertThat(b.c(3), equalTo((byte)'U'));
        assertThat(b.c(4), equalTo((byte)'S'));
        assertThat(b.c(5), equalTo((byte)'D'));
    }

    @Test
    void disallowsEncodingAsciiInsideGroupBeforeCallingNext1()
    {
        final AsciiInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c("EURUSD"));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsEncodingAsciiInsideGroupBeforeCallingNext2()
    {
        final AsciiInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(0, (byte)'E'));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsEncodingAsciiInsideGroupBeforeCallingNext3()
    {
        final AsciiInsideGroupEncoder.BEncoder bEncoder = encodeUntilGroupWithAsciiInside();
        final byte[] eurUsdBytes = "EURUSD".getBytes(StandardCharsets.US_ASCII);
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.putC(eurUsdBytes, 0));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    private AsciiInsideGroupEncoder.BEncoder encodeUntilGroupWithAsciiInside()
    {
        final AsciiInsideGroupEncoder encoder = new AsciiInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a("GBPUSD");
        return encoder.bCount(1);
    }

    @Test
    void disallowsDecodingAsciiInsideGroupBeforeCallingNext1()
    {
        final AsciiInsideGroupDecoder.BDecoder b = decodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.c(0));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingAsciiInsideGroupBeforeCallingNext2()
    {
        final AsciiInsideGroupDecoder.BDecoder b = decodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.getC(new byte[6], 0));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingAsciiInsideGroupBeforeCallingNext3()
    {
        final AsciiInsideGroupDecoder.BDecoder b = decodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, b::c);
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    @Test
    void disallowsDecodingAsciiInsideGroupBeforeCallingNext4()
    {
        final AsciiInsideGroupDecoder.BDecoder b = decodeUntilGroupWithAsciiInside();
        final Exception exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> b.getC(new StringBuilder()));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_B_N"));
    }

    private AsciiInsideGroupDecoder.BDecoder decodeUntilGroupWithAsciiInside()
    {
        final AsciiInsideGroupEncoder encoder = new AsciiInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a("GBPUSD");
        encoder.bCount(1)
            .next()
            .c("EURUSD");
        encoder.checkEncodingIsComplete();

        final AsciiInsideGroupDecoder decoder = new AsciiInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo("GBPUSD"));
        final AsciiInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        return b;
    }

    @Test
    void allowsReEncodingTopLevelAsciiViaReWrap()
    {
        final AsciiInsideGroupEncoder encoder = new AsciiInsideGroupEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a("GBPUSD");
        encoder.bCount(1)
            .next()
            .c("EURUSD");

        encoder.a("CADUSD");
        encoder.checkEncodingIsComplete();

        final AsciiInsideGroupDecoder decoder = new AsciiInsideGroupDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo("CADUSD"));
        final AsciiInsideGroupDecoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        b.next();
        assertThat(b.c(), equalTo("EURUSD"));
    }

    @Test
    void allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup1()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .d("EURUSD")
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo("EURUSD"));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup2()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .d("EURUSD")
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final StringBuilder aValue = new StringBuilder();
        decoder.getD(aValue);
        assertThat(aValue.toString(), equalTo("EURUSD"));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }


    @Test
    void allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup3()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        final byte[] eurUsdBytes = "EURUSD".getBytes(StandardCharsets.US_ASCII);
        encoder.a(1)
            .putD(eurUsdBytes, 0)
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final byte[] aBytes = new byte[6];
        decoder.getD(aBytes, 0);
        assertThat(aBytes, equalTo(eurUsdBytes));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }


    @Test
    void allowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup4()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .d(0, (byte)'E')
            .d(1, (byte)'U')
            .d(2, (byte)'R')
            .d(3, (byte)'U')
            .d(4, (byte)'S')
            .d(5, (byte)'D')
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(0), equalTo((byte)'E'));
        assertThat(decoder.d(1), equalTo((byte)'U'));
        assertThat(decoder.d(2), equalTo((byte)'R'));
        assertThat(decoder.d(3), equalTo((byte)'U'));
        assertThat(decoder.d(4), equalTo((byte)'S'));
        assertThat(decoder.d(5), equalTo((byte)'D'));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue1()
    {
        final AddAsciiBeforeGroupV0Encoder encoder = new AddAsciiBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(0), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(1), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(2), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(3), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(4), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        assertThat(decoder.d(5), equalTo(AddAsciiBeforeGroupV1Decoder.dNullValue()));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue2()
    {
        final AddAsciiBeforeGroupV0Encoder encoder = new AddAsciiBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.d(), equalTo(""));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue3()
    {
        final AddAsciiBeforeGroupV0Encoder encoder = new AddAsciiBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final StringBuilder aValue = new StringBuilder();
        assertThat(decoder.getD(aValue), equalTo(0));
        assertThat(aValue.length(), equalTo(0));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue4()
    {
        final AddAsciiBeforeGroupV0Encoder encoder = new AddAsciiBeforeGroupV0Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion0();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        assertThat(decoder.getD(new byte[6], 0), equalTo(0));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsNewDecoderToSkipPresentButAddedAsciiFieldBeforeGroup()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .d("EURUSD")
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        final AddAsciiBeforeGroupV1Decoder decoder = new AddAsciiBeforeGroupV1Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddAsciiBeforeGroupV1Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsOldDecoderToSkipAddedAsciiFieldBeforeGroup()
    {
        final AddAsciiBeforeGroupV1Encoder encoder = new AddAsciiBeforeGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1)
            .d("EURUSD")
            .bCount(1)
            .next().c(2);
        encoder.checkEncodingIsComplete();

        modifyHeaderToLookLikeVersion1();

        final AddAsciiBeforeGroupV0Decoder decoder = new AddAsciiBeforeGroupV0Decoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final AddAsciiBeforeGroupV0Decoder.BDecoder b = decoder.b();
        assertThat(b.count(), equalTo(1));
        assertThat(b.next().c(), equalTo(2));
    }

    @Test
    void allowsEncodeAndDecodeOfMessagesWithNoBlock()
    {
        final NoBlockEncoder encoder = new NoBlockEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a("abc");
        encoder.checkEncodingIsComplete();

        final NoBlockDecoder decoder = new NoBlockDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo("abc"));
    }

    @Test
    void allowsEncodeAndDecodeOfGroupsWithNoBlock()
    {
        final GroupWithNoBlockEncoder encoder = new GroupWithNoBlockEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.aCount(1).next().b("abc");
        encoder.checkEncodingIsComplete();

        final GroupWithNoBlockDecoder decoder = new GroupWithNoBlockDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        final GroupWithNoBlockDecoder.ADecoder a = decoder.a();
        assertThat(a.count(), equalTo(1));
        assertThat(a.next().b(), equalTo("abc"));
    }

    @Test
    void disallowsEncodingElementOfEmptyGroup1()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final MultipleGroupsEncoder.BEncoder bEncoder = encoder.bCount(0);
        encoder.dCount(1).next().e(43);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(44));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_D_1_BLOCK"));
    }

    @Test
    void disallowsEncodingElementOfEmptyGroup2()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final NestedGroupsEncoder.BEncoder bEncoder = encoder.bCount(1);
        bEncoder.next();
        bEncoder.c(43);
        final NestedGroupsEncoder.BEncoder.DEncoder dEncoder = bEncoder.dCount(0);
        bEncoder.fCount(0);
        encoder.hCount(0);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> dEncoder.e(44));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.d.e\" in state: V0_H_DONE"));
    }

    @Test
    void disallowsEncodingElementOfEmptyGroup3()
    {
        final NestedGroupsEncoder encoder = new NestedGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final NestedGroupsEncoder.BEncoder bEncoder = encoder.bCount(1);
        bEncoder.next();
        bEncoder.c(43);
        final NestedGroupsEncoder.BEncoder.DEncoder dEncoder = bEncoder.dCount(0);
        bEncoder.fCount(0);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> dEncoder.e(44));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.d.e\" in state: V0_B_1_F_DONE"));
    }

    @Test
    void disallowsEncodingElementOfEmptyGroup4()
    {
        final AddPrimitiveInsideGroupV1Encoder encoder = new AddPrimitiveInsideGroupV1Encoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final AddPrimitiveInsideGroupV1Encoder.BEncoder bEncoder = encoder.bCount(0);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(43));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V1_B_DONE"));
    }

    @Test
    void disallowsEncodingElementOfEmptyGroup5()
    {
        final GroupAndVarLengthEncoder encoder = new GroupAndVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(42);
        final GroupAndVarLengthEncoder.BEncoder bEncoder = encoder.bCount(0);
        encoder.d("abc");
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, () -> bEncoder.c(43));
        assertThat(exception.getMessage(), containsString("Cannot access field \"b.c\" in state: V0_D_DONE"));
    }

    @Test
    void allowsEncodingAndDecodingNestedGroupWithVarDataInSchemaDefinedOrder()
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(3);
        bEncoder.next().c(2).dCount(0);
        bEncoder.next().c(3).dCount(1).next().e(4).f("abc");
        bEncoder.next().c(5).dCount(2).next().e(6).f("def").next().e(7).f("ghi");
        encoder.checkEncodingIsComplete();

        final NestedGroupWithVarLengthDecoder decoder = new NestedGroupWithVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final NestedGroupWithVarLengthDecoder.BDecoder bDecoder = decoder.b();
        assertThat(bDecoder.count(), equalTo(3));
        assertThat(bDecoder.next().c(), equalTo(2));
        assertThat(bDecoder.d().count(), equalTo(0));
        assertThat(bDecoder.next().c(), equalTo(3));
        final NestedGroupWithVarLengthDecoder.BDecoder.DDecoder dDecoder = bDecoder.d();
        assertThat(dDecoder.count(), equalTo(1));
        assertThat(dDecoder.next().e(), equalTo(4));
        assertThat(dDecoder.f(), equalTo("abc"));
        assertThat(bDecoder.next().c(), equalTo(5));
        assertThat(bDecoder.d(), sameInstance(dDecoder));
        assertThat(dDecoder.count(), equalTo(2));
        assertThat(dDecoder.next().e(), equalTo(6));
        assertThat(dDecoder.f(), equalTo("def"));
        assertThat(dDecoder.next().e(), equalTo(7));
        assertThat(dDecoder.f(), equalTo("ghi"));
    }

    @CsvSource(value = {
        "1,V0_B_1_D_N_BLOCK",
        "2,V0_B_N_D_N_BLOCK"
    })
    @ParameterizedTest
    void disallowsMissedEncodingOfVarLengthFieldInNestedGroupToNextInnerElement(
        final int bCount,
        final String expectedState)
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(bCount);
        final NestedGroupWithVarLengthEncoder.BEncoder.DEncoder dEncoder = bEncoder.next().c(5).dCount(2);
        dEncoder.next().e(7);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, dEncoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b.d\" in state: " + expectedState));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @CsvSource(value = {
        "1,V0_B_N_D_1_BLOCK",
        "2,V0_B_N_D_N_BLOCK",
    })
    @ParameterizedTest
    void disallowsMissedEncodingOfVarLengthFieldInNestedGroupToNextOuterElement(
        final int dCount,
        final String expectedState)
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder.next().c(3).dCount(dCount).next().e(4);
        final IllegalStateException exception =
            assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bEncoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b\" in state: " + expectedState));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @Test
    void disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement1()
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(1);
        bEncoder.next().c(2).dCount(2).next().e(3).f("abc").next().e(4).f("def");
        encoder.checkEncodingIsComplete();

        final NestedGroupWithVarLengthDecoder decoder = new NestedGroupWithVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final NestedGroupWithVarLengthDecoder.BDecoder bDecoder = decoder.b();
        assertThat(bDecoder.count(), equalTo(1));
        assertThat(bDecoder.next().c(), equalTo(2));
        final NestedGroupWithVarLengthDecoder.BDecoder.DDecoder dDecoder = bDecoder.d();
        assertThat(dDecoder.count(), equalTo(2));
        assertThat(dDecoder.next().e(), equalTo(3));
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, dDecoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b.d\" in state: V0_B_1_D_N_BLOCK."));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @Test
    void disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement2()
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder.next().c(2).dCount(2).next().e(3).f("abc").next().e(4).f("def");
        bEncoder.next().c(5).dCount(0);
        encoder.checkEncodingIsComplete();

        final NestedGroupWithVarLengthDecoder decoder = new NestedGroupWithVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final NestedGroupWithVarLengthDecoder.BDecoder bDecoder = decoder.b();
        assertThat(bDecoder.count(), equalTo(2));
        assertThat(bDecoder.next().c(), equalTo(2));
        final NestedGroupWithVarLengthDecoder.BDecoder.DDecoder dDecoder = bDecoder.d();
        assertThat(dDecoder.count(), equalTo(2));
        assertThat(dDecoder.next().e(), equalTo(3));
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, dDecoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b.d\" in state: V0_B_N_D_N_BLOCK."));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @Test
    void disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement1()
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder.next().c(2).dCount(2).next().e(3).f("abc").next().e(4).f("def");
        bEncoder.next().c(5).dCount(0);
        encoder.checkEncodingIsComplete();

        final NestedGroupWithVarLengthDecoder decoder = new NestedGroupWithVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final NestedGroupWithVarLengthDecoder.BDecoder bDecoder = decoder.b();
        assertThat(bDecoder.count(), equalTo(2));
        assertThat(bDecoder.next().c(), equalTo(2));
        final NestedGroupWithVarLengthDecoder.BDecoder.DDecoder dDecoder = bDecoder.d();
        assertThat(dDecoder.count(), equalTo(2));
        assertThat(dDecoder.next().e(), equalTo(3));
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bDecoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b\" in state: V0_B_N_D_N_BLOCK."));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @Test
    void disallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement2()
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final NestedGroupWithVarLengthEncoder.BEncoder bEncoder = encoder.bCount(2);
        bEncoder.next().c(2).dCount(1).next().e(3).f("abc");
        bEncoder.next().c(5).dCount(0);
        encoder.checkEncodingIsComplete();

        final NestedGroupWithVarLengthDecoder decoder = new NestedGroupWithVarLengthDecoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderDecoder);
        assertThat(decoder.a(), equalTo(1));
        final NestedGroupWithVarLengthDecoder.BDecoder bDecoder = decoder.b();
        assertThat(bDecoder.count(), equalTo(2));
        assertThat(bDecoder.next().c(), equalTo(2));
        final NestedGroupWithVarLengthDecoder.BDecoder.DDecoder dDecoder = bDecoder.d();
        assertThat(dDecoder.count(), equalTo(1));
        assertThat(dDecoder.next().e(), equalTo(3));
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, bDecoder::next);
        assertThat(exception.getMessage(),
            containsString("Cannot access next element in repeating group \"b\" in state: V0_B_N_D_1_BLOCK."));
        assertThat(exception.getMessage(),
            containsString("Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
    }

    @Test
    void disallowsIncompleteMessagesDueToMissingVarLengthField1()
    {
        final MultipleVarLengthEncoder encoder = new MultipleVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).b("abc");
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString(
            "Not fully encoded, current state: V0_B_DONE, allowed transitions: \"cLength()\", \"c(?)\""));
    }

    @Test
    void disallowsIncompleteMessagesDueToMissingVarLengthField2()
    {
        final NoBlockEncoder encoder = new NoBlockEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString(
            "Not fully encoded, current state: V0_BLOCK, allowed transitions: \"aLength()\", \"a(?)\""));
    }

    @Test
    void disallowsIncompleteMessagesDueToMissingTopLevelGroup1()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(0);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString(
            "Not fully encoded, current state: V0_B_DONE, allowed transitions: " +
            "\"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\""));
    }

    @Test
    void disallowsIncompleteMessagesDueToMissingTopLevelGroup2()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(1).next().c(2);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString(
            "Not fully encoded, current state: V0_B_1_BLOCK, allowed transitions: " +
            "\"b.c(?)\", \"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\""));
    }

    @Test
    void disallowsIncompleteMessagesDueToMissingTopLevelGroup3()
    {
        final MultipleGroupsEncoder encoder = new MultipleGroupsEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString(
            "Not fully encoded, current state: V0_BLOCK, allowed transitions: " +
            "\"a(?)\", \"bCount(0)\", \"bCount(>0)\""));
    }

    @CsvSource(value = {
        "1,V0_B_1_BLOCK",
        "2,V0_B_N_BLOCK",
    })
    @ParameterizedTest
    void disallowsIncompleteMessagesDueToMissingNestedGroup1(final int bCount, final String expectedState)
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(bCount).next().c(2);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString("Not fully encoded, current state: " + expectedState));
    }

    @CsvSource(value = {
        "1,1,V0_B_1_D_N",
        "1,2,V0_B_1_D_N",
        "2,0,V0_B_N_D_DONE",
        "2,1,V0_B_N_D_N",
        "2,2,V0_B_N_D_N",
    })
    @ParameterizedTest
    void disallowsIncompleteMessagesDueToMissingNestedGroup2(
        final int bCount,
        final int dCount,
        final String expectedState)
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(bCount).next().c(2).dCount(dCount);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString("Not fully encoded, current state: " + expectedState));
    }

    @CsvSource(value = {
        "1,1,V0_B_1_D_1_BLOCK",
        "1,2,V0_B_1_D_N_BLOCK",
        "2,1,V0_B_N_D_1_BLOCK",
        "2,2,V0_B_N_D_N_BLOCK",
    })
    @ParameterizedTest
    void disallowsIncompleteMessagesDueToMissingVarDataInNestedGroup(
        final int bCount,
        final int dCount,
        final String expectedState)
    {
        final NestedGroupWithVarLengthEncoder encoder = new NestedGroupWithVarLengthEncoder()
            .wrapAndApplyHeader(buffer, OFFSET, messageHeaderEncoder);
        encoder.a(1).bCount(bCount).next().c(2).dCount(dCount).next().e(10);
        final Exception exception = assertThrows(INCORRECT_ORDER_EXCEPTION_CLASS, encoder::checkEncodingIsComplete);
        assertThat(exception.getMessage(), containsString("Not fully encoded, current state: " + expectedState));
    }

    private void modifyHeaderToLookLikeVersion0()
    {
        messageHeaderDecoder.wrap(buffer, OFFSET);
        final int v1TemplateId = messageHeaderDecoder.templateId() + 1_000;
        messageHeaderEncoder.wrap(buffer, OFFSET);
        messageHeaderEncoder.templateId(v1TemplateId).version(0);
    }

    private void modifyHeaderToLookLikeVersion1()
    {
        messageHeaderDecoder.wrap(buffer, OFFSET);
        assert messageHeaderDecoder.version() == 1;
        final int v0TemplateId = messageHeaderDecoder.templateId() - 1_000;
        messageHeaderEncoder.wrap(buffer, OFFSET);
        messageHeaderEncoder.templateId(v0TemplateId);
    }
}
