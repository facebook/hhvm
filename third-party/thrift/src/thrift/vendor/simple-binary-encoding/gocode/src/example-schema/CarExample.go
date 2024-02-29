// golang example code for SBE's example-schema.xml

package main

import (
	"baseline" // Car
	"bytes"
	"extension" // Car extended with cupholder
	"fmt"
	"io"
	"net"
	"os"
	"reflect"
	"time"
)

// String Preallocations which matches what the Java and C++ benchmarks do
var vehicleCode [6]byte = [6]byte{'a', 'b', 'c', 'd', 'e', 'f'}
var manufacturerCode [3]byte = [3]byte{'1', '2', '3'}
var manufacturer []uint8 = []uint8("Honda")
var model []uint8 = []uint8("Civic VTi")
var activationCode []uint8 = []uint8("abcdef")

// Both Java and C++ benchmarks ignore ignore CarFuelFigures.UsageDescription
var urban []uint8 = []uint8("Urban Cycle")
var combined []uint8 = []uint8("Combined Cycle")
var highway []uint8 = []uint8("Highway Cycle")

func main() {

	fmt.Println("Example encode and decode")
	ExampleEncodeDecode()

	fmt.Println("Example of Car->Extension")
	ExampleCarToExtension()

	fmt.Println("Example Extension->Car")
	ExampleExtensionToCar()

	fmt.Println("Example decode using bytes.buffer")
	ExampleDecodeBuffer()

	fmt.Println("Example decode using io.Pipe")
	ExampleDecodePipe()

	fmt.Println("Example decode using socket")
	ExampleDecodeSocket()

	return
}

func ExampleEncodeDecode() bool {
	in := makeCar()
	min := baseline.NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	if err := in.Encode(min, buf, true); err != nil {
		fmt.Println("Encoding Error", err)
		os.Exit(1)
	}

	var out baseline.Car = *new(baseline.Car)
	if err := out.Decode(min, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		fmt.Println("Decoding Error", err)
		os.Exit(1)
	}

	if in.SerialNumber != out.SerialNumber {
		fmt.Println("in.SerialNumber != out.SerialNumber:\n", in.SerialNumber, out.SerialNumber)
		os.Exit(1)
	}
	if in.ModelYear != out.ModelYear {
		fmt.Println("in.ModelYear != out.ModelYear:\n", in.ModelYear, out.ModelYear)
		os.Exit(1)
	}
	if in.Available != out.Available {
		fmt.Println("in.Available != out.Available:\n", in.Available, out.Available)
		os.Exit(1)
	}
	if in.Code != out.Code {
		fmt.Println("in.Code != out.Code:\n", in.Code, out.Code)
		os.Exit(1)
	}
	if in.SomeNumbers != out.SomeNumbers {
		fmt.Println("in.SomeNumbers != out.SomeNumbers:\n", in.SomeNumbers, out.SomeNumbers)
		os.Exit(1)
	}
	if in.VehicleCode != out.VehicleCode {
		fmt.Println("in.VehicleCode != out.VehicleCode:\n", in.VehicleCode, out.VehicleCode)
		os.Exit(1)
	}
	if in.Extras != out.Extras {
		fmt.Println("in.Extras != out.Extras:\n", in.Extras, out.Extras)
		os.Exit(1)
	}

	// DiscountedModel is constant
	if baseline.Model.C != out.DiscountedModel {
		fmt.Println("in.DiscountedModel != out.DiscountedModel:\n", in.DiscountedModel, out.DiscountedModel)
		os.Exit(1)
	}

	// Engine has two constant values which should come back filled in
	if in.Engine == out.Engine {
		fmt.Println("in.Engine == out.Engine (and they should be different):\n", in.Engine, "\n", out.Engine)
		os.Exit(1)
	}

	// Engine has constant elements so We should have used our the
	// EngineInit() function to fill those in when we created the
	// object, and then they will correctly compare
	baseline.EngineInit(&in.Engine)
	if in.Engine != out.Engine {
		fmt.Println("in.Engine != out.Engine:\n", in.Engine, "\n", out.Engine)
		os.Exit(1)
	}

	return true
}

