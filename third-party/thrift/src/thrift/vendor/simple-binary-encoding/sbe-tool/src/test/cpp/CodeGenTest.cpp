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
#include <cstring>

#include "gtest/gtest.h"
#include "code_generation_test/MessageHeader.h"
#include "code_generation_test/Car.h"

using namespace code::generation::test;

static const std::size_t BUFFER_LEN = 2048;

static const std::uint32_t SERIAL_NUMBER = 1234;
static const std::uint16_t MODEL_YEAR = 2013;
static const BooleanType::Value AVAILABLE = BooleanType::T;
static const Model::Value CODE = Model::A;
static const bool CRUISE_CONTROL = true;
static const bool SPORTS_PACK = true;
static const bool SUNROOF = false;
static const BoostType::Value BOOST_TYPE = BoostType::NITROUS;
static const std::uint8_t BOOSTER_HORSEPOWER = 200;

static char VEHICLE_CODE[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
static char MANUFACTURER_CODE[] = { '1', '2', '3' };
static const char *FUEL_FIGURES_1_USAGE_DESCRIPTION = "Urban Cycle";
static const char *FUEL_FIGURES_2_USAGE_DESCRIPTION = "Combined Cycle";
static const char *FUEL_FIGURES_3_USAGE_DESCRIPTION = "Highway Cycle";
static const char *MANUFACTURER = "Honda";
static const char *MODEL = "Civic VTi";
static const char *ACTIVATION_CODE = "deadbeef";
static const char *COLOR = "racing green";

static const std::size_t VEHICLE_CODE_LENGTH = sizeof(VEHICLE_CODE);
static const std::size_t MANUFACTURER_CODE_LENGTH = sizeof(MANUFACTURER_CODE);
static const std::size_t FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION);
static const std::size_t FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION);
static const std::size_t FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_3_USAGE_DESCRIPTION);
static const std::size_t MANUFACTURER_LENGTH = strlen(MANUFACTURER);
static const std::size_t MODEL_LENGTH = strlen(MODEL);
static const std::size_t ACTIVATION_CODE_LENGTH = strlen(ACTIVATION_CODE);
static const std::size_t COLOR_LENGTH = strlen(COLOR);
static const std::uint8_t PERFORMANCE_FIGURES_COUNT = 2;
static const std::uint8_t FUEL_FIGURES_COUNT = 3;
static const std::uint8_t ACCELERATION_COUNT = 3;

static const std::uint64_t expectedHeaderSize = 8;
static const std::uint64_t expectedCarEncodedLength = 207;

static const std::uint16_t fuel1Speed = 30;
static const float fuel1Mpg = 35.9f;
static const std::uint16_t fuel2Speed = 55;
static const float fuel2Mpg = 49.0f;
static const std::uint16_t fuel3Speed = 75;
static const float fuel3Mpg = 40.0f;

static const std::uint8_t perf1Octane = 95;
static const std::uint16_t perf1aMph = 30;
static const float perf1aSeconds = 4.0f;
static const std::uint16_t perf1bMph = 60;
static const float perf1bSeconds = 7.5f;
static const std::uint16_t perf1cMph = 100;
static const float perf1cSeconds = 12.2f;

static const std::uint8_t perf2Octane = 99;
static const std::uint16_t perf2aMph = 30;
static const float perf2aSeconds = 3.8f;
static const std::uint16_t perf2bMph = 60;
static const float perf2bSeconds = 7.1f;
static const std::uint16_t perf2cMph = 100;
static const float perf2cSeconds = 11.8f;

static const std::uint16_t engineCapacity = 2000;
static const std::uint8_t engineNumCylinders = 4;

class CodeGenTest : public testing::Test
{
public:

    static std::uint64_t encodeHdr(MessageHeader &hdr)
    {
        hdr.blockLength(Car::sbeBlockLength())
            .templateId(Car::sbeTemplateId())
            .schemaId(Car::sbeSchemaId())
            .version(Car::sbeSchemaVersion());

        return hdr.encodedLength();
    }

    static std::uint64_t encodeCar(Car &car)
    {
        car.serialNumber(SERIAL_NUMBER)
            .modelYear(MODEL_YEAR)
            .available(AVAILABLE)
            .code(CODE)
            .putVehicleCode(VEHICLE_CODE);

        for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
        {
            car.someNumbers(i, static_cast<std::int32_t>(i));
        }

        car.extras().clear()
            .cruiseControl(CRUISE_CONTROL)
            .sportsPack(SPORTS_PACK)
            .sunRoof(SUNROOF);

        car.engine()
            .capacity(engineCapacity)
            .numCylinders(engineNumCylinders)
            .putManufacturerCode(MANUFACTURER_CODE)
            .booster().boostType(BOOST_TYPE).horsePower(BOOSTER_HORSEPOWER);

        Car::FuelFigures &fuelFigures = car.fuelFiguresCount(FUEL_FIGURES_COUNT);

        fuelFigures
            .next().speed(fuel1Speed).mpg(fuel1Mpg)
            .putUsageDescription(
                FUEL_FIGURES_1_USAGE_DESCRIPTION, static_cast<int>(strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION)));

