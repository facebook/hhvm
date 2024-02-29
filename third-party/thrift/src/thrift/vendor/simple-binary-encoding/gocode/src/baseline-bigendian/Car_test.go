package baseline_bigendian

import (
	"bytes"
	_ "fmt"
	"testing"
)

func TestEncodeDecodeCar(t *testing.T) {

	m := NewSbeGoMarshaller()
	var vehicleCode [6]byte
	copy(vehicleCode[:], "abcdef")

	var manufacturerCode [3]byte
	copy(manufacturerCode[:], "123")

	var optionalExtras [8]bool
	optionalExtras[OptionalExtrasChoice.CruiseControl] = true
	optionalExtras[OptionalExtrasChoice.SportsPack] = true

	var engine Engine
	engine = Engine{2000, 4, 0, manufacturerCode, [6]byte{}, 42, BooleanType.T, EngineBooster{BoostType.NITROUS, 200}}

	manufacturer := []uint8("Honda")
	model := []uint8("Civic VTi")
	activationCode := []uint8("deadbeef")

	var fuel []CarFuelFigures
	fuel = append(fuel, CarFuelFigures{30, 35.9, []uint8("Urban Cycle")})
	fuel = append(fuel, CarFuelFigures{55, 49.0, []uint8("Combined Cycle")})
	fuel = append(fuel, CarFuelFigures{75, 40.0, []uint8("Highway Cycle")})

	var acc1 []CarPerformanceFiguresAcceleration
	acc1 = append(acc1, CarPerformanceFiguresAcceleration{30, 3.8})
	acc1 = append(acc1, CarPerformanceFiguresAcceleration{60, 7.5})
	acc1 = append(acc1, CarPerformanceFiguresAcceleration{100, 12.2})

	var acc2 []CarPerformanceFiguresAcceleration
	acc2 = append(acc2, CarPerformanceFiguresAcceleration{30, 3.8})
	acc2 = append(acc2, CarPerformanceFiguresAcceleration{60, 7.5})
	acc2 = append(acc2, CarPerformanceFiguresAcceleration{100, 12.2})

	var pf []CarPerformanceFigures
	pf = append(pf, CarPerformanceFigures{95, acc1})
	pf = append(pf, CarPerformanceFigures{99, acc2})

	in := Car{1234, 2013, BooleanType.T, Model.A, [5]uint32{0, 1, 2, 3, 4}, vehicleCode, optionalExtras, Model.A, engine, fuel, pf, manufacturer, model, activationCode}

	var buf = new(bytes.Buffer)
	if err := in.Encode(m, buf, true); err != nil {
		t.Log("Encoding Error", err)
		t.Fail()
	}

	var out Car = *new(Car)
	if err := out.Decode(m, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		t.Log("Decoding Error", err)
		t.Fail()
	}

	if in.SerialNumber != out.SerialNumber {
		t.Log("in.SerialNumber != out.SerialNumber:\n", in.SerialNumber, out.SerialNumber)
		t.Fail()
	}
	if in.ModelYear != out.ModelYear {
		t.Log("in.ModelYear != out.ModelYear:\n", in.ModelYear, out.ModelYear)
		t.Fail()
	}
	if in.Available != out.Available {
		t.Log("in.Available != out.Available:\n", in.Available, out.Available)
		t.Fail()
	}
	if in.Code != out.Code {
		t.Log("in.Code != out.Code:\n", in.Code, out.Code)
		t.Fail()
	}
	if in.SomeNumbers != out.SomeNumbers {
		t.Log("in.SomeNumbers != out.SomeNumbers:\n", in.SomeNumbers, out.SomeNumbers)
		t.Fail()
	}
	if in.VehicleCode != out.VehicleCode {
		t.Log("in.VehicleCode != out.VehicleCode:\n", in.VehicleCode, out.VehicleCode)
		t.Fail()
	}
	if in.Extras != out.Extras {
		t.Log("in.Extras != out.Extras:\n", in.Extras, out.Extras)
		t.Fail()
	}

	// DiscountedModel is constant
	if Model.C != out.DiscountedModel {
		t.Log("in.DiscountedModel != out.DiscountedModel:\n", in.DiscountedModel, out.DiscountedModel)
		t.Fail()
	}

	// Engine has two constant values which should come back filled in
	if in.Engine == out.Engine {
		t.Log("in.Engine == out.Engine (and they should be different):\n", in.Engine, out.Engine)
		t.Fail()
	}

	copy(in.Engine.Fuel[:], "Petrol")
	in.Engine.MaxRpm = 9000
	if in.Engine != out.Engine {
		t.Log("in.Engine != out.Engine:\n", in.Engine, out.Engine)
		t.Fail()
	}

	return

}

func TestDecodeJavaBuffer(t *testing.T) {

	// The byte array is from the java example for interop test
	// made by editing example-schgema to be bigendian and running
	// with the example with -Dsbe.encoding.filename
	// and then decoded using od -tu1
	data := []byte{0, 49, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 4, 210, 7, 221, 1, 65, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 97, 98, 99, 100, 101, 102, 6, 7, 208, 4, 49, 50, 51, 35, 1, 78, 200, 0, 6, 0, 3, 0, 30, 66, 15, 153, 154, 0, 0, 0, 11, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 0, 55, 66, 68, 0, 0, 0, 0, 0, 14, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 0, 75, 66, 32, 0, 0, 0, 0, 0, 13, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 0, 1, 0, 2, 95, 0, 6, 0, 3, 0, 30, 64, 128, 0, 0, 0, 60, 64, 240, 0, 0, 0, 100, 65, 67, 51, 51, 99, 0, 6, 0, 3, 0, 30, 64, 115, 51, 51, 0, 60, 64, 227, 51, 51, 0, 100, 65, 60, 204, 205, 0, 0, 0, 5, 72, 111, 110, 100, 97, 0, 0, 0, 9, 67, 105, 118, 105, 99, 32, 86, 84, 105, 0, 0, 0, 6, 97, 98, 99, 100, 101, 102}

	buf := bytes.NewBuffer(data)
	m := NewSbeGoMarshaller()

	var hdr SbeGoMessageHeader
	if err := hdr.Decode(m, buf); err != nil {
		t.Log("Failed to decode message header", err)
		t.Fail()
	}

	// fmt.Println("BlockLength = ", m.BlockLength)
	// fmt.Println("TemplateId = ", m.TemplateId)
	// fmt.Println("SchemaId = ", m.SchemaId)
	// fmt.Println("Version = ", m.Version)
	// fmt.Println("bytes: ", buf.Len())
	var c Car
	if err := c.Decode(m, buf, hdr.Version, hdr.BlockLength, true); err != nil {
		t.Log("Failed to decode car", err)
		t.Fail()
	}
	// fmt.Println(c)
	return
}
