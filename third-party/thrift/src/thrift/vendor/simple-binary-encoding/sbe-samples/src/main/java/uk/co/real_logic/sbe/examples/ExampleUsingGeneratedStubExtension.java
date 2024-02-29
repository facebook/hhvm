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
package uk.co.real_logic.sbe.examples;

import baseline.BooleanType;
import baseline.BoostType;
import extension.CarDecoder.PerformanceFiguresDecoder.AccelerationDecoder;
import extension.MessageHeaderEncoder;
import org.agrona.concurrent.UnsafeBuffer;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;

import static extension.CarEncoder.cupHolderCountNullValue;
import static java.nio.file.StandardOpenOption.CREATE;
import static java.nio.file.StandardOpenOption.READ;
import static java.nio.file.StandardOpenOption.WRITE;

/**
 * Example encode and decode of a complex message using generated stub codecs which has been extended.
 */
public class ExampleUsingGeneratedStubExtension
{
    private static final String ENCODING_FILENAME = "sbe.encoding.filename";
    private static final byte[] VEHICLE_CODE;
    private static final byte[] MANUFACTURER_CODE;
    private static final byte[] MANUFACTURER;
    private static final byte[] MODEL;
    private static final UnsafeBuffer ACTIVATION_CODE;

    static
    {
        try
        {
            VEHICLE_CODE = "abcdef".getBytes(baseline.CarEncoder.vehicleCodeCharacterEncoding());
            MANUFACTURER_CODE = "123".getBytes(baseline.EngineEncoder.manufacturerCodeCharacterEncoding());
            MANUFACTURER = "Honda".getBytes(baseline.CarEncoder.manufacturerCharacterEncoding());
            MODEL = "Civic VTi".getBytes(baseline.CarEncoder.modelCharacterEncoding());
            ACTIVATION_CODE = new UnsafeBuffer(
                "abcdef".getBytes(baseline.CarEncoder.activationCodeCharacterEncoding()));
        }
        catch (final UnsupportedEncodingException ex)
        {
            throw new RuntimeException(ex);
        }
    }

    /**
     * Main entry point for the example.
     *
     * @param args which are ignored.
     * @throws Exception if an error occurs when parsing the XML or doing IO.
     */
    public static void main(final String[] args) throws Exception
    {
        System.out.println("\n*** Extension Stub Example ***");

        final ByteBuffer byteBuffer = ByteBuffer.allocate(4096);
        final UnsafeBuffer directBuffer = new UnsafeBuffer(byteBuffer);

        final extension.MessageHeaderDecoder messageHeaderDecoder = new extension.MessageHeaderDecoder();
        final baseline.MessageHeaderEncoder messageHeaderEncoder = new baseline.MessageHeaderEncoder();
        final baseline.CarEncoder carEncoder = new baseline.CarEncoder();
        final extension.CarDecoder carDecoder = new extension.CarDecoder();

        final int encodingLengthPlusHeader = encode(carEncoder, directBuffer, messageHeaderEncoder);

        // Optionally write the encoded buffer to a file for decoding by the On-The-Fly decoder

        final String encodingFilename = System.getProperty(ENCODING_FILENAME);
        if (encodingFilename != null)
        {
            try (FileChannel channel = FileChannel.open(Paths.get(encodingFilename), READ, WRITE, CREATE))
            {
                byteBuffer.limit(encodingLengthPlusHeader);
                channel.write(byteBuffer);
            }
        }

        // Decode the encoded message

        final int bufferOffset = 0;
        messageHeaderDecoder.wrap(directBuffer, bufferOffset);

        // Lookup the applicable flyweight to decode this type of message based on templateId and version.
        final int templateId = messageHeaderDecoder.templateId();
        if (templateId != extension.CarEncoder.TEMPLATE_ID)
        {
            throw new IllegalStateException("Template ids do not match");
        }

        decode(carDecoder, directBuffer, messageHeaderDecoder);
    }

    static int encode(
        final baseline.CarEncoder car,
        final UnsafeBuffer directBuffer,
        final baseline.MessageHeaderEncoder headerEncoder)
    {
        car.wrapAndApplyHeader(directBuffer, 0, headerEncoder)
            .serialNumber(1234)
            .modelYear(2013)
            .available(baseline.BooleanType.T)
            .code(baseline.Model.A)
            .putVehicleCode(VEHICLE_CODE, 0);

        for (int i = 0, size = baseline.CarEncoder.someNumbersLength(); i < size; i++)
        {
            car.someNumbers(i, i);
        }

        car.extras()
            .clear()
            .cruiseControl(true)
            .sportsPack(true)
            .sunRoof(false);

        car.engine()
            .capacity(2000)
            .numCylinders((short)4)
            .putManufacturerCode(MANUFACTURER_CODE, 0)
            .efficiency((byte)35)
            .boosterEnabled(BooleanType.T)
            .booster().boostType(BoostType.NITROUS).horsePower((short)200);

        car.fuelFiguresCount(3)
            .next().speed(30).mpg(35.9f).usageDescription("Urban Cycle")
            .next().speed(55).mpg(49.0f).usageDescription("Combined Cycle")
            .next().speed(75).mpg(40.0f).usageDescription("Highway Cycle");

        final baseline.CarEncoder.PerformanceFiguresEncoder figures = car.performanceFiguresCount(2);
        figures.next()
            .octaneRating((short)95)
            .accelerationCount(3)
            .next().mph(30).seconds(4.0f)
            .next().mph(60).seconds(7.5f)
            .next().mph(100).seconds(12.2f);
        figures.next()
            .octaneRating((short)99)
            .accelerationCount(3)
            .next().mph(30).seconds(3.8f)
            .next().mph(60).seconds(7.1f)
            .next().mph(100).seconds(11.8f);

        car.putManufacturer(MANUFACTURER, 0, MANUFACTURER.length)
            .putModel(MODEL, 0, MODEL.length)
            .putActivationCode(ACTIVATION_CODE, 0, ACTIVATION_CODE.capacity());

        return MessageHeaderEncoder.ENCODED_LENGTH + car.encodedLength();
    }

