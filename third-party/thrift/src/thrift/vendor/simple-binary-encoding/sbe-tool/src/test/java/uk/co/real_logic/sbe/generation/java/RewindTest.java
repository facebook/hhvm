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
import java.util.ArrayList;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotEquals;

class RewindTest extends EncodedCarTestBase
{
    private static final int MSG_BUFFER_CAPACITY = 4 * 1024;

    @Test
    void shouldRewindAfterReadingFullMessage()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final MessageHeaderDecoder header = new MessageHeaderDecoder();
        final CarDecoder carDecoder = new CarDecoder();
        carDecoder.wrapAndApplyHeader(new UnsafeBuffer(encodedMsgBuffer), 0, header);

        final ArrayList<Object> passOne = CarDecodeTestUtil.getValues(carDecoder);
        carDecoder.sbeRewind();
        final ArrayList<Object> passTwo = CarDecodeTestUtil.getValues(carDecoder);
        assertEquals(passOne, passTwo);

        carDecoder.sbeRewind();
        final ArrayList<Object> partialPassOne = CarDecodeTestUtil.getPartialValues(carDecoder);
        carDecoder.sbeRewind();
        final ArrayList<Object> partialPassTwo = CarDecodeTestUtil.getPartialValues(carDecoder);
        assertNotEquals(passOne, partialPassOne);
        assertEquals(partialPassOne, partialPassTwo);

        carDecoder.sbeRewind();
        final ArrayList<Object> passThree = CarDecodeTestUtil.getValues(carDecoder);
        assertEquals(passOne, passThree);
    }
}