func ExampleDecodeBuffer() bool {
	buf := bytes.NewBuffer(data)
	m := baseline.NewSbeGoMarshaller()

	var hdr baseline.SbeGoMessageHeader
	if err := hdr.Decode(m, buf); err != nil {
		fmt.Println("Failed to decode message header", err)
		os.Exit(1)
	}

	// fmt.Println("\tbuffer is length:", buf.Len())
	var c baseline.Car
	if err := c.Decode(m, buf, hdr.Version, hdr.BlockLength, true); err != nil {
		fmt.Println("FIXME: Failed to decode car", err)
		os.Exit(1)
	}

	return true
}

func ExampleDecodePipe() bool {
	var r, w = io.Pipe()
	m := baseline.NewSbeGoMarshaller()

	go func() {
		defer w.Close()

		// By way of test, stream the bytes into the pipe a
		// chunk at a time
		msg := data[0:]
		for len(msg) > 0 {
			min := MinInt(len(msg), 64)
			// fmt.Println("writing: ", msg[0:min])
			n, err := w.Write(msg[0:min])
			if err != nil {
				fmt.Println("write error is", err)
				os.Exit(1)
			}
			if n < 8 {
				fmt.Println("short write of", n, "bytes")
				os.Exit(1)
			}
			msg = msg[n:]
			time.Sleep(time.Second)
		}
	}()

	var hdr baseline.SbeGoMessageHeader
	hdr.Decode(m, r)

	var c baseline.Car
	if err := c.Decode(m, r, hdr.Version, hdr.BlockLength, true); err != nil {
		fmt.Println("Failed to decode car", err)
		os.Exit(1)
	}
	r.Close()

	return true
}

func ExampleDecodeSocket() bool {
	addr := "127.0.0.1:15678"
	writerDone := make(chan bool)
	readerDone := make(chan bool)

	// Reader
	go func() {
		m := baseline.NewSbeGoMarshaller()
		// fmt.Println("resolve")
		tcpAddr, err := net.ResolveTCPAddr("tcp", addr)
		if err != nil {
			fmt.Println("Resolve failed", err.Error())
			os.Exit(1)
		}

		// fmt.Println("create listener")
		listener, err := net.ListenTCP("tcp", tcpAddr)
		if err != nil {
			fmt.Println("Listen failed", err.Error())
			os.Exit(1)
		}
		defer listener.Close()

		// fmt.Println("listen")
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept failed", err.Error())
			os.Exit(1)
		}
		defer conn.Close()

		// fmt.Println("reading messageheader")
		var hdr baseline.SbeGoMessageHeader
		hdr.Decode(m, conn)

		// fmt.Println("reading car")
		var c baseline.Car
		if err := c.Decode(m, conn, hdr.Version, hdr.BlockLength, true); err != nil {
			fmt.Println("Failed to decode car", err)
			os.Exit(1)
		}

		// fmt.Printf("%+v\n", c)
		readerDone <- true
	}()

	// Let that get started
	time.Sleep(time.Second)

	// Writer
	go func() {
		//fmt.Println("dial")
		conn, err := net.Dial("tcp", addr)
		if err != nil {
			fmt.Println("Dial failed", err.Error())
			os.Exit(1)
		}
		defer conn.Close()

		// By way of test, stream the bytes into the pipe a
		// chunk at a time
		msg := data[0:]
		for len(msg) > 0 {
			min := MinInt(len(msg), 64)
			// fmt.Println("writing: ", msg[0:min])
			n, err := conn.Write(msg[0:min])
			if err != nil {
				fmt.Println("write error is", err)
				os.Exit(1)
			}
			if n < 8 {
				fmt.Println("short write of", n, "bytes")
				os.Exit(1)
			}
			// fmt.Println("wrote", n, "bytes")
			msg = msg[n:]
			time.Sleep(time.Second)
		}
		<-readerDone
		writerDone <- true
	}()

	<-writerDone

	return true
}

