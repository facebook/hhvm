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
	"bytes"
	"io"
	"math"
	"sync"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

const PROTOCOL_BINARY_DATA_SIZE = 155

type fieldData struct {
	name  string
	typ   types.Type
	id    int16
	value any
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
	doubleValues   = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float64(), NEGATIVE_INFINITY.Float64(), NAN.Float64()}
	floatValues    = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}
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

type protocolTest func(t testing.TB, p types.Format)
type protocolReaderTest func(t testing.TB, p types.Format)
type protocolWriterTest func(t testing.TB, p types.Format)

// ReadWriteProtocolParallelTest tests that a given protocol is safe to read
// from and write to in different goroutines. This requires both a protocol
// and a transport are only using shared state contained either within the set
// of read funcs or within the  set of write funcs.
// It also should only be used with an underlying Transport that is capable of
// blocking reads and writes (socket, stream), since other golang Transport
// implementations require that the data exists to be read when they are called (like bytes.Buffer)
func ReadWriteProtocolParallelTest(t *testing.T, newFormat func(types.ReadWriteSizer) types.Format) {
	transports := []func() types.ReadWriteSizer{}
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
					write(t, p)
				}
			}()
			for i := 0; i < iterations; i++ {
				read(t, p)
			}
			wg.Wait()
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
	doForAllTransportsParallel(func(t testing.TB, p types.Format) {
		ReadBool(t, p)
		ReadByte(t, p)
		ReadI16(t, p)
		ReadI32(t, p)
		ReadI64(t, p)
		ReadDouble(t, p)
		ReadFloat(t, p)
		ReadString(t, p)
		ReadBinary(t, p)
		ReadStruct(t, p)
	}, func(t testing.TB, p types.Format) {
		WriteBool(t, p)
		WriteByte(t, p)
		WriteI16(t, p)
		WriteI32(t, p)
		WriteI64(t, p)
		WriteDouble(t, p)
		WriteFloat(t, p)
		WriteString(t, p)
		WriteBinary(t, p)
		WriteStruct(t, p)
	})
}

func ReadWriteProtocolTest(t *testing.T, newFormat func(types.ReadWriteSizer) types.Format) {
	transports := []func() types.ReadWriteSizer{
		func() types.ReadWriteSizer { return new(bytes.Buffer) },
	}

	doForAllTransports := func(protTest protocolTest) {
		for _, tf := range transports {
			trans := tf()
			p := newFormat(trans)
			protTest(t, p)
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
	doForAllTransports(func(t testing.TB, p types.Format) {
		ReadWriteI64(t, p)
		ReadWriteDouble(t, p)
		ReadWriteFloat(t, p)
		ReadWriteBinary(t, p)
		ReadWriteByte(t, p)
		ReadWriteStruct(t, p)
	})
}

func ReadBool(t testing.TB, p types.Format) {
	thetype := types.BOOL
	thelen := len(boolValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %v", "ReadBool", p, err, boolValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadBool", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadBool", p, thelen, thelen2)
		}
	}
	for k, v := range boolValues {
		value, err := p.ReadBool()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading bool at index %d: %t", "ReadBool", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: index %d %q %q %t != %t", "ReadBool", k, p, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadBool", p, err)
	}
}

func WriteBool(t testing.TB, p types.Format) {
	thetype := types.BOOL
	thelen := len(boolValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Fatalf("%s: %T %q Error writing list begin: %q", "WriteBool", p, err, thetype)
	}
	for k, v := range boolValues {
		err = p.WriteBool(v)
		if err != nil {
			t.Fatalf("%s: %T %q Error writing bool in list at index %d: %t", "WriteBool", p, err, k, v)
		}
	}
	p.WriteListEnd()
	if err != nil {
		t.Fatalf("%s: %T %q Error writing list end: %v", "WriteBool", p, err, boolValues)
	}
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T Unable to flush: %q", "WriteBool", p, err)
	}
}

func ReadWriteBool(t testing.TB, p types.Format) {
	WriteBool(t, p)
	ReadBool(t, p)
}

