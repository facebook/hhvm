// Portions Copyright (C) 2017 MarketFactory, Inc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using System.Text;
using Baseline;
using Org.SbeTool.Sbe.Dll;

class Example
{
    public static void ExampleMain()
    {
        // This byte array is used for encoding and decoding, this is what you would send on the wire or save to disk
        var byteBuffer = new byte[4096];
        // You need to "wrap" the array with a DirectBuffer, this class is used by the generated code to read and write efficiently to the underlying byte array
        var directBuffer = new DirectBuffer(byteBuffer);
        const short SchemaVersion = 0;
        int bufferOffset = 0;

        var MessageHeader = new MessageHeader();
        var Car = new Car();

        // Before encoding a message we need to create a SBE header which specify what we are going to encode (this will allow the decoder to detect that it's an encoded 'car' object)
        // We will probably simplify this part soon, so the header gets applied automatically, but for now it's manual
        MessageHeader.Wrap(directBuffer, bufferOffset, MessageHeader.SbeSchemaVersion); // position the MessageHeader on the DirectBuffer, at the correct position
        MessageHeader.BlockLength = Car.BlockLength; // size that a car takes on the wire
        MessageHeader.SchemaId = Car.SchemaId;
        MessageHeader.TemplateId = Car.TemplateId;   // identifier for the car object (SBE template ID)
        MessageHeader.Version = Car.SchemaVersion; // this can be overridden if we want to support different versions of the car object (advanced functionality)

        // Now that we have encoded the header in the byte array we can encode the car object itself
        bufferOffset += MessageHeader.Size;
        CarExample.Encode(Car, directBuffer, bufferOffset);

        // Now we have encoded the message is the byte array, we are going to decode it

        // first we decode the header (in a real world scenario you would need the header to decide which SBE decoder you are going to use
        bufferOffset = 0;
        // position the MessageHeader object at the beginning of the array
        MessageHeader.Wrap(directBuffer, bufferOffset, SchemaVersion);

        // Extract info from the header
        // In a real app you would use that to lookup the applicable flyweight to decode this type of message based on templateId and version.
        int actingBlockLength = MessageHeader.BlockLength;
        int actingVersion = MessageHeader.Version;

        bufferOffset += MessageHeader.Size;
        // now we decode the message
        CarExample.Decode(Car, directBuffer, bufferOffset, actingBlockLength, actingVersion);
    }
}

namespace Baseline
{
    public static class CarExample
    {
        private static readonly byte[] ManufacturerCode;
        private static readonly byte[] ActivationCode;

        private static readonly string VehicleCode = "abcdef";
        private static readonly string Model = "Civic VTi";
        private static readonly string Manufacturer = "Honda";

        static CarExample()
        {
            try
            {
                // convert some sample strings to the correct encoding for this sample
                ManufacturerCode = Engine.ManufacturerCodeResolvedCharacterEncoding.GetBytes("123");
                ActivationCode = Car.ActivationCodeResolvedCharacterEncoding.GetBytes("abcdef");
            }
            catch (EncoderFallbackException ex)
            {
                throw new Exception("An error occurred while reading encodings", ex);
            }
        }

        public static int Encode(Car car, DirectBuffer directBuffer, int bufferOffset)
        {
            const int srcOffset = 0;

            // we position the car encoder on the direct buffer, at the correct offset (ie. just after the header)
            car.WrapForEncode(directBuffer, bufferOffset);
            car.SerialNumber = 1234; // we set the different fields, just as normal properties and they get written straight to the underlying byte buffer
            car.ModelYear = 2013;
            car.Available = BooleanType.T; // enums are directly supported
            car.Code = Baseline.Model.A;
            car.SetVehicleCode(VehicleCode); // we set a constant string

            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                car.SetSomeNumbers(i, (uint)i); // this property is defined as a constant length array of integers
            }

            car.Extras = OptionalExtras.CruiseControl | OptionalExtras.SportsPack; // bit set (flag enums in C#) are supported

            car.Engine.Capacity = 2000;
            car.Engine.NumCylinders = 4;
            car.Engine.SetManufacturerCode(ManufacturerCode, srcOffset);
            car.Engine.Efficiency = 35;
            car.Engine.BoosterEnabled = BooleanType.T;
            car.Engine.Booster.BoostType = BoostType.NITROUS;
            car.Engine.Booster.HorsePower = 200;

            // we have written all the constant length fields, now we can write the repeatable groups

            var fuelFigures = car.FuelFiguresCount(3); // we specify that we are going to write 3 FuelFigures (the API is not very .NET friendly yet, we will address that)
            fuelFigures.Next(); // move to the first element
            fuelFigures.Speed = 30;
            fuelFigures.Mpg = 35.9f;
            fuelFigures.SetUsageDescription("Urban Cycle");

            fuelFigures.Next(); // second
            fuelFigures.Speed = 55;
            fuelFigures.Mpg = 49.0f;
            fuelFigures.SetUsageDescription("Combined Cycle");

            fuelFigures.Next();
            fuelFigures.Speed = 75;
            fuelFigures.Mpg = 40.0f;
            fuelFigures.SetUsageDescription("Highway Cycle");


            Car.PerformanceFiguresGroup perfFigures = car.PerformanceFiguresCount(2); // demonstrates how to create a nested group
            perfFigures.Next();
            perfFigures.OctaneRating = 95;

            Car.PerformanceFiguresGroup.AccelerationGroup acceleration = perfFigures.AccelerationCount(3).Next(); // this group is going to be nested in the first element of the previous group
            acceleration.Mph = 30;
            acceleration.Seconds = 4.0f;

