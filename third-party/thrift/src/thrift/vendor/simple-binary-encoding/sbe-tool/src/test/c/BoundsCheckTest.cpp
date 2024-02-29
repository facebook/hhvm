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

#include <cstring>
#include <iostream>
#include <memory>

#include <gtest/gtest.h>

#include "code_generation_test/car.h"
#include "code_generation_test/messageHeader.h"

#define CGT(name) code_generation_test_##name

#define SERIAL_NUMBER 1234u
#define MODEL_YEAR 2013
#define AVAILABLE (CGT(booleanType_T))
#define CODE (CGT(model_A))
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
protected:
    std::uint64_t encodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!CGT(messageHeader_wrap)(&m_hdr, buffer, offset, 0, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(messageHeader_set_blockLength)(&m_hdr, CGT(car_sbe_block_length)());
        CGT(messageHeader_set_templateId)(&m_hdr, CGT(car_sbe_template_id)());
        CGT(messageHeader_set_schemaId)(&m_hdr, CGT(car_sbe_schema_id)());
        CGT(messageHeader_set_version)(&m_hdr, CGT(car_sbe_schema_version)());

        return CGT(messageHeader_encoded_length)();
    }

    std::uint64_t decodeHdr(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!CGT(messageHeader_wrap)(&m_hdrDecoder, buffer, offset, 0, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(messageHeader_blockLength)(&m_hdrDecoder), CGT(car_sbe_block_length)());
        EXPECT_EQ(CGT(messageHeader_templateId)(&m_hdrDecoder), CGT(car_sbe_template_id)());
        EXPECT_EQ(CGT(messageHeader_schemaId)(&m_hdrDecoder), CGT(car_sbe_schema_id)());
        EXPECT_EQ(CGT(messageHeader_version)(&m_hdrDecoder), CGT(car_sbe_schema_version)());

        return CGT(messageHeader_encoded_length)();
    }

    std::uint64_t encodeCarRoot(char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
    {
        if (!CGT(car_wrap_for_encode)(&m_car, buffer, offset, bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_set_serialNumber)(&m_car, SERIAL_NUMBER);
        CGT(car_set_modelYear)(&m_car, MODEL_YEAR);
        CGT(car_set_available)(&m_car, AVAILABLE);
        CGT(car_set_code)(&m_car, CODE);
        CGT(car_put_vehicleCode)(&m_car, VEHICLE_CODE);

        for (uint64_t i = 0; i < CGT(car_someNumbers_length)(); i++)
        {
            CGT(car_set_someNumbers_unsafe)(&m_car, i, (int32_t)(i));
        }

        CGT(optionalExtras) extras;
        if (!CGT(car_extras)(&m_car, &extras))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(optionalExtras_clear)(&extras);
        CGT(optionalExtras_set_cruiseControl)(&extras, CRUISE_CONTROL);
        CGT(optionalExtras_set_sportsPack)(&extras, SPORTS_PACK);
        CGT(optionalExtras_set_sunRoof)(&extras, SUNROOF);

        CGT(engine) engine;
        if (!CGT(car_engine)(&m_car, &engine))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(engine_set_capacity)(&engine, 2000);
        CGT(engine_set_numCylinders)(&engine, (short)4);
        CGT(engine_put_manufacturerCode)(&engine, MANUFACTURER_CODE);

        CGT(boosterT) booster;
        if (!CGT(engine_booster)(&engine, &booster))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(boosterT_set_boostType)(&booster, CGT(boostType_NITROUS));
        CGT(boosterT_set_horsePower)(&booster, 200);

        return CGT(car_encoded_length)(&m_car);
    }

    std::uint64_t encodeCarFuelFigures()
    {
        CGT(car_fuelFigures) fuelFigures;
        if (!CGT(car_fuelFigures_set_count)(&m_car, &fuelFigures, 3))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_fuelFigures_set_speed)(&fuelFigures, 30);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, 35.9f);
        if (!CGT(car_fuelFigures_put_usageDescription)(&fuelFigures, "Urban Cycle", 11))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_fuelFigures_set_speed)(&fuelFigures, 55);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, 49.0f);
        if (!CGT(car_fuelFigures_put_usageDescription)(&fuelFigures, "Combined Cycle", 14))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_fuelFigures_set_speed)(&fuelFigures, 75);
        CGT(car_fuelFigures_set_mpg)(&fuelFigures, 40.0f);
        if (!CGT(car_fuelFigures_put_usageDescription)(&fuelFigures, "Highway Cycle", 13))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        return CGT(car_encoded_length)(&m_car);
    }

    std::uint64_t encodeCarPerformanceFigures()
    {
        CGT(car_performanceFigures) perf_figs;
        if (!CGT(car_performanceFigures_set_count)(&m_car, &perf_figs, 2))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        if (!CGT(car_performanceFigures_next)(&perf_figs))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        CGT(car_performanceFigures_set_octaneRating)(&perf_figs, (short)95);
        CGT(car_performanceFigures_acceleration) acc;
        if (!CGT(car_performanceFigures_acceleration_set_count)(&perf_figs, &acc, 3))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 30);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 4.0f);
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 60);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 7.5f);
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 100);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 12.2f);

        if (!CGT(car_performanceFigures_next)(&perf_figs))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_set_octaneRating)(&perf_figs, (short)99);
        if (!CGT(car_performanceFigures_acceleration_set_count)(&perf_figs, &acc, 3))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 30);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 3.8f);
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 60);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 7.1f);
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(car_performanceFigures_acceleration_set_mph)(&acc, 100);
        CGT(car_performanceFigures_acceleration_set_seconds)(&acc, 11.8f);

        return CGT(car_encoded_length)(&m_car);
    }

    std::uint64_t encodeCarManufacturerModelAndActivationCode()
    {
        if (!CGT(car_put_manufacturer)(&m_car, MANUFACTURER, (int)(strlen(MANUFACTURER))))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        if (!CGT(car_put_model)(&m_car, MODEL, (int)(strlen(MODEL))))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        if (!CGT(car_put_activationCode)(&m_car, ACTIVATION_CODE, (int)(strlen(ACTIVATION_CODE))))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        return CGT(car_encoded_length)(&m_car);
    }

    std::uint64_t decodeCarRoot(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        if (!CGT(car_wrap_for_decode)(
            &m_carDecoder,
            buffer,
            offset,
            CGT(car_sbe_block_length)(),
            CGT(car_sbe_schema_version)(),
            bufferLength))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_serialNumber)(&m_carDecoder), SERIAL_NUMBER);
        EXPECT_EQ(CGT(car_modelYear)(&m_carDecoder), MODEL_YEAR);
        {
            CGT(booleanType) out;
            EXPECT_TRUE(CGT(car_available)(&m_carDecoder, &out));
            EXPECT_EQ(out, AVAILABLE);
        }
        {
            CGT(model) out;
            EXPECT_TRUE(CGT(car_code)(&m_carDecoder, &out));
            EXPECT_EQ(out, CODE);
        }

        EXPECT_EQ(CGT(car_someNumbers_length)(), 5u);
        for (std::uint64_t i = 0; i < 5; i++)
        {
            EXPECT_EQ(CGT(car_someNumbers_unsafe)(&m_carDecoder, i), (int32_t)(i));
        }

        EXPECT_EQ(CGT(car_vehicleCode_length)(), 6u);
        EXPECT_EQ(std::string(CGT(car_vehicleCode_buffer)(&m_carDecoder), 6), std::string(VEHICLE_CODE, 6));
        CGT(optionalExtras) extras;
        if (!CGT(car_extras)(&m_carDecoder, &extras))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_TRUE(CGT(optionalExtras_cruiseControl)(&extras));
        EXPECT_TRUE(CGT(optionalExtras_sportsPack)(&extras));
        EXPECT_FALSE(CGT(optionalExtras_sunRoof)(&extras));

        CGT(engine) engine;
        if (!CGT(car_engine)(&m_carDecoder, &engine))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(engine_capacity)(&engine), 2000);
        EXPECT_EQ(CGT(engine_numCylinders)(&engine), 4);
        EXPECT_EQ(CGT(engine_maxRpm)(), 9000);
        EXPECT_EQ(CGT(engine_manufacturerCode_length)(), 3u);
        EXPECT_EQ(std::string(CGT(engine_manufacturerCode_buffer)(&engine), 3), std::string(MANUFACTURER_CODE, 3));
        EXPECT_EQ(CGT(engine_fuel_length)(), 6u);
        EXPECT_EQ(std::string(CGT(engine_fuel)(), 6), "Petrol");
        CGT(boosterT) booster;
        if (!CGT(engine_booster)(&engine, &booster))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        CGT(boostType) out;
        EXPECT_TRUE(CGT(boosterT_boostType)(&booster, &out));
        EXPECT_EQ(out, CGT(boostType_NITROUS));
        EXPECT_EQ(CGT(boosterT_horsePower)(&booster), 200);

        return CGT(car_encoded_length)(&m_carDecoder);
    }

    std::uint64_t decodeCarFuelFigures()
    {
        char tmp[256] = { 0 };
        CGT(car_fuelFigures) fuelFigures;
        if (!CGT(car_get_fuelFigures)(&m_carDecoder, &fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_fuelFigures_count)(&fuelFigures), 3u);

        EXPECT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }

        EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), 30);
        EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), 35.9f);
        std::uint64_t bytesToCopy =
            CGT(car_fuelFigures_get_usageDescription)(&fuelFigures, tmp, sizeof(tmp));
        if (!bytesToCopy)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(bytesToCopy, 11u);
        EXPECT_EQ(std::string(tmp, 11), "Urban Cycle");

        EXPECT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), 55);
        EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), 49.0f);
        bytesToCopy = CGT(car_fuelFigures_get_usageDescription)(&fuelFigures, tmp, sizeof(tmp));
        if (!bytesToCopy)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(bytesToCopy, 14u);
        EXPECT_EQ(std::string(tmp, 14), "Combined Cycle");

        EXPECT_TRUE(CGT(car_fuelFigures_has_next)(&fuelFigures));
        if (!CGT(car_fuelFigures_next)(&fuelFigures))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_fuelFigures_speed)(&fuelFigures), 75);
        EXPECT_EQ(CGT(car_fuelFigures_mpg)(&fuelFigures), 40.0f);
        bytesToCopy = CGT(car_fuelFigures_get_usageDescription)(&fuelFigures, tmp, sizeof(tmp));
        if (!bytesToCopy)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(bytesToCopy, 13u);
        EXPECT_EQ(std::string(tmp, 13), "Highway Cycle");

        return CGT(car_encoded_length)(&m_carDecoder);
    }

    std::uint64_t decodeCarPerformanceFigures()
    {
        CGT(car_performanceFigures) perfFigs;
        if (!CGT(car_get_performanceFigures)(&m_carDecoder, &perfFigs))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_count)(&perfFigs), 2u);

        EXPECT_TRUE(CGT(car_performanceFigures_has_next)(&perfFigs));
        if (!CGT(car_performanceFigures_next)(&perfFigs))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_octaneRating)(&perfFigs), 95);

        CGT(car_performanceFigures_acceleration) acc;
        if (!CGT(car_performanceFigures_get_acceleration)(&perfFigs, &acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_count)(&acc), 3u);
        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 30);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 4.0f);

        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 60);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 7.5f);

        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 100);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 12.2f);

        EXPECT_TRUE(CGT(car_performanceFigures_has_next)(&perfFigs));
        if (!CGT(car_performanceFigures_next)(&perfFigs))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_octaneRating)(&perfFigs), 99);

        if (!CGT(car_performanceFigures_get_acceleration)(&perfFigs, &acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_count)(&acc), 3u);
        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 30);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 3.8f);

        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 60);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 7.1f);

        EXPECT_TRUE(CGT(car_performanceFigures_acceleration_has_next)(&acc));
        if (!CGT(car_performanceFigures_acceleration_next)(&acc))
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_mph)(&acc), 100);
        EXPECT_EQ(CGT(car_performanceFigures_acceleration_seconds)(&acc), 11.8f);

        return CGT(car_encoded_length)(&m_carDecoder);
    }

    std::uint64_t decodeCarManufacturerModelAndActivationCode()
    {
        char tmp[256] = { 0 };
        std::uint64_t lengthOfField = CGT(car_get_manufacturer)(&m_carDecoder, tmp, sizeof(tmp));
        if (!lengthOfField)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(lengthOfField, 5u);
        EXPECT_EQ(std::string(tmp, 5), "Honda");

        lengthOfField = CGT(car_get_model)(&m_carDecoder, tmp, sizeof(tmp));
        if (!lengthOfField)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(lengthOfField, 9u);
        EXPECT_EQ(std::string(tmp, 9), "Civic VTi");

        lengthOfField = CGT(car_get_activationCode)(&m_carDecoder, tmp, sizeof(tmp));
        if (!lengthOfField)
        {
            throw std::runtime_error(sbe_strerror(errno));
        }
        EXPECT_EQ(lengthOfField, 8u);
        EXPECT_EQ(std::string(tmp, 8), "deadbeef");

        EXPECT_EQ(CGT(car_encoded_length)(&m_carDecoder), encodedCarSz);

        return CGT(car_encoded_length)(&m_carDecoder);
    }

private:
    CGT(messageHeader) m_hdr = {};
    CGT(messageHeader) m_hdrDecoder = {};
    CGT(car) m_car = {};
    CGT(car) m_carDecoder = {};
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
    char encodeBuffer[8] = { 0 };
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
    char encodeBuffer[191] = { 0 };
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
