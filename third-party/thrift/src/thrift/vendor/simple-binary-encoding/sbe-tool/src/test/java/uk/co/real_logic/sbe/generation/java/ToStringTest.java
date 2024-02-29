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

import org.agrona.concurrent.UnsafeBuffer;
import org.junit.jupiter.api.Test;
import uk.co.real_logic.sbe.EncodedCarTestBase;

import java.nio.ByteBuffer;

import static org.junit.jupiter.api.Assertions.assertEquals;

class ToStringTest extends EncodedCarTestBase
{
    private static final int MSG_BUFFER_CAPACITY = 4 * 1024;

    @Test
    void shouldToStringMessage()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        encodeTestMessage(encodedMsgBuffer);

        final String result = CAR.toString();
        assertEquals(
            "[Car]" +
            "(sbeTemplateId=1|sbeSchemaId=1|sbeSchemaVersion=2|sbeBlockLength=62):" +
            "serialNumber=1234|modelYear=2013|available=T|code=A|" +
            "someNumbers=[0,1,2,3,4]|" +
            "vehicleCode=ab\"def|" +
            "extras={sportsPack,cruiseControl}|" +
            "engine=(capacity=2000|numCylinders=4|manufacturerCode=123|)|" +
            "uuid=[7,3]|" +
            "cupHolderCount=5|" +
            "fuelFigures=[" +
            "(speed=30|mpg=35.9)," +
            "(speed=55|mpg=49.0)," +
            "(speed=75|mpg=40.0)]|" +
            "performanceFigures=[" +
            "(octaneRating=95|acceleration=[(mph=30|seconds=4.0),(mph=60|seconds=7.5),(mph=100|seconds=12.2)])," +
            "(octaneRating=99|acceleration=[(mph=30|seconds=3.8),(mph=60|seconds=7.1),(mph=100|seconds=11.8)])]|" +
            "manufacturer='Honda'|model='Civic VTi'|activationCode='315\\8'",
            result);
    }

    @Test
    void shouldToStringBlankMessage()
    {
        final ByteBuffer encodedMsgBuffer = ByteBuffer.allocate(MSG_BUFFER_CAPACITY);
        CAR.wrap(new UnsafeBuffer(encodedMsgBuffer), 0);

        final String result = CAR.toString();
        assertEquals(
            "[Car]" +
            "(sbeTemplateId=1|sbeSchemaId=1|sbeSchemaVersion=2|sbeBlockLength=62):" +
            "serialNumber=0|modelYear=0|available=F|code=NULL_VAL|someNumbers=[0,0,0,0,0]|vehicleCode=|extras={}|" +
            "engine=(capacity=0|numCylinders=0|manufacturerCode=|)|" +
            "uuid=[0,0]|cupHolderCount=0|" +
            "fuelFigures=[]|performanceFigures=[]|manufacturer=''|model=''|activationCode=''",
            result);
    }
}