        fuelFigures
            .next().speed(fuel2Speed).mpg(fuel2Mpg)
            .putUsageDescription(
                FUEL_FIGURES_2_USAGE_DESCRIPTION, static_cast<int>(strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION)));

        fuelFigures
            .next().speed(fuel3Speed).mpg(fuel3Mpg)
            .putUsageDescription(
                FUEL_FIGURES_3_USAGE_DESCRIPTION, static_cast<int>(strlen(FUEL_FIGURES_3_USAGE_DESCRIPTION)));

        Car::PerformanceFigures &perfFigs = car.performanceFiguresCount(PERFORMANCE_FIGURES_COUNT);

        perfFigs.next()
            .octaneRating(perf1Octane)
            .accelerationCount(ACCELERATION_COUNT)
                .next().mph(perf1aMph).seconds(perf1aSeconds)
                .next().mph(perf1bMph).seconds(perf1bSeconds)
                .next().mph(perf1cMph).seconds(perf1cSeconds);

        perfFigs.next()
            .octaneRating(perf2Octane)
            .accelerationCount(ACCELERATION_COUNT)
                .next().mph(perf2aMph).seconds(perf2aSeconds)
                .next().mph(perf2bMph).seconds(perf2bSeconds)
                .next().mph(perf2cMph).seconds(perf2cSeconds);

        car.putManufacturer(MANUFACTURER, static_cast<int>(strlen(MANUFACTURER)))
            .putModel(MODEL, static_cast<int>(strlen(MODEL)))
            .putActivationCode(ACTIVATION_CODE, static_cast<int>(strlen(ACTIVATION_CODE)))
            .putColor(COLOR, static_cast<int>(strlen(COLOR)));

        return car.encodedLength();
    }

    std::uint64_t encodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_hdr.wrap(buffer, offset, 0, bufferLength);
        return encodeHdr(m_hdr);
    }

    std::uint64_t encodeCar(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        m_car.wrapForEncode(buffer, offset, bufferLength);
        return encodeCar(m_car);
    }

    static void expectDisplayString(const char *expectedDisplayString, Car &carDecoder)
    {
        std::stringstream displayStream;

        displayStream << carDecoder;
        EXPECT_STREQ(expectedDisplayString, displayStream.str().c_str());
        displayStream.clear();
    }

    std::string walkCar(Car& carDecoder)
    {
        std::stringstream output;

        output <<
            carDecoder.serialNumber() << ';' <<
            carDecoder.modelYear() << ';' <<
            carDecoder.available() << ';' <<
            carDecoder.code() << ';';

        for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
        {
            output << carDecoder.someNumbers(i) << ';';
        }

        output << carDecoder.getVehicleCodeAsString() << ';';

        OptionalExtras &extras = carDecoder.extras();
        output <<
        extras.sunRoof() << ';' <<
        extras.sportsPack() << ';' <<
        extras.cruiseControl() << ';';

        Engine &engine = carDecoder.engine();
        output <<
        engine.capacity() << ';' <<
        static_cast<int>(engine.numCylinders()) << ';' <<
        Engine::maxRpm() << ';' <<
        engine.getManufacturerCodeAsString() << ';' <<
        engine.getFuelAsString() << ';';

        Car::FuelFigures &fuelFigures = carDecoder.fuelFigures();
        while (fuelFigures.hasNext())
        {
            fuelFigures.next();
            output <<
            fuelFigures.speed() << ';' <<
            fuelFigures.mpg() << ';' <<
            fuelFigures.getUsageDescriptionAsString() << ';';
        }

        Car::PerformanceFigures &performanceFigures = carDecoder.performanceFigures();
        output << performanceFigures.count() << ';';
        while (performanceFigures.hasNext())
        {
            performanceFigures.next();
            output << performanceFigures.octaneRating() << ';';
            Car::PerformanceFigures::Acceleration &acceleration = performanceFigures.acceleration();
            while (acceleration.hasNext())
            {
                acceleration.next();
                output <<
                acceleration.mph() << ';' <<
                acceleration.seconds() << ';';
            }
        }

        output << carDecoder.getManufacturerAsString() << ';';
        output << carDecoder.getModelAsString() << ';';

        return output.str();
    }

    std::string partialWalkCar(Car& carDecoder)
    {
        std::stringstream output;

        output <<
            carDecoder.serialNumber() << ';' <<
            carDecoder.modelYear() << ';' <<
            carDecoder.available() << ';' <<
            carDecoder.code() << ';';

        for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
        {
            output << carDecoder.someNumbers(i) << ';';
        }

        output << carDecoder.getVehicleCodeAsString() << ';';

        OptionalExtras &extras = carDecoder.extras();
        output <<
        extras.sunRoof() << ';' <<
        extras.sportsPack() << ';' <<
        extras.cruiseControl() << ';';

        Engine &engine = carDecoder.engine();
        output <<
        engine.capacity() << ';' <<
        static_cast<int>(engine.numCylinders()) << ';' <<
        Engine::maxRpm() << ';' <<
        engine.getManufacturerCodeAsString() << ';' <<
        engine.getFuelAsString() << ';';

        Car::FuelFigures &fuelFigures = carDecoder.fuelFigures();
        while (fuelFigures.hasNext())
        {
            fuelFigures.next();
            output <<
            fuelFigures.speed() << ';' <<
            fuelFigures.mpg() << ';' <<
            fuelFigures.getUsageDescriptionAsString() << ';';
        }

        Car::PerformanceFigures &performanceFigures = carDecoder.performanceFigures();
        output << performanceFigures.count() << ';';
        if (performanceFigures.hasNext())
        {
            performanceFigures.next();
            output << performanceFigures.octaneRating() << ';';
            Car::PerformanceFigures::Acceleration &acceleration = performanceFigures.acceleration();
            if (acceleration.hasNext())
            {
                acceleration.next();
                output <<
                acceleration.mph() << ';' <<
                acceleration.seconds() << ';';
            }
        }

        return output.str();
    }

    MessageHeader m_hdr = {};
    MessageHeader m_hdrDecoder = {};
    Car m_car = {};
    Car m_carDecoder = {};
};

TEST_F(CodeGenTest, shouldReturnCorrectValuesForMessageHeaderStaticFields)
{
    EXPECT_EQ(MessageHeader::encodedLength(), 8u);
    // only checking the block length field
    EXPECT_EQ(MessageHeader::blockLengthNullValue(), 65535);
    EXPECT_EQ(MessageHeader::blockLengthMinValue(), 0);
    EXPECT_EQ(MessageHeader::blockLengthMaxValue(), 65534);
}

