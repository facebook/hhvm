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

#include <string>
#include <stdexcept>
#include <cstring>

#include <gtest/gtest.h>

#include "code_generation_test/car.h"
#include "code_generation_test/messageHeader.h"


#define CGT(name) code_generation_test_##name

static const std::size_t BUFFER_LEN = 2048;

static const std::uint32_t SERIAL_NUMBER = 1234;
static const std::uint16_t MODEL_YEAR = 2013;
static const CGT(booleanType) AVAILABLE = CGT(booleanType_T);
static const CGT(model) CODE = CGT(model_A);
static const bool CRUISE_CONTROL = true;
static const bool SPORTS_PACK = true;
static const bool SUNROOF = false;
static const CGT(boostType) BOOST_TYPE = CGT(boostType_NITROUS);
static const std::uint8_t BOOSTER_HORSEPOWER = 200;

static char VEHICLE_CODE[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
static char MANUFACTURER_CODE[] = { '1', '2', '3' };
static const char FUEL_FIGURES_1_USAGE_DESCRIPTION[] = "Urban Cycle";
static const char FUEL_FIGURES_2_USAGE_DESCRIPTION[] = "Combined Cycle";
static const char FUEL_FIGURES_3_USAGE_DESCRIPTION[] = "Highway Cycle";
static const char MANUFACTURER[] = "Honda";
static const char MODEL[] = "Civic VTi";
static const char ACTIVATION_CODE[] = "deadbeef";

static const std::size_t VEHICLE_CODE_LENGTH = sizeof(VEHICLE_CODE);
static const std::size_t MANUFACTURER_CODE_LENGTH = sizeof(MANUFACTURER_CODE);
static const std::size_t FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION);
static const std::size_t FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION);
static const std::size_t FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH = strlen(FUEL_FIGURES_3_USAGE_DESCRIPTION);
static const std::size_t MANUFACTURER_LENGTH = strlen(MANUFACTURER);
static const std::size_t MODEL_LENGTH = strlen(MODEL);
static const std::size_t ACTIVATION_CODE_LENGTH = strlen(ACTIVATION_CODE);
static const std::uint8_t PERFORMANCE_FIGURES_COUNT = 2;
static const std::uint8_t FUEL_FIGURES_COUNT = 3;
static const std::uint8_t ACCELERATION_COUNT = 3;

