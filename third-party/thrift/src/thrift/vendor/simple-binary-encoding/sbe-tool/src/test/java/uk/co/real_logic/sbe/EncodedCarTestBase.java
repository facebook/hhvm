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

import baseline.*;
import org.agrona.concurrent.UnsafeBuffer;
import org.junit.jupiter.api.BeforeAll;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

public class EncodedCarTestBase
{
    protected static final MessageHeaderEncoder MESSAGE_HEADER = new MessageHeaderEncoder();
    protected static final CarEncoder CAR = new CarEncoder();

    private static byte[] vehicleCode;
    private static byte[] manufacturerCode;
    private static byte[] manufacturer;
    private static byte[] model;

    @BeforeAll
    public static void setupExampleData()
    {
        try
        {
            vehicleCode = "ab\"def".getBytes(CarEncoder.vehicleCodeCharacterEncoding());
            manufacturerCode = "123".getBytes(EngineEncoder.manufacturerCodeCharacterEncoding());
            manufacturer = "Honda".getBytes(CarEncoder.manufacturerCharacterEncoding());
            model = "Civic VTi".getBytes(CarEncoder.modelCharacterEncoding());
        }
        catch (final UnsupportedEncodingException ex)
        {
            throw new RuntimeException(ex);
        }
    }

    protected static void encodeTestMessage(final ByteBuffer buffer)
    {
        final UnsafeBuffer directBuffer = new UnsafeBuffer(buffer);

        int bufferOffset = 0;
        MESSAGE_HEADER
            .wrap(directBuffer, bufferOffset)
            .blockLength(CAR.sbeBlockLength())
            .templateId(CAR.sbeTemplateId())
            .schemaId(CAR.sbeSchemaId())
            .version(CAR.sbeSchemaVersion());

        bufferOffset += MESSAGE_HEADER.encodedLength();

        final int srcOffset = 0;

        CAR.wrap(directBuffer, bufferOffset)
            .serialNumber(1234)
            .modelYear(2013)
            .available(BooleanType.T)
            .code(Model.A)
            .putVehicleCode(vehicleCode, srcOffset);

        for (int i = 0, size = CarEncoder.someNumbersLength(); i < size; i++)
        {
            CAR.someNumbers(i, i);
        }

        CAR.extras()
            .clear()
            .cruiseControl(true)
            .sportsPack(true)
            .sunRoof(false);

        CAR.engine()
            .capacity(2000)
            .numCylinders((short)4)
            .putManufacturerCode(manufacturerCode, srcOffset);

        CAR.putUuid(7L, 3L)
            .cupHolderCount((byte)5);

        CAR.fuelFiguresCount(3)
            .next().speed(30).mpg(35.9f)
            .next().speed(55).mpg(49.0f)
            .next().speed(75).mpg(40.0f);

        final CarEncoder.PerformanceFiguresEncoder perfFigures = CAR.performanceFiguresCount(2);
        perfFigures.next()
            .octaneRating((short)95)
            .accelerationCount(3)
            .next().mph(30).seconds(4.0f)
            .next().mph(60).seconds(7.5f)
            .next().mph(100).seconds(12.2f);
        perfFigures.next()
            .octaneRating((short)99)
            .accelerationCount(3)
            .next().mph(30).seconds(3.8f)
            .next().mph(60).seconds(7.1f)
            .next().mph(100).seconds(11.8f);

        CAR.manufacturer(new String(manufacturer));
        CAR.putModel(model, srcOffset, model.length);
        CAR.activationCode("315\\8");

        bufferOffset += CAR.encodedLength();

        buffer.position(bufferOffset);
    }
}