TEST_F(CodeGenTest, shouldReturnCorrectValuesForCarStaticFields)
{
    EXPECT_EQ(Car::sbeBlockLength(), 47u);
    EXPECT_EQ(Car::sbeTemplateId(), 1u);
    EXPECT_EQ(Car::sbeSchemaId(), 6u);
    EXPECT_EQ(Car::sbeSchemaVersion(), 0u);
    EXPECT_EQ(std::string(Car::sbeSemanticType()), std::string(""));
    EXPECT_EQ(std::string(Car::sbeSemanticVersion()), std::string("5.2"));
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeMessageHeaderCorrectly)
{
    char buffer[BUFFER_LEN] = {};
    const char *bp = buffer;

    std::uint64_t sz = encodeHdr(buffer, 0, sizeof(buffer));

    EXPECT_EQ(*((::uint16_t *)bp), Car::sbeBlockLength());
    EXPECT_EQ(*((::uint16_t *)(bp + 2)), Car::sbeTemplateId());
    EXPECT_EQ(*((::uint16_t *)(bp + 4)), Car::sbeSchemaId());
    EXPECT_EQ(*((::uint16_t *)(bp + 6)), Car::sbeSchemaVersion());
    EXPECT_EQ(sz, 8u);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeAndDecodeMessageHeaderCorrectly)
{
    char buffer[BUFFER_LEN] = {};

    encodeHdr(buffer, 0, sizeof(buffer));

    m_hdrDecoder.wrap(buffer, 0, 0, sizeof(buffer));
    EXPECT_EQ(m_hdrDecoder.blockLength(), Car::sbeBlockLength());
    EXPECT_EQ(m_hdrDecoder.templateId(), Car::sbeTemplateId());
    EXPECT_EQ(m_hdrDecoder.schemaId(), Car::sbeSchemaId());
    EXPECT_EQ(m_hdrDecoder.version(), Car::sbeSchemaVersion());
}

static const uint8_t fieldIdSerialNumber = 1;
static const uint8_t fieldIdModelYear = 2;
static const uint8_t fieldIdAvailable = 3;
static const uint8_t fieldIdCode = 4;
static const uint8_t fieldIdSomeNumbers = 5;
static const uint8_t fieldIdVehicleCode = 6;
static const uint8_t fieldIdExtras = 7;
static const uint8_t fieldIdDiscountedModel = 8;
static const uint8_t fieldIdEngine = 9;
static const uint8_t fieldIdFuelFigures = 10;
static const uint8_t fieldIdFuelSpeed = 11;
static const uint8_t fieldIdFuelMpg = 12;
static const uint8_t fieldIdFuelUsageDescription = 200;
static const uint8_t fieldIdPerformanceFigures = 13;
static const uint8_t fieldIdPerfOctaneRating = 14;
static const uint8_t fieldIdPerfAcceleration = 15;
static const uint8_t fieldIdPerfAccMph = 16;
static const uint8_t fieldIdPerfAccSeconds = 17;
static const uint8_t fieldIdManufacturer = 18;
static const uint8_t fieldIdModel = 19;
static const uint8_t fieldIdActivationCode = 20;

