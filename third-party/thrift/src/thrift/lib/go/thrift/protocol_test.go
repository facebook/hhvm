/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package thrift

import (
	"fmt"
	"io"
	"math"
	"net"
	"net/http"
	"sync"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

const PROTOCOL_BINARY_DATA_SIZE = 155

type fieldData struct {
	name  string
	typ   types.Type
	id    int16
	value interface{}
}

type structData struct {
	name   string
	fields []fieldData
}

var (
	data           string // test data for writing
	protocolBdata  []byte // test data for writing; same as data
	boolValues     = []bool{false, true, false, false, true}
	byteValues     = []byte{117, 0, 1, 32, 127, 128, 255}
	int16Values    = []int16{459, 0, 1, -1, -128, 127, 32767, -32768}
	int32Values    = []int32{459, 0, 1, -1, -128, 127, 32767, 2147483647, -2147483535}
	int64Values    = []int64{459, 0, 1, -1, -128, 127, 32767, 2147483647, -2147483535, 34359738481, -35184372088719, -9223372036854775808, 9223372036854775807}
	doubleValues   = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float64(), types.NEGATIVE_INFINITY.Float64(), types.NAN.Float64()}
	floatValues    = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float32(), types.NEGATIVE_INFINITY.Float32(), types.NAN.Float32()}
	stringValues   = []string{"", "a", "st[uf]f", "st,u:ff with spaces", "stuff\twith\nescape\\characters'...\"lots{of}fun</xml>"}
	structTestData = structData{
		name: "test struct",
		fields: []fieldData{
			{
				name:  "field1",
				typ:   types.BOOL,
				id:    1,
				value: true,
			},
			{
				name:  "field2",
				typ:   types.STRING,
				id:    2,
				value: "hi",
			},
		},
	}
)

// var floatValues   []float32 = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}

// func floatValues() []
func init() {
	protocolBdata = make([]byte, PROTOCOL_BINARY_DATA_SIZE)
	for i := 0; i < PROTOCOL_BINARY_DATA_SIZE; i++ {
		protocolBdata[i] = byte((i + 'a') % 255)
	}
	data = string(protocolBdata)
}

type HTTPEchoServer struct{}
type HTTPHeaderEchoServer struct{}

func (p *HTTPEchoServer) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	buf, err := io.ReadAll(req.Body)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write(buf)
	} else {
		w.WriteHeader(http.StatusOK)
		w.Write(buf)
	}
}

func (p *HTTPHeaderEchoServer) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	buf, err := io.ReadAll(req.Body)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write(buf)
	} else {
		w.WriteHeader(http.StatusOK)
		w.Write(buf)
	}
}

func HTTPClientSetupForTest(t *testing.T) net.Listener {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("Unable to setup tcp listener on local port: %s", err)
		return l
	}
	go http.Serve(l, &HTTPEchoServer{})
	return l
}

func HTTPClientSetupForHeaderTest(t *testing.T) net.Listener {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("Unable to setup tcp listener on local port: %s", err)
		return l
	}
	go http.Serve(l, &HTTPHeaderEchoServer{})
	return l
}

func tcpStreamSetupForTest(t *testing.T) (io.Reader, io.Writer) {
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	rCh := make(chan io.Reader)
	errCh := make(chan error)
	go func() {
		defer close(rCh)
		for {
			conn, err := l.Accept()
			if err != nil {
				errCh <- fmt.Errorf("could not accept tcp: %s", err.Error())
			}
			rCh <- conn
		}
	}()

	var (
		rConn io.Reader
		wConn io.Writer
	)

	wConn, err = net.Dial(l.Addr().Network(), l.Addr().String())
	if err != nil {
		t.Fatalf("could not dial tcp: %s", err.Error())
	}
	select {
	case rConn = <-rCh:
	case err = <-errCh:
		t.Fatalf(err.Error())
	case <-time.After(1 * time.Second):
		t.Fatalf("timed out waiting on a tcp connection")
	}
	return rConn, wConn
}

type protocolTest func(t testing.TB, p types.Format, trans io.ReadWriter)
type protocolReaderTest func(t testing.TB, p types.Format, trans io.Reader)
type protocolWriterTest func(t testing.TB, p types.Format, trans io.Writer)

