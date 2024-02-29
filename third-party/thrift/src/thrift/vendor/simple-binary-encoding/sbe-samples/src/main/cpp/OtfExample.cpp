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

#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <iomanip>

#include "baseline/MessageHeader.h"
#include "baseline/Car.h"

#include "otf/IrDecoder.h"
#include "otf/OtfHeaderDecoder.h"
#include "otf/OtfMessageDecoder.h"

using namespace sbe::otf;
using namespace baseline;

class ExampleTokenListener
{
public:
    std::vector<std::string> scope;
    int compositeLevel = 0;

    void printScope() const
    {
        std::for_each(scope.begin(), scope.end(),
            [&](const std::string &s)
            {
                std::cout << s;
            });
    }

    virtual std::string determineName(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex)
    {
        return compositeLevel > 1 ? tokens.at(fromIndex).name() : fieldToken.name();
    }

    virtual void onBeginMessage(Token &token)
    {
        scope.push_back(token.name() + ".");
    }

    virtual void onEndMessage(Token &token)
    {
        scope.pop_back();
    }

    static std::string asString(const Token &token, const char *buffer)
    {
        const Encoding &encoding = token.encoding();
        const PrimitiveType type = encoding.primitiveType();
        const std::uint64_t length = token.isConstantEncoding() ?
            encoding.constValue().size() : static_cast<std::uint64_t>(token.encodedLength());
        std::ostringstream result;

        std::uint64_t num = length / lengthOfType(type);

        switch (type)
        {
            case PrimitiveType::CHAR:
            {
                if (num > 1)
                {
                    if (token.isConstantEncoding())
                    {
                        buffer = encoding.constValue().getArray();
                    }

                    result << std::string(buffer, length);
                }
                break;
            }

            case PrimitiveType::INT8:
            case PrimitiveType::INT16:
            case PrimitiveType::INT32:
            case PrimitiveType::INT64:
            {
                if (num > 1)
                {
                    const char *separator = "";

                    for (std::size_t i = 0; i < num; i++)
                    {
                        result << separator
                               << Encoding::getInt(type, encoding.byteOrder(), buffer + (i * lengthOfType(type)));
                        separator = ", ";
                    }
                }
                else
                {
                    if (token.isConstantEncoding())
                    {
                        result << encoding.constValue().getAsInt();
                    }
                    else
                    {
                        result << encoding.getAsInt(buffer);
                    }
                }
                break;
            }

            case PrimitiveType::UINT8:
            case PrimitiveType::UINT16:
            case PrimitiveType::UINT32:
            case PrimitiveType::UINT64:
            {
                if (num == 1)
                {
                    if (token.isConstantEncoding())
                    {
                        result << encoding.constValue().getAsUInt();
                    }
                    else
                    {
                        result << encoding.getAsUInt(buffer);
                    }
                }
                break;
            }

            case PrimitiveType::FLOAT:
            case PrimitiveType::DOUBLE:
            {
                if (num == 1)
                {
                    result.setf(std::ios::fixed);
                    result << std::setprecision(1) << encoding.getAsDouble(buffer);
                }
                break;
            }

            default:
                break;
        }

        return result.str();
    }

    virtual void onEncoding(
        Token &fieldToken,
        const char *buffer,
        Token &typeToken,
        std::uint64_t actingVersion)
    {
        printScope();
        std::string name = compositeLevel > 1 ? typeToken.name() : fieldToken.name();

        std::cout << name << "=" << asString(typeToken, buffer) << std::endl;
    }

    virtual void onEnum(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion)
    {
        const Token &typeToken = tokens.at(fromIndex + 1);
        const Encoding &encoding = typeToken.encoding();

        printScope();
        std::cout << fieldToken.name() << "=";

        for (std::size_t i = fromIndex + 1; i < toIndex; i++)
        {
            const Token &token = tokens.at(i);
            const PrimitiveValue constValue = token.encoding().constValue();

            if (typeToken.isConstantEncoding())
            {
                std::cout << token.name();
                break;
            }

            if (encoding.primitiveType() == PrimitiveType::CHAR)
            {
                if (encoding.getAsInt(buffer) == constValue.getAsInt())
                {
                    std::cout << token.name();
                    break;
                }
            }
            else if (encoding.primitiveType() == PrimitiveType::UINT8)
            {
                if (encoding.getAsUInt(buffer) == constValue.getAsUInt())
                {
                    std::cout << token.name();
                    break;
                }
            }
        }

        std::cout << std::endl;
    }

    virtual void onBitSet(
        Token &fieldToken,
        const char *buffer,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex,
        std::uint64_t actingVersion)
    {
        const Token &typeToken = tokens.at(fromIndex + 1);
        const Encoding &encoding = typeToken.encoding();
        const std::uint64_t value = encoding.getAsUInt(buffer);

        printScope();
        std::cout << fieldToken.name() << ":";

        for (std::size_t i = fromIndex + 1; i < toIndex; i++)
        {
            const Token &token = tokens.at(i);
            const std::uint64_t constValue = token.encoding().constValue().getAsUInt();

            std::cout << " " << token.name() << "=";
            if (constValue && value)
            {
                std::cout << "true";
            }
            else
            {
                std::cout << "false";
            }
        }

        std::cout << std::endl;
    }

