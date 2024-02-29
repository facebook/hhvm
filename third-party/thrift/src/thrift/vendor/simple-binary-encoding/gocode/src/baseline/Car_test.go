package baseline

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

	in := Car{1234, 2013, BooleanType.T, Model.A, [4]uint32{0, 1, 2, 3}, vehicleCode, optionalExtras, Model.A, engine, fuel, pf, manufacturer, model, activationCode}

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
	// See ~gocode/src/example-schema/CarExample.go for how this is generated
	data := []byte{45, 0, 1, 0, 1, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 221, 7, 1, 65, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 97, 98, 99, 100, 101, 102, 6, 208, 7, 4, 49, 50, 51, 35, 1, 78, 200, 6, 0, 3, 0, 30, 0, 154, 153, 15, 66, 11, 0, 0, 0, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 55, 0, 0, 0, 68, 66, 14, 0, 0, 0, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 75, 0, 0, 0, 32, 66, 13, 0, 0, 0, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 1, 0, 2, 0, 95, 6, 0, 3, 0, 30, 0, 0, 0, 128, 64, 60, 0, 0, 0, 240, 64, 100, 0, 51, 51, 67, 65, 99, 6, 0, 3, 0, 30, 0, 51, 51, 115, 64, 60, 0, 51, 51, 227, 64, 100, 0, 205, 204, 60, 65, 5, 0, 0, 0, 72, 111, 110, 100, 97, 9, 0, 0, 0, 67, 105, 118, 105, 99, 32, 86, 84, 105, 6, 0, 0, 0, 97, 98, 99, 100, 101, 102}

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
