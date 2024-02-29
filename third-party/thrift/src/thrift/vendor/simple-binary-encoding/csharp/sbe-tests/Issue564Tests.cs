using System;
using Baseline;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Org.SbeTool.Sbe.Dll;

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public class Issue564Tests
    {
        [TestMethod]
        public void ShouldFailToEncodeGroupsThatExceedTheMaximumRepresentableSize()
        {
            // Arrange
            var expectedSize = 1024 + (8 + 8 + 8 + 20) * ushort.MaxValue;
            var buffer = new DirectBuffer(new byte[expectedSize]);
            var car = new Car();
            car.WrapForEncode(buffer, 0);
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
            
            // Act + Assert
            Assert.ThrowsException<ArgumentOutOfRangeException>(() =>
            {
                int countUnrepresentableByGroupSizeEncoding = 1 + ushort.MaxValue;
                var fuelFigures = car.FuelFiguresCount(countUnrepresentableByGroupSizeEncoding);
                for (int i = 0; i < countUnrepresentableByGroupSizeEncoding; i++)
                {
                    fuelFigures.Next();
                    fuelFigures.Speed = 30;
                    fuelFigures.Mpg = 35.9f;
                    var description = System.Text.Encoding.ASCII.GetBytes($"i={i}");
                    fuelFigures.SetUsageDescription(description, 0, description.Length);
                }
            });
        }
    }
}