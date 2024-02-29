using System.Text;
using Baseline;
using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Diagnosers;
using Org.SbeTool.Sbe.Dll;

// Whilst running decoding timing tests there are values we don't use
#pragma warning disable 219

namespace Org.SbeTool.Sbe.Benchmarks.Bench.Benchmarks
{
    [MemoryDiagnoser]
    public class CarBenchmark_with_strings_original
    {
        private static readonly Encoding VehicleCodeEncoding = Encoding.GetEncoding(Car.VehicleCodeCharacterEncoding);
        private static readonly Encoding ManufacturerCodeEncoding = Encoding.GetEncoding(Engine.ManufacturerCodeCharacterEncoding);
        private static readonly Encoding ManufacturerEncoding = Encoding.GetEncoding(Car.ManufacturerCharacterEncoding);
        private static readonly Encoding ModelEncoding = Encoding.GetEncoding(Car.ModelCharacterEncoding);
        private static readonly Encoding ActivationCodeEncoding = Encoding.GetEncoding(Car.ActivationCodeCharacterEncoding);
        private static readonly Encoding UsageDescriptionEncoding = Encoding.GetEncoding(Car.FuelFiguresGroup.UsageDescriptionCharacterEncoding);

        private readonly byte[] _eBuffer = new byte[1024];
        private readonly byte[] _dBuffer = new byte[1024];
        private DirectBuffer _encodeBuffer;
        private DirectBuffer _decodeBuffer;
        private readonly Car _car = new Car();
        private readonly MessageHeader _messageHeader = new MessageHeader();

        [GlobalSetup]
        public void Setup()
        {
            _encodeBuffer = new DirectBuffer(_eBuffer);
            _decodeBuffer = new DirectBuffer(_dBuffer);
            Encode(_car, _decodeBuffer, 0);
        }

        [Benchmark]
        public int Encode()
        {
            return Encode(_car, _encodeBuffer, 0);
        }
        
        [Benchmark]
        public int Decode()
        {
            return Decode(_car, _decodeBuffer, 0);
        }

        public int Encode(Car car, DirectBuffer directBuffer, int bufferOffset)
        {
            _car.WrapForEncodeAndApplyHeader(directBuffer, bufferOffset, _messageHeader);

            const int srcOffset = 0;

            car.SerialNumber = 1234;
            car.ModelYear = 2013;
            car.Available = BooleanType.T;
            car.Code = Baseline.Model.A;
            car.SetVehicleCode(VehicleCodeEncoding.GetBytes("CODE12"), 0);

            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                car.SetSomeNumbers(i, (uint)i);
            }

            car.Extras = OptionalExtras.CruiseControl | OptionalExtras.SportsPack;
            car.Engine.Capacity = 2000;
            car.Engine.NumCylinders = 4;
            car.Engine.SetManufacturerCode(ManufacturerCodeEncoding.GetBytes("123"), 0);
            car.Engine.Efficiency = 35;
            car.Engine.BoosterEnabled = BooleanType.T;
            car.Engine.Booster.BoostType = BoostType.NITROUS;
            car.Engine.Booster.HorsePower = 200;

            var fuelFigures = car.FuelFiguresCount(3);
            fuelFigures.Next();
            fuelFigures.Speed = 30;
            fuelFigures.Mpg = 35.9f;
            fuelFigures.SetUsageDescription(UsageDescriptionEncoding.GetBytes("Urban Cycle"));

            fuelFigures.Next();
            fuelFigures.Speed = 55;
            fuelFigures.Mpg = 49.0f;
            fuelFigures.SetUsageDescription(UsageDescriptionEncoding.GetBytes("Combined Cycle"));

            fuelFigures.Next();
            fuelFigures.Speed = 75;
            fuelFigures.Mpg = 40.0f;
            fuelFigures.SetUsageDescription(UsageDescriptionEncoding.GetBytes("Highway Cycle"));

            Car.PerformanceFiguresGroup perfFigures = car.PerformanceFiguresCount(2);
            perfFigures.Next();
            perfFigures.OctaneRating = 95;