static const std::uint64_t expectedHeaderSize = 8;
static const std::uint64_t expectedCarEncodedLength = 191;

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
    static std::uint64_t encodeHdr(CGT(messageHeader) &hdr)
    {
        CGT(messageHeader_set_blockLength)(&hdr, CGT(car_sbe_block_length)());
        CGT(messageHeader_set_templateId)(&hdr, CGT(car_sbe_template_id)());
        CGT(messageHeader_set_schemaId)(&hdr, CGT(car_sbe_schema_id)());
        CGT(messageHeader_set_version)(&hdr, CGT(car_sbe_schema_version)());

        return CGT(messageHeader_encoded_length)();
    }

    static std::uint64_t encodeCar(CGT(car) &car)
    {
        CGT(car_set_serialNumber)(&car, SERIAL_NUMBER);
        CGT(car_set_modelYear)(&car, MODEL_YEAR);
        CGT(car_set_available)(&car, AVAILABLE);
        CGT(car_set_code)(&car, CODE);
        CGT(car_put_vehicleCode)(&car, VEHICLE_CODE);

        for (std::uint64_t i = 0; i < CGT(car_someNumbers_length)(); i++)
        {
            CGT(car_set_someNumbers_unsafe)(&car, i, static_cast<std::int32_t>(i));
        }

        CGT(optionalExtras) extras;
        if (!CGT(car_extras)(&car, &extras))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(optionalExtras_clear)(&extras);
        CGT(optionalExtras_set_cruiseControl)(&extras, CRUISE_CONTROL);
        CGT(optionalExtras_set_sportsPack)(&extras, SPORTS_PACK);
        CGT(optionalExtras_set_sunRoof)(&extras, SUNROOF);

        CGT(engine) engine;
        if (!CGT(car_engine)(&car, &engine))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(engine_set_capacity)(&engine, engineCapacity);
        CGT(engine_set_numCylinders)(&engine, engineNumCylinders);
        CGT(engine_put_manufacturerCode)(&engine, MANUFACTURER_CODE);

        CGT(boosterT) booster;
        if (!CGT(engine_booster)(&engine, &booster))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(boosterT_set_boostType)(&booster, BOOST_TYPE);
        CGT(boosterT_set_horsePower)(&booster, BOOSTER_HORSEPOWER);

        CGT(car_fuelFigures) fuelFigures;
        if (!CGT(car_fuelFigures_set_count)(&car, &fuelFigures, FUEL_FIGURES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_fuelFigures_next)(&fuelFigures);
        CGT(car_fuelFigures_set_speed)(&fuelFigures, fuel1Speed);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, fuel1Mpg);
        CGT(car_fuelFigures_put_usageDescription)(
            &fuelFigures,
            FUEL_FIGURES_1_USAGE_DESCRIPTION,
            static_cast<int>(strlen(FUEL_FIGURES_1_USAGE_DESCRIPTION)));

        CGT(car_fuelFigures_next)(&fuelFigures);
        CGT(car_fuelFigures_set_speed)(&fuelFigures, fuel2Speed);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, fuel2Mpg);
        CGT(car_fuelFigures_put_usageDescription)(
            &fuelFigures,
            FUEL_FIGURES_2_USAGE_DESCRIPTION,
            static_cast<int>(strlen(FUEL_FIGURES_2_USAGE_DESCRIPTION)));

        CGT(car_fuelFigures_next)(&fuelFigures);
        CGT(car_fuelFigures_set_speed)(&fuelFigures, fuel3Speed);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, fuel3Mpg);
        CGT(car_fuelFigures_put_usageDescription)(
            &fuelFigures,
            FUEL_FIGURES_3_USAGE_DESCRIPTION,
            static_cast<int>(strlen(FUEL_FIGURES_3_USAGE_DESCRIPTION)));

        CGT(car_performanceFigures) perfFigs;
        if (!CGT(car_performanceFigures_set_count)(
            &car,
            &perfFigs,
            PERFORMANCE_FIGURES_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_next)(&perfFigs);
        CGT(car_performanceFigures_set_octaneRating)(&perfFigs, perf1Octane);

        CGT(car_performanceFigures_acceleration) acc;
        if (!CGT(car_performanceFigures_acceleration_set_count)(&perfFigs, &acc, ACCELERATION_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf1aMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf1aSeconds);
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf1bMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf1bSeconds);
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf1cMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf1cSeconds);

        CGT(car_performanceFigures_next)(&perfFigs);
        CGT(car_performanceFigures_set_octaneRating)(&perfFigs, perf2Octane);

        if (!CGT(car_performanceFigures_acceleration_set_count)(&perfFigs, &acc, ACCELERATION_COUNT))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf2aMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf2aSeconds);
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf2bMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf2bSeconds);
        CGT(car_performanceFigures_acceleration_next)(&acc);
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, perf2cMph);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, perf2cSeconds);

        CGT(car_put_manufacturer)(&car, MANUFACTURER, static_cast<int>(strlen(MANUFACTURER)));
        CGT(car_put_model)(&car, MODEL, static_cast<int>(strlen(MODEL)));
        CGT(car_put_activationCode)(&car, ACTIVATION_CODE, static_cast<int>(strlen(ACTIVATION_CODE)));

        return CGT(car_encoded_length)(&car);
    }

    static std::string walkCar(CGT(car)& car)
    {
        std::stringstream output;

        output <<
            CGT(car_serialNumber)(&car) << ';' <<
            CGT(car_modelYear) << ';' <<
            CGT(car_available) << ';' <<
            CGT(car_code) << ';';

        for (std::uint64_t i = 0; i < CGT(car_someNumbers_length()); i++)
        {
            output << (int)CGT(car_someNumbers_buffer(&car))[i] << ';';
        }

        output << std::string(CGT(car_vehicleCode_buffer(&car)), CGT(car_vehicleCode_length())) << ';';

        CGT(optionalExtras) extras = {};
        if (!CGT(car_extras)(&car, &extras))
        {
            output <<
                CGT(optionalExtras_sunRoof)(&extras) << ';' <<
                CGT(optionalExtras_sportsPack)(&extras) << ';' <<
                CGT(optionalExtras_cruiseControl)(&extras) << ';';
        }

        char code_buf[4];
        CGT(engine) engine = {};
        if (CGT(car_engine)(&car, &engine))
        {
            output <<
                CGT(engine_capacity(&engine)) << ';' <<
                (int)CGT(engine_numCylinders(&engine)) << ';' <<
                CGT(engine_maxRpm()) << ';' <<
                CGT(engine_get_manufacturerCode(&engine, code_buf, 3)) << ';' <<
                std::string(CGT(engine_fuel()), CGT(engine_fuel_length())) << ';';
        }

        CGT(car_fuelFigures) fuelFigures = {};
        if (CGT(car_get_fuelFigures)(&car, &fuelFigures))
        {
            while (CGT(car_fuelFigures_has_next)(&fuelFigures))
            {
                CGT(car_fuelFigures_next)(&fuelFigures);
                output <<
                    CGT(car_fuelFigures_speed(&fuelFigures)) << ';' <<
                    CGT(car_fuelFigures_mpg(&fuelFigures)) << ';' <<
                    std::string(
                        CGT(car_fuelFigures_usageDescription(&fuelFigures)),
                        CGT(car_fuelFigures_usageDescription_length(&fuelFigures))) << ';';
            }
        }

        CGT(car_performanceFigures) perfFigures = {};
        if (CGT(car_get_performanceFigures(&car, &perfFigures)))
        {
            output << CGT(car_performanceFigures_count)(&perfFigures) << ';';

            while (CGT(car_performanceFigures_has_next)(&perfFigures))
            {
                CGT(car_performanceFigures_next(&perfFigures));
                output << CGT(car_performanceFigures_octaneRating(&perfFigures)) << ';';

                CGT(car_performanceFigures_acceleration) acceleration = {};
                if (CGT(car_performanceFigures_get_acceleration(&perfFigures, &acceleration)))
                {
                    while (CGT(car_performanceFigures_acceleration_has_next)(&acceleration))
                    {
                        CGT(car_performanceFigures_acceleration_next(&acceleration));
                        output <<
                            CGT(car_performanceFigures_acceleration_mph(&acceleration)) << ';' <<
                            CGT(car_performanceFigures_acceleration_seconds(&acceleration)) << ';';
                    }
                }
            }
        }

        CGT(car_string_view) manufacturer = CGT(car_get_manufacturer_as_string_view(&car));
        if (nullptr != manufacturer.data)
        {
            output << std::string(manufacturer.data, manufacturer.length) << ';';
        }
        CGT(car_string_view) model = CGT(car_get_model_as_string_view(&car));
        if (nullptr != model.data)
        {
            output << std::string(model.data, model.length) << ';';
        }

        return output.str();
    }

    static std::string partialWalkCar(CGT(car)& car)
    {
        std::stringstream output;

        output <<
        CGT(car_serialNumber)(&car) << ';' <<
        CGT(car_modelYear) << ';' <<
        CGT(car_available) << ';' <<
        CGT(car_code) << ';';

        for (std::uint64_t i = 0; i < CGT(car_someNumbers_length()); i++)
        {
            output << (int)CGT(car_someNumbers_buffer(&car))[i] << ';';
        }

        output << std::string(CGT(car_vehicleCode_buffer(&car)), CGT(car_vehicleCode_length())) << ';';

        CGT(optionalExtras) extras = {};
        if (!CGT(car_extras)(&car, &extras))
        {
            output <<
            CGT(optionalExtras_sunRoof)(&extras) << ';' <<
            CGT(optionalExtras_sportsPack)(&extras) << ';' <<
            CGT(optionalExtras_cruiseControl)(&extras) << ';';
        }

        char code_buf[4];
        CGT(engine) engine = {};
        if (CGT(car_engine)(&car, &engine))
        {
            output <<
            CGT(engine_capacity(&engine)) << ';' <<
            (int)CGT(engine_numCylinders(&engine)) << ';' <<
            CGT(engine_maxRpm()) << ';' <<
            CGT(engine_get_manufacturerCode(&engine, code_buf, 3)) << ';' <<
            std::string(CGT(engine_fuel()), CGT(engine_fuel_length())) << ';';
        }

        CGT(car_fuelFigures) fuelFigures = {};
        if (CGT(car_get_fuelFigures)(&car, &fuelFigures))
        {
            while (CGT(car_fuelFigures_has_next)(&fuelFigures))
            {
                CGT(car_fuelFigures_next)(&fuelFigures);
                output <<
                CGT(car_fuelFigures_speed(&fuelFigures)) << ';' <<
                CGT(car_fuelFigures_mpg(&fuelFigures)) << ';' <<
                std::string(
                    CGT(car_fuelFigures_usageDescription(&fuelFigures)),
                    CGT(car_fuelFigures_usageDescription_length(&fuelFigures))) << ';';
            }
        }

        CGT(car_performanceFigures) perfFigures = {};
        if (CGT(car_get_performanceFigures(&car, &perfFigures)))
        {
            output << CGT(car_performanceFigures_count)(&perfFigures) << ';';

            if (CGT(car_performanceFigures_has_next)(&perfFigures))
            {
                CGT(car_performanceFigures_next(&perfFigures));
                output << CGT(car_performanceFigures_octaneRating(&perfFigures)) << ';';

                CGT(car_performanceFigures_acceleration) acceleration = {};
                if (CGT(car_performanceFigures_get_acceleration(&perfFigures, &acceleration)))
                {
                    if (CGT(car_performanceFigures_acceleration_has_next)(&acceleration))
                    {
                        CGT(car_performanceFigures_acceleration_next(&acceleration));
                        output <<
                        CGT(car_performanceFigures_acceleration_mph(&acceleration)) << ';' <<
                        CGT(car_performanceFigures_acceleration_seconds(&acceleration)) << ';';
                    }
                }
            }
        }

        return output.str();
    }


    std::uint64_t encodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!CGT(messageHeader_wrap)(&m_hdr, buffer, offset, 0, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        return encodeHdr(m_hdr);
    }

    std::uint64_t encodeCar(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!CGT(car_wrap_for_encode)(&m_car, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        return encodeCar(m_car);
    }

    CGT(messageHeader) m_hdr = {};
    CGT(messageHeader) m_hdrDecoder ={};
    CGT(car) m_car = {};
    CGT(car) m_carDecoder = {};
};

