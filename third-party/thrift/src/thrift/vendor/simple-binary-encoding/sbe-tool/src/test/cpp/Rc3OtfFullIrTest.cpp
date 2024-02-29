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

#include "gtest/gtest.h"
#include "code_generation_test/MessageHeader.h"
#include "code_generation_test/Car.h"
#include "otf/Token.h"
#include "otf/IrDecoder.h"
#include "otf/OtfHeaderDecoder.h"
#include "otf/OtfMessageDecoder.h"

using namespace code::generation::test;

SBE_CONSTEXPR const char *SCHEMA_FILENAME = "code-generation-schema.sbeir";

SBE_CONSTEXPR std::uint8_t fieldIdSerialNumber = 1;
SBE_CONSTEXPR std::uint8_t fieldIdModelYear = 2;
SBE_CONSTEXPR std::uint8_t fieldIdAvailable = 3;
SBE_CONSTEXPR std::uint8_t fieldIdCode = 4;
SBE_CONSTEXPR std::uint8_t fieldIdSomeNumbers = 5;
SBE_CONSTEXPR std::uint8_t fieldIdVehicleCode = 6;
SBE_CONSTEXPR std::uint8_t fieldIdExtras = 7;
SBE_CONSTEXPR std::uint8_t fieldIdDiscountedModel = 8;
SBE_CONSTEXPR std::uint8_t fieldIdEngine = 9;
SBE_CONSTEXPR std::uint8_t fieldIdFuelFigures = 10;
SBE_CONSTEXPR std::uint8_t fieldIdFuelSpeed = 11;
SBE_CONSTEXPR std::uint8_t fieldIdFuelMpg = 12;
SBE_CONSTEXPR std::uint8_t fieldIdFuelUsageDescription = 200;
SBE_CONSTEXPR std::uint8_t fieldIdPerformanceFigures = 13;
SBE_CONSTEXPR std::uint8_t fieldIdPerfOctaneRating = 14;
SBE_CONSTEXPR std::uint8_t fieldIdPerfAcceleration = 15;
SBE_CONSTEXPR std::uint8_t fieldIdPerfAccMph = 16;
SBE_CONSTEXPR std::uint8_t fieldIdPerfAccSeconds = 17;
SBE_CONSTEXPR std::uint8_t fieldIdManufacturer = 18;
SBE_CONSTEXPR std::uint8_t fieldIdModel = 19;
SBE_CONSTEXPR std::uint8_t fieldIdActivationCode = 20;
SBE_CONSTEXPR std::uint8_t fieldIdColor = 21;

SBE_CONSTEXPR std::uint32_t SERIAL_NUMBER = 1234;
SBE_CONSTEXPR std::uint16_t MODEL_YEAR = 2013;
SBE_CONSTEXPR BooleanType::Value AVAILABLE = BooleanType::T;
SBE_CONSTEXPR Model::Value CODE = Model::A;
SBE_CONSTEXPR bool CRUISE_CONTROL = true;
SBE_CONSTEXPR bool SPORTS_PACK = true;
SBE_CONSTEXPR bool SUNROOF = false;