func ExampleCarToExtension() bool {
	in := makeCar()
	min := baseline.NewSbeGoMarshaller()
	mout := extension.NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	if err := in.Encode(min, buf, true); err != nil {
		fmt.Println("Encoding Error", err)
		os.Exit(1)
	}

	var out extension.Car = *new(extension.Car)
	if err := out.Decode(mout, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		fmt.Println("Decoding Error", err)
		os.Exit(1)
	}

	if in.SerialNumber != out.SerialNumber {
		fmt.Println("in.SerialNumber != out.SerialNumber:\n", in.SerialNumber, out.SerialNumber)
		os.Exit(1)
	}
	if in.ModelYear != out.ModelYear {
		fmt.Println("in.ModelYear != out.ModelYear:\n", in.ModelYear, out.ModelYear)
		os.Exit(1)
	}

	// Note casts so we can compare
	if in.Available != baseline.BooleanTypeEnum(out.Available) {
		fmt.Println("in.Available != out.Available:\n", in.Available, out.Available)
		os.Exit(1)
	}
	if in.Code != baseline.ModelEnum(out.Code) {
		fmt.Println("in.Code != out.Code:\n", in.Code, out.Code)
		os.Exit(1)
	}
	if in.SomeNumbers != out.SomeNumbers {
		fmt.Println("in.SomeNumbers != out.SomeNumbers:\n", in.SomeNumbers, out.SomeNumbers)
		os.Exit(1)
	}
	if in.VehicleCode != out.VehicleCode {
		fmt.Println("in.VehicleCode != out.VehicleCode:\n", in.VehicleCode, out.VehicleCode)
		os.Exit(1)
	}
	if in.Extras != baseline.OptionalExtras(out.Extras) {
		fmt.Println("in.Extras != out.Extras:\n", in.Extras, out.Extras)
		os.Exit(1)
	}

	// DiscountedModel is constant
	if baseline.Model.C != baseline.ModelEnum(out.DiscountedModel) {
		fmt.Println("in.DiscountedModel != out.DiscountedModel:\n", in.DiscountedModel, out.DiscountedModel)
		os.Exit(1)
	}

	// Engine has two constant values which should come back filled in
	if in.Engine.MaxRpm == out.Engine.MaxRpm {
		fmt.Println("in.Engine.MaxRpm == out.Engine/MaxRpm (and they should be different):\n", in.Engine.MaxRpm, out.Engine.MaxRpm)
		os.Exit(1)
	}

	// Engine has constant elements so We should have used our the
	// EngineInit() function to fill those in when we created the
	// object, and then they will correctly compare
	baseline.EngineInit(&in.Engine)
	if in.Engine.MaxRpm != out.Engine.MaxRpm {
		fmt.Println("in.Engine.MaxRpm != out.Engine.MaxRpm:\n", in.Engine.MaxRpm, out.Engine.MaxRpm)
		os.Exit(1)
	}

	if !reflect.DeepEqual(in.ActivationCode, out.ActivationCode) {
		fmt.Println("in.ActivationCode != out.ActivationCode:\n", in.ActivationCode, out.ActivationCode)
		os.Exit(1)
	}

	// Cupholder is not in example-schema and was introduced in
	// extension-schema so it should be NullValue
	if out.CupHolderCount != out.CupHolderCountNullValue() {
		fmt.Println("out.cupholderCount not successfully nulled:\n", out.CupHolderCount)
		os.Exit(1)
	}

	return true
}