// ReadWriteProtocolParallelTest tests that a given protocol is safe to read
// from and write to in different goroutines. This requires both a protocol
// and a transport are only using shared state contained either within the set
// of read funcs or within the  set of write funcs.
// It also should only be used with an underlying Transport that is capable of
// blocking reads and writes (socket, stream), since other golang Transport
// implementations require that the data exists to be read when they are called (like bytes.Buffer)
func ReadWriteProtocolParallelTest(t *testing.T, newFormat func(io.ReadWriteCloser) types.Format) {
	rConn, wConn := tcpStreamSetupForTest(t)
	rdr, writer := io.Pipe()

	transports := []func() io.ReadWriteCloser{
		func() io.ReadWriteCloser {
			return newFramedTransportMaxLength(newStreamTransport(rdr, writer), DEFAULT_MAX_LENGTH)
		}, // framed over pipe
		func() io.ReadWriteCloser {
			return newFramedTransportMaxLength(newStreamTransport(rConn, wConn), DEFAULT_MAX_LENGTH)
		}, // framed over tcp
	}
	const iterations = 100

	doForAllTransportsParallel := func(read protocolReaderTest, write protocolWriterTest) {
		for _, tf := range transports {
			trans := tf()
			p := newFormat(trans)
			var wg sync.WaitGroup
			wg.Add(1)
			go func() {
				defer wg.Done()
				for i := 0; i < iterations; i++ {
					write(t, p, trans)
				}
			}()
			for i := 0; i < iterations; i++ {
				read(t, p, trans)
			}
			wg.Wait()
			trans.Close()
		}
	}

	doForAllTransportsParallel(ReadBool, WriteBool)
	doForAllTransportsParallel(ReadByte, WriteByte)
	doForAllTransportsParallel(ReadI16, WriteI16)
	doForAllTransportsParallel(ReadI32, WriteI32)
	doForAllTransportsParallel(ReadI64, WriteI64)
	doForAllTransportsParallel(ReadDouble, WriteDouble)
	doForAllTransportsParallel(ReadFloat, WriteFloat)
	doForAllTransportsParallel(ReadString, WriteString)
	doForAllTransportsParallel(ReadBinary, WriteBinary)
	doForAllTransportsParallel(ReadStruct, WriteStruct)

	// perform set of many sequenced sets of reads and writes
	doForAllTransportsParallel(func(t testing.TB, p types.Format, trans io.Reader) {
		ReadBool(t, p, trans)
		ReadByte(t, p, trans)
		ReadI16(t, p, trans)
		ReadI32(t, p, trans)
		ReadI64(t, p, trans)
		ReadDouble(t, p, trans)
		ReadFloat(t, p, trans)
		ReadString(t, p, trans)
		ReadBinary(t, p, trans)
		ReadStruct(t, p, trans)
	}, func(t testing.TB, p types.Format, trans io.Writer) {
		WriteBool(t, p, trans)
		WriteByte(t, p, trans)
		WriteI16(t, p, trans)
		WriteI32(t, p, trans)
		WriteI64(t, p, trans)
		WriteDouble(t, p, trans)
		WriteFloat(t, p, trans)
		WriteString(t, p, trans)
		WriteBinary(t, p, trans)
		WriteStruct(t, p, trans)
	})
}

func ReadWriteProtocolTest(t *testing.T, newFormat func(io.ReadWriteCloser) types.Format) {
	l := HTTPClientSetupForTest(t)
	defer l.Close()

	transports := []func() io.ReadWriteCloser{
		func() io.ReadWriteCloser { return NewMemoryBufferLen(1024) },
		func() io.ReadWriteCloser {
			return newFramedTransportMaxLength(NewMemoryBufferLen(1024), DEFAULT_MAX_LENGTH)
		},
		func() io.ReadWriteCloser {
			http, err := newHTTPPostClient("http://" + l.Addr().String())
			if err != nil {
				panic(err)
			}
			return http
		},
	}

	doForAllTransports := func(protTest protocolTest) {
		for _, tf := range transports {
			trans := tf()
			p := newFormat(trans)
			protTest(t, p, trans)
			trans.Close()
		}
	}

	doForAllTransports(ReadWriteBool)
	doForAllTransports(ReadWriteByte)
	doForAllTransports(ReadWriteI16)
	doForAllTransports(ReadWriteI32)
	doForAllTransports(ReadWriteI64)
	doForAllTransports(ReadWriteDouble)
	doForAllTransports(ReadWriteFloat)
	doForAllTransports(ReadWriteString)
	doForAllTransports(ReadWriteBinary)
	doForAllTransports(ReadWriteStruct)

	// perform set of many sequenced reads and writes
	doForAllTransports(func(t testing.TB, p types.Format, trans io.ReadWriter) {
		ReadWriteI64(t, p, trans)
		ReadWriteDouble(t, p, trans)
		ReadWriteFloat(t, p, trans)
		ReadWriteBinary(t, p, trans)
		ReadWriteByte(t, p, trans)
		ReadWriteStruct(t, p, trans)
	})
}