    virtual void onBeginComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex)
    {
        compositeLevel++;
        scope.push_back(determineName(fieldToken, tokens, fromIndex) + ".");
    }

    virtual void onEndComposite(
        Token &fieldToken,
        std::vector<Token> &tokens,
        std::size_t fromIndex,
        std::size_t toIndex)
    {
        compositeLevel--;
        scope.pop_back();
    }

    virtual void onGroupHeader(Token &token, std::uint64_t numInGroup)
    {
        printScope();
        std::cout << token.name() << " Group Header: numInGroup=" << numInGroup << "\n";
    }

    virtual void onBeginGroup(
        Token &token,
        std::uint64_t groupIndex,
        std::uint64_t numInGroup)
    {
        scope.push_back(token.name() + ".");
    }

    virtual void onEndGroup(
        Token &token,
        std::uint64_t groupIndex,
        std::uint64_t numInGroup)
    {
        scope.pop_back();
    }

    virtual void onVarData(
        Token &fieldToken,
        const char *buffer,
        std::uint64_t length,
        Token &typeToken)
    {
        printScope();
        std::cout << fieldToken.name() << "=" << std::string(buffer, length) << std::endl;
    }
};

static const std::uint32_t SERIAL_NUMBER = 1234;
static const std::uint16_t MODEL_YEAR = 2013;
static const BooleanType::Value AVAILABLE = BooleanType::T;
static const Model::Value CODE = Model::A;
static const bool CRUISE_CONTROL = true;
static const bool SPORTS_PACK = true;
static const bool SUNROOF = false;

static char VEHICLE_CODE[] = { 'a', 'b', 'c', 'd', 'e', 'f' };
static char MANUFACTURER_CODE[] = { '1', '2', '3' };
static const char *FUEL_FIGURES_1_USAGE_DESCRIPTION = "Urban Cycle";
static const char *FUEL_FIGURES_2_USAGE_DESCRIPTION = "Combined Cycle";
static const char *FUEL_FIGURES_3_USAGE_DESCRIPTION = "Highway Cycle";
static const char *MANUFACTURER = "Honda";
static const char *MODEL = "Civic VTi";
static const char *ACTIVATION_CODE = "deadbeef";

static const size_t PERFORMANCE_FIGURES_COUNT = 2;
static const size_t FUEL_FIGURES_COUNT = 3;
static const size_t ACCELERATION_COUNT = 3;

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

std::uint64_t encodeHdrAndCar(char *buffer, std::uint64_t length)
{
    MessageHeader hdr;
    Car car;

    hdr.wrap(buffer, 0, 0, length)
        .blockLength(Car::sbeBlockLength())
        .templateId(Car::sbeTemplateId())
        .schemaId(Car::sbeSchemaId())
        .version(Car::sbeSchemaVersion());

    car.wrapForEncode(buffer, hdr.encodedLength(), length - hdr.encodedLength())
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

    car.putManufacturer(MANUFACTURER, static_cast<int>(strlen(MANUFACTURER)))
        .putModel(MODEL, static_cast<int>(strlen(MODEL)))
        .putActivationCode(ACTIVATION_CODE, static_cast<int>(strlen(ACTIVATION_CODE)));

    return hdr.encodedLength() + car.encodedLength();
}

int main(int argc, char **argv)
{
    char buffer[2048] = {};
    ExampleTokenListener tokenListener;

    std::uint64_t sz = encodeHdrAndCar(buffer, sizeof(buffer));

    IrDecoder irDecoder;

    if (irDecoder.decode("generated/example-schema.sbeir") != 0)
    {
        std::cerr << "could not load SBE IR" << std::endl;
        return -1;
    }

    std::shared_ptr<std::vector<Token>> headerTokens = irDecoder.header();

    OtfHeaderDecoder headerDecoder(headerTokens);

    const char *messageBuffer = buffer + headerDecoder.encodedLength();
    std::uint64_t length = sz - headerDecoder.encodedLength();
    std::uint64_t templateId = headerDecoder.getTemplateId(buffer);
    std::uint64_t actingVersion = headerDecoder.getSchemaVersion(buffer);
    std::uint64_t blockLength = headerDecoder.getBlockLength(buffer);

    std::shared_ptr<std::vector<Token>> messageTokens = irDecoder.message(
        static_cast<int>(templateId), static_cast<int>(actingVersion));

    std::size_t result = OtfMessageDecoder::decode(
        messageBuffer, length, actingVersion, blockLength, messageTokens, tokenListener);

    std::cout << "result = " << result << std::endl;

    if (result != static_cast<std::size_t>(sz - headerDecoder.encodedLength()))
    {
        std::cerr << "Message not properly decoded " << result << "/(" << sz << " - " << headerDecoder.encodedLength()
                  << ")" << std::endl;
        return -1;
    }

    return 0;
}