            acceleration.Next();
            acceleration.Mph = 60;
            acceleration.Seconds = 7.5f;

            acceleration.Next();
            acceleration.Mph = 100;
            acceleration.Seconds = 12.2f;

            perfFigures.Next();
            perfFigures.OctaneRating = 99;
            acceleration = perfFigures.AccelerationCount(3).Next();

            acceleration.Mph = 30;
            acceleration.Seconds = 3.8f;

            acceleration.Next();
            acceleration.Mph = 60;
            acceleration.Seconds = 7.1f;

            acceleration.Next();
            acceleration.Mph = 100;
            acceleration.Seconds = 11.8f;

            // once we have written all the repeatable groups we can write the variable length properties (you would use that for strings, byte[], etc)

            car.SetManufacturer(Manufacturer);
            car.SetModel(Model);
            car.SetActivationCode(ActivationCode, srcOffset, ActivationCode.Length);

            return car.Size;
        }

        public static void Decode(Car car,
            DirectBuffer directBuffer,
            int bufferOffset,
            int actingBlockLength,
            int actingVersion)
        {
            var buffer = new byte[128];
            var sb = new StringBuilder();

            // position the car flyweight just after the header on the DirectBuffer
            car.WrapForDecode(directBuffer, bufferOffset, actingBlockLength, actingVersion);

            // decode the car properties on by one, directly from the buffer
            sb.Append("\ncar.templateId=").Append(Car.TemplateId);
            sb.Append("\ncar.schemaVersion=").Append(Car.SchemaVersion);
            sb.Append("\ncar.serialNumber=").Append(car.SerialNumber);
            sb.Append("\ncar.modelYear=").Append(car.ModelYear);
            sb.Append("\ncar.available=").Append(car.Available);
            sb.Append("\ncar.code=").Append(car.Code);

            sb.Append("\ncar.someNumbers=");
            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                sb.Append(car.GetSomeNumbers(i)).Append(", ");
            }

            sb.Append("\ncar.vehicleCode=");
            sb.Append(car.GetVehicleCode());
   
            OptionalExtras extras = car.Extras;
            sb.Append("\ncar.extras.cruiseControl=").Append((extras & OptionalExtras.CruiseControl) == OptionalExtras.CruiseControl); // this is how you can find out if a specific flag is set in a flag enum
            sb.Append("\ncar.extras.sportsPack=").Append((extras & OptionalExtras.SportsPack) == OptionalExtras.SportsPack);
            sb.Append("\ncar.extras.sunRoof=").Append((extras & OptionalExtras.SunRoof) == OptionalExtras.SunRoof);

            Engine engine = car.Engine;
            sb.Append("\ncar.engine.capacity=").Append(engine.Capacity);
            sb.Append("\ncar.engine.numCylinders=").Append(engine.NumCylinders);
            sb.Append("\ncar.engine.maxRpm=").Append(engine.MaxRpm);
            sb.Append("\ncar.engine.manufacturerCode=");
            for (int i = 0, size = Engine.ManufacturerCodeLength; i < size; i++)
            {
                sb.Append((char)engine.GetManufacturerCode(i));
            }

            int length = engine.GetFuel(buffer, 0, buffer.Length);

            sb.Append("\ncar.engine.fuel=").Append(Encoding.ASCII.GetString(buffer, 0, length)); // string requires a bit of work to decode

            sb.Append("\ncar.engine.Efficiency=").Append(engine.Efficiency);
            sb.Append("\ncar.engine.BoosterEnabled=").Append(engine.BoosterEnabled);
            sb.Append("\ncar.engine.Booster.BoostType=").Append(engine.Booster.BoostType);
            sb.Append("\ncar.engine.Booster.HorsePower=").Append(engine.Booster.HorsePower);

            // The first way to access a repeating group is by using Next()
            var fuelFiguresGroup = car.FuelFigures; 
            while (fuelFiguresGroup.HasNext)
            {
                var fuelFigures = fuelFiguresGroup.Next();
                sb.Append("\ncar.fuelFigures.speed=").Append(fuelFigures.Speed);
                sb.Append("\ncar.fuelFigures.mpg=").Append(fuelFigures.Mpg);
                sb.Append("\ncar.fuelFigures.UsageDescription=").Append(fuelFigures.GetUsageDescription());
            }

            // The second way to access a repeating group is to use an iterator
            foreach (Car.PerformanceFiguresGroup performanceFigures in car.PerformanceFigures)
            {
                sb.Append("\ncar.performanceFigures.octaneRating=").Append(performanceFigures.OctaneRating);

                // The third way to access a repeating group is loop over the count of elements
                var accelerationGroup = performanceFigures.Acceleration;
                for (int i = 0; i < accelerationGroup.Count; i++)
                {
                    var acceleration = accelerationGroup.Next();
                    sb.Append("\ncar.performanceFigures.acceleration.mph=").Append(acceleration.Mph);
                    sb.Append("\ncar.performanceFigures.acceleration.seconds=").Append(acceleration.Seconds);
                }
            }

            // variable length fields
            sb.Append("\ncar.manufacturer.semanticType=").Append(Car.ManufacturerMetaAttribute(MetaAttribute.SemanticType));
            length = car.GetManufacturer(buffer, 0, buffer.Length);
            sb.Append("\ncar.manufacturer=").Append(Car.ManufacturerResolvedCharacterEncoding.GetString(buffer, 0, length));

            sb.Append("\ncar.model=").Append(car.GetModel());

            sb.Append("\ncar.activationcode=").Append(car.GetActivationCode());

            sb.Append("\ncar.size=").Append(car.Size);

            Console.WriteLine(sb);
        }
    }
}