func WriteByte(t testing.TB, p types.Format) {
	thetype := types.BYTE
	thelen := len(byteValues)
	err := p.WriteListBegin(thetype, thelen)
	if err != nil {
		t.Fatalf("%s: %T %q Error writing list begin: %q", "WriteByte", p, err, thetype)
	}
	for k, v := range byteValues {
		err = p.WriteByte(v)
		if err != nil {
			t.Fatalf("%s: %T %q Error writing byte in list at index %d: %q", "WriteByte", p, err, k, v)
		}
	}
	err = p.WriteListEnd()
	if err != nil {
		t.Fatalf("%s: %T %q Error writing list end: %q", "WriteByte", p, err, byteValues)
	}
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T %q Error flushing list of bytes: %q", "WriteByte", p, err, byteValues)
	}
}

func ReadByte(t testing.TB, p types.Format) {
	thetype := types.BYTE
	thelen := len(byteValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %q", "ReadByte", p, err, byteValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadByte", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadByte", p, thelen, thelen2)
		}
	}
	for k, v := range byteValues {
		value, err := p.ReadByte()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading byte at index %d: %q", "ReadByte", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %d != %d", "ReadByte", p, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadByte", p, err)
	}
}

func ReadWriteByte(t testing.TB, p types.Format) {
	WriteByte(t, p)
	ReadByte(t, p)
}