    static void decode(
        final extension.CarDecoder car,
        final UnsafeBuffer directBuffer,
        final extension.MessageHeaderDecoder headerDecoder)
        throws Exception
    {
        final byte[] buffer = new byte[128];
        final StringBuilder sb = new StringBuilder();

        car.wrapAndApplyHeader(directBuffer, 0, headerDecoder);

        sb.append("\ncar.serialNumber=").append(car.serialNumber());
        sb.append("\ncar.modelYear=").append(car.modelYear());
        sb.append("\ncar.available=").append(car.available());
        sb.append("\ncar.code=").append(car.code());

        sb.append("\ncar.someNumbers=");
        for (int i = 0, size = extension.CarEncoder.someNumbersLength(); i < size; i++)
        {
            sb.append(car.someNumbers(i)).append(", ");
        }

        sb.append("\ncar.vehicleCode=");
        for (int i = 0, size = extension.CarEncoder.vehicleCodeLength(); i < size; i++)
        {
            sb.append((char)car.vehicleCode(i));
        }

        final extension.OptionalExtrasDecoder extras = car.extras();
        sb.append("\ncar.extras.cruiseControl=").append(extras.cruiseControl());
        sb.append("\ncar.extras.sportsPack=").append(extras.sportsPack());
        sb.append("\ncar.extras.sunRoof=").append(extras.sunRoof());

        sb.append("\ncar.discountedModel=").append(car.discountedModel());

        final extension.EngineDecoder engine = car.engine();
        sb.append("\ncar.engine.capacity=").append(engine.capacity());
        sb.append("\ncar.engine.numCylinders=").append(engine.numCylinders());
        sb.append("\ncar.engine.maxRpm=").append(engine.maxRpm());
        sb.append("\ncar.engine.manufacturerCode=");
        for (int i = 0, size = extension.EngineDecoder.manufacturerCodeLength(); i < size; i++)
        {
            sb.append((char)engine.manufacturerCode(i));
        }
        sb.append("\ncar.engine.efficiency=").append(engine.efficiency());
        sb.append("\ncar.engine.boosterEnabled=").append(engine.boosterEnabled());
        sb.append("\ncar.engine.booster.boostType=").append(engine.booster().boostType());
        sb.append("\ncar.engine.booster.horsePower=").append(engine.booster().horsePower());

        sb.append("\ncar.engine.fuel=").append(
            new String(buffer, 0, engine.getFuel(buffer, 0, buffer.length), StandardCharsets.US_ASCII));

        final String cupHolderCount =
            car.cupHolderCount() == cupHolderCountNullValue() ? "null" : car.cupHolderCount() + "";
        sb.append("\ncar.cupHolderCount=").append(cupHolderCount);

        for (final extension.CarDecoder.FuelFiguresDecoder fuelFigures : car.fuelFigures())
        {
            sb.append("\ncar.fuelFigures.speed=").append(fuelFigures.speed());
            sb.append("\ncar.fuelFigures.mpg=").append(fuelFigures.mpg());
            sb.append("\ncar.fuelFigures.usageDescription=").append(fuelFigures.usageDescription());
        }

        for (final extension.CarDecoder.PerformanceFiguresDecoder performanceFigures : car.performanceFigures())
        {
            sb.append("\ncar.performanceFigures.octaneRating=").append(performanceFigures.octaneRating());

            for (final AccelerationDecoder acceleration : performanceFigures.acceleration())
            {
                sb.append("\ncar.performanceFigures.acceleration.mph=").append(acceleration.mph());
                sb.append("\ncar.performanceFigures.acceleration.seconds=").append(acceleration.seconds());
            }
        }

        sb.append("\ncar.manufacturer=").append(new String(buffer, 0, car.getManufacturer(
            buffer, 0, buffer.length), extension.CarEncoder.manufacturerCharacterEncoding()));

        sb.append("\ncar.model=").append(new String(buffer, 0, car.getModel(
            buffer, 0, buffer.length), extension.CarEncoder.modelCharacterEncoding()));

        final UnsafeBuffer tempBuffer = new UnsafeBuffer(buffer);
        final int tempBufferLength = car.getActivationCode(tempBuffer, 0, tempBuffer.capacity());
        sb.append("\ncar.activationCode=").append(
            new String(buffer, 0, tempBufferLength, extension.CarEncoder.activationCodeCharacterEncoding()));

        sb.append("\ncar.encodedLength=").append(car.encodedLength());

        System.out.println(sb);
    }
}
