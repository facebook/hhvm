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
package uk.co.real_logic.sbe.generation.java;

import baseline.CarDecoder;
import baseline.MessageHeaderDecoder;
import org.agrona.concurrent.UnsafeBuffer;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.EncodedCarTestBase;

import java.nio.ByteBuffer;

import static org.junit.jupiter.api.Assertions.assertEquals;

class SkipAndDecodedLengthTest extends EncodedCarTestBase
{
    private static final int MSG_BUFFER_CAPACITY = 4 * 1024;

    @Test
    void shouldRewindAfterReadingFullMessage()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final int encodedLength = CAR.encodedLength();

        final MessageHeaderDecoder header = new MessageHeaderDecoder();
        final CarDecoder carDecoder = new CarDecoder();
        carDecoder.wrapAndApplyHeader(new UnsafeBuffer(encodedMsgBuffer), 0, header);
        final int decodedLengthNoRead = carDecoder.sbeDecodedLength();

        final int initialLimit = carDecoder.limit();
        CarDecodeTestUtil.getValues(carDecoder);
        final int readLimit = carDecoder.limit();

        carDecoder.sbeRewind();
        final int rewindLimit = carDecoder.limit();
        carDecoder.sbeSkip();

        final int skipLimit = carDecoder.limit();
        final int decodedLengthFullSkip = carDecoder.sbeDecodedLength();
        carDecoder.sbeRewind();
        final int decodedLengthAfterRewind = carDecoder.sbeDecodedLength();
        CarDecodeTestUtil.getPartialValues(carDecoder);
        final int decodedLengthPartialRead = carDecoder.sbeDecodedLength();

        assertEquals(initialLimit, rewindLimit);
        assertEquals(readLimit, skipLimit);
        assertEquals(encodedLength, decodedLengthNoRead);
        assertEquals(encodedLength, decodedLengthFullSkip);
        assertEquals(encodedLength, decodedLengthAfterRewind);
        assertEquals(encodedLength, decodedLengthPartialRead);
    }
}
