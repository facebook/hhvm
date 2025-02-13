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
