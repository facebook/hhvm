package main

import (
	"baseline"
	"bufio"
	"bytes"
	"io"
	"testing"
)

func TestNoop(t *testing.T) {
}

func BenchmarkInstantiateCar(b *testing.B) {
	var car baseline.Car
	for i := 0; i < b.N; i++ {
		car = makeCar()
	}

	// Compiler insists that car is used
	car.Available = baseline.BooleanType.T
}

// Narrower encode so we can focus in on some areas
func BenchmarkEncodeFuelFigures(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()

	var buf = new(bytes.Buffer)
	buf.Grow(32768) /// remove this cost from benchmark

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if err := car.FuelFigures[0].Encode(m, buf); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
		buf.Reset()
	}
	// car.FuelFigures[0].Encode(buf, binary.LittleEndian)
	// fmt.Printf("%v\n", buf.Bytes())
}

// Try without the err
func XBenchmarkEncodeFuelFiguresNoErr(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	buf.Grow(32768) /// remove this cost from benchmark

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		car.FuelFigures[0].Encode(m, buf)
		buf.Reset()
	}

}

// and without the reset
func XBenchmarkEncodeFuelFiguresNoRest(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	buf.Grow(32768) /// remove this cost from benchmark

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		car.FuelFigures[0].Encode(m, buf)
	}

}

func BenchmarkEncodeStrict(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if err := car.Encode(m, buf, true); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
		buf.Reset()
	}

}

func BenchmarkEncodeLax(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		if err := car.Encode(m, buf, false); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
		buf.Reset()
	}
}

func BenchmarkEncodeLaxReset(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	b.ResetTimer()

	for i := 0; i < b.N; i++ {
		car.Encode(m, buf, false)
		buf.Reset()
	}
}

func BenchmarkDecodeStrict(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	for i := 0; i < b.N; i++ {
		if err := car.Encode(m, buf, true); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if err := car.Decode(m, buf, car.SbeSchemaVersion(), car.SbeBlockLength(), true); err != nil {
			b.Log("Decoding Error", err)
			b.Fail()
		}
	}
}

func BenchmarkDecodeLax(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	for i := 0; i < b.N; i++ {
		if err := car.Encode(m, buf, false); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if err := car.Decode(m, buf, car.SbeSchemaVersion(), car.SbeBlockLength(), false); err != nil {
			b.Log("Decoding Error", err)
			b.Fail()
		}
	}
}
func xBenchmarkDecodeLaxBufio(b *testing.B) {
	car := makeCar()
	m := baseline.NewSbeGoMarshaller()
	var buf = new(bytes.Buffer)
	for i := 0; i < b.N; i++ {
		if err := car.Encode(m, buf, false); err != nil {
			b.Log("Encoding Error", err)
			b.Fail()
		}
	}

	buf2 := bufio.NewReader(buf)
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		if err := car.Decode(m, buf2, car.SbeSchemaVersion(), car.SbeBlockLength(), false); err != nil {
			b.Log("Decoding Error", err)
			b.Fail()
		}
	}
}

func BenchmarkPipe(b *testing.B) {
	var r, w = io.Pipe()
	var data []byte = []byte{49, 0, 1, 0, 1, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 221, 7, 1, 65, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 97, 98, 99, 100, 101, 102, 6, 208, 7, 4, 49, 50, 51, 35, 1, 78, 200, 6, 0, 3, 0, 30, 0, 154, 153, 15, 66, 11, 0, 0, 0, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 55, 0, 0, 0, 68, 66, 14, 0, 0, 0, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 75, 0, 0, 0, 32, 66, 13, 0, 0, 0, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 1, 0, 2, 0, 95, 6, 0, 3, 0, 30, 0, 0, 0, 128, 64, 60, 0, 0, 0, 240, 64, 100, 0, 51, 51, 67, 65, 99, 6, 0, 3, 0, 30, 0, 51, 51, 115, 64, 60, 0, 51, 51, 227, 64, 100, 0, 205, 204, 60, 65, 5, 0, 0, 0, 72, 111, 110, 100, 97, 9, 0, 0, 0, 67, 105, 118, 105, 99, 32, 86, 84, 105, 6, 0, 0, 0, 97, 98, 99, 100, 101, 102}
	writerReady := make(chan bool)

	go func() {
		defer w.Close()
		writerReady <- true
		// By way of test, stream the bytes into the pipe a
		// chunk at a time
		for looping := true; looping; {
			if _, err := w.Write(data); err != nil {
				looping = false
			}
		}
	}()

	var hdr baseline.SbeGoMessageHeader
	var c baseline.Car
	m := baseline.NewSbeGoMarshaller()
	<-writerReady
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		hdr.Decode(m, r)
		if err := c.Decode(m, r, hdr.Version, hdr.BlockLength, false); err != nil {
			b.Log("Failed to decode car", err)
			b.Fail()
		}
	}
	r.Close()
}

func BenchmarkPipeBufio(b *testing.B) {
	var r, w = io.Pipe()
	var data []byte = []byte{49, 0, 1, 0, 1, 0, 0, 0, 210, 4, 0, 0, 0, 0, 0, 0, 221, 7, 1, 65, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 0, 97, 98, 99, 100, 101, 102, 6, 208, 7, 4, 49, 50, 51, 35, 1, 78, 200, 6, 0, 3, 0, 30, 0, 154, 153, 15, 66, 11, 0, 0, 0, 85, 114, 98, 97, 110, 32, 67, 121, 99, 108, 101, 55, 0, 0, 0, 68, 66, 14, 0, 0, 0, 67, 111, 109, 98, 105, 110, 101, 100, 32, 67, 121, 99, 108, 101, 75, 0, 0, 0, 32, 66, 13, 0, 0, 0, 72, 105, 103, 104, 119, 97, 121, 32, 67, 121, 99, 108, 101, 1, 0, 2, 0, 95, 6, 0, 3, 0, 30, 0, 0, 0, 128, 64, 60, 0, 0, 0, 240, 64, 100, 0, 51, 51, 67, 65, 99, 6, 0, 3, 0, 30, 0, 51, 51, 115, 64, 60, 0, 51, 51, 227, 64, 100, 0, 205, 204, 60, 65, 5, 0, 0, 0, 72, 111, 110, 100, 97, 9, 0, 0, 0, 67, 105, 118, 105, 99, 32, 86, 84, 105, 6, 0, 0, 0, 97, 98, 99, 100, 101, 102}

	writerReady := make(chan bool)

	go func() {
		defer w.Close()
		writerReady <- true
		// By way of test, stream the bytes into the pipe a
		// chunk at a time
		for looping := true; looping; {
			if _, err := w.Write(data); err != nil {
				looping = false
			}
		}
	}()

	var hdr baseline.SbeGoMessageHeader
	var c baseline.Car
	m := baseline.NewSbeGoMarshaller()
	<-writerReady
	b.ResetTimer()
	buf := bufio.NewReader(r)

	for i := 0; i < b.N; i++ {
		hdr.Decode(m, r)
		if err := c.Decode(m, buf, hdr.Version, hdr.BlockLength, false); err != nil {
			b.Log("Failed to decode car", err)
			b.Fail()
		}
	}
	r.Close()
}
