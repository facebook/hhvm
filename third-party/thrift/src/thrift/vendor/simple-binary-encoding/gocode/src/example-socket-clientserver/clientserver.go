package main

import (
	"baseline"
	"bufio"
	"flag"
	"fmt"
	"net"
	"os"
)

var data []byte = []byte{47, 0, 1, 0, 1, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 221, 7, 1, 65, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 97, 98, 99, 100, 101, 102, 6, 208, 7, 4, 49, 50, 51, 78, 200, 6, 0, 3, 0, 30, 0, 154, 153, 15, 66, 11, 0, 0, 0, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 55, 0, 0, 0, 68, 66, 14, 0, 0, 0, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 75, 0, 0, 0, 32, 66, 13, 0, 0, 0, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 1, 0, 2, 0, 95, 6, 0, 3, 0, 30, 0, 0, 0, 128, 64, 60, 0, 0, 0, 240, 64, 100, 0, 51, 51, 67, 65, 99, 6, 0, 3, 0, 30, 0, 51, 51, 115, 64, 60, 0, 51, 51, 227, 64, 100, 0, 205, 204, 60, 65, 5, 0, 0, 0, 72, 111, 110, 100, 97, 9, 0, 0, 0, 67, 105, 118, 105, 99, 32, 86, 84, 105, 6, 0, 0, 0, 97, 98, 99, 100, 101, 102}

var addr string = "localhost:6666"

var vehicleCode [6]byte = [6]byte{'a', 'b', 'c', 'd', 'e', 'f'}
var manufacturerCode [3]byte = [3]byte{'1', '2', '3'}
var manufacturer []uint8 = []uint8("Honda")
var model []uint8 = []uint8("Civic VTi")
var activationCode []uint8 = []uint8("abcdef")
var urban []uint8 = []uint8("Urban Cycle")
var combined []uint8 = []uint8("Combined Cycle")
var highway []uint8 = []uint8("Highway Cycle")
var fuel [6]byte = [6]byte{'P', 'e', 't', 'r', 'o', 'l'}

var car = baseline.Car{
	1234,
	2013,
	baseline.BooleanType.T,
	baseline.Model.A,
	[5]uint32{0, 1, 2, 3, 4},
	vehicleCode,
	[8]bool{false, true, true, false, false, false, false, false},
	baseline.Model.C,
	baseline.Engine{2000,
		4,
		9000,
		manufacturerCode,
		fuel,
		baseline.EngineBooster{baseline.BoostType.NITROUS, 200}},
	[]baseline.CarFuelFigures{
		baseline.CarFuelFigures{30, 35.9, urban},
		baseline.CarFuelFigures{55, 49.0, combined},
		baseline.CarFuelFigures{75, 40.0, highway}},
	[]baseline.CarPerformanceFigures{
		baseline.CarPerformanceFigures{95,
			[]baseline.CarPerformanceFiguresAcceleration{
				baseline.CarPerformanceFiguresAcceleration{30, 4},
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

func mainReader(doDecode bool, iterations int) {
	raw, err := net.Dial("tcp", addr)
	conn := bufio.NewReader(raw)
	if err != nil {
		fmt.Println("Dial failed", err.Error())
		os.Exit(1)
	}

	if !doDecode {
		b := make([]byte, len(data))
		for i := 0; i < iterations; i++ {
			_, err = conn.Read(b)
			if err != nil {
				fmt.Println("Read failed:", err.Error())
				os.Exit(1)
			}
		}
	} else {
		var h baseline.MessageHeader
		m := baseline.NewSbeGoMarshaller()
		for i := 0; i < iterations; i++ {
			if err := h.Decode(m, conn); err != nil {
				fmt.Println("Read failed:", err.Error())
				break
			}
			// fmt.Println(h)
			if err := car.Decode(m, conn, h.Version, h.BlockLength, false); err != nil {
				fmt.Println("Read failed:", err.Error(), i)
				break
			}
			// fmt.Println(car)
		}
	}
	raw.Close()
}

func mainWriter(doEncode bool, iterations int) {
	tcpAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		fmt.Println("ResolveTCPAddr failed:", err.Error())
		os.Exit(1)
	}

	listener, err := net.ListenTCP("tcp", tcpAddr)
	if err != nil {
		fmt.Println("Listen failed", err.Error())
		os.Exit(1)
	}

	for {
		raw, err := listener.Accept()
		if err != nil {
			fmt.Println("Dial failed:", err.Error())
			os.Exit(1)
		}
		conn := bufio.NewWriter(raw)

		if !doEncode {
			for i := 0; i < iterations; i++ {
				n, err := conn.Write(data)
				if err != nil || n != len(data) {
					fmt.Println("Write to client failed:", err.Error())
					break
				}
			}
		} else {
			var h = baseline.MessageHeader{car.SbeBlockLength(), car.SbeTemplateId(), car.SbeSchemaId(), car.SbeSchemaVersion()}
			m := baseline.NewSbeGoMarshaller()
			for i := 0; i < iterations; i++ {
				if err := h.Encode(m, conn); err != nil {
					fmt.Println("Encode failed", err.Error())
				}
				if err := car.Encode(m, conn, false); err != nil {
					fmt.Println("Encode failed", err.Error())
				}
			}

		}
		conn.Flush()
		raw.Close()
	}
	listener.Close()
}

func main() {
	var reader, writer, encode, decode bool
	var iterations int
	flag.BoolVar(&reader, "reader", false, "Become a reader")
	flag.BoolVar(&writer, "writer", false, "Become a writer")
	flag.BoolVar(&encode, "encode", false, "As a writer do encoding")
	flag.BoolVar(&decode, "decode", false, "As a reader do decoding")
	flag.IntVar(&iterations, "iterations", 100000, "Number of iterations")
	flag.Parse()

	// Must be one but not both
	if reader == writer {
		fmt.Println("Error: Must be one of reader or writer")
		flag.PrintDefaults()
		os.Exit(1)
	}

	if reader {
		mainReader(decode, iterations)
	} else {
		mainWriter(encode, iterations)
	}
}