static char VEHICLE_CODE[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
static char MANUFACTURER_CODE[] = { '1', '2', '3' };
SBE_CONSTEXPR const char *FUEL_FIGURES_1_USAGE_DESCRIPTION = "Urban Cycle";
SBE_CONSTEXPR const char *FUEL_FIGURES_2_USAGE_DESCRIPTION = "Combined Cycle";
SBE_CONSTEXPR const char *FUEL_FIGURES_3_USAGE_DESCRIPTION = "Highway Cycle";
SBE_CONSTEXPR const char *MANUFACTURER = "Honda";
SBE_CONSTEXPR const char *MODEL = "Civic VTi";
SBE_CONSTEXPR const char *ACTIVATION_CODE = "deadbeef";
SBE_CONSTEXPR const char *COLOR = "red";

static const int VEHICLE_CODE_LENGTH = sizeof(VEHICLE_CODE);
static const int MANUFACTURER_CODE_LENGTH = sizeof(MANUFACTURER_CODE);
SBE_CONSTEXPR std::size_t MANUFACTURER_LENGTH = 5;
SBE_CONSTEXPR std::size_t MODEL_LENGTH = 9;
SBE_CONSTEXPR std::size_t ACTIVATION_CODE_LENGTH = 8;
SBE_CONSTEXPR std::size_t COLOR_LENGTH = 3;
SBE_CONSTEXPR std::uint16_t PERFORMANCE_FIGURES_COUNT = 2;
SBE_CONSTEXPR std::uint16_t FUEL_FIGURES_COUNT = 3;
SBE_CONSTEXPR std::uint16_t ACCELERATION_COUNT = 3;

SBE_CONSTEXPR std::uint16_t fuel1Speed = 30;
SBE_CONSTEXPR float fuel1Mpg = 35.9f;
SBE_CONSTEXPR std::uint16_t fuel2Speed = 55;
SBE_CONSTEXPR float fuel2Mpg = 49.0f;
SBE_CONSTEXPR std::uint16_t fuel3Speed = 75;
SBE_CONSTEXPR float fuel3Mpg = 40.0f;

SBE_CONSTEXPR std::uint8_t perf1Octane = 95;
SBE_CONSTEXPR std::uint16_t perf1aMph = 30;
SBE_CONSTEXPR float perf1aSeconds = 4.0f;
SBE_CONSTEXPR std::uint16_t perf1bMph = 60;
SBE_CONSTEXPR float perf1bSeconds = 7.5f;
SBE_CONSTEXPR std::uint16_t perf1cMph = 100;
SBE_CONSTEXPR float perf1cSeconds = 12.2f;

SBE_CONSTEXPR std::uint8_t perf2Octane = 99;
SBE_CONSTEXPR std::uint16_t perf2aMph = 30;
SBE_CONSTEXPR float perf2aSeconds = 3.8f;
SBE_CONSTEXPR std::uint16_t perf2bMph = 60;
SBE_CONSTEXPR float perf2bSeconds = 7.1f;
SBE_CONSTEXPR std::uint16_t perf2cMph = 100;
SBE_CONSTEXPR float perf2cSeconds = 11.8f;

SBE_CONSTEXPR std::uint16_t engineCapacity = 2000;
SBE_CONSTEXPR std::uint8_t engineNumCylinders = 4;

SBE_CONSTEXPR std::uint64_t encodedCarAndHdrLength = 198 + 8;

// This enum represents the expected events that
// will be received during the decoding process.
// Warning: this is for testing only.  Do not use this technique in production code.
enum EventNumber
{
    EN_beginMessage = 0,
    EN_serialNumber,
    EN_modelYear,
    EN_available,
    EN_code,
    EN_someNumbers,
    EN_vehicleCode,
    EN_extras,
    EN_discountedModel,
    EN_beginEngine,
    EN_engine_capacity,
    EN_engine_numCylinders,
    EN_engine_maxRpm,
    EN_engine_manufacturerCode,
    EN_engine_fuel,
    EN_beginBooster,
    EN_engine_booster_boostType,
    EN_engine_booster_horsePower,
    EN_endBooster,
    EN_endEngine,
    EN_groupFuelFigures,
    EN_beginFuelFigures1,
    EN_fuelFigures1_speed,
    EN_fuelFigures1_mpg,
    EN_fuelFigures1_usageDescription,
    EN_endFuelFigures1,
    EN_beginFuelFigures2,
    EN_fuelFigures2_speed,
    EN_fuelFigures2_mpg,
    EN_fuelFigures2_usageDescription,
    EN_endFuelFigures2,
    EN_beginFuelFigures3,
    EN_fuelFigures3_speed,
    EN_fuelFigures3_mpg,
    EN_fuelFigures3_usageDescription,
    EN_endFuelFigures3,
    EN_groupPerformanceFigures,
    EN_beginPerformanceFigures1,
    EN_performanceFigures1_octaneRating,
    EN_performanceFigures1_groupAcceleration1,
    EN_performanceFigures1_beginAcceleration1,
    EN_performanceFigures1_acceleration1_mph,
    EN_performanceFigures1_acceleration1_seconds,
    EN_performanceFigures1_endAcceleration1,
    EN_performanceFigures1_beginAcceleration2,
    EN_performanceFigures1_acceleration2_mph,
    EN_performanceFigures1_acceleration2_seconds,
    EN_performanceFigures1_endAcceleration2,
    EN_performanceFigures1_beginAcceleration3,
    EN_performanceFigures1_acceleration3_mph,
    EN_performanceFigures1_acceleration3_seconds,
    EN_performanceFigures1_endAcceleration3,
    EN_endPerformanceFigures1,
    EN_beginPerformanceFigures2,
    EN_performanceFigures2_octaneRating,
    EN_performanceFigures2_groupAcceleration1,
    EN_performanceFigures2_beginAcceleration1,
    EN_performanceFigures2_acceleration1_mph,
    EN_performanceFigures2_acceleration1_seconds,
    EN_performanceFigures2_endAcceleration1,
    EN_performanceFigures2_beginAcceleration2,
    EN_performanceFigures2_acceleration2_mph,
    EN_performanceFigures2_acceleration2_seconds,
    EN_performanceFigures2_endAcceleration2,
    EN_performanceFigures2_beginAcceleration3,
    EN_performanceFigures2_acceleration3_mph,
    EN_performanceFigures2_acceleration3_seconds,
    EN_performanceFigures2_endAcceleration3,
    EN_endPerformanceFigures2,
    EN_manufacturer,
    EN_model,
    EN_activationCode,
    EN_color,
    EN_endMessage
};

class Rc3OtfFullIrTest : public testing::Test
{
public:
    char m_buffer[2048] = {};
    IrDecoder m_irDecoder = {};
    int m_eventNumber = 0;
    int m_compositeLevel = 0;

    void SetUp() override
    {
        m_eventNumber = 0;
        m_compositeLevel = 0;
    }

    std::string determineName(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex) const
    {
        return (m_compositeLevel > 1) ? tokens.at(fromIndex).name() : fieldToken.name();
    }

    std::uint64_t encodeHdrAndCar()
    {
        MessageHeader hdr;
        Car car;

        hdr.wrap(m_buffer, 0, 0, sizeof(m_buffer))
            .blockLength(Car::sbeBlockLength())
            .templateId(Car::sbeTemplateId())
            .schemaId(Car::sbeSchemaId())
            .version(Car::sbeSchemaVersion());

        car.wrapForEncode(m_buffer, hdr.encodedLength(), sizeof(m_buffer))
            .serialNumber(SERIAL_NUMBER)
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
            .booster().boostType(BoostType::NITROUS).horsePower(200);

        Car::FuelFigures &fuelFigures = car.fuelFiguresCount(FUEL_FIGURES_COUNT);

        fuelFigures
            .next().speed(fuel1Speed).mpg(fuel1Mpg);

        fuelFigures.putUsageDescription(
            FUEL_FIGURES_1_USAGE_DESCRIPTION, static_cast<int>(strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION)));

        fuelFigures
            .next().speed(fuel2Speed).mpg(fuel2Mpg);
        fuelFigures.putUsageDescription(
            FUEL_FIGURES_2_USAGE_DESCRIPTION, static_cast<int>(strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION)));

        fuelFigures
            .next().speed(fuel3Speed).mpg(fuel3Mpg);
        fuelFigures.putUsageDescription(
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

        car.putManufacturer(MANUFACTURER, static_cast<int>(strlen(MANUFACTURER)));
        car.putModel(MODEL, static_cast<int>(strlen(MODEL)));
        car.putActivationCode(ACTIVATION_CODE, static_cast<int>(strlen(ACTIVATION_CODE)));
        car.putColor(COLOR, static_cast<int>(strlen(COLOR)));

        return hdr.encodedLength() + car.encodedLength();
    }

    void onBeginMessage(Token &token)
    {
        std::cout << m_eventNumber << ": Begin Message " << token.name() << " id " << token.fieldId() << "\n";

        EXPECT_EQ(EventNumber(m_eventNumber), EN_beginMessage);
        EXPECT_EQ(token.fieldId(), Car::sbeTemplateId());
        m_eventNumber++;
    }

    void onEndMessage(Token &token)
    {
        std::cout << m_eventNumber << ": End Message " << token.name() << "\n";

        EXPECT_EQ(EventNumber(m_eventNumber), EN_endMessage);
        EXPECT_EQ(token.fieldId(), Car::sbeTemplateId());
        m_eventNumber++;
    }

    void onEncoding(
        Token &fieldToken,
        const char *buffer,
        Token &typeToken,
        std::uint64_t actingVersion)
    {
        std::string name = (m_compositeLevel > 1) ? typeToken.name() : fieldToken.name();
        std::cout << m_eventNumber << ": Encoding " << name << " offset " << typeToken.offset() << "\n";

        const Encoding &encoding = typeToken.encoding();

        switch (EventNumber(m_eventNumber))
        {
            case EN_serialNumber:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdSerialNumber);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT64);
                EXPECT_EQ(encoding.getAsUInt(buffer), static_cast<std::uint64_t>(SERIAL_NUMBER));
                break;
            }

            case EN_modelYear:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdModelYear);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), static_cast<std::uint64_t>(MODEL_YEAR));
                break;
            }

            case EN_someNumbers:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdSomeNumbers);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::INT32);
                EXPECT_EQ(typeToken.encodedLength(), 4 * 5);

                for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
                {
                    EXPECT_EQ(encoding.getAsInt(buffer + (i * 4)), static_cast<std::int64_t>(i));
                }
                break;
            }

            case EN_vehicleCode:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdVehicleCode);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);
                EXPECT_EQ(typeToken.encodedLength(), VEHICLE_CODE_LENGTH);
                EXPECT_EQ(std::string(buffer, VEHICLE_CODE_LENGTH), std::string(VEHICLE_CODE, VEHICLE_CODE_LENGTH));
                break;
            }

            case EN_engine_capacity:
            {
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), static_cast<std::uint64_t>(engineCapacity));
                break;
            }

            case EN_engine_numCylinders:
            {
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), static_cast<std::uint64_t>(engineNumCylinders));
                break;
            }

            case EN_engine_maxRpm:
            {
                EXPECT_TRUE(typeToken.isConstantEncoding());
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.constValue().getAsUInt(), static_cast<std::uint64_t>(9000));
                break;
            }

            case EN_engine_manufacturerCode:
            {
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);
                EXPECT_EQ(typeToken.encodedLength(), MANUFACTURER_CODE_LENGTH);
                EXPECT_EQ(std::string(buffer, MANUFACTURER_CODE_LENGTH),
                    std::string(MANUFACTURER_CODE, MANUFACTURER_CODE_LENGTH));
                break;
            }

            case EN_engine_fuel:
            {
                EXPECT_TRUE(typeToken.isConstantEncoding());
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);

                const PrimitiveValue &value = encoding.constValue();
                EXPECT_EQ(value.size(), static_cast<std::size_t>(6));
                EXPECT_EQ(std::string(value.getArray(), value.size()), std::string("Petrol"));
                break;
            }

            case EN_engine_booster_horsePower:
            {
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), static_cast<std::uint64_t>(200));
                break;
            }

            case EN_fuelFigures1_speed:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelSpeed);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), fuel1Speed);
                break;
            }

            case EN_fuelFigures1_mpg:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelMpg);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), fuel1Mpg);
                break;
            }

            case EN_fuelFigures2_speed:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelSpeed);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), fuel2Speed);
                break;
            }

            case EN_fuelFigures2_mpg:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelMpg);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), fuel2Mpg);
                break;
            }

            case EN_fuelFigures3_speed:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelSpeed);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), fuel3Speed);
                break;
            }

            case EN_fuelFigures3_mpg:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelMpg);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), fuel3Mpg);
                break;
            }

            case EN_performanceFigures1_octaneRating:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfOctaneRating);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf1Octane);
                break;
            }

            case EN_performanceFigures1_acceleration1_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf1aMph);
                break;
            }

            case EN_performanceFigures1_acceleration1_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf1aSeconds);
                break;
            }

            case EN_performanceFigures1_acceleration2_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf1bMph);
                break;
            }

            case EN_performanceFigures1_acceleration2_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf1bSeconds);
                break;
            }

            case EN_performanceFigures1_acceleration3_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf1cMph);
                break;
            }

            case EN_performanceFigures1_acceleration3_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf1cSeconds);
                break;
            }

            case EN_performanceFigures2_octaneRating:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfOctaneRating);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf2Octane);
                break;
            }

            case EN_performanceFigures2_acceleration1_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf2aMph);
                break;
            }

            case EN_performanceFigures2_acceleration1_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf2aSeconds);
                break;
            }

            case EN_performanceFigures2_acceleration2_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf2bMph);
                break;
            }

            case EN_performanceFigures2_acceleration2_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf2bSeconds);
                break;
            }

            case EN_performanceFigures2_acceleration3_mph:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccMph);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT16);
                EXPECT_EQ(encoding.getAsUInt(buffer), perf2cMph);
                break;
            }

            case EN_performanceFigures2_acceleration3_seconds:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdPerfAccSeconds);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::FLOAT);
                EXPECT_EQ(encoding.getAsDouble(buffer), perf2cSeconds);
                break;
            }

            default:
                FAIL() << "unknown Encoding event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onEnum(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion)
    {
        std::cout << m_eventNumber << ": Enum " << determineName(fieldToken, tokens, fromIndex) << "\n";

        const Token &typeToken = tokens.at(fromIndex + 1);
        const Encoding &encoding = typeToken.encoding();

        switch (EventNumber(m_eventNumber))
        {
            case EN_available:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdAvailable);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);

                const std::uint64_t value = encoding.getAsUInt(buffer);
                EXPECT_EQ(value, static_cast<std::uint64_t>(1));

                bool found = false;
                for (std::size_t i = fromIndex + 1; i < toIndex; i++)
                {
                    const Token &token = tokens.at(i);
                    const std::uint64_t constValue = token.encoding().constValue().getAsUInt();

                    std::cout << "    " << token.name() << " = " << constValue << "\n";

                    if (constValue == value)
                    {
                        EXPECT_EQ(token.name(), std::string("T"));
                        found = true;
                    }
                }
                EXPECT_TRUE(found);
                break;
            }

            case EN_code:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdCode);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);

                const std::int64_t value = encoding.getAsInt(buffer);
                EXPECT_EQ(value, static_cast<std::int64_t>('A'));

                bool found = false;
                for (std::size_t i = fromIndex + 1; i < toIndex; i++)
                {
                    const Token &token = tokens.at(i);
                    const std::int64_t constValue = token.encoding().constValue().getAsUInt();

                    std::cout << "    " << token.name() << " = " << constValue << "\n";

                    if (constValue == value)
                    {
                        EXPECT_EQ(token.name(), std::string("A"));
                        found = true;
                    }
                }
                EXPECT_TRUE(found);
                break;
            }

            case EN_discountedModel:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdDiscountedModel);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);
                EXPECT_TRUE(fieldToken.isConstantEncoding());
                EXPECT_EQ(fieldToken.encodedLength(), 0);
                EXPECT_EQ(std::string(fieldToken.encoding().constValue().getArray()), std::string("Model.C"));
                break;
            }

            case EN_engine_booster_boostType:
            {
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::CHAR);

                const std::int64_t value = encoding.getAsInt(buffer);
                EXPECT_EQ(value, static_cast<std::int64_t>('N'));

                bool found = false;
                for (std::size_t i = fromIndex + 1; i < toIndex; i++)
                {
                    const Token &token = tokens.at(i);
                    const std::int64_t constValue = token.encoding().constValue().getAsUInt();

                    std::cout << "    " << token.name() << " = " << constValue << "\n";

                    if (constValue == value)
                    {
                        EXPECT_EQ(token.name(), std::string("NITROUS"));
                        found = true;
                    }
                }
                EXPECT_TRUE(found);
                break;
            }

            default:
                FAIL() << "unknown Enum event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onBitSet(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion)
    {
        std::cout << m_eventNumber << ": Bit Set " << fieldToken.name() << "\n";

        const Token &typeToken = tokens.at(fromIndex + 1);
        const Encoding &encoding = typeToken.encoding();

        switch (EventNumber(m_eventNumber))
        {
            case EN_extras:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdExtras);
                EXPECT_EQ(encoding.primitiveType(), PrimitiveType::UINT8);

                const std::uint64_t value = encoding.getAsUInt(buffer);
                EXPECT_EQ(value, static_cast<std::uint64_t>(0x6));

                int bitsSet = 0;
                for (std::size_t i = fromIndex + 1; i < toIndex; i++)
                {
                    const Token &token = tokens.at(i);
                    const std::uint64_t constValue = token.encoding().constValue().getAsUInt();

                    if (constValue && value)
                    {
                        std::cout << "    * ";
                        bitsSet++;
                    }
                    else
                    {
                        std::cout << "      ";
                    }

                    std::cout << token.name() << " = " << constValue << "\n";
                }
                EXPECT_EQ(bitsSet, 2);
                break;
            }

            default:
                FAIL() << "unknown BitSet event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onBeginComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex)
    {
        m_compositeLevel++;
        std::string name = determineName(fieldToken, tokens, fromIndex);

        std::cout << m_eventNumber << ": Begin Composite " << name << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_beginEngine:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdEngine);
                break;
            }

            case EN_beginBooster:
            {
                EXPECT_EQ(tokens.at(fromIndex).name(), "booster");
                break;
            }

            default:
                FAIL() << "unknown BeginComposite event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onEndComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex)
    {
        std::string name = determineName(fieldToken, tokens, fromIndex);
        m_compositeLevel--;

        std::cout << m_eventNumber << ": End Composite " << name << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_endEngine:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdEngine);
                break;
            }

            case EN_endBooster:
            {
                EXPECT_EQ(tokens.at(fromIndex).name(), "booster");
                break;
            }

            default:
                FAIL() << "unknown BeginComposite event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onGroupHeader(Token &token, std::uint64_t numInGroup)
    {
        std::cout << m_eventNumber << ": Group Header " << token.name() << " num " << numInGroup << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_groupFuelFigures:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_groupPerformanceFigures:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerformanceFigures);
                EXPECT_EQ(numInGroup, PERFORMANCE_FIGURES_COUNT);
                break;
            }

            case EN_performanceFigures1_groupAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures2_groupAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            default:
                FAIL() << "unknown GroupHeader event number " << m_eventNumber;
        }

        m_eventNumber++;

    }

    void onBeginGroup(Token &token, std::uint64_t groupIndex, std::uint64_t numInGroup)
    {
        std::cout << m_eventNumber << ": Begin Group " << token.name()
                  << " " << groupIndex + 1 << "/" << numInGroup << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_beginFuelFigures1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_beginFuelFigures2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_beginFuelFigures3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_beginPerformanceFigures1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerformanceFigures);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, PERFORMANCE_FIGURES_COUNT);
                break;
            }

            case EN_performanceFigures1_beginAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures1_beginAcceleration2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures1_beginAcceleration3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_beginPerformanceFigures2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerformanceFigures);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, PERFORMANCE_FIGURES_COUNT);
                break;
            }

            case EN_performanceFigures2_beginAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures2_beginAcceleration2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures2_beginAcceleration3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            default:
                FAIL() << "unknown BeginGroup event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onEndGroup(Token &token, std::uint64_t groupIndex, std::uint64_t numInGroup)
    {
        std::cout << m_eventNumber << ": End Group " << token.name()
                  << " " << groupIndex + 1 << "/" << numInGroup << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_endFuelFigures1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_endFuelFigures2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_endFuelFigures3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdFuelFigures);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, FUEL_FIGURES_COUNT);
                break;
            }

            case EN_endPerformanceFigures1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerformanceFigures);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, PERFORMANCE_FIGURES_COUNT);
                break;
            }

            case EN_performanceFigures1_endAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures1_endAcceleration2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures1_endAcceleration3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_endPerformanceFigures2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerformanceFigures);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, PERFORMANCE_FIGURES_COUNT);
                break;
            }

            case EN_performanceFigures2_endAcceleration1:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 0u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures2_endAcceleration2:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 1u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            case EN_performanceFigures2_endAcceleration3:
            {
                EXPECT_EQ(token.fieldId(), fieldIdPerfAcceleration);
                EXPECT_EQ(groupIndex, 2u);
                EXPECT_EQ(numInGroup, ACCELERATION_COUNT);
                break;
            }

            default:
                FAIL() << "unknown EndGroup event number " << m_eventNumber;
        }

        m_eventNumber++;
    }

    void onVarData(
        Token &fieldToken,
        const char *buffer,
        std::uint64_t length,
        Token &typeToken)
    {
        std::cout << m_eventNumber << ": Data " << fieldToken.name() << "\n";

        switch (EventNumber(m_eventNumber))
        {
            case EN_fuelFigures1_usageDescription:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelUsageDescription);
                EXPECT_EQ(length, strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION));
                EXPECT_EQ(std::string(buffer, length), FUEL_FIGURES_1_USAGE_DESCRIPTION);
                break;
            }

            case EN_fuelFigures2_usageDescription:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelUsageDescription);
                EXPECT_EQ(length, strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION));
                EXPECT_EQ(std::string(buffer, length), FUEL_FIGURES_2_USAGE_DESCRIPTION);
                break;
            }

            case EN_fuelFigures3_usageDescription:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdFuelUsageDescription);
                EXPECT_EQ(length, strlen(FUEL_FIGURES_3_USAGE_DESCRIPTION));
                EXPECT_EQ(std::string(buffer, length), FUEL_FIGURES_3_USAGE_DESCRIPTION);
                break;
            }

            case EN_manufacturer:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdManufacturer);
                EXPECT_EQ(length, MANUFACTURER_LENGTH);
                EXPECT_EQ(std::string(buffer, MANUFACTURER_LENGTH), MANUFACTURER);
                break;
            }

            case EN_model:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdModel);
                EXPECT_EQ(length, MODEL_LENGTH);
                EXPECT_EQ(std::string(buffer, MODEL_LENGTH), MODEL);
                break;
            }

            case EN_activationCode:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdActivationCode);
                EXPECT_EQ(length, ACTIVATION_CODE_LENGTH);
                EXPECT_EQ(std::string(buffer, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);
                break;
            }

            case EN_color:
            {
                EXPECT_EQ(fieldToken.fieldId(), fieldIdColor);
                EXPECT_EQ(length, COLOR_LENGTH);
                EXPECT_EQ(std::string(buffer, COLOR_LENGTH), COLOR);
                break;
            }

            default:
                FAIL() << "unknown Data event number " << m_eventNumber;
        }

        m_eventNumber++;
    }
};

