// Copyright (C) 2017 MarketFactory, Inc
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

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Org.SbeTool.Sbe.Dll;
using BaselineBigendian;

// If you make modifications here you probably have to modify
// the Bigendian tests as well

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public unsafe class BigendianTests
    {
        // The byte array is from the java example made by running 
        // with -Dsbe.encoding.filename and then decoded using od -tu1
        private readonly byte[] _decodeBuffer = { 0, 49, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 210, 7, 221, 1, 65, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 97, 98, 99, 100, 101, 102, 6, 7, 208, 4, 49, 50, 51, 35, 1, 78, 200, 0, 6, 0, 3, 0, 30, 66, 15, 153, 154, 0, 0, 0, 11, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 0, 55, 66, 68, 0, 0, 0, 0, 0, 14, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 0, 75, 66, 32, 0, 0, 0, 0, 0, 13, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 0, 1, 0, 2, 95, 0, 6, 0, 3, 0, 30, 64, 128, 0, 0, 0, 60, 64, 240, 0, 0, 0, 100, 65, 67, 51, 51, 99, 0, 6, 0, 3, 0, 30, 64, 115, 51, 51, 0, 60, 64, 227, 51, 51, 0, 100, 65, 60, 204, 205, 0, 0, 0, 5, 72, 111, 110, 100, 97, 0, 0, 0, 9, 67, 105, 118, 105, 99, 32, 86, 84, 105, 0, 0, 0, 6, 97, 98, 99, 100, 101, 102 };
        private byte[] _encodeBuffer;

        private DirectBuffer _directBuffer;
        private MessageHeader _messageHeader;

        [TestInitialize]
        public void SetUp()
        {
            _encodeBuffer = new byte[_decodeBuffer.Length];
            _messageHeader = new MessageHeader();
        }

        [TestMethod]
        public void BigendianDecodeTest()
        {
            _directBuffer = new DirectBuffer(_decodeBuffer);
            _messageHeader.Wrap(_directBuffer, 0, Baseline.MessageHeader.SbeSchemaVersion);

            var car = new Car();
            car.WrapForDecode(_directBuffer, MessageHeader.Size, _messageHeader.BlockLength, _messageHeader.Version);

            Assert.AreEqual(1234UL, car.SerialNumber);
            Assert.AreEqual((ushort)2013, car.ModelYear);
            Assert.AreEqual(BooleanType.T, car.Available);
            Assert.AreEqual(Model.A, car.Code);
            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                Assert.AreEqual(car.GetSomeNumbers(i), (uint)i);
            }

            var btmp = new byte[6];
            car.GetVehicleCode(btmp, 0);
            var s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("abcdef", s);
            Assert.AreEqual(OptionalExtras.CruiseControl | OptionalExtras.SportsPack, car.Extras);
            Assert.AreEqual((ushort)2000, car.Engine.Capacity);
            Assert.AreEqual((byte)4, car.Engine.NumCylinders);

            btmp = new byte[3];
            car.Engine.GetManufacturerCode(btmp, 0);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("123", s);
            Assert.AreEqual((sbyte)35, car.Engine.Efficiency);
            Assert.AreEqual(BooleanType.T, car.Engine.BoosterEnabled);
            Assert.AreEqual(BoostType.NITROUS, car.Engine.Booster.BoostType);
            Assert.AreEqual((byte)200, car.Engine.Booster.HorsePower);

            var fuelFigures = car.FuelFigures;
            Assert.AreEqual(3, fuelFigures.Count);

            fuelFigures.Next();
            Assert.AreEqual(30, fuelFigures.Speed);
            Assert.AreEqual(35.9f, fuelFigures.Mpg);
            btmp = new byte[11];
            fuelFigures.GetUsageDescription(btmp, 0, 11);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("Urban Cycle", s);

            fuelFigures.Next();
            Assert.AreEqual(55, fuelFigures.Speed);
            Assert.AreEqual(49.0f, fuelFigures.Mpg);
            btmp = new byte[14];
            fuelFigures.GetUsageDescription(btmp, 0, 14);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("Combined Cycle", s);

            fuelFigures.Next();
            Assert.AreEqual(75, fuelFigures.Speed);
            Assert.AreEqual(40.0f, fuelFigures.Mpg);
            btmp = new byte[13];
            fuelFigures.GetUsageDescription(btmp, 0, 13);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("Highway Cycle", s);

            var perfFigures = car.PerformanceFigures;
            Assert.AreEqual(2, perfFigures.Count);

            perfFigures.Next();
            Assert.AreEqual((byte)95, perfFigures.OctaneRating);
            var acceleration = perfFigures.Acceleration;
            Assert.AreEqual(3, acceleration.Count);
            acceleration.Next();
            Assert.AreEqual((ushort)30, acceleration.Mph);
            Assert.AreEqual(4.0f, acceleration.Seconds);
            acceleration.Next();
            Assert.AreEqual((ushort)60, acceleration.Mph);
            Assert.AreEqual(7.5f, acceleration.Seconds);
            acceleration.Next();
            Assert.AreEqual((ushort)100, acceleration.Mph);
            Assert.AreEqual(12.2f, acceleration.Seconds);

            perfFigures.Next();
            Assert.AreEqual((byte)99, perfFigures.OctaneRating);
            acceleration = perfFigures.Acceleration;
            Assert.AreEqual(3, acceleration.Count);
            acceleration.Next();
            Assert.AreEqual((ushort)30, acceleration.Mph);
            Assert.AreEqual(3.8f, acceleration.Seconds);
            acceleration.Next();
            Assert.AreEqual((ushort)60, acceleration.Mph);
            Assert.AreEqual(7.1f, acceleration.Seconds);
            acceleration.Next();
            Assert.AreEqual((ushort)100, acceleration.Mph);
            Assert.AreEqual(11.8f, acceleration.Seconds);

            btmp = new byte[5];
            car.GetManufacturer(btmp, 0, 5);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("Honda", s);
            btmp = new byte[9];
            car.GetModel(btmp, 0, 9);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("Civic VTi", s);
            btmp = new byte[6];
            car.GetActivationCode(btmp, 0, 6);
            s = System.Text.Encoding.UTF8.GetString(btmp, 0, btmp.Length);
            Assert.AreEqual("abcdef", s);
        }

        [TestMethod]
        public void BigendianEncodeTest()
        {
            _directBuffer = new DirectBuffer(_encodeBuffer);
            _messageHeader.Wrap(_directBuffer, 0, Baseline.MessageHeader.SbeSchemaVersion);
            _messageHeader.BlockLength = Car.BlockLength;
            _messageHeader.SchemaId = Car.SchemaId;
            _messageHeader.TemplateId = Car.TemplateId;
            _messageHeader.Version = Car.SchemaVersion;

            var car = new Car();

            // Populate the car with the known values
            car.WrapForEncode(_directBuffer, Baseline.MessageHeader.Size);
            car.SerialNumber = 1234;
            car.ModelYear = 2013;
            car.Available = BooleanType.T;
            car.Code = Model.A;
            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                car.SetSomeNumbers(i, (uint)i);
            }
            car.SetVehicleCode(System.Text.Encoding.ASCII.GetBytes("abcdef"), 0);
            car.Extras = OptionalExtras.CruiseControl | OptionalExtras.SportsPack;

            car.Engine.Capacity = 2000;
            car.Engine.NumCylinders = 4;
            car.Engine.SetManufacturerCode(System.Text.Encoding.ASCII.GetBytes("123"), 0);
            car.Engine.Efficiency = 35;
            car.Engine.BoosterEnabled = BooleanType.T;
            car.Engine.Booster.BoostType = BoostType.NITROUS;
            car.Engine.Booster.HorsePower = 200;

            var fuelFigures = car.FuelFiguresCount(3);
            fuelFigures.Next();
            fuelFigures.Speed = 30;
            fuelFigures.Mpg = 35.9f;
            fuelFigures.SetUsageDescription(System.Text.Encoding.ASCII.GetBytes("Urban Cycle"), 0, 11);

            fuelFigures.Next();
            fuelFigures.Speed = 55;
            fuelFigures.Mpg = 49.0f;
            fuelFigures.SetUsageDescription(System.Text.Encoding.ASCII.GetBytes("Combined Cycle"), 0, 14);

            fuelFigures.Next();
            fuelFigures.Speed = 75;
            fuelFigures.Mpg = 40.0f;
            fuelFigures.SetUsageDescription(System.Text.Encoding.ASCII.GetBytes("Highway Cycle"), 0, 13);

            var perfFigures = car.PerformanceFiguresCount(2);
            perfFigures.Next();
            perfFigures.OctaneRating = 95;

            var acceleration = perfFigures.AccelerationCount(3).Next();
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

            car.SetManufacturer(System.Text.Encoding.ASCII.GetBytes("Honda"), 0, 5);
            car.SetModel(System.Text.Encoding.ASCII.GetBytes("Civic VTi"), 0, 9);
            car.SetActivationCode(System.Text.Encoding.ASCII.GetBytes("abcdef"), 0, 6);

            for (int i = 0; i < _decodeBuffer.Length; i++)
            {
                Assert.AreEqual(_encodeBuffer[i], _decodeBuffer[i]);
            }
        }
    }
}