func ExampleExtensionToCar() bool {
	in := makeExtension()
	min := extension.NewSbeGoMarshaller()
	mout := baseline.NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	if err := in.Encode(min, buf, true); err != nil {
		fmt.Println("Encoding Error", err)
		os.Exit(1)
	}

	var out baseline.Car = *new(baseline.Car)
	if err := out.Decode(mout, buf, in.SbeSchemaVersion(), in.SbeBlockLength(), true); err != nil {
		fmt.Println("Decoding Error", err)
		os.Exit(1)
	}

	if in.SerialNumber != out.SerialNumber {
		fmt.Println("in.SerialNumber != out.SerialNumber:\n", in.SerialNumber, out.SerialNumber)
		os.Exit(1)
	}
	if in.ModelYear != out.ModelYear {
		fmt.Println("in.ModelYear != out.ModelYear:\n", in.ModelYear, out.ModelYear)
		os.Exit(1)
	}

	// Note casts so we can compare
	if in.Available != extension.BooleanTypeEnum(out.Available) {
		fmt.Println("in.Available != out.Available:\n", in.Available, out.Available)
		os.Exit(1)
	}
	if in.Code != extension.ModelEnum(out.Code) {
		fmt.Println("in.Code != out.Code:\n", in.Code, out.Code)
		os.Exit(1)
	}
	if in.SomeNumbers != out.SomeNumbers {
		fmt.Println("in.SomeNumbers != out.SomeNumbers:\n", in.SomeNumbers, out.SomeNumbers)
		os.Exit(1)
	}
	if in.VehicleCode != out.VehicleCode {
		fmt.Println("in.VehicleCode != out.VehicleCode:\n", in.VehicleCode, out.VehicleCode)
		os.Exit(1)
	}
	if in.Extras != extension.OptionalExtras(out.Extras) {
		fmt.Println("in.Extras != out.Extras:\n", in.Extras, out.Extras)
		os.Exit(1)
	}

	// DiscountedModel is constant
	if extension.Model.C != extension.ModelEnum(out.DiscountedModel) {
		fmt.Println("in.DiscountedModel != out.DiscountedModel:\n", in.DiscountedModel, out.DiscountedModel)
		os.Exit(1)
	}

	// Engine has two constant values which in this case should match
	if in.Engine.MaxRpm != out.Engine.MaxRpm {
		fmt.Println("in.Engine.MaxRpm != out.Engine/MaxRpm:\n", in.Engine.MaxRpm, out.Engine.MaxRpm)
		os.Exit(1)
	}

	// Engine has constant elements so We should have used our the
	// EngineInit() function to fill those in when we created the
	// object, and then they will correctly compare
	extension.EngineInit(&in.Engine)
	if in.Engine.MaxRpm != out.Engine.MaxRpm {
		fmt.Println("in.Engine.MaxRpm != out.Engine.MaxRpm:\n", in.Engine.MaxRpm, out.Engine.MaxRpm)
		os.Exit(1)
	}

	if !reflect.DeepEqual(in.ActivationCode, out.ActivationCode) {
		fmt.Println("in.ActivationCode != out.ActivationCode:\n", in.ActivationCode, out.ActivationCode)
		os.Exit(1)
	}

	return true
}

// Helper to make a Car object as per the Java example
func makeCar() baseline.Car {
	return baseline.Car{
		1234,
		2013,
		baseline.BooleanType.T,
		baseline.Model.A,
		[4]uint32{0, 1, 2, 3},
		vehicleCode,
		[8]bool{false, true, true, false, false, false, false, false},
		baseline.Model.A,
		baseline.Engine{2000,
			4,
			9001, // will come back as constant value 9000
			manufacturerCode,
			[6]byte{'P', 'e', 't', 'r', 'o', 'l'},
			35,
			baseline.BooleanType.T,
			baseline.EngineBooster{baseline.BoostType.NITROUS, 200}},
		[]baseline.CarFuelFigures{
			baseline.CarFuelFigures{30, 35.9, urban},
			baseline.CarFuelFigures{55, 49.0, combined},
			baseline.CarFuelFigures{75, 40.0, highway}},
		[]baseline.CarPerformanceFigures{
			baseline.CarPerformanceFigures{95,
				[]baseline.CarPerformanceFiguresAcceleration{
					baseline.CarPerformanceFiguresAcceleration{30, 4.0},
					baseline.CarPerformanceFiguresAcceleration{60, 7.5},
					baseline.CarPerformanceFiguresAcceleration{100, 12.2}}},
			baseline.CarPerformanceFigures{99,
				[]baseline.CarPerformanceFiguresAcceleration{
					baseline.CarPerformanceFiguresAcceleration{30, 3.8},
					baseline.CarPerformanceFiguresAcceleration{60, 7.1},
					baseline.CarPerformanceFiguresAcceleration{100, 11.8}}}},
		manufacturer,
		model,
		activationCode}
}