class Rc3OtfFullIrLengthTest : public Rc3OtfFullIrTest, public ::testing::WithParamInterface<int>
{
};

TEST_F(Rc3OtfFullIrTest, shouldHandleDecodingOfMessageHeaderCorrectly)
{
    ASSERT_EQ(encodeHdrAndCar(), encodedCarAndHdrLength);

    ASSERT_GE(m_irDecoder.decode(SCHEMA_FILENAME), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();

    ASSERT_TRUE(headerTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    EXPECT_EQ(headerDecoder.getTemplateId(m_buffer), Car::sbeTemplateId());
    EXPECT_EQ(headerDecoder.getBlockLength(m_buffer), Car::sbeBlockLength());
    EXPECT_EQ(headerDecoder.getSchemaId(m_buffer), Car::sbeSchemaId());
    EXPECT_EQ(headerDecoder.getSchemaVersion(m_buffer), Car::sbeSchemaVersion());
}

TEST_F(Rc3OtfFullIrTest, shouldHandleAllEventsCorrectlyAndInOrder)
{
    ASSERT_EQ(encodeHdrAndCar(), encodedCarAndHdrLength);

    ASSERT_GE(m_irDecoder.decode(SCHEMA_FILENAME), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();
    std::shared_ptr<std::vector<Token>> messageTokens = m_irDecoder.message(
        Car::sbeTemplateId(), Car::sbeSchemaVersion());

    ASSERT_TRUE(headerTokens != nullptr);
    ASSERT_TRUE(messageTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    const char *messageBuffer = m_buffer + headerDecoder.encodedLength();
    std::size_t length = encodedCarAndHdrLength - headerDecoder.encodedLength();
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(m_buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(m_buffer);

    const std::size_t result = OtfMessageDecoder::decode(
        messageBuffer, length, actingVersion, blockLength, messageTokens, *this);
    EXPECT_EQ(result, static_cast<std::size_t>(encodedCarAndHdrLength - MessageHeader::encodedLength()));
}

TEST_P(Rc3OtfFullIrLengthTest, shouldExceptionIfLengthTooShort)
{
    ASSERT_EQ(encodeHdrAndCar(), encodedCarAndHdrLength);

    ASSERT_GE(m_irDecoder.decode(SCHEMA_FILENAME), 0);

    std::shared_ptr<std::vector<Token>> headerTokens = m_irDecoder.header();
    std::shared_ptr<std::vector<Token>> messageTokens = m_irDecoder.message(
        Car::sbeTemplateId(), Car::sbeSchemaVersion());

    ASSERT_TRUE(headerTokens != nullptr);
    ASSERT_TRUE(messageTokens != nullptr);

    OtfHeaderDecoder headerDecoder(headerTokens);

    EXPECT_EQ(headerDecoder.encodedLength(), MessageHeader::encodedLength());
    auto length = static_cast<std::size_t>(GetParam());
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(m_buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(m_buffer);

    // set up so that if an error occurs, we intentionally write off the end of a new buffer so that valgrind can help
    // catch errors as well.
    EXPECT_THROW(
        {
            std::unique_ptr<char[]> decodeBuffer(new char[length]);

            ::memcpy(decodeBuffer.get(), m_buffer + headerDecoder.encodedLength(), length);
            OtfMessageDecoder::decode(decodeBuffer.get(), length, actingVersion, blockLength, messageTokens, *this);
        },
        std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(
    LengthUpToHdrAndCar,
    Rc3OtfFullIrLengthTest,
    ::testing::Range(0, static_cast<int>(encodedCarAndHdrLength - MessageHeader::encodedLength()), 1));
