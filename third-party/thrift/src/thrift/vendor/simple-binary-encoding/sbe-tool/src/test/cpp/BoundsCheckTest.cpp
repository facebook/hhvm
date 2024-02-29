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
#include <memory>
#include <cstring>

#include "gtest/gtest.h"
#include "code_generation_test/MessageHeader.h"
#include "code_generation_test/Car.h"

using namespace code::generation::test;

#define SERIAL_NUMBER 1234u
#define MODEL_YEAR 2013
#define AVAILABLE (BooleanType::T)
#define CODE (Model::A)
#define CRUISE_CONTROL (true)
#define SPORTS_PACK (true)
#define SUNROOF (false)

static char VEHICLE_CODE[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
static char MANUFACTURER_CODE[] = { '1', '2', '3' };
static const char *MANUFACTURER = "Honda";
static const char *MODEL = "Civic VTi";
static const char *ACTIVATION_CODE = "deadbeef";

static const std::uint64_t encodedHdrSz = 8;
static const std::uint64_t encodedCarSz = 191;

class BoundsCheckTest : public testing::Test
{
public:

    std::uint64_t encodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_hdr.wrap(buffer, offset, 0, bufferLength)
            .blockLength(Car::sbeBlockLength())
            .templateId(Car::sbeTemplateId())
            .schemaId(Car::sbeSchemaId())
            .version(Car::sbeSchemaVersion());

        return m_hdr.encodedLength();
    }

    std::uint64_t decodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_hdrDecoder.wrap(buffer, offset, 0, bufferLength);

        EXPECT_EQ(m_hdrDecoder.blockLength(), Car::sbeBlockLength());
        EXPECT_EQ(m_hdrDecoder.templateId(), Car::sbeTemplateId());
        EXPECT_EQ(m_hdrDecoder.schemaId(), Car::sbeSchemaId());
        EXPECT_EQ(m_hdrDecoder.version(), Car::sbeSchemaVersion());

        return m_hdrDecoder.encodedLength();
    }

    std::uint64_t encodeCarRoot(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_car.wrapForEncode(buffer, offset, bufferLength)
            .serialNumber(SERIAL_NUMBER)
            .modelYear(MODEL_YEAR)
            .available(AVAILABLE)
            .code(CODE)
            .putVehicleCode(VEHICLE_CODE);

        for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
        {
            m_car.someNumbers(i, static_cast<std::int32_t>(i));
        }

        m_car.extras().clear()
            .cruiseControl(CRUISE_CONTROL)
            .sportsPack(SPORTS_PACK)
            .sunRoof(SUNROOF);

        m_car.engine()
            .capacity(2000)
            .numCylinders((short)4)
            .putManufacturerCode(MANUFACTURER_CODE)
            .booster().boostType(BoostType::NITROUS).horsePower(200);

        return m_car.encodedLength();
    }

    std::uint64_t encodeCarFuelFigures()
    {
        Car::FuelFigures &fuelFigures = m_car.fuelFiguresCount(3);

        fuelFigures
            .next().speed(30).mpg(35.9f);
        fuelFigures.putUsageDescription("Urban Cycle", 11);

        fuelFigures
            .next().speed(55).mpg(49.0f);
        fuelFigures.putUsageDescription("Combined Cycle", 14);

        fuelFigures
            .next().speed(75).mpg(40.0f);
        fuelFigures.putUsageDescription("Highway Cycle", 13);

        return m_car.encodedLength();
    }

    std::uint64_t encodeCarPerformanceFigures()
    {
        Car::PerformanceFigures &perfFigs = m_car.performanceFiguresCount(2);

        perfFigs.next()
            .octaneRating((short)95)
            .accelerationCount(3)
                .next().mph(30).seconds(4.0f)
                .next().mph(60).seconds(7.5f)
                .next().mph(100).seconds(12.2f);

        perfFigs.next()
            .octaneRating((short)99)
            .accelerationCount(3)
                .next().mph(30).seconds(3.8f)
                .next().mph(60).seconds(7.1f)
                .next().mph(100).seconds(11.8f);

        return m_car.encodedLength();
    }

    std::uint64_t encodeCarManufacturerModelAndActivationCode()
    {
        m_car.putManufacturer(MANUFACTURER, static_cast<int>(strlen(MANUFACTURER)));
        m_car.putModel(MODEL, static_cast<int>(strlen(MODEL)));
        m_car.putActivationCode(ACTIVATION_CODE, static_cast<int>(strlen(ACTIVATION_CODE)));

        return m_car.encodedLength();
    }

    std::uint64_t decodeCarRoot(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        m_carDecoder.wrapForDecode(buffer, offset, Car::sbeBlockLength(), Car::sbeSchemaVersion(), bufferLength);
        EXPECT_EQ(m_carDecoder.serialNumber(), SERIAL_NUMBER);
        EXPECT_EQ(m_carDecoder.modelYear(), MODEL_YEAR);
        EXPECT_EQ(m_carDecoder.available(), AVAILABLE);
        EXPECT_EQ(m_carDecoder.code(), CODE);

        EXPECT_EQ(m_carDecoder.someNumbersLength(), 5u);
        for (std::uint64_t i = 0; i < 5; i++)
        {
            EXPECT_EQ(m_carDecoder.someNumbers(i), static_cast<std::int32_t>(i));
        }

        EXPECT_EQ(m_carDecoder.vehicleCodeLength(), 6u);
        EXPECT_EQ(std::string(m_carDecoder.vehicleCode(), 6), std::string(VEHICLE_CODE, 6));
        EXPECT_EQ(m_carDecoder.extras().cruiseControl(), true);
        EXPECT_EQ(m_carDecoder.extras().sportsPack(), true);
        EXPECT_EQ(m_carDecoder.extras().sunRoof(), false);

        Engine &engine = m_carDecoder.engine();
        EXPECT_EQ(engine.capacity(), 2000);
        EXPECT_EQ(engine.numCylinders(), 4);
        EXPECT_EQ(engine.maxRpm(), 9000);
        EXPECT_EQ(engine.manufacturerCodeLength(), 3u);
        EXPECT_EQ(std::string(engine.manufacturerCode(), 3), std::string(MANUFACTURER_CODE, 3));
        EXPECT_EQ(engine.fuelLength(), 6u);
        EXPECT_EQ(std::string(engine.fuel(), 6), std::string("Petrol"));
        EXPECT_EQ(engine.booster().boostType(), BoostType::NITROUS);
        EXPECT_EQ(engine.booster().horsePower(), 200);

        return m_carDecoder.encodedLength();
    }

    std::uint64_t decodeCarFuelFigures()
    {
        char tmp[256] = {};
        Car::FuelFigures &fuelFigures = m_carDecoder.fuelFigures();
        EXPECT_EQ(fuelFigures.count(), 3u);

        EXPECT_TRUE(fuelFigures.hasNext());
        fuelFigures.next();
        EXPECT_EQ(fuelFigures.speed(), 30);
        EXPECT_EQ(fuelFigures.mpg(), 35.9f);
        EXPECT_EQ(fuelFigures.getUsageDescription(tmp, sizeof(tmp)), 11u);
        EXPECT_EQ(std::string(tmp, 11), "Urban Cycle");

        EXPECT_TRUE(fuelFigures.hasNext());
        fuelFigures.next();
        EXPECT_EQ(fuelFigures.speed(), 55);
        EXPECT_EQ(fuelFigures.mpg(), 49.0f);
        EXPECT_EQ(fuelFigures.getUsageDescription(tmp, sizeof(tmp)), 14u);
        EXPECT_EQ(std::string(tmp, 14), "Combined Cycle");

        EXPECT_TRUE(fuelFigures.hasNext());
        fuelFigures.next();
        EXPECT_EQ(fuelFigures.speed(), 75);
        EXPECT_EQ(fuelFigures.mpg(), 40.0f);
        EXPECT_EQ(fuelFigures.getUsageDescription(tmp, sizeof(tmp)), 13u);
        EXPECT_EQ(std::string(tmp, 13), "Highway Cycle");

        return m_carDecoder.encodedLength();
    }

    std::uint64_t decodeCarPerformanceFigures()
    {
        Car::PerformanceFigures &performanceFigures = m_carDecoder.performanceFigures();
        EXPECT_EQ(performanceFigures.count(), 2u);

        EXPECT_TRUE(performanceFigures.hasNext());
        performanceFigures.next();
        EXPECT_EQ(performanceFigures.octaneRating(), 95);

        Car::PerformanceFigures::Acceleration &acceleration1 = performanceFigures.acceleration();
        EXPECT_EQ(acceleration1.count(), 3u);
        EXPECT_TRUE(acceleration1.hasNext());
        acceleration1.next();
        EXPECT_EQ(acceleration1.mph(), 30);
        EXPECT_EQ(acceleration1.seconds(), 4.0f);

        EXPECT_TRUE(acceleration1.hasNext());
        acceleration1.next();
        EXPECT_EQ(acceleration1.mph(), 60);
        EXPECT_EQ(acceleration1.seconds(), 7.5f);

        EXPECT_TRUE(acceleration1.hasNext());
        acceleration1.next();
        EXPECT_EQ(acceleration1.mph(), 100);
        EXPECT_EQ(acceleration1.seconds(), 12.2f);

        EXPECT_TRUE(performanceFigures.hasNext());
        performanceFigures.next();
        EXPECT_EQ(performanceFigures.octaneRating(), 99);

        Car::PerformanceFigures::Acceleration &acceleration2 = performanceFigures.acceleration();
        EXPECT_EQ(acceleration2.count(), 3u);
        EXPECT_TRUE(acceleration2.hasNext());
        acceleration2.next();
        EXPECT_EQ(acceleration2.mph(), 30);
        EXPECT_EQ(acceleration2.seconds(), 3.8f);

        EXPECT_TRUE(acceleration2.hasNext());
        acceleration2.next();
        EXPECT_EQ(acceleration2.mph(), 60);
        EXPECT_EQ(acceleration2.seconds(), 7.1f);

        EXPECT_TRUE(acceleration2.hasNext());
        acceleration2.next();
        EXPECT_EQ(acceleration2.mph(), 100);
        EXPECT_EQ(acceleration2.seconds(), 11.8f);

        return m_carDecoder.encodedLength();
    }

    std::uint64_t decodeCarManufacturerModelAndActivationCode()
    {
        char tmp[256] = {};

        EXPECT_EQ(m_carDecoder.getManufacturer(tmp, sizeof(tmp)), 5u);
        EXPECT_EQ(std::string(tmp, 5), "Honda");

        EXPECT_EQ(m_carDecoder.getModel(tmp, sizeof(tmp)), 9u);
        EXPECT_EQ(std::string(tmp, 9), "Civic VTi");

        EXPECT_EQ(m_carDecoder.getActivationCode(tmp, sizeof(tmp)), 8u);
        EXPECT_EQ(std::string(tmp, 8), "deadbeef");

        EXPECT_EQ(m_carDecoder.encodedLength(), encodedCarSz);

        return m_carDecoder.encodedLength();
    }

    MessageHeader m_hdr = {};
    MessageHeader m_hdrDecoder = {};
    Car m_car = {};
    Car m_carDecoder = {};
};