TEST_F(CodeGenTest, shouldReturnCorrectValuesForCarFieldIdsAndCharacterEncoding)
{
    EXPECT_EQ(Car::serialNumberId(), fieldIdSerialNumber);
    EXPECT_EQ(Car::modelYearId(), fieldIdModelYear);
    EXPECT_EQ(Car::availableId(), fieldIdAvailable);
    EXPECT_EQ(Car::codeId(), fieldIdCode);
    EXPECT_EQ(Car::someNumbersId(), fieldIdSomeNumbers);
    EXPECT_EQ(Car::vehicleCodeId(), fieldIdVehicleCode);
    EXPECT_EQ(Car::extrasId(), fieldIdExtras);
    EXPECT_EQ(Car::discountedModelId(), fieldIdDiscountedModel);
    EXPECT_EQ(Car::engineId(), fieldIdEngine);
    EXPECT_EQ(Car::fuelFiguresId(), fieldIdFuelFigures);
    EXPECT_EQ(Car::FuelFigures::speedId(), fieldIdFuelSpeed);
    EXPECT_EQ(Car::FuelFigures::mpgId(), fieldIdFuelMpg);
    EXPECT_EQ(Car::FuelFigures::usageDescriptionId(), fieldIdFuelUsageDescription);
    EXPECT_EQ(Car::FuelFigures::usageDescriptionCharacterEncoding(), std::string("UTF-8"));
    EXPECT_EQ(Car::performanceFiguresId(), fieldIdPerformanceFigures);
    EXPECT_EQ(Car::PerformanceFigures::octaneRatingId(), fieldIdPerfOctaneRating);
    EXPECT_EQ(Car::PerformanceFigures::accelerationId(), fieldIdPerfAcceleration);
    EXPECT_EQ(Car::PerformanceFigures::Acceleration::mphId(), fieldIdPerfAccMph);
    EXPECT_EQ(Car::PerformanceFigures::Acceleration::secondsId(), fieldIdPerfAccSeconds);
    EXPECT_EQ(Car::manufacturerId(), fieldIdManufacturer);
    EXPECT_EQ(Car::modelId(), fieldIdModel);
    EXPECT_EQ(Car::activationCodeId(), fieldIdActivationCode);
    EXPECT_EQ(std::string(Car::manufacturerCharacterEncoding()), std::string("UTF-8"));
    EXPECT_EQ(std::string(Car::modelCharacterEncoding()), std::string("UTF-8"));
    EXPECT_EQ(std::string(Car::activationCodeCharacterEncoding()), std::string("UTF-8"));
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeCarCorrectly)
{
    char buffer[BUFFER_LEN] = {};
    const char *bp = buffer;
    std::uint64_t sz = encodeCar(buffer, 0, sizeof(buffer));

    std::uint64_t offset = 0;
    EXPECT_EQ(*(std::uint64_t *)(bp + offset), SERIAL_NUMBER);
    offset += sizeof(std::uint64_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), MODEL_YEAR);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), 1);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(*(bp + offset), 'A');
    offset += sizeof(char);

    EXPECT_EQ(*(std::int32_t *)(bp + offset), 0);
    offset += sizeof(std::int32_t);
    EXPECT_EQ(*(std::int32_t *)(bp + offset), 1);
    offset += sizeof(std::int32_t);
    EXPECT_EQ(*(std::int32_t *)(bp + offset), 2);
    offset += sizeof(std::int32_t);
    EXPECT_EQ(*(std::int32_t *)(bp + offset), 3);
    offset += sizeof(std::int32_t);
    EXPECT_EQ(*(std::int32_t *)(bp + offset), 4);
    offset += sizeof(std::int32_t);

    EXPECT_EQ(std::string(bp + offset, VEHICLE_CODE_LENGTH), std::string(VEHICLE_CODE, VEHICLE_CODE_LENGTH));
    offset += VEHICLE_CODE_LENGTH;
    EXPECT_EQ(*(bp + offset), 0x6);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), engineCapacity);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(bp + offset), engineNumCylinders);
    offset += sizeof(std::uint8_t);
    EXPECT_EQ(std::string(bp + offset, MANUFACTURER_CODE_LENGTH),
        std::string(MANUFACTURER_CODE, MANUFACTURER_CODE_LENGTH));
    offset += MANUFACTURER_CODE_LENGTH;
    EXPECT_EQ(*(bp + offset), 'N');
    offset += sizeof(char);
    EXPECT_EQ(*(std::uint8_t *)(bp + offset), BOOSTER_HORSEPOWER);
    offset += sizeof(std::uint8_t);

    // fuel figures
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 6);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), FUEL_FIGURES_COUNT);
    offset += sizeof(std::uint16_t);

    EXPECT_EQ(*(::uint16_t *)(bp + offset), fuel1Speed);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), fuel1Mpg);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_1_USAGE_DESCRIPTION);
    offset += FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH;

    EXPECT_EQ(*(std::uint16_t *)(bp + offset), fuel2Speed);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), fuel2Mpg);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_2_USAGE_DESCRIPTION);
    offset += FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH;

    EXPECT_EQ(*(std::uint16_t *)(bp + offset), fuel3Speed);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), fuel3Mpg);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_3_USAGE_DESCRIPTION);
    offset += FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH;

    // performance figures
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 1);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), PERFORMANCE_FIGURES_COUNT);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(bp + offset), perf1Octane);
    offset += sizeof(std::uint8_t);
    // acceleration
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 6);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), ACCELERATION_COUNT);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf1aMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf1aSeconds);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf1bMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf1bSeconds);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf1cMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf1cSeconds);
    offset += sizeof(float);

    EXPECT_EQ(*(bp + offset), perf2Octane);
    offset += sizeof(std::uint8_t);
    // acceleration
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), 6);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), ACCELERATION_COUNT);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf2aMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf2aSeconds);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf2bMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf2bSeconds);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), perf2cMph);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), perf2cSeconds);
    offset += sizeof(float);

    // manufacturer & model
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), MANUFACTURER_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, MANUFACTURER_LENGTH), MANUFACTURER);
    offset += MANUFACTURER_LENGTH;
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), MODEL_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, MODEL_LENGTH), MODEL);
    offset += MODEL_LENGTH;
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), ACTIVATION_CODE_LENGTH);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);
    offset += ACTIVATION_CODE_LENGTH;
    EXPECT_EQ(*(std::uint32_t *)(bp + offset), COLOR_LENGTH);
    offset += sizeof(std::uint32_t);
    EXPECT_EQ(std::string(bp + offset, COLOR_LENGTH), COLOR);
    offset += COLOR_LENGTH;

    EXPECT_EQ(sz, offset);

    std::uint64_t predictedCarSz = Car::computeLength(
        {
            FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH,
            FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH,
            FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH
        },
        {
            ACCELERATION_COUNT,
            ACCELERATION_COUNT
        },
        MANUFACTURER_LENGTH,
        MODEL_LENGTH,
        ACTIVATION_CODE_LENGTH,
        COLOR_LENGTH);

    EXPECT_EQ(sz, predictedCarSz);
    EXPECT_EQ(Car::isConstLength(), false);
    EXPECT_EQ(Car::PerformanceFigures::Acceleration::isConstLength(), true);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeHeaderPlusCarCorrectly)
{
    char buffer[BUFFER_LEN] = {};
    const char *bp = buffer;

    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t carEncodedLength = encodeCar(buffer, m_hdr.encodedLength(), sizeof(buffer) - m_hdr.encodedLength());

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    EXPECT_EQ(*((std::uint16_t *)bp), Car::sbeBlockLength());
    const std::size_t activationCodePosition =
        (hdrSz + carEncodedLength) - (ACTIVATION_CODE_LENGTH + sizeof(std::uint32_t) + COLOR_LENGTH);
    const std::size_t activationCodeLengthPosition = activationCodePosition - sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + activationCodeLengthPosition), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(bp + activationCodePosition, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeAndDecodeHeaderPlusCarCorrectly)
{
    char buffer[BUFFER_LEN] = {};

    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t carEncodedLength = encodeCar(buffer, m_hdr.encodedLength(), sizeof(buffer) - m_hdr.encodedLength());

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    m_hdrDecoder.wrap(buffer, 0, 0, hdrSz);

    EXPECT_EQ(m_hdrDecoder.blockLength(), Car::sbeBlockLength());
    EXPECT_EQ(m_hdrDecoder.templateId(), Car::sbeTemplateId());
    EXPECT_EQ(m_hdrDecoder.schemaId(), Car::sbeSchemaId());
    EXPECT_EQ(m_hdrDecoder.version(), Car::sbeSchemaVersion());
    EXPECT_EQ(m_hdrDecoder.encodedLength(), expectedHeaderSize);

    m_carDecoder.wrapForDecode(
        buffer, m_hdrDecoder.encodedLength(), Car::sbeBlockLength(), Car::sbeSchemaVersion(), hdrSz + carEncodedLength);

    EXPECT_EQ(m_carDecoder.decodeLength(), expectedCarEncodedLength);

    EXPECT_EQ(m_carDecoder.serialNumber(), SERIAL_NUMBER);
    EXPECT_EQ(m_carDecoder.modelYear(), MODEL_YEAR);
    EXPECT_EQ(m_carDecoder.available(), AVAILABLE);
    EXPECT_EQ(m_carDecoder.code(), CODE);

    EXPECT_EQ(m_carDecoder.someNumbersLength(), 5u);
    for (std::uint64_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(m_carDecoder.someNumbers(i), static_cast<std::int32_t>(i));
    }

    EXPECT_EQ(m_carDecoder.vehicleCodeLength(), VEHICLE_CODE_LENGTH);
    EXPECT_EQ(std::string(m_carDecoder.vehicleCode(), VEHICLE_CODE_LENGTH),
        std::string(VEHICLE_CODE, VEHICLE_CODE_LENGTH));

    EXPECT_EQ(m_carDecoder.extras().cruiseControl(), true);
    EXPECT_EQ(m_carDecoder.extras().sportsPack(), true);
    EXPECT_EQ(m_carDecoder.extras().sunRoof(), false);

    EXPECT_EQ(m_carDecoder.discountedModel(), Model::C);

    Engine &engine = m_carDecoder.engine();
    EXPECT_EQ(engine.capacity(), engineCapacity);
    EXPECT_EQ(engine.numCylinders(), engineNumCylinders);
    EXPECT_EQ(engine.maxRpm(), 9000);
    EXPECT_EQ(engine.manufacturerCodeLength(), MANUFACTURER_CODE_LENGTH);
    EXPECT_EQ(std::string(engine.manufacturerCode(), MANUFACTURER_CODE_LENGTH),
        std::string(MANUFACTURER_CODE, MANUFACTURER_CODE_LENGTH));
    EXPECT_EQ(engine.fuelLength(), 6u);
    EXPECT_EQ(std::string(engine.fuel(), 6), std::string("Petrol"));

    Car::FuelFigures &fuelFigures = m_carDecoder.fuelFigures();
    EXPECT_EQ(fuelFigures.count(), FUEL_FIGURES_COUNT);

    ASSERT_TRUE(fuelFigures.hasNext());
    fuelFigures.next();
    EXPECT_EQ(fuelFigures.speed(), fuel1Speed);
    EXPECT_EQ(fuelFigures.mpg(), fuel1Mpg);
    EXPECT_EQ(fuelFigures.usageDescriptionLength(), FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(std::string(fuelFigures.usageDescription(), FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_1_USAGE_DESCRIPTION);

    ASSERT_TRUE(fuelFigures.hasNext());
    fuelFigures.next();
    EXPECT_EQ(fuelFigures.speed(), fuel2Speed);
    EXPECT_EQ(fuelFigures.mpg(), fuel2Mpg);
    EXPECT_EQ(fuelFigures.usageDescriptionLength(), FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(std::string(fuelFigures.usageDescription(), FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_2_USAGE_DESCRIPTION);

    ASSERT_TRUE(fuelFigures.hasNext());
    fuelFigures.next();
    EXPECT_EQ(fuelFigures.speed(), fuel3Speed);
    EXPECT_EQ(fuelFigures.mpg(), fuel3Mpg);
    EXPECT_EQ(fuelFigures.usageDescriptionLength(), FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(std::string(fuelFigures.usageDescription(), FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_3_USAGE_DESCRIPTION);

    Car::PerformanceFigures &performanceFigures = m_carDecoder.performanceFigures();
    EXPECT_EQ(performanceFigures.count(), PERFORMANCE_FIGURES_COUNT);

    ASSERT_TRUE(performanceFigures.hasNext());
    performanceFigures.next();
    EXPECT_EQ(performanceFigures.octaneRating(), perf1Octane);

    Car::PerformanceFigures::Acceleration &acceleration1 = performanceFigures.acceleration();
    EXPECT_EQ(acceleration1.count(), ACCELERATION_COUNT);
    ASSERT_TRUE(acceleration1.hasNext());
    acceleration1.next();
    EXPECT_EQ(acceleration1.mph(), perf1aMph);
    EXPECT_EQ(acceleration1.seconds(), perf1aSeconds);

    ASSERT_TRUE(acceleration1.hasNext());
    acceleration1.next();
    EXPECT_EQ(acceleration1.mph(), perf1bMph);
    EXPECT_EQ(acceleration1.seconds(), perf1bSeconds);

    ASSERT_TRUE(acceleration1.hasNext());
    acceleration1.next();
    EXPECT_EQ(acceleration1.mph(), perf1cMph);
    EXPECT_EQ(acceleration1.seconds(), perf1cSeconds);

    ASSERT_TRUE(performanceFigures.hasNext());
    performanceFigures.next();
    EXPECT_EQ(performanceFigures.octaneRating(), perf2Octane);

    Car::PerformanceFigures::Acceleration &acceleration2 = performanceFigures.acceleration();
    EXPECT_EQ(acceleration2.count(), ACCELERATION_COUNT);
    ASSERT_TRUE(acceleration2.hasNext());
    acceleration2.next();
    EXPECT_EQ(acceleration2.mph(), perf2aMph);
    EXPECT_EQ(acceleration2.seconds(), perf2aSeconds);

    ASSERT_TRUE(acceleration2.hasNext());
    acceleration2.next();
    EXPECT_EQ(acceleration2.mph(), perf2bMph);
    EXPECT_EQ(acceleration2.seconds(), perf2bSeconds);

    ASSERT_TRUE(acceleration2.hasNext());
    acceleration2.next();
    EXPECT_EQ(acceleration2.mph(), perf2cMph);
    EXPECT_EQ(acceleration2.seconds(), perf2cSeconds);

    EXPECT_EQ(m_carDecoder.manufacturerLength(), MANUFACTURER_LENGTH);
    EXPECT_EQ(std::string(m_carDecoder.manufacturer(), MANUFACTURER_LENGTH), MANUFACTURER);

    EXPECT_EQ(m_carDecoder.modelLength(), MODEL_LENGTH);
    EXPECT_EQ(std::string(m_carDecoder.model(), MODEL_LENGTH), MODEL);

    EXPECT_EQ(m_carDecoder.activationCodeLength(), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(m_carDecoder.activationCode(), ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);

    EXPECT_EQ(m_carDecoder.colorLength(), COLOR_LENGTH);
    EXPECT_EQ(std::string(m_carDecoder.color(), COLOR_LENGTH), COLOR);

    EXPECT_EQ(m_carDecoder.encodedLength(), expectedCarEncodedLength);
    EXPECT_EQ(m_carDecoder.decodeLength(), expectedCarEncodedLength);
}

struct CallbacksForEach
{
    int countOfFuelFigures = 0;
    int countOfPerformanceFigures = 0;
    int countOfAccelerations = 0;

    void operator()(Car::FuelFigures &fuelFigures)
    {
        countOfFuelFigures++;
        static_cast<void>(fuelFigures.usageDescription());
    }

    void operator()(Car::PerformanceFigures &performanceFigures)
    {
        Car::PerformanceFigures::Acceleration &acceleration = performanceFigures.acceleration();

        countOfPerformanceFigures++;
        acceleration.forEach(*this);
    }

    void operator()(Car::PerformanceFigures::Acceleration &)
    {
        countOfAccelerations++;
    }
};

TEST_F(CodeGenTest, shouldBeAbleUseOnStackCodecsAndGroupForEach)
{
    char buffer[BUFFER_LEN] = {};
    MessageHeader hdr(buffer, sizeof(buffer), 0);
    Car car(buffer + hdr.encodedLength(), sizeof(buffer) - hdr.encodedLength());

    std::uint64_t hdrSz = encodeHdr(hdr);
    std::uint64_t carEncodedLength = encodeCar(car);

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    const MessageHeader hdrDecoder(buffer, hdrSz, 0);

    EXPECT_EQ(hdrDecoder.blockLength(), Car::sbeBlockLength());
    EXPECT_EQ(hdrDecoder.templateId(), Car::sbeTemplateId());
    EXPECT_EQ(hdrDecoder.schemaId(), Car::sbeSchemaId());
    EXPECT_EQ(hdrDecoder.version(), Car::sbeSchemaVersion());
    EXPECT_EQ(hdrDecoder.encodedLength(), expectedHeaderSize);

    Car carDecoder(
        buffer + hdrDecoder.encodedLength(), carEncodedLength, hdrDecoder.blockLength(), hdrDecoder.version());
    CallbacksForEach cbs;

    Car::FuelFigures &fuelFigures = carDecoder.fuelFigures();
    EXPECT_EQ(fuelFigures.count(), FUEL_FIGURES_COUNT);

#if __cplusplus >= 201103L
    fuelFigures.forEach(
        [&](Car::FuelFigures &figures)
        {
            cbs.countOfFuelFigures++;

            char tmp[256] = {};
            figures.getUsageDescription(tmp, sizeof(tmp));
        });
#else
    fuelFigures.forEach(cbs);
#endif

    Car::PerformanceFigures &performanceFigures = carDecoder.performanceFigures();
    EXPECT_EQ(performanceFigures.count(), PERFORMANCE_FIGURES_COUNT);

#if __cplusplus >= 201103L
    performanceFigures.forEach([&](Car::PerformanceFigures &figures)
    {
        Car::PerformanceFigures::Acceleration &acceleration = figures.acceleration();

        cbs.countOfPerformanceFigures++;
        acceleration.forEach(
            [&](Car::PerformanceFigures::Acceleration &)
            {
                cbs.countOfAccelerations++;
            });
    });
#else
    performanceFigures.forEach(cbs);
#endif

    EXPECT_EQ(cbs.countOfFuelFigures, FUEL_FIGURES_COUNT);
    EXPECT_EQ(cbs.countOfPerformanceFigures, PERFORMANCE_FIGURES_COUNT);
    EXPECT_EQ(cbs.countOfAccelerations, ACCELERATION_COUNT * PERFORMANCE_FIGURES_COUNT);

    char tmp[256] = {};

    EXPECT_EQ(carDecoder.getManufacturer(tmp, sizeof(tmp)), MANUFACTURER_LENGTH);
    EXPECT_EQ(std::string(tmp, MANUFACTURER_LENGTH), MANUFACTURER);

    EXPECT_EQ(carDecoder.getModel(tmp, sizeof(tmp)), MODEL_LENGTH);
    EXPECT_EQ(std::string(tmp, MODEL_LENGTH), MODEL);

    EXPECT_EQ(carDecoder.getManufacturer(tmp, sizeof(tmp)), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(tmp, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);

    EXPECT_EQ(carDecoder.getColor(tmp, sizeof(tmp)), COLOR_LENGTH);
    EXPECT_EQ(std::string(tmp, COLOR_LENGTH), COLOR);

    EXPECT_EQ(carDecoder.encodedLength(), expectedCarEncodedLength);
}

static const std::size_t offsetVehicleCode = 32;
static const std::size_t offsetUsageDesc1Length = 57;
static const std::size_t offsetUsageDesc1Data = offsetUsageDesc1Length + sizeof(std::uint16_t);
static const std::size_t offsetUsageDesc2Length = 76;
static const std::size_t offsetUsageDesc2Data = offsetUsageDesc2Length + sizeof(std::uint16_t);
static const std::size_t offsetUsageDesc3Length = 98;
static const std::size_t offsetUsageDesc3Data = offsetUsageDesc3Length + sizeof(std::uint16_t);
static const std::size_t offsetManufacturerLength = 163;
static const std::size_t offsetManufacturerData = offsetManufacturerLength + sizeof(std::uint16_t);
static const std::size_t offsetModelLength = 170;
static const std::size_t offsetModelData = offsetModelLength + sizeof(std::uint16_t);
static const std::size_t offsetActivationCodeLength = 181;
static const std::size_t offsetActivationCodeData = offsetActivationCodeLength + sizeof(std::uint16_t);
static const std::size_t offsetColorLength = 191;
static const std::size_t offsetColorData = offsetColorLength + sizeof(std::uint32_t);

TEST_F(CodeGenTest, shouldBeAbleToUseStdStringMethodsForEncode)
{
    std::string vehicleCode(VEHICLE_CODE, Car::vehicleCodeLength());
    std::string usageDesc1(FUEL_FIGURES_1_USAGE_DESCRIPTION, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc2(FUEL_FIGURES_2_USAGE_DESCRIPTION, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc3(FUEL_FIGURES_3_USAGE_DESCRIPTION, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    std::string manufacturer(MANUFACTURER, MANUFACTURER_LENGTH);
    std::string model(MODEL, MODEL_LENGTH);
    std::string activationCode(ACTIVATION_CODE, ACTIVATION_CODE_LENGTH);
    std::string color(COLOR, COLOR_LENGTH);

    char buffer[BUFFER_LEN] = {};
    std::uint64_t baseOffset = MessageHeader::encodedLength();
    Car car;
    car.wrapForEncode(buffer, baseOffset, sizeof(buffer));

    car.putVehicleCode(vehicleCode);

    car.fuelFiguresCount(FUEL_FIGURES_COUNT)
        .next().putUsageDescription(usageDesc1)
        .next().putUsageDescription(usageDesc2)
        .next().putUsageDescription(usageDesc3);

    Car::PerformanceFigures &perfFigs = car.performanceFiguresCount(PERFORMANCE_FIGURES_COUNT);

    perfFigs.next()
        .accelerationCount(ACCELERATION_COUNT).next().next().next();

    perfFigs.next()
        .accelerationCount(ACCELERATION_COUNT).next().next().next();

    car.putManufacturer(manufacturer)
        .putModel(model)
        .putActivationCode(activationCode)
        .putColor(color);

    EXPECT_EQ(car.encodedLength(), expectedCarEncodedLength);

    EXPECT_EQ(std::string(buffer + baseOffset + offsetVehicleCode, VEHICLE_CODE_LENGTH), vehicleCode);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetUsageDesc1Length),
        static_cast<std::uint16_t>(FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetUsageDesc1Data, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH),
        usageDesc1);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetUsageDesc2Length),
        static_cast<std::uint16_t>(FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetUsageDesc2Data, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH),
        usageDesc2);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetUsageDesc3Length),
        static_cast<std::uint16_t>(FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetUsageDesc3Data, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH),
        usageDesc3);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetManufacturerLength),
        static_cast<std::uint16_t>(MANUFACTURER_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetManufacturerData, MANUFACTURER_LENGTH), manufacturer);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetModelLength),
        static_cast<std::uint16_t>(MODEL_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetModelData, MODEL_LENGTH), model);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetActivationCodeLength),
        static_cast<std::uint16_t>(ACTIVATION_CODE_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetActivationCodeData, ACTIVATION_CODE_LENGTH), activationCode);

    EXPECT_EQ(*(std::uint16_t *)(buffer + baseOffset + offsetColorLength),
        static_cast<std::uint16_t>(COLOR_LENGTH));
    EXPECT_EQ(std::string(buffer + baseOffset + offsetColorData, COLOR_LENGTH), color);
}

