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
	"crypto/rand"
	"fmt"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
)

func TestBasicSerDes(t *testing.T) {
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

	encodeDecodeTestFn := func(t *testing.T, encodeFn func(types.WritableStruct) ([]byte, error), decodeFn func([]byte, types.ReadableStruct) error) {
		data, err := encodeFn(val)
		require.NoError(t, err)
		var res dummy.DummyStruct1
		err = decodeFn(data, &res)
		require.NoError(t, err)
		require.Equal(t, *val, res)
	}

	t.Run("Binary", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeBinary, DecodeBinary)
	})
	t.Run("Compact", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeCompact, DecodeCompact)
	})
	t.Run("CompactJSON", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeCompactJSON, DecodeCompactJSON)
	})
	t.Run("SimpleJSON", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeSimpleJSON, DecodeSimpleJSON)
	})
}

func TestConsequentSerDes(t *testing.T) {
	// A single Serializer/Deserializer instance should be able to
	// perform multiple sequential serializations/deserializations.

	encodeDecodeTestFn := func(t *testing.T, encoder *Serializer, decoder *Deserializer) {
		for i := range 1000 {
			val := dummy.NewDummyStruct1().
				SetField9(fmt.Sprintf("sequential_test_value_%d", i))

			err := encoder.Encode(val)
			require.NoError(t, err)
			var res dummy.DummyStruct1
			err = decoder.Decode(&res)
			require.NoError(t, err)
			require.Equal(t, *val, res)
		}
	}

	t.Run("Binary", func(t *testing.T) {
		buffer := new(bytes.Buffer)
		serializer := NewBinarySerializer(buffer)
		deserializer := NewBinaryDeserializer(buffer)
		encodeDecodeTestFn(t, serializer, deserializer)
	})
	t.Run("Compact", func(t *testing.T) {
		buffer := new(bytes.Buffer)
		serializer := NewCompactSerializer(buffer)
		deserializer := NewCompactDeserializer(buffer)
		encodeDecodeTestFn(t, serializer, deserializer)
	})
	t.Run("CompactJSON", func(t *testing.T) {
		buffer := new(bytes.Buffer)
		serializer := NewCompactJSONSerializer(buffer)
		deserializer := NewCompactJSONDeserializer(buffer)
		encodeDecodeTestFn(t, serializer, deserializer)
	})
	t.Run("SimpleJSON", func(t *testing.T) {
		buffer := new(bytes.Buffer)
		serializer := NewSimpleJSONSerializer(buffer)
		deserializer := NewSimpleJSONDeserializer(buffer)
		encodeDecodeTestFn(t, serializer, deserializer)
	})
}

func TestGiantStructSerDes(t *testing.T) {
	// Serializer should be able to serialize a giant struct.

	giantByteBlob := make([]byte, 32*1024*1024 /* 32MB */)
	_, err := rand.Read(giantByteBlob)
	require.NoError(t, err)
	val := dummy.NewDummyStruct1().
		SetField8(giantByteBlob).
		SetField9("giant_struct")

	encodeDecodeTestFn := func(t *testing.T, encodeFn func(types.WritableStruct) ([]byte, error), decodeFn func([]byte, types.ReadableStruct) error) {
		data, err := encodeFn(val)
		require.NoError(t, err)
		var res dummy.DummyStruct1
		err = decodeFn(data, &res)
		require.NoError(t, err)
		require.Equal(t, *val, res)
	}

	t.Run("Binary", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeBinary, DecodeBinary)
	})
	t.Run("Compact", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeCompact, DecodeCompact)
	})
	t.Run("CompactJSON", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeCompactJSON, DecodeCompactJSON)
	})
	t.Run("SimpleJSON", func(t *testing.T) {
		encodeDecodeTestFn(t, EncodeSimpleJSON, DecodeSimpleJSON)
	})
}