func WriteI16(t testing.TB, p types.Format) {
	thetype := types.I16
	thelen := len(int16Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int16Values {
		p.WriteI16(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI16(t testing.TB, p types.Format) {
	thetype := types.I16
	thelen := len(int16Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %q", "ReadI16", p, err, int16Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadI16", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadI16", p, thelen, thelen2)
		}
	}
	for k, v := range int16Values {
		value, err := p.ReadI16()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading int16 at index %d: %q", "ReadI16", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %d != %d", "ReadI16", p, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadI16", p, err)
	}
}

func ReadWriteI16(t testing.TB, p types.Format) {
	WriteI16(t, p)
	ReadI16(t, p)
}

func WriteI32(t testing.TB, p types.Format) {
	thetype := types.I32
	thelen := len(int32Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int32Values {
		p.WriteI32(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI32(t testing.TB, p types.Format) {
	thetype := types.I32
	thelen := len(int32Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %q", "ReadI32", p, err, int32Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadI32", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadI32", p, thelen, thelen2)
		}
	}
	for k, v := range int32Values {
		value, err := p.ReadI32()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading int32 at index %d: %q", "ReadI32", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %d != %d", "ReadI32", p, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadI32", p, err)
	}
}

func ReadWriteI32(t testing.TB, p types.Format) {
	WriteI32(t, p)
	ReadI32(t, p)
}

func WriteI64(t testing.TB, p types.Format) {
	thetype := types.I64
	thelen := len(int64Values)
	p.WriteListBegin(thetype, thelen)
	for _, v := range int64Values {
		p.WriteI64(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadI64(t testing.TB, p types.Format) {
	thetype := types.I64
	thelen := len(int64Values)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %q", "ReadI64", p, err, int64Values)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadI64", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadI64", p, thelen, thelen2)
		}
	}
	for k, v := range int64Values {
		value, err := p.ReadI64()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading int64 at index %d: %q", "ReadI64", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %q != %q", "ReadI64", p, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadI64", p, err)
	}
}

func ReadWriteI64(t testing.TB, p types.Format) {
	WriteI64(t, p)
	ReadI64(t, p)
}

func WriteDouble(t testing.TB, p types.Format) {
	doubleValues = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float64(), NEGATIVE_INFINITY.Float64(), NAN.Float64()}
	thetype := types.DOUBLE
	thelen := len(doubleValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range doubleValues {
		p.WriteDouble(v)
	}
	p.WriteListEnd()
	p.Flush()

}

func ReadDouble(t testing.TB, p types.Format) {
	doubleValues = []float64{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float64(), NEGATIVE_INFINITY.Float64(), NAN.Float64()}
	thetype := types.DOUBLE
	thelen := len(doubleValues)
	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %v", "ReadDouble", p, err, doubleValues)
	}
	if thetype != thetype2 {
		t.Fatalf("%s: %T type %s != type %s", "ReadDouble", p, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Fatalf("%s: %T len %d != len %d", "ReadDouble", p, thelen, thelen2)
	}
	for k, v := range doubleValues {
		value, err := p.ReadDouble()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading double at index %d: %f", "ReadDouble", p, err, k, v)
		}
		if math.IsNaN(v) {
			if !math.IsNaN(value) {
				t.Fatalf("%s: %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadDouble", p, v, value)
			}
		} else if v != value {
			t.Fatalf("%s: %T %f != %f", "ReadDouble", p, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadDouble", p, err)
	}
}

func ReadWriteDouble(t testing.TB, p types.Format) {
	WriteDouble(t, p)
	ReadDouble(t, p)
}

func WriteFloat(t testing.TB, p types.Format) {
	floatValues = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}

	thetype := types.FLOAT
	thelen := len(floatValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range floatValues {
		p.WriteFloat(v)
	}
	p.WriteListEnd()
	p.Flush()

}

func ReadFloat(t testing.TB, p types.Format) {
	floatValues = []float32{459.3, 0.0, -1.0, 1.0, 0.5, 0.3333, 3.14159, 1.537e-38, 1.673e25, 6.02214179e23, -6.02214179e23, INFINITY.Float32(), NEGATIVE_INFINITY.Float32(), NAN.Float32()}

	thetype := types.FLOAT
	thelen := len(floatValues)

	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %v", "ReadFloat", p, err, floatValues)
	}
	if thetype != thetype2 {
		t.Fatalf("%s: %T type %s != type %s", "ReadFloat", p, thetype, thetype2)
	}
	if thelen != thelen2 {
		t.Fatalf("%s: %T len %d != len %d", "ReadFloat", p, thelen, thelen2)
	}
	for k, v := range floatValues {
		value, err := p.ReadFloat()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading double at index %d: %f", "ReadFloat", p, err, k, v)
		}
		if math.IsNaN(float64(v)) {
			if !math.IsNaN(float64(value)) {
				t.Fatalf("%s: %T math.IsNaN(%f) != math.IsNaN(%f)", "ReadFloat", p, v, value)
			}
		} else if v != value {
			t.Fatalf("%s: %T %f != %f", "ReadFloat", p, v, value)
		}
	}
	err = p.ReadListEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadFloat", p, err)
	}

}

func ReadWriteFloat(t testing.TB, p types.Format) {
	WriteFloat(t, p)
	ReadFloat(t, p)
}

func WriteString(t testing.TB, p types.Format) {
	thetype := types.STRING
	thelen := len(stringValues)
	p.WriteListBegin(thetype, thelen)
	for _, v := range stringValues {
		p.WriteString(v)
	}
	p.WriteListEnd()
	p.Flush()
}

func ReadString(t testing.TB, p types.Format) {
	thetype := types.STRING
	thelen := len(stringValues)

	thetype2, thelen2, err := p.ReadListBegin()
	if err != nil {
		t.Fatalf("%s: %T %q Error reading list: %q", "ReadString", p, err, stringValues)
	}
	_, ok := p.(*simpleJSONFormat)
	if !ok {
		if thetype != thetype2 {
			t.Fatalf("%s: %T type %s != type %s", "ReadString", p, thetype, thetype2)
		}
		if thelen != thelen2 {
			t.Fatalf("%s: %T len %d != len %d", "ReadString", p, thelen, thelen2)
		}
	}
	for k, v := range stringValues {
		value, err := p.ReadString()
		if err != nil {
			t.Fatalf("%s: %T %q Error reading string at index %d: %q", "ReadString", p, err, k, v)
		}
		if v != value {
			t.Fatalf("%s: %T %s != %s", "ReadString", p, v, value)
		}
	}
	if err != nil {
		t.Fatalf("%s: %T Unable to read list end: %q", "ReadString", p, err)
	}
}

func ReadWriteString(t testing.TB, p types.Format) {
	WriteString(t, p)
	ReadString(t, p)
}

func WriteBinary(t testing.TB, p types.Format) {
	v := protocolBdata
	p.WriteBinary(v)
	p.Flush()
}

func ReadBinary(t testing.TB, p types.Format) {
	v := protocolBdata
	value, err := p.ReadBinary()
	if err != nil {
		t.Fatalf("%s: %T Unable to read binary: %s", "ReadBinary", p, err.Error())
	}
	if len(v) != len(value) {
		t.Fatalf("%s: %T len(v) != len(value)... %d != %d", "ReadBinary", p, len(v), len(value))
	} else {
		for i := 0; i < len(v); i++ {
			if v[i] != value[i] {
				t.Fatalf("%s: %T %s != %s", "ReadBinary", p, v, value)
			}
		}
	}

}

func ReadWriteBinary(t testing.TB, p types.Format) {
	WriteBinary(t, p)
	ReadBinary(t, p)
}

func WriteStruct(t testing.TB, p types.Format) {
	v := structTestData
	p.WriteStructBegin(v.name)
	p.WriteFieldBegin(v.fields[0].name, v.fields[0].typ, v.fields[0].id)
	err := p.WriteBool(v.fields[0].value.(bool))
	if err != nil {
		t.Fatalf("%s: %T Unable to read bool: %s", "WriteStruct", p, err.Error())
	}
	p.WriteFieldEnd()
	p.WriteFieldBegin(v.fields[1].name, v.fields[1].typ, v.fields[1].id)
	err = p.WriteString(v.fields[1].value.(string))
	if err != nil {
		t.Fatalf("%s: %T Unable to read string: %s", "WriteStruct", p, err.Error())
	}
	p.WriteFieldEnd()
	p.WriteStructEnd()
	err = p.Flush()
	if err != nil {
		t.Fatalf("%s: %T Unable to flush: %s", "WriteStruct", p, err.Error())
	}
}

func ReadStruct(t testing.TB, p types.Format) {
	v := structTestData
	_, err := p.ReadStructBegin()
	if err != nil {
		t.Fatalf("%s: %T Unable to read struct begin: %s", "ReadStruct", p, err.Error())
	}
	_, typeID, id, err := p.ReadFieldBegin()
	if err != nil {
		t.Fatalf("%s: %T Unable to read field begin: %s", "ReadStruct", p, err.Error())
	}
	if v.fields[0].typ != typeID {
		t.Fatalf("%s: %T type (%d) != (%d)", "ReadStruct", p, v.fields[0].typ, typeID)
	}
	if v.fields[0].id != id {
		t.Fatalf("%s: %T id (%d) != (%d)", "ReadStruct", p, v.fields[0].id, id)
	}

	val, err := p.ReadBool()
	if err != nil {
		t.Fatalf("%s: %T Unable to read bool: %s", "ReadStruct", p, err.Error())
	}
	if v.fields[0].value != val {
		t.Fatalf("%s: %T value (%v) != (%v)", "ReadStruct", p, v.fields[0].value, val)
	}

	err = p.ReadFieldEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read field end: %s", "ReadStruct", p, err.Error())
	}

	_, typeID, id, err = p.ReadFieldBegin()
	if err != nil {
		t.Fatalf("%s: %T Unable to read field begin: %s", "ReadStruct", p, err.Error())
	}
	if v.fields[1].typ != typeID {
		t.Fatalf("%s: %T type (%d) != (%d)", "ReadStruct", p, v.fields[1].typ, typeID)
	}
	if v.fields[1].id != id {
		t.Fatalf("%s: %T id (%d) != (%d)", "ReadStruct", p, v.fields[1].id, id)
	}

	strVal, err := p.ReadString()
	if err != nil {
		t.Fatalf("%s: %T Unable to read string: %s", "ReadStruct", p, err.Error())
	}
	if v.fields[1].value != strVal {
		t.Fatalf("%s: %T value %T (%s) != (%s)", "ReadStruct", p, v.fields[1].value, strVal)
	}

	err = p.ReadFieldEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read field end: %s", "ReadStruct", p, err.Error())
	}

	err = p.ReadStructEnd()
	if err != nil {
		t.Fatalf("%s: %T Unable to read struct end: %s", "ReadStruct", p, err.Error())
	}
}

func ReadWriteStruct(t testing.TB, p types.Format) {
	WriteStruct(t, p)
	ReadStruct(t, p)
}

func UnmatchedBeginEndProtocolTest(t *testing.T, formatFactory func(io.ReadWriter) types.Format) {
	// NOTE: not all protocol implementations do strict state check to
	// return an error on unmatched Begin/End calls.
	// This test is only meant to make sure that those unmatched Begin/End
	// calls won't cause panic. There's no real "test" here.
	trans := new(bytes.Buffer)
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
}
