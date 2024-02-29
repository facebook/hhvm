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
import baseline.EngineDecoder;
import baseline.OptionalExtrasDecoder;

import java.util.ArrayList;

class CarDecodeTestUtil
{
    static ArrayList<Object> getValues(final CarDecoder carDecoder)
    {
        final ArrayList<Object> values = new ArrayList<>();

        values.add(carDecoder.serialNumber());
        values.add(carDecoder.modelYear());
        values.add(carDecoder.available());
        values.add(carDecoder.code());
        values.add(CarDecoder.someNumbersLength());
        for (int i = 0, n = CarDecoder.someNumbersLength(); i < n; i++)
        {
            values.add(carDecoder.someNumbers(i));
        }
        values.add(carDecoder.vehicleCode());
        final OptionalExtrasDecoder extras = carDecoder.extras();
        values.add(extras.sunRoof());
        values.add(extras.sportsPack());
        values.add(extras.cruiseControl());
        final EngineDecoder engine = carDecoder.engine();
        values.add(engine.capacity());
        values.add(engine.numCylinders());
        values.add(engine.maxRpm());
        values.add(engine.manufacturerCode());
        values.add(engine.fuel());
        final CarDecoder.FuelFiguresDecoder fuelFigures = carDecoder.fuelFigures();
        while (fuelFigures.hasNext())
        {
            fuelFigures.next();
            values.add(fuelFigures.speed());
            values.add(fuelFigures.mpg());
        }
        final CarDecoder.PerformanceFiguresDecoder performanceFigures = carDecoder.performanceFigures();
        while (performanceFigures.hasNext())
        {
            performanceFigures.next();
            values.add(performanceFigures.octaneRating());
            final CarDecoder.PerformanceFiguresDecoder.AccelerationDecoder acceleration =
                performanceFigures.acceleration();
            while (acceleration.hasNext())
            {
                acceleration.next();
                values.add(acceleration.mph());
                values.add(acceleration.seconds());
            }
        }
        values.add(carDecoder.manufacturer());
        values.add(carDecoder.model());
        values.add(carDecoder.activationCode());
        return values;
    }

    static ArrayList<Object> getPartialValues(final CarDecoder carDecoder)
    {
        final ArrayList<Object> values = new ArrayList<>();

        values.add(carDecoder.serialNumber());
        values.add(carDecoder.modelYear());
        values.add(carDecoder.available());
        values.add(carDecoder.code());
        values.add(CarDecoder.someNumbersLength());
        for (int i = 0, n = CarDecoder.someNumbersLength(); i < n; i++)
        {
            values.add(carDecoder.someNumbers(i));
        }
        values.add(carDecoder.vehicleCode());
        final OptionalExtrasDecoder extras = carDecoder.extras();
        values.add(extras.sunRoof());
        values.add(extras.sportsPack());
        values.add(extras.cruiseControl());
        final EngineDecoder engine = carDecoder.engine();
        values.add(engine.capacity());
        values.add(engine.numCylinders());
        values.add(engine.maxRpm());
        values.add(engine.manufacturerCode());
        values.add(engine.fuel());
        final CarDecoder.FuelFiguresDecoder fuelFigures = carDecoder.fuelFigures();
        while (fuelFigures.hasNext())
        {
            fuelFigures.next();
            values.add(fuelFigures.speed());
            values.add(fuelFigures.mpg());
        }
        final CarDecoder.PerformanceFiguresDecoder performanceFigures = carDecoder.performanceFigures();

        // Stop decoding part way through the message.
        if (performanceFigures.hasNext())
        {
            performanceFigures.next();
            values.add(performanceFigures.octaneRating());
            final CarDecoder.PerformanceFiguresDecoder.AccelerationDecoder acceleration =
                performanceFigures.acceleration();
            if (acceleration.hasNext())
            {
                acceleration.next();
                values.add(acceleration.mph());
            }
        }

        return values;
    }
}