TEST_F(CodeGenTest, shouldBeAbleToUseStdStringMethodsForDecode)
{
    char buffer[BUFFER_LEN] = {};
    Car carEncoder(buffer, sizeof(buffer));

    std::uint64_t carEncodedLength = encodeCar(carEncoder);

    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    Car carDecoder(buffer, carEncodedLength, Car::sbeBlockLength(), Car::sbeSchemaVersion());

    std::string vehicleCode(VEHICLE_CODE, Car::vehicleCodeLength());
    std::string usageDesc1(FUEL_FIGURES_1_USAGE_DESCRIPTION, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc2(FUEL_FIGURES_2_USAGE_DESCRIPTION, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc3(FUEL_FIGURES_3_USAGE_DESCRIPTION, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    std::string manufacturer(MANUFACTURER, MANUFACTURER_LENGTH);
    std::string model(MODEL, MODEL_LENGTH);
    std::string activationCode(ACTIVATION_CODE, ACTIVATION_CODE_LENGTH);
    std::string color(COLOR, COLOR_LENGTH);

    EXPECT_EQ(carDecoder.getVehicleCodeAsString(), vehicleCode);

    Car::FuelFigures &fuelFigures = carDecoder.fuelFigures();

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc1);

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc2);

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc3);

    Car::PerformanceFigures &perfFigures = carDecoder.performanceFigures();

    perfFigures.next();
    Car::PerformanceFigures::Acceleration &acceleration1 = perfFigures.acceleration();

    acceleration1.next().next().next();

    perfFigures.next();
    Car::PerformanceFigures::Acceleration &acceleration2 = perfFigures.acceleration();
    acceleration2.next().next().next();

    EXPECT_EQ(carDecoder.getManufacturerAsString(), manufacturer);
    EXPECT_EQ(carDecoder.getModelAsString(), model);
    EXPECT_EQ(carDecoder.getActivationCodeAsString(), activationCode);
    EXPECT_EQ(carDecoder.getColorAsString(), color);

    EXPECT_EQ(carDecoder.encodedLength(), expectedCarEncodedLength);
}