func ReadBool(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.BOOL
	thelen := len(boolValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %v", "ReadBool", p, trans, err, boolValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadBool", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadBool", p, trans, thelen, thelen2)
		}
	}
	for k, v := range boolValues {
		value, err := p.ReadBool()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading bool at index %d: %t", "ReadBool", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: index %d %q %q %t != %t", "ReadBool", k, p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadBool", p, trans, err)
	}
}

func WriteBool(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.BOOL
	thelen := len(boolValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Fatalf("%s: %T %T %q Error writing list begin: %q", "WriteBool", p, trans, err, thetype)
	}
	for k, v := range boolValues {
		err = p.WriteBool(v)
		if err != nil {
			t.Fatalf("%s: %T %T %q Error writing bool in list at index %d: %t", "WriteBool", p, trans, err, k, v)
		}
	}
	p.WriteListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error writing list end: %v", "WriteBool", p, trans, err, boolValues)
	}
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to flush: %q", "WriteBool", p, trans, err)
	}
}

func ReadWriteBool(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteBool(t, p, trans)
	ReadBool(t, p, trans)
}

func WriteByte(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.BYTE
	thelen := len(byteValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Fatalf("%s: %T %T %q Error writing list begin: %q", "WriteByte", p, trans, err, thetype)
	}
	for k, v := range byteValues {
		err = p.WriteByte(v)
		if err != nil {
			t.Fatalf("%s: %T %T %q Error writing byte in list at index %d: %q", "WriteByte", p, trans, err, k, v)
		}
	}
	err = p.WriteListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error writing list end: %q", "WriteByte", p, trans, err, byteValues)
	}
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error flushing list of bytes: %q", "WriteByte", p, trans, err, byteValues)
	}
}

func ReadByte(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.BYTE
	thelen := len(byteValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %q", "ReadByte", p, trans, err, byteValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadByte", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadByte", p, trans, thelen, thelen2)
		}
	}
	for k, v := range byteValues {
		value, err := p.ReadByte()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading byte at index %d: %q", "ReadByte", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %T %d != %d", "ReadByte", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadByte", p, trans, err)
	}
}

func ReadWriteByte(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteByte(t, p, trans)
	ReadByte(t, p, trans)
}