TEST_F(CodeGenTest, shouldReturnCorrectValuesForMessageHeaderStaticFields)
{
    EXPECT_EQ(CGT(messageHeader_encoded_length)(), 8u);
    // only checking the block length field
    EXPECT_EQ(CGT(messageHeader_blockLength_null_value)(), 65535);
    EXPECT_EQ(CGT(messageHeader_blockLength_min_value)(), 0);
    EXPECT_EQ(CGT(messageHeader_blockLength_max_value)(), 65534);
}

TEST_F(CodeGenTest, shouldReturnCorrectValuesForCarStaticFields)
{
    EXPECT_EQ(CGT(car_sbe_block_length)(), 47u);
    EXPECT_EQ(CGT(car_sbe_template_id)(), 1u);
    EXPECT_EQ(CGT(car_sbe_schema_id)(), 6u);
    EXPECT_EQ(CGT(car_sbe_schema_version)(), 0u);
    EXPECT_EQ(std::string(CGT(car_sbe_semantic_type)()), std::string(""));
    EXPECT_EQ(CGT(car_sbe_semantic_version)(), "5.2");
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeMessageHeaderCorrectly)
{
    char buffer[BUFFER_LEN] = {};
    const char *bp = buffer;

    std::uint64_t sz = encodeHdr(buffer, 0, sizeof(buffer));

    EXPECT_EQ(*((uint16_t *)bp), CGT(car_sbe_block_length)());
    EXPECT_EQ(*((uint16_t *)(bp + 2)), CGT(car_sbe_template_id)());
    EXPECT_EQ(*((uint16_t *)(bp + 4)), CGT(car_sbe_schema_id)());
    EXPECT_EQ(*((uint16_t *)(bp + 6)), CGT(car_sbe_schema_version)());
    EXPECT_EQ(sz, 8u);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeAndDecodeMessageHeaderCorrectly)
{
    char buffer[BUFFER_LEN] = {};

    encodeHdr(buffer, 0, sizeof(buffer));

    if (!CGT(messageHeader_wrap)(&m_hdrDecoder, buffer, 0, 0, sizeof(buffer)))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(CGT(messageHeader_blockLength)(&m_hdrDecoder), CGT(car_sbe_block_length)());
    EXPECT_EQ(CGT(messageHeader_templateId)(&m_hdrDecoder), CGT(car_sbe_template_id)());
    EXPECT_EQ(CGT(messageHeader_schemaId)(&m_hdrDecoder), CGT(car_sbe_schema_id)());
    EXPECT_EQ(CGT(messageHeader_version)(&m_hdrDecoder), CGT(car_sbe_schema_version)());
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
    EXPECT_EQ(CGT(car_serialNumber_id)(), fieldIdSerialNumber);
    EXPECT_EQ(CGT(car_modelYear_id)(), fieldIdModelYear);
    EXPECT_EQ(CGT(car_available_id)(), fieldIdAvailable);
    EXPECT_EQ(CGT(car_code_id)(), fieldIdCode);
    EXPECT_EQ(CGT(car_someNumbers_id)(), fieldIdSomeNumbers);
    EXPECT_EQ(CGT(car_vehicleCode_id)(), fieldIdVehicleCode);
    EXPECT_EQ(CGT(car_extras_id)(), fieldIdExtras);
    EXPECT_EQ(CGT(car_discountedModel_id)(), fieldIdDiscountedModel);
    EXPECT_EQ(CGT(car_engine_id)(), fieldIdEngine);
    EXPECT_EQ(CGT(car_fuelFigures_id)(), fieldIdFuelFigures);
    EXPECT_EQ(CGT(car_fuelFigures_speed_id)(), fieldIdFuelSpeed);
    EXPECT_EQ(CGT(car_fuelFigures_mpg_id)(), fieldIdFuelMpg);
    EXPECT_EQ(CGT(car_fuelFigures_usageDescription_id)(), fieldIdFuelUsageDescription);
    EXPECT_EQ(CGT(car_fuelFigures_usageDescription_character_encoding)(), std::string("UTF-8"));
    EXPECT_EQ(CGT(car_performanceFigures_id)(), fieldIdPerformanceFigures);
    EXPECT_EQ(CGT(car_performanceFigures_octaneRating_id)(), fieldIdPerfOctaneRating);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_id)(), fieldIdPerfAcceleration);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph_id)(), fieldIdPerfAccMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds_id)(), fieldIdPerfAccSeconds);
    EXPECT_EQ(CGT(car_manufacturer_id)(), fieldIdManufacturer);
    EXPECT_EQ(CGT(car_model_id)(), fieldIdModel);
    EXPECT_EQ(CGT(car_activationCode_id)(), fieldIdActivationCode);
    EXPECT_EQ(std::string(CGT(car_manufacturer_character_encoding())), std::string("UTF-8"));
    EXPECT_EQ(std::string(CGT(car_model_character_encoding())), std::string("UTF-8"));
    EXPECT_EQ(std::string(CGT(car_activationCode_character_encoding())), std::string("UTF-8"));
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
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(
        std::string(bp + offset, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_1_USAGE_DESCRIPTION);
    offset += FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH;

    EXPECT_EQ(*(std::uint16_t *)(bp + offset), fuel2Speed);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), fuel2Mpg);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(
        std::string(bp + offset, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_2_USAGE_DESCRIPTION);
    offset += FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH;

    EXPECT_EQ(*(std::uint16_t *)(bp + offset), fuel3Speed);
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(*(float *)(bp + offset), fuel3Mpg);
    offset += sizeof(float);
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(
        std::string(bp + offset, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH), FUEL_FIGURES_3_USAGE_DESCRIPTION);
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
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(MANUFACTURER_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, MANUFACTURER_LENGTH), MANUFACTURER);
    offset += MANUFACTURER_LENGTH;
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(MODEL_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, MODEL_LENGTH), MODEL);
    offset += MODEL_LENGTH;
    EXPECT_EQ(*(std::uint16_t *)(bp + offset), static_cast<std::uint16_t>(ACTIVATION_CODE_LENGTH));
    offset += sizeof(std::uint16_t);
    EXPECT_EQ(std::string(bp + offset, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);
    offset += ACTIVATION_CODE_LENGTH;

    EXPECT_EQ(sz, offset);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeHeaderPlusCarCorrectly)
{
    char buffer[BUFFER_LEN] = {};
    const char *bp = buffer;

    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t carEncodedLength = encodeCar(
        buffer, CGT(messageHeader_encoded_length)(), sizeof(buffer) - CGT(messageHeader_encoded_length)());

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    EXPECT_EQ(*((std::uint16_t *)bp), CGT(car_sbe_block_length)());
    const size_t activationCodePosition = hdrSz + carEncodedLength - ACTIVATION_CODE_LENGTH;
    const size_t activationCodeLengthPosition = activationCodePosition - sizeof(std::uint16_t);
    EXPECT_EQ(*(std::uint16_t *)(bp + activationCodeLengthPosition), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(bp + activationCodePosition, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);
}

TEST_F(CodeGenTest, shouldBeAbleToEncodeAndDecodeHeaderPlusCarCorrectly)
{
    char buffer[BUFFER_LEN] = {};

    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t carEncodedLength = encodeCar(
        buffer, CGT(messageHeader_encoded_length)(), sizeof(buffer) - CGT(messageHeader_encoded_length)());

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    if (!CGT(messageHeader_wrap)(&m_hdrDecoder, buffer, 0, 0, hdrSz))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(CGT(messageHeader_blockLength)(&m_hdrDecoder), CGT(car_sbe_block_length)());
    EXPECT_EQ(CGT(messageHeader_templateId)(&m_hdrDecoder), CGT(car_sbe_template_id)());
    EXPECT_EQ(CGT(messageHeader_schemaId)(&m_hdrDecoder), CGT(car_sbe_schema_id)());
    EXPECT_EQ(CGT(messageHeader_version)(&m_hdrDecoder), CGT(car_sbe_schema_version)());
    EXPECT_EQ(CGT(messageHeader_encoded_length)(), expectedHeaderSize);

    if (!CGT(car_wrap_for_decode)(
        &m_carDecoder,
        buffer,
        CGT(messageHeader_encoded_length)(),
        CGT(car_sbe_block_length)(),
        CGT(car_sbe_schema_version)(),
        hdrSz + carEncodedLength))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(CGT(car_serialNumber)(&m_carDecoder), SERIAL_NUMBER);
    EXPECT_EQ(CGT(car_modelYear)(&m_carDecoder), MODEL_YEAR);
    {
        CGT(booleanType) out = CGT(booleanType_NULL_VALUE);
        ASSERT_TRUE(CGT(car_available)(&m_carDecoder, &out));
        EXPECT_EQ(out, AVAILABLE);
    }
    {
        CGT(model) out = CGT(model_NULL_VALUE);
        ASSERT_TRUE(CGT(car_code)(&m_carDecoder, &out));
        EXPECT_EQ(out, CODE);
    }
    EXPECT_EQ(CGT(car_someNumbers_length)(), 5u);
    for (std::uint64_t i = 0; i < 5; i++)
    {
        EXPECT_EQ(CGT(car_someNumbers_unsafe)(&m_carDecoder, i), static_cast<std::int32_t>(i));
    }

    EXPECT_EQ(CGT(car_vehicleCode_length)(), VEHICLE_CODE_LENGTH);
    EXPECT_EQ(
        std::string(CGT(car_vehicleCode_buffer)(&m_carDecoder), VEHICLE_CODE_LENGTH),
        std::string(VEHICLE_CODE, VEHICLE_CODE_LENGTH));

    CGT(optionalExtras) extras;
    if (!CGT(car_extras)(&m_carDecoder, &extras))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_TRUE(CGT(optionalExtras_cruiseControl)(&extras));
    EXPECT_TRUE(CGT(optionalExtras_sportsPack)(&extras));
    EXPECT_FALSE(CGT(optionalExtras_sunRoof)(&extras));
    EXPECT_EQ(CGT(car_discountedModel)(&m_carDecoder), CGT(model_C));

    CGT(engine) engine;
    if (!CGT(car_engine)(&m_carDecoder, &engine))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(CGT(engine_capacity)(&engine), engineCapacity);
    EXPECT_EQ(CGT(engine_numCylinders)(&engine), engineNumCylinders);
    EXPECT_EQ(CGT(engine_maxRpm)(), 9000);
    EXPECT_EQ(CGT(engine_manufacturerCode_length)(), MANUFACTURER_CODE_LENGTH);
    EXPECT_EQ(
        std::string(CGT(engine_manufacturerCode_buffer)(&engine), MANUFACTURER_CODE_LENGTH),
        std::string(MANUFACTURER_CODE, MANUFACTURER_CODE_LENGTH));
    EXPECT_EQ(CGT(engine_fuel_length)(), 6u);
    EXPECT_EQ(std::string(CGT(engine_fuel)(), 6), std::string("Petrol"));

    CGT(car_fuelFigures) fuelFigures;
    CGT(car_get_fuelFigures)(&m_carDecoder, &fuelFigures);
    EXPECT_EQ(CGT(car_fuelFigures_count)(&fuelFigures), FUEL_FIGURES_COUNT);

    ASSERT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
    CGT(car_fuelFigures_next)(&fuelFigures);
    EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), fuel1Speed);
    EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), fuel1Mpg);
    EXPECT_EQ(CGT(car_fuelFigures_usageDescription_length)(&fuelFigures), FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(
        std::string(CGT(car_fuelFigures_usageDescription)(&fuelFigures), FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_1_USAGE_DESCRIPTION);

    ASSERT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
    CGT(car_fuelFigures_next)(&fuelFigures);
    EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), fuel2Speed);
    EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), fuel2Mpg);
    EXPECT_EQ(CGT(car_fuelFigures_usageDescription_length)(&fuelFigures), FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(
        std::string(CGT(car_fuelFigures_usageDescription)(&fuelFigures), FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_2_USAGE_DESCRIPTION);

    ASSERT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
    CGT(car_fuelFigures_next)(&fuelFigures);
    EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), fuel3Speed);
    EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), fuel3Mpg);
    EXPECT_EQ(CGT(car_fuelFigures_usageDescription_length)(&fuelFigures), FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    EXPECT_EQ(
        std::string(CGT(car_fuelFigures_usageDescription)(&fuelFigures), FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH),
        FUEL_FIGURES_3_USAGE_DESCRIPTION);

    CGT(car_performanceFigures) performanceFigures;
    CGT(car_get_performanceFigures)(&m_carDecoder, &performanceFigures);
    EXPECT_EQ(CGT(car_performanceFigures_count)(&performanceFigures), PERFORMANCE_FIGURES_COUNT);

    ASSERT_TRUE(CGT(car_performanceFigures_has_next)(&performanceFigures));
    CGT(car_performanceFigures_next)(&performanceFigures);
    EXPECT_EQ(CGT(car_performanceFigures_octaneRating)(&performanceFigures), perf1Octane);

    CGT(car_performanceFigures_acceleration) acc;
    CGT(car_performanceFigures_get_acceleration)(&performanceFigures, &acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_count)(&acc), ACCELERATION_COUNT);
    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf1aMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf1aSeconds);

    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf1bMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf1bSeconds);

    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf1cMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf1cSeconds);

    ASSERT_TRUE(CGT(car_performanceFigures_has_next)(&performanceFigures));
    CGT(car_performanceFigures_next)(&performanceFigures);
    EXPECT_EQ(CGT(car_performanceFigures_octaneRating)(&performanceFigures), perf2Octane);

    CGT(car_performanceFigures_get_acceleration)(&performanceFigures, &acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_count)(&acc), ACCELERATION_COUNT);
    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf2aMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf2aSeconds);

    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf2bMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf2bSeconds);

    ASSERT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
    CGT(car_performanceFigures_acceleration_next)(&acc);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), perf2cMph);
    EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), perf2cSeconds);

    EXPECT_EQ(CGT(car_manufacturer_length)(&m_carDecoder), MANUFACTURER_LENGTH);
    EXPECT_EQ(std::string(CGT(car_manufacturer)(&m_carDecoder), MANUFACTURER_LENGTH), MANUFACTURER);

    EXPECT_EQ(CGT(car_model_length)(&m_carDecoder), MODEL_LENGTH);
    EXPECT_EQ(std::string(CGT(car_model)(&m_carDecoder), MODEL_LENGTH), MODEL);

    EXPECT_EQ(CGT(car_activationCode_length)(&m_carDecoder), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(CGT(car_activationCode)(&m_carDecoder), ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);

    EXPECT_EQ(CGT(car_encoded_length)(&m_carDecoder), expectedCarEncodedLength);
}