TEST_F(CodeGenTest, shouldPrintFullDecodedFlyweightRegardlessOfReadPosition)
{
    const char *expectedDisplayString =
        "{\"Name\": \"Car\", \"sbeTemplateId\": 1, \"serialNumber\": 1234, \"modelYear\": 2013, \"available\": \"T\", "
        "\"code\": \"A\", \"someNumbers\": [0,1,2,3,4], \"vehicleCode\": \"abcdef\", "
        "\"extras\": [\"sportsPack\",\"cruiseControl\"], \"discountedModel\": \"C\", "
        "\"engine\": {\"capacity\": 2000, \"numCylinders\": 4, \"manufacturerCode\": \"123\", "
        "\"booster\": {\"BoostType\": \"NITROUS\", \"horsePower\": 200}}, "
        "\"fuelFigures\": [{\"speed\": 30, \"mpg\": 35.9, \"usageDescription\": \"Urban Cycle\"}, "
        "{\"speed\": 55, \"mpg\": 49, \"usageDescription\": \"Combined Cycle\"}, "
        "{\"speed\": 75, \"mpg\": 40, \"usageDescription\": \"Highway Cycle\"}], "
        "\"performanceFigures\": [{\"octaneRating\": 95, \"acceleration\": ["
        "{\"mph\": 30, \"seconds\": 4}, {\"mph\": 60, \"seconds\": 7.5}, {\"mph\": 100, \"seconds\": 12.2}]}, "
        "{\"octaneRating\": 99, \"acceleration\": [{\"mph\": 30, \"seconds\": 3.8}, {\"mph\": 60, \"seconds\": 7.1}, "
        "{\"mph\": 100, \"seconds\": 11.8}]}], \"manufacturer\": \"Honda\", \"model\": \"Civic VTi\", "
        "\"activationCode\": \"deadbeef\", \"color\": \"racing green\"}";

    char buffer[BUFFER_LEN] = {};
    Car carEncoder(buffer, sizeof(buffer));

    std::uint64_t carEncodedLength = encodeCar(carEncoder);

    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    Car carDecoder(buffer, carEncodedLength, Car::sbeBlockLength(), Car::sbeSchemaVersion());

    EXPECT_EQ(carDecoder.decodeLength(), expectedCarEncodedLength);

    std::string vehicleCode(VEHICLE_CODE, Car::vehicleCodeLength());
    std::string usageDesc1(FUEL_FIGURES_1_USAGE_DESCRIPTION, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc2(FUEL_FIGURES_2_USAGE_DESCRIPTION, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc3(FUEL_FIGURES_3_USAGE_DESCRIPTION, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    std::string manufacturer(MANUFACTURER, MANUFACTURER_LENGTH);
    std::string model(MODEL, MODEL_LENGTH);
    std::string activationCode(ACTIVATION_CODE, ACTIVATION_CODE_LENGTH);
    std::string color(COLOR, COLOR_LENGTH);

    expectDisplayString(expectedDisplayString, carDecoder);

    EXPECT_EQ(carDecoder.getVehicleCodeAsString(), vehicleCode);

    Car::FuelFigures &fuelFigures = carDecoder.fuelFigures();

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc1);

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc2);

    fuelFigures.next();
    EXPECT_EQ(fuelFigures.getUsageDescriptionAsString(), usageDesc3);

    expectDisplayString(expectedDisplayString, carDecoder);

    Car::PerformanceFigures &perfFigures = carDecoder.performanceFigures();

    perfFigures.next();
    Car::PerformanceFigures::Acceleration &acceleration1 = perfFigures.acceleration();

    acceleration1.next().next().next();

    expectDisplayString(expectedDisplayString, carDecoder);

    perfFigures.next();
    Car::PerformanceFigures::Acceleration &acceleration2 = perfFigures.acceleration();
    acceleration2.next().next().next();

    EXPECT_EQ(carDecoder.getManufacturerAsString(), manufacturer);
    EXPECT_EQ(carDecoder.getModelAsString(), model);

    expectDisplayString(expectedDisplayString, carDecoder);

    EXPECT_EQ(carDecoder.getActivationCodeAsString(), activationCode);
    EXPECT_EQ(carDecoder.getColorAsString(), color);

    expectDisplayString(expectedDisplayString, carDecoder);

    EXPECT_EQ(carDecoder.encodedLength(), expectedCarEncodedLength);
}

TEST_F(CodeGenTest, shouldAllowForMultipleIterations)
{
    char buffer[BUFFER_LEN] = {};

    Car carEncoder(buffer, sizeof(buffer));
    uint64_t encodedLength = encodeCar(carEncoder);
    Car carDecoder(buffer, encodedLength, Car::sbeBlockLength(), Car::sbeSchemaVersion());

    std::string passOne = walkCar(carDecoder);
    carDecoder.sbeRewind();
    std::string passTwo = walkCar(carDecoder);
    EXPECT_EQ(passOne, passTwo);

    carDecoder.sbeRewind();
    std::string passThree = partialWalkCar(carDecoder);
    carDecoder.sbeRewind();
    std::string passFour = partialWalkCar(carDecoder);
    EXPECT_EQ(passThree, passFour);

    carDecoder.sbeRewind();
    std::string passFive = walkCar(carDecoder);
    EXPECT_EQ(passOne, passFive);
}
