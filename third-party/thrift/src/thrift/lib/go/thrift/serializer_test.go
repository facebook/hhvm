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
	"fmt"
	"reflect"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
)

func TestGiantStructSerialization(t *testing.T) {
	// Serializer should be able to serialize a struct
	// much larger than the default size memory buffer.

	ser := NewBinarySerializer()

	giantByteBlob := make([]byte, defaultBufferSize*10)
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