class HeaderBoundsCheckTest : public BoundsCheckTest, public ::testing::WithParamInterface<int>
{
};

TEST_P(HeaderBoundsCheckTest, shouldExceptionWhenBufferTooShortForEncodeOfHeader)
{
    const int length = GetParam();
    std::unique_ptr<char[]> buffer(new char[length]);

    EXPECT_THROW(
        {
            encodeHdr(buffer.get(), 0, length);
        },
        std::runtime_error);
}

TEST_P(HeaderBoundsCheckTest, shouldExceptionWhenBufferTooShortForDecodeOfHeader)
{
    const int length = GetParam();
    char encodeBuffer[8] = {};
    std::unique_ptr<char[]> buffer(new char[length]);

    encodeHdr(encodeBuffer, 0, sizeof(encodeBuffer));

    EXPECT_THROW(
        {
            std::memcpy(buffer.get(), encodeBuffer, length);
            decodeHdr(buffer.get(), 0, length);
        },
        std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(
    HeaderLengthTest,
    HeaderBoundsCheckTest,
    ::testing::Range(0, static_cast<int>(encodedHdrSz), 1));

class MessageBoundsCheckTest : public BoundsCheckTest, public ::testing::WithParamInterface<int>
{
};

TEST_P(MessageBoundsCheckTest, shouldExceptionWhenBufferTooShortForEncodeOfMessage)
{
    const int length = GetParam();
    std::unique_ptr<char[]> buffer(new char[length]);

    EXPECT_THROW(
        {
            encodeCarRoot(buffer.get(), 0, length);
            encodeCarFuelFigures();
            encodeCarPerformanceFigures();
            encodeCarManufacturerModelAndActivationCode();
        },
        std::runtime_error);
}

TEST_P(MessageBoundsCheckTest, shouldExceptionWhenBufferTooShortForDecodeOfMessage)
{
    const int length = GetParam();
    char encodeBuffer[191] = {};
    std::unique_ptr<char[]> buffer(new char[length]);

    encodeCarRoot(encodeBuffer, 0, sizeof(encodeBuffer));
    encodeCarFuelFigures();
    encodeCarPerformanceFigures();
    encodeCarManufacturerModelAndActivationCode();

    EXPECT_THROW(
        {
            std::memcpy(buffer.get(), encodeBuffer, length);
            decodeCarRoot(buffer.get(), 0, length);
            decodeCarFuelFigures();
            decodeCarPerformanceFigures();
            decodeCarManufacturerModelAndActivationCode();
        },
        std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(
    MessageLengthTest,
    MessageBoundsCheckTest,
    ::testing::Range(0, static_cast<int>(encodedCarSz), 1));