struct CallbacksForEach
{
    int countOfFuelFigures = 0;
    int countOfPerformanceFigures = 0;
    int countOfAccelerations = 0;
};

TEST_F(CodeGenTest, shouldBeAbleUseOnStackCodecsAndGroupForEach)
{
    char buffer[BUFFER_LEN] = {};
    CGT(messageHeader) hdr;
    if (!CGT(messageHeader_reset)(&hdr, buffer, 0, sizeof(buffer), 0))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    CGT(car) car;
    if (!CGT(car_reset)(
        &car,
        buffer + CGT(messageHeader_encoded_length)(),
        0,
        sizeof(buffer) - CGT(messageHeader_encoded_length)(),
        CGT(car_sbe_block_length)(),
        CGT(car_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    std::uint64_t hdrSz = encodeHdr(hdr);
    std::uint64_t carEncodedLength = encodeCar(car);

    EXPECT_EQ(hdrSz, expectedHeaderSize);
    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    CGT(messageHeader) hdrDecoder;
    if (!CGT(messageHeader_reset)(&hdrDecoder, buffer, 0, hdrSz, 0))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(CGT(messageHeader_blockLength)(&hdrDecoder), CGT(car_sbe_block_length)());
    EXPECT_EQ(CGT(messageHeader_templateId)(&hdrDecoder), CGT(car_sbe_template_id)());
    EXPECT_EQ(CGT(messageHeader_schemaId)(&hdrDecoder), CGT(car_sbe_schema_id)());
    EXPECT_EQ(CGT(messageHeader_version)(&hdrDecoder), CGT(car_sbe_schema_version)());
    EXPECT_EQ(CGT(messageHeader_encoded_length)(), expectedHeaderSize);

    CGT(car) carDecoder;
    if (!CGT(car_reset)(
        &carDecoder,
        buffer + CGT(messageHeader_encoded_length)(),
        0,
        carEncodedLength,
        CGT(car_sbe_block_length)(),
        CGT(car_sbe_schema_version)()))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    CallbacksForEach cbs = {};

    CGT(car_fuelFigures) fuelFigures;
    if (!CGT(car_get_fuelFigures)(&carDecoder, &fuelFigures))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(CGT(car_fuelFigures_count)(&fuelFigures), FUEL_FIGURES_COUNT);

    ASSERT_TRUE(CGT(car_fuelFigures_for_each)(
        &fuelFigures,
        [](CGT(car_fuelFigures) *const figures, void *cbs)
        {
            reinterpret_cast<CallbacksForEach*>(cbs)->countOfFuelFigures++;

            char tmp[256] = {};
            CGT(car_fuelFigures_get_usageDescription)(figures, tmp, sizeof(tmp));
        },
        &cbs));

    CGT(car_performanceFigures) performanceFigures;
    if (!CGT(car_get_performanceFigures)(&carDecoder, &performanceFigures))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    EXPECT_EQ(CGT(car_performanceFigures_count)(&performanceFigures), PERFORMANCE_FIGURES_COUNT);

    ASSERT_TRUE(CGT(car_performanceFigures_for_each)(
        &performanceFigures,
        [](CGT(car_performanceFigures) *const figures, void *cbs)
        {
            CGT(car_performanceFigures_acceleration) acceleration;
            if (!CGT(car_performanceFigures_get_acceleration(figures, &acceleration)))
            {
                throw std::runtime_error(sbe_strerror(errno));
            }
            reinterpret_cast<CallbacksForEach*>(cbs)->countOfPerformanceFigures++;
            ASSERT_TRUE(CGT(car_performanceFigures_acceleration_for_each)(
            &acceleration,
            [](CGT(car_performanceFigures_acceleration) *const, void *cbs)
            {
                reinterpret_cast<CallbacksForEach*>(cbs)->countOfAccelerations++;
            },
                cbs
            ));
        },
        &cbs));

    EXPECT_EQ(cbs.countOfFuelFigures, FUEL_FIGURES_COUNT);
    EXPECT_EQ(cbs.countOfPerformanceFigures, PERFORMANCE_FIGURES_COUNT);
    EXPECT_EQ(cbs.countOfAccelerations, ACCELERATION_COUNT * PERFORMANCE_FIGURES_COUNT);

    char tmp[256] = {};

    EXPECT_EQ(CGT(car_get_manufacturer)(&carDecoder, tmp, sizeof(tmp)), MANUFACTURER_LENGTH);
    EXPECT_EQ(std::string(tmp, MANUFACTURER_LENGTH), MANUFACTURER);

    EXPECT_EQ(CGT(car_get_model)(&carDecoder, tmp, sizeof(tmp)), MODEL_LENGTH);
    EXPECT_EQ(std::string(tmp, MODEL_LENGTH), MODEL);

    EXPECT_EQ(CGT(car_get_manufacturer)(&carDecoder, tmp, sizeof(tmp)), ACTIVATION_CODE_LENGTH);
    EXPECT_EQ(std::string(tmp, ACTIVATION_CODE_LENGTH), ACTIVATION_CODE);

    EXPECT_EQ(CGT(car_encoded_length)(&carDecoder), expectedCarEncodedLength);
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

TEST_F(CodeGenTest, shouldBeAbleToUseStdStringMethodsForEncode)
{
    std::string vehicleCode(VEHICLE_CODE, CGT(car_vehicleCode_length)());
    std::string usageDesc1(FUEL_FIGURES_1_USAGE_DESCRIPTION, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc2(FUEL_FIGURES_2_USAGE_DESCRIPTION, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH);
    std::string usageDesc3(FUEL_FIGURES_3_USAGE_DESCRIPTION, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH);
    std::string manufacturer(MANUFACTURER, MANUFACTURER_LENGTH);
    std::string model(MODEL, MODEL_LENGTH);
    std::string activationCode(ACTIVATION_CODE, ACTIVATION_CODE_LENGTH);

    char buffer[BUFFER_LEN] = {};
    auto baseOffset = static_cast<std::uint64_t>(CGT(messageHeader_encoded_length)());
    CGT(car) car;
    if (!CGT(car_wrap_for_encode)(&car, buffer, baseOffset, sizeof(buffer)))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    CGT(car_put_vehicleCode)(&car, vehicleCode.c_str());
    CGT(car_fuelFigures) fuelFig;
    if (!CGT(car_fuelFigures_set_count)(&car, &fuelFig, FUEL_FIGURES_COUNT))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    CGT(car_fuelFigures_next)(&fuelFig);
    const char *desc1 = usageDesc1.c_str();
    CGT(car_fuelFigures_put_usageDescription)(&fuelFig, desc1, static_cast<std::uint16_t>(strlen(desc1)));
    CGT(car_fuelFigures_next)(&fuelFig);
    const char *desc2 = usageDesc2.c_str();
    CGT(car_fuelFigures_put_usageDescription)(&fuelFig, desc2, static_cast<std::uint16_t>(strlen(desc2)));
    CGT(car_fuelFigures_next)(&fuelFig);
    const char *desc3 = usageDesc3.c_str();
    CGT(car_fuelFigures_put_usageDescription)(&fuelFig, desc3, static_cast<std::uint16_t>(strlen(desc3)));

    CGT(car_performanceFigures) perfFigs;
    if (!CGT(car_performanceFigures_set_count)(&car, &perfFigs, PERFORMANCE_FIGURES_COUNT))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    CGT(car_performanceFigures_next)(&perfFigs);
    CGT(car_performanceFigures_acceleration) acc;
    CGT(car_performanceFigures_acceleration_set_count)(&perfFigs, &acc, ACCELERATION_COUNT);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);

    CGT(car_performanceFigures_next)(&perfFigs);
    CGT(car_performanceFigures_acceleration_set_count)(&perfFigs, &acc, ACCELERATION_COUNT);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);

    const char *manu = manufacturer.c_str();
    CGT(car_put_manufacturer)(&car, manu, static_cast<std::uint16_t>(strlen(manu)));
    const char *model_c = model.c_str();
    CGT(car_put_model)(&car, model_c, static_cast<std::uint16_t>(strlen(model_c)));
    const char *acti = activationCode.c_str();
    CGT(car_put_activationCode)(&car, acti, static_cast<std::uint16_t>(strlen(acti)));

    EXPECT_EQ(CGT(car_encoded_length)(&car), expectedCarEncodedLength);

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
}

void testUsageDescription(CGT(car_fuelFigures) *const fuelFigures, const std::string &expected)
{
    CGT(car_fuelFigures_next)(fuelFigures);
    const std::uint16_t length = CGT(car_fuelFigures_usageDescription_length)(fuelFigures);
    const char *const ptr = CGT(car_fuelFigures_usageDescription)(fuelFigures);
    if (!ptr)
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    EXPECT_EQ(std::string(ptr, length), expected);
}

TEST_F(CodeGenTest, shouldBeAbleToUseStdStringMethodsForDecode)
{
    char buffer[2048] = {};
    CGT(car) carEncoder;
    CGT(car_reset)(&carEncoder, buffer, 0, sizeof(buffer), CGT(car_sbe_block_length)(), CGT(car_sbe_schema_version)());

    std::uint64_t carEncodedLength = encodeCar(carEncoder);

    EXPECT_EQ(carEncodedLength, expectedCarEncodedLength);

    CGT(car) carDecoder;
    CGT(car_reset)(
        &carDecoder, buffer, 0, carEncodedLength, CGT(car_sbe_block_length)(), CGT(car_sbe_schema_version)());

    std::string vehicleCode(VEHICLE_CODE, CGT(car_vehicleCode_length)());

    EXPECT_EQ(std::string(CGT(car_vehicleCode_buffer)(&carDecoder), CGT(car_vehicleCode_length)()), vehicleCode);

    CGT(car_fuelFigures) fuelFigures;
    if (!CGT(car_get_fuelFigures)(&carDecoder, &fuelFigures))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    testUsageDescription(
        &fuelFigures, std::string(FUEL_FIGURES_1_USAGE_DESCRIPTION, FUEL_FIGURES_1_USAGE_DESCRIPTION_LENGTH));
    testUsageDescription(
        &fuelFigures, std::string(FUEL_FIGURES_2_USAGE_DESCRIPTION, FUEL_FIGURES_2_USAGE_DESCRIPTION_LENGTH));
    testUsageDescription(
        &fuelFigures, std::string(FUEL_FIGURES_3_USAGE_DESCRIPTION, FUEL_FIGURES_3_USAGE_DESCRIPTION_LENGTH));

    CGT(car_performanceFigures) perfFigures;
    if (!CGT(car_get_performanceFigures)(&carDecoder, &perfFigures))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    CGT(car_performanceFigures_next)(&perfFigures);
    CGT(car_performanceFigures_acceleration) acc;
    if (!CGT(car_performanceFigures_get_acceleration)(&perfFigures, &acc))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_next)(&perfFigures);

    CGT(car_performanceFigures_get_acceleration)(&perfFigures, &acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);
    CGT(car_performanceFigures_acceleration_next)(&acc);

    {
        const uint16_t length = CGT(car_manufacturer_length)(&carDecoder);
        const char *const ptr = CGT(car_manufacturer)(&carDecoder);
        if (!ptr)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        EXPECT_EQ(
            std::string(ptr, length),
            std::string(MANUFACTURER, MANUFACTURER_LENGTH));
    }
    {
        const uint16_t length = CGT(car_model_length)(&carDecoder);
        const char *const ptr = CGT(car_model)(&carDecoder);
        if (!ptr)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        EXPECT_EQ(
            std::string(ptr, length),
            std::string(MODEL, MODEL_LENGTH));
    }
    {
        const uint16_t length = CGT(car_activationCode_length)(&carDecoder);
        const char *const ptr = CGT(car_activationCode)(&carDecoder);
        if (!ptr)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        EXPECT_EQ(
            std::string(ptr, length),
            std::string(ACTIVATION_CODE, ACTIVATION_CODE_LENGTH));
    }

    EXPECT_EQ(CGT(car_encoded_length)(&carDecoder), expectedCarEncodedLength);
}

TEST_F(CodeGenTest, shouldAllowForMultipleIterations2)
{
    char buffer[2048] = {};

    std::uint64_t hdrSz = encodeHdr(buffer, 0, sizeof(buffer));
    std::uint64_t carEncodedLength = encodeCar(
        buffer, CGT(messageHeader_encoded_length)(), sizeof(buffer) - CGT(messageHeader_encoded_length)());

    if (!CGT(messageHeader_wrap)(&m_hdrDecoder, buffer, 0, 0, hdrSz))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    if (!CGT(car_wrap_for_decode)(
        &m_carDecoder,
        buffer,
        CGT(messageHeader_encoded_length)(),
        CGT(car_sbe_block_length)(),
        CGT(car_sbe_schema_version)(),
        hdrSz + carEncodedLength))
    {
        throw std::runtime_error(sbe_strerror(errno));
    }

    std::string passOne = walkCar(m_carDecoder);
    CGT(car_sbe_rewind(&m_carDecoder));
    std::string passTwo = walkCar(m_carDecoder);
    EXPECT_EQ(passOne, passTwo);

    CGT(car_sbe_rewind(&m_carDecoder));
    std::string passThree = partialWalkCar(m_carDecoder);
    CGT(car_sbe_rewind(&m_carDecoder));
    std::string passFour = partialWalkCar(m_carDecoder);
    EXPECT_EQ(passThree, passFour);

    CGT(car_sbe_rewind(&m_carDecoder));
    std::string passFive = walkCar(m_carDecoder);
    EXPECT_EQ(passOne, passFive);
}
