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

func TestGiantStructDeserialization(t *testing.T) {
	// Deserializer should be able to deserialize a struct
	// much larger than the default size memory buffer.

	des := NewBinaryDeserializer()

	giantByteBlob := make([]byte, defaultMemoryBufferSize*10)
	_, err := rand.Read(giantByteBlob)
	if err != nil {
		t.Fatalf("failed to rand read: %v", err)
	}
	val := dummy.NewDummyStruct1().
		SetField8(giantByteBlob).
		SetField9("giant_struct")
	pay, err := EncodeBinary(val)
	if err != nil {
		t.Fatalf("failed to encode: %v", err)
	}

	var res dummy.DummyStruct1
	err = des.Read(&res, pay)
	if err != nil {
		t.Fatalf("failed to read: %v", err)
	}
	if !reflect.DeepEqual(*val, res) {
		t.Fatalf("values are not equal: %+v != %+v", *val, res)
	}
}

func TestConsequentDeserialization(t *testing.T) {
	// A single Deserializer instance should be able to
	// perform multiple sequential deserializations.

	des := NewBinaryDeserializer()

	for i := range 1000 {
		val := dummy.NewDummyStruct1().
			SetField9(fmt.Sprintf("sequential_test_value_%d", i))
		pay, err := EncodeBinary(val)
		if err != nil {
			t.Fatalf("failed to encode: %v", err)
		}

		var res dummy.DummyStruct1
		err = des.Read(&res, pay)
		if err != nil {
			t.Fatalf("failed to read: %v", err)
		}
		if !reflect.DeepEqual(*val, res) {
			t.Fatalf("values are not equal: %+v != %+v", *val, res)
		}
	}

	// The internal memory buffer should not have grown at all,
	// since we were deserializing small structs.
	if des.transport.Cap() != defaultMemoryBufferSize {
		t.Fatalf("deserializer memory buffer grew: %d", des.transport.Cap())
	}
}

func BenchmarkBinaryDeserializer(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkBinaryDeserializer -test.benchtime=100x

	des := NewBinaryDeserializer()

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
	pay, err := EncodeBinary(val)
	if err != nil {
		b.Fatalf("failed to encode: %v", err)
	}

	for b.Loop() {
		var res dummy.DummyStruct1
		err = des.Read(&res, pay)
		if err != nil {
			b.Fatalf("failed to read: %v", err)
		}
	}
}