            Car.PerformanceFiguresGroup.AccelerationGroup acceleration = perfFigures.AccelerationCount(3).Next();
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

            byte[] Manufacturer = ManufacturerEncoding.GetBytes("Honda");
            byte[] Model = ModelEncoding.GetBytes("Civic VTi");
            byte[] ActivationCode = ActivationCodeEncoding.GetBytes("code");

            car.SetManufacturer(Manufacturer, srcOffset, Manufacturer.Length);
            car.SetModel(Model, srcOffset, Model.Length);
            car.SetActivationCode(ActivationCode, srcOffset, ActivationCode.Length);

            return car.Size;
        }

        private readonly byte[] _buffer = new byte[128];

        public int Decode(Car car, DirectBuffer directBuffer, int bufferOffset)
        {
            _messageHeader.Wrap(directBuffer, bufferOffset, 0);

            car.WrapForDecode(directBuffer, bufferOffset + MessageHeader.Size, _messageHeader.BlockLength, _messageHeader.Version);

            var templateId = Car.TemplateId;
            var schemaVersion = Car.SchemaVersion;
            var serialNumber = car.SerialNumber;
            var modelYear = car.ModelYear;
            var available = car.Available;
            var code = car.Code;

            for (int i = 0, size = Car.SomeNumbersLength; i < size; i++)
            {
                var number = car.GetSomeNumbers(i);
            }

            var length = car.GetVehicleCode(_buffer, 0);
            var vehicleCode = VehicleCodeEncoding.GetString(_buffer, 0, length);

            OptionalExtras extras = car.Extras;
            var cruiseControl = (extras & OptionalExtras.CruiseControl) == OptionalExtras.CruiseControl;
            var sportsPack = (extras & OptionalExtras.SportsPack) == OptionalExtras.SportsPack;
            var sunRoof = (extras & OptionalExtras.SunRoof) == OptionalExtras.SunRoof;

            Engine engine = car.Engine;
            var capacity = engine.Capacity;
            var numCylinders = engine.NumCylinders;
            var maxRpm = engine.MaxRpm;
            for (int i = 0, size = Engine.ManufacturerCodeLength; i < size; i++)
            {
                engine.GetManufacturerCode(i);
            }

            length = engine.GetFuel(_buffer, 0, _buffer.Length);

            var efficiency = engine.Efficiency;
            var boosterEnabled = engine.BoosterEnabled;
            var boostType = engine.Booster.BoostType;
            var horsePower = engine.Booster.HorsePower;

            var fuelFiguresGroup = car.FuelFigures;
            while (fuelFiguresGroup.HasNext)
            {
                var fuelFigures = fuelFiguresGroup.Next();
                var speed = fuelFigures.Speed;
                var mpg = fuelFigures.Mpg;
                length = fuelFigures.GetUsageDescription(_buffer, 0, _buffer.Length);
                var fuelUsage = UsageDescriptionEncoding.GetString(_buffer, 0, length);
            }

            var performanceFiguresGroup = car.PerformanceFigures;
            while (performanceFiguresGroup.HasNext)
            {
                performanceFiguresGroup.Next();
                var octaneRating = performanceFiguresGroup.OctaneRating;

                var accelerationGroup = performanceFiguresGroup.Acceleration;
                for (int i = 0; i < accelerationGroup.Count; i++)
                {
                    var acceleration = accelerationGroup.Next();
                    var mpg = acceleration.Mph;
                    var seconds = acceleration.Seconds;
                }
            }

            length = car.GetManufacturer(_buffer, 0, _buffer.Length);
            var usage = ManufacturerEncoding.GetString(_buffer, 0, length);

            length = car.GetModel(_buffer, 0, _buffer.Length);
            usage = ModelEncoding.GetString(_buffer, 0, length);

            length = car.GetActivationCode(_buffer, 0, _buffer.Length);
            usage = ActivationCodeEncoding.GetString(_buffer, 0, length);

            return car.Size;
        }
    }
}