// Helper to make an Extension (car with cupholder) object
func makeExtension() extension.Car {
	return extension.Car{
		1234,
		2013,
		extension.BooleanType.T,
		extension.Model.A,
		[4]uint32{0, 1, 2, 3},
		vehicleCode,
		[8]bool{false, true, true, false, false, false, false, false},
		extension.Model.A,
		extension.Engine{2000,
			4,
			9000,
			manufacturerCode,
			[6]byte{'P', 'e', 't', 'r', 'o', 'l'},
			35,
			extension.BooleanType.T,
			extension.EngineBooster{extension.BoostType.NITROUS, 200}},
		[2]int64{119, 120}, // uuid sinceVersion = 1
		121,                // cupHoldercount, sinceVersion = 1
		[]extension.CarFuelFigures{
			extension.CarFuelFigures{30, 35.9, urban},
			extension.CarFuelFigures{55, 49.0, combined},
			extension.CarFuelFigures{75, 40.0, highway}},
		[]extension.CarPerformanceFigures{
			extension.CarPerformanceFigures{95,
				[]extension.CarPerformanceFiguresAcceleration{
					extension.CarPerformanceFiguresAcceleration{30, 4.0},
					extension.CarPerformanceFiguresAcceleration{60, 7.5},
					extension.CarPerformanceFiguresAcceleration{100, 12.2}}},
			extension.CarPerformanceFigures{99,
				[]extension.CarPerformanceFiguresAcceleration{
					extension.CarPerformanceFiguresAcceleration{30, 3.8},
					extension.CarPerformanceFiguresAcceleration{60, 7.1},
					extension.CarPerformanceFiguresAcceleration{100, 11.8}}}},
		manufacturer,
		model,
		activationCode}
}

// MaxInt returns the larger of two ints.
func MaxInt(a, b int) int {
	if a > b {
		return a
	}
	return b
}

// MinInt returns the larger of two ints.
func MinInt(a, b int) int {
	if a < b {
		return a
	}
	return b
}

// The byte array can be made at ~rust/car_example/car_example_data.sbe by running gradlew generateCarExampleDataFile
// This can then be decoded using od -tu1
var data []byte = []byte{45, 0, 1, 0, 1, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 221, 7, 1, 65, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 97, 98, 99, 100, 101, 102, 6, 208, 7, 4, 49, 50, 51, 35, 1, 78, 200, 6, 0, 3, 0, 30, 0, 154, 153, 15, 66, 11, 0, 0, 0, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 55, 0, 0, 0, 68, 66, 14, 0, 0, 0, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 75, 0, 0, 0, 32, 66, 13, 0, 0, 0, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 1, 0, 2, 0, 95, 6, 0, 3, 0, 30, 0, 0, 0, 128, 64, 60, 0, 0, 0, 240, 64, 100, 0, 51, 51, 67, 65, 99, 6, 0, 3, 0, 30, 0, 51, 51, 115, 64, 60, 0, 51, 51, 227, 64, 100, 0, 205, 204, 60, 65, 5, 0, 0, 0, 72, 111, 110, 100, 97, 9, 0, 0, 0, 67, 105, 118, 105, 99, 32, 86, 84, 105, 6, 0, 0, 0, 97, 98, 99, 100, 101, 102}
