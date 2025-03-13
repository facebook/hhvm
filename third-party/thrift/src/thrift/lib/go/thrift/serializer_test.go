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
	"crypto/rand"
	"errors"
	"fmt"
	"reflect"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/test/go/if/my_test_struct"
)

type MyTestStruct = my_test_struct.MyTestStruct

func compareStructs(m, m1 MyTestStruct) (bool, error) {
	switch {
	case m.On != m1.On:
		return false, errors.New("Boolean not equal")
	case m.B != m1.B:
		return false, errors.New("Byte not equal")
	case m.Int16 != m1.Int16:
		return false, errors.New("Int16 not equal")
	case m.Int32 != m1.Int32:
		return false, errors.New("Int32 not equal")
	case m.Int64 != m1.Int64:
		return false, errors.New("Int64 not equal")
	case m.D != m1.D:
		return false, errors.New("Double not equal")
	case m.F != m1.F:
		return false, errors.New("Float not equal")
	case m.St != m1.St:
		return false, errors.New("String not equal")

	case len(m.Bin) != len(m1.Bin):
		return false, errors.New("Binary size not equal")
	case len(m.Bin) == len(m1.Bin):
		for i := range m.Bin {
			if m.Bin[i] != m1.Bin[i] {
				return false, errors.New("Binary not equal")
			}
		}
	case len(m.StringMap) != len(m1.StringMap):
		return false, errors.New("StringMap size not equal")
	case len(m.StringList) != len(m1.StringList):
		return false, errors.New("StringList size not equal")
	case len(m.StringSet) != len(m1.StringSet):
		return false, errors.New("StringSet size not equal")

	case m.E != m1.E:
		return false, errors.New("MyTestEnum not equal")

	default:
		return true, nil

	}
	return true, nil
}

func ProtocolTest1(test *testing.T, serial *Serializer, deserial *Deserializer) (bool, error) {
	var m = MyTestStruct{}
	m.On = true
	m.B = int8(0)
	m.Int16 = 1
	m.Int32 = 2
	m.Int64 = 3
	m.D = 4.1
	m.St = "Test"
	m.Bin = make([]byte, 10)
	m.StringMap = make(map[string]string, 5)
	m.StringList = make([]string, 5)
	m.StringSet = make([]string, 5)
	m.E = 2

	data, err := serial.Write(&m)
	if err != nil {
		return false, fmt.Errorf("Unable to Serialize struct\n\t %w", err)
	}

	var m1 = MyTestStruct{}
	if err = deserial.Read(&m1, data); err != nil {
		return false, fmt.Errorf("Unable to Deserialize struct\n\t %w", err)
	}

	return compareStructs(m, m1)

}

func ProtocolTest2(test *testing.T, serial *Serializer, deserial *Deserializer) (bool, error) {
	var m = MyTestStruct{}
	m.On = false
	m.B = int8(0)
	m.Int16 = 1
	m.Int32 = 2
	m.Int64 = 3
	m.D = 4.1
	m.St = "Test"
	m.Bin = make([]byte, 10)
	m.StringMap = make(map[string]string, 5)
	m.StringList = make([]string, 5)
	m.StringSet = make([]string, 5)
	m.E = 2

	data, err := serial.Write(&m)
	if err != nil {
		return false, fmt.Errorf("Unable to Serialize struct\n\t %w", err)

	}

	var m1 = MyTestStruct{}
	if err = deserial.Read(&m1, data); err != nil {
		return false, fmt.Errorf("Unable to Deserialize struct\n\t %w", err)
	}

	return compareStructs(m, m1)

}

func TestSerializer(t *testing.T) {

	serializers := make(map[string]*Serializer)
	serializers["Binary"] = NewBinarySerializer()
	serializers["Compact"] = NewCompactSerializer()
	serializers["JSON"] = NewCompactJSONSerializer()
	deserializers := make(map[string]*Deserializer)
	deserializers["Binary"] = NewBinaryDeserializer()
	deserializers["Compact"] = NewCompactDeserializer()
	deserializers["JSON"] = NewCompactJSONDeserializer()

	tests := make(map[string]func(*testing.T, *Serializer, *Deserializer) (bool, error))
	tests["Test 1"] = ProtocolTest1
	tests["Test 2"] = ProtocolTest2
	//tests["Test 3"] = ProtocolTest3 // Example of how to add additional tests

	for name, serial := range serializers {

		for test, f := range tests {

			if s, err := f(t, serial, deserializers[name]); !s || err != nil {
				t.Errorf("%s Failed for %s protocol\n\t %s", test, name, err)
			}

		}
	}

}

func TestGiantStructSerialization(t *testing.T) {
	// Serializer should be able to serialize a struct
	// much larger than the default size memory buffer.

	ser := NewBinarySerializer()

	giantByteBlob := make([]byte, defaultMemoryBufferSize*10)
	_, err := rand.Read(giantByteBlob)
	if err != nil {
		t.Fatalf("failed to rand read: %v", err)
	}
	val := dummy.NewDummyStruct1().
		SetField8(giantByteBlob).
		SetField9("giant_struct")
	pay, err := ser.Write(val)
	if err != nil {
		t.Fatalf("failed to write: %v", err)
	}

	var res dummy.DummyStruct1
	err = DecodeBinary(pay, &res)
	if err != nil {
		t.Fatalf("failed to decode: %v", err)
	}
	if !reflect.DeepEqual(*val, res) {
		t.Fatalf("values are not equal: %+v != %+v", *val, res)
	}
}

func TestConsequentSerialization(t *testing.T) {
	// A single Serializer instance should be able to
	// perform multiple sequential serializations.

	ser := NewBinarySerializer()

	for i := range 1000 {
		val := dummy.NewDummyStruct1().
			SetField9(fmt.Sprintf("sequential_test_value_%d", i))
		pay, err := ser.Write(val)
		if err != nil {
			t.Fatalf("failed to write: %v", err)
		}

		var res dummy.DummyStruct1
		err = DecodeBinary(pay, &res)
		if err != nil {
			t.Fatalf("failed to decode: %v", err)
		}
		if !reflect.DeepEqual(*val, res) {
			t.Fatalf("values are not equal: %+v != %+v", *val, res)
		}
	}
}

func TestSerializationBufferOwnership(t *testing.T) {
	// Serialized payload should reside in its own byte slice,
	// and not in the internal Serializer memory buffer.

	ser := NewBinarySerializer()
	prevRes := ([]byte)(nil)

	for range 10 {
		val := dummy.NewDummyStruct1()
		pay, err := ser.Write(val)
		if err != nil {
			t.Fatalf("failed to write: %v", err)
		}
		if reflect.ValueOf(pay).Pointer() == reflect.ValueOf(prevRes).Pointer() {
			t.Fatalf("payload pointers match")
		}
		prevRes = pay
	}
}

func BenchmarkBinarySerializer(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkBinarySerializer -test.benchtime=100x

	ser := NewBinarySerializer()

	val := dummy.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")

	for b.Loop() {
		_, err := ser.Write(val)
		if err != nil {
			b.Fatalf("failed to write: %v", err)
		}
	}
}

func BenchmarkEncodeBinary(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeBinary -test.benchtime=1000x

	val := dummy.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")

	for b.Loop() {
		_, err := EncodeBinary(val)
		if err != nil {
			b.Fatalf("failed to encode: %v", err)
		}
	}
}

func BenchmarkEncodeCompact(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkEncodeCompact -test.benchtime=1000x

	val := dummy.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")

	for b.Loop() {
		_, err := EncodeCompact(val)
		if err != nil {
			b.Fatalf("failed to encode: %v", err)
		}
	}
}