func WriteI16(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.I16
	thelen := len(int16Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int16Values {
		p.WriteI16(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI16(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.I16
	thelen := len(int16Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %q", "ReadI16", p, trans, err, int16Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadI16", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadI16", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int16Values {
		value, err := p.ReadI16()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading int16 at index %d: %q", "ReadI16", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %T %d != %d", "ReadI16", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadI16", p, trans, err)
	}
}

func ReadWriteI16(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteI16(t, p, trans)
	ReadI16(t, p, trans)
}

func WriteI32(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.I32
	thelen := len(int32Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int32Values {
		p.WriteI32(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI32(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.I32
	thelen := len(int32Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %q", "ReadI32", p, trans, err, int32Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadI32", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadI32", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int32Values {
		value, err := p.ReadI32()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading int32 at index %d: %q", "ReadI32", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %T %d != %d", "ReadI32", p, trans, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadI32", p, trans, err)
	}
}

func ReadWriteI32(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteI32(t, p, trans)
	ReadI32(t, p, trans)
}

func WriteI64(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.I64
	thelen := len(int64Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int64Values {
		p.WriteI64(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI64(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.I64
	thelen := len(int64Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %q", "ReadI64", p, trans, err, int64Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadI64", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadI64", p, trans, thelen, thelen2)
		}
	}
	for k, v := range int64Values {
		value, err := p.ReadI64()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading int64 at index %d: %q", "ReadI64", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %T %q != %q", "ReadI64", p, trans, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadI64", p, trans, err)
	}
}

func ReadWriteI64(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteI64(t, p, trans)
	ReadI64(t, p, trans)
}

func WriteDouble(t testing.TB, p types.Format, trans io.Writer) {
	doubleValues = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float64(), types.NEGATIVE_INFINITY.Float64(), types.NAN.Float64()}
	thetype := types.DOUBLE
	thelen := len(doubleValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range doubleValues {
		p.WriteDouble(v)
	}
	p.WriteListEnd()
	p.Flush()

}

func ReadDouble(t testing.TB, p types.Format, trans io.Reader) {
	doubleValues = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float64(), types.NEGATIVE_INFINITY.Float64(), types.NAN.Float64()}
	thetype := types.DOUBLE
	thelen := len(doubleValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %v", "ReadDouble", p, trans, err, doubleValues)
	}
	if thetype != thetype2 {
		t.Fatalf("%s: %T %T type %s != type %s", "ReadDouble", p, trans, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Fatalf("%s: %T %T len %d != len %d", "ReadDouble", p, trans, thelen, thelen2)
	}
	for k, v := range doubleValues {
		value, err := p.ReadDouble()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading double at index %d: %f", "ReadDouble", p, trans, err, k, v)
		}
		if math.IsNaN(v) {
			if !math.IsNaN(value) {
				t.Fatalf("%s: %T %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadDouble", p, trans, v, value)
			}
		} else if v != value {
			t.Fatalf("%s: %T %T %f != %f", "ReadDouble", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadDouble", p, trans, err)
	}
}

func ReadWriteDouble(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteDouble(t, p, trans)
	ReadDouble(t, p, trans)
}

func WriteFloat(t testing.TB, p types.Format, trans io.Writer) {
	floatValues = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float32(), types.NEGATIVE_INFINITY.Float32(), types.NAN.Float32()}

	thetype := types.FLOAT
	thelen := len(floatValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range floatValues {
		p.WriteFloat(v)
	}
	p.WriteListEnd()
	p.Flush()

}

func ReadFloat(t testing.TB, p types.Format, trans io.Reader) {
	floatValues = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, types.INFINITY.Float32(), types.NEGATIVE_INFINITY.Float32(), types.NAN.Float32()}

	thetype := types.FLOAT
	thelen := len(floatValues)

	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %v", "ReadFloat", p, trans, err, floatValues)
	}
	if thetype != thetype2 {
		t.Fatalf("%s: %T %T type %s != type %s", "ReadFloat", p, trans, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Fatalf("%s: %T %T len %d != len %d", "ReadFloat", p, trans, thelen, thelen2)
	}
	for k, v := range floatValues {
		value, err := p.ReadFloat()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading double at index %d: %f", "ReadFloat", p, trans, err, k, v)
		}
		if math.IsNaN(float64(v)) {
			if !math.IsNaN(float64(value)) {
				t.Fatalf("%s: %T %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadFloat", p, trans, v, value)
			}
		} else if v != value {
			t.Fatalf("%s: %T %T %f != %f", "ReadFloat", p, trans, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadFloat", p, trans, err)
	}

}

func ReadWriteFloat(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteFloat(t, p, trans)
	ReadFloat(t, p, trans)
}

func WriteString(t testing.TB, p types.Format, trans io.Writer) {
	thetype := types.STRING
	thelen := len(stringValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range stringValues {
		p.WriteString(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadString(t testing.TB, p types.Format, trans io.Reader) {
	thetype := types.STRING
	thelen := len(stringValues)

	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %T %q Error reading list: %q", "ReadString", p, trans, err, stringValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T %T type %s != type %s", "ReadString", p, trans, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T %T len %d != len %d", "ReadString", p, trans, thelen, thelen2)
		}
	}
	for k, v := range stringValues {
		value, err := p.ReadString()
		if err != nil {
			t.Fatalf("%s: %T %T %q Error reading string at index %d: %q", "ReadString", p, trans, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %T %s != %s", "ReadString", p, trans, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read list end: %q", "ReadString", p, trans, err)
	}
}

func ReadWriteString(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteString(t, p, trans)
	ReadString(t, p, trans)
}

func WriteBinary(t testing.TB, p types.Format, trans io.Writer) {
	v := protocolBdata
	p.WriteBinary(v)
	p.Flush()
}

func ReadBinary(t testing.TB, p types.Format, trans io.Reader) {
	v := protocolBdata
	value, err := p.ReadBinary()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read binary: %s", "ReadBinary", p, trans, err.Error())
	}
	if len(v) != len(value) {
		t.Fatalf("%s: %T %T len(v) != len(value)... %d != %d", "ReadBinary", p, trans, len(v), len(value))
	} else {
		for i := 0; i < len(v); i++ {
			if v[i] != value[i] {
				t.Fatalf("%s: %T %T %s != %s", "ReadBinary", p, trans, v, value)
			}
		}
	}

}

func ReadWriteBinary(t testing.TB, p types.Format, trans io.ReadWriter) {
	WriteBinary(t, p, trans)
	ReadBinary(t, p, trans)
}

func WriteStruct(t testing.TB, p types.Format, trans io.Writer) {
	v := structTestData
	p.WriteStructBegin(v.name)
	p.WriteFieldBegin(v.fields[0].name, v.fields[0].typ, v.fields[0].id)
	err := p.WriteBool(v.fields[0].value.(bool))
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read bool: %s", "WriteStruct", p, trans, err.Error())
	}
	p.WriteFieldEnd()
	p.WriteFieldBegin(v.fields[1].name, v.fields[1].typ, v.fields[1].id)
	err = p.WriteString(v.fields[1].value.(string))
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read string: %s", "WriteStruct", p, trans, err.Error())
	}
	p.WriteFieldEnd()
	p.WriteStructEnd()
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to flush: %s", "WriteStruct", p, trans, err.Error())
	}
}

func ReadStruct(t testing.TB, p types.Format, trans io.Reader) {
	v := structTestData
	_, err := p.ReadStructBegin()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read struct begin: %s", "ReadStruct", p, trans, err.Error())
	}
	_, typeID, id, err := p.ReadFieldBegin()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read field begin: %s", "ReadStruct", p, trans, err.Error())
	}
	if v.fields[0].typ != typeID {
		t.Fatalf("%s: %T %T type (%d) != (%d)", "ReadStruct", p, trans, v.fields[0].typ, typeID)
	}
	if v.fields[0].id != id {
		t.Fatalf("%s: %T %T id (%d) != (%d)", "ReadStruct", p, trans, v.fields[0].id, id)
	}

	val, err := p.ReadBool()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read bool: %s", "ReadStruct", p, trans, err.Error())
	}
	if v.fields[0].value != val {
		t.Fatalf("%s: %T %T value (%v) != (%v)", "ReadStruct", p, trans, v.fields[0].value, val)
	}

	err = p.ReadFieldEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read field end: %s", "ReadStruct", p, trans, err.Error())
	}

	_, typeID, id, err = p.ReadFieldBegin()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read field begin: %s", "ReadStruct", p, trans, err.Error())
	}
	if v.fields[1].typ != typeID {
		t.Fatalf("%s: %T %T type (%d) != (%d)", "ReadStruct", p, trans, v.fields[1].typ, typeID)
	}
	if v.fields[1].id != id {
		t.Fatalf("%s: %T %T id (%d) != (%d)", "ReadStruct", p, trans, v.fields[1].id, id)
	}

	strVal, err := p.ReadString()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read string: %s", "ReadStruct", p, trans, err.Error())
	}
	if v.fields[1].value != strVal {
		t.Fatalf("%s: %T value %T (%s) != (%s)", "ReadStruct", p, trans, v.fields[1].value, strVal)
	}

	err = p.ReadFieldEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read field end: %s", "ReadStruct", p, trans, err.Error())
	}

	err = p.ReadStructEnd()
	if err != nil {
		t.Fatalf("%s: %T %T Unable to read struct end: %s", "ReadStruct", p, trans, err.Error())
	}
}

func ReadWriteStruct(t testing.TB, p types.Format, buffer io.ReadWriter) {
	WriteStruct(t, p, buffer)
	ReadStruct(t, p, buffer)
}

func UnmatchedBeginEndProtocolTest(t *testing.T, formatFactory func(io.ReadWriter) types.Format) {
	// NOTE: not all protocol implementations do strict state check to
	// return an error on unmatched Begin/End calls.
	// This test is only meant to make sure that those unmatched Begin/End
	// calls won't cause panic. There's no real "test" here.
	trans := NewMemoryBuffer()
	t.Run("Read", func(t *testing.T) {
		t.Run("Message", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadMessageEnd()
			p.ReadMessageEnd()
		})
		t.Run("Struct", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadStructEnd()
			p.ReadStructEnd()
		})
		t.Run("Field", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadFieldEnd()
			p.ReadFieldEnd()
		})
		t.Run("Map", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadMapEnd()
			p.ReadMapEnd()
		})
		t.Run("List", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadListEnd()
			p.ReadListEnd()
		})
		t.Run("Set", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.ReadSetEnd()
			p.ReadSetEnd()
		})
	})
	t.Run("Write", func(t *testing.T) {
		t.Run("Message", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteMessageEnd()
			p.WriteMessageEnd()
		})
		t.Run("Struct", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteStructEnd()
			p.WriteStructEnd()
		})
		t.Run("Field", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteFieldEnd()
			p.WriteFieldEnd()
		})
		t.Run("Map", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteMapEnd()
			p.WriteMapEnd()
		})
		t.Run("List", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteListEnd()
			p.WriteListEnd()
		})
		t.Run("Set", func(t *testing.T) {
			trans.Reset()
			p := formatFactory(trans)
			p.WriteSetEnd()
			p.WriteSetEnd()
		})
	})
	trans.Close()
}
