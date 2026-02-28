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

package format

import (
	"testing"

	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func BenchmarkEncodeBinary(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeBinary -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
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
		require.NoError(b, err)
	}
}

func BenchmarkEncodeCompact(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkEncodeCompact -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
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
		require.NoError(b, err)
	}
}

func BenchmarkEncodeCompactJSON(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkEncodeCompact -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
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
		_, err := EncodeCompactJSON(val)
		require.NoError(b, err)
	}
}

func BenchmarkEncodeSimpleJSON(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkEncodeCompact -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
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
		_, err := EncodeSimpleJSON(val)
		require.NoError(b, err)
	}
}

func BenchmarkDecodeBinary(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeBinary -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
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
	require.NoError(b, err)

	for b.Loop() {
		var res dummyif.DummyStruct1
		err = DecodeBinary(pay, &res)
		require.NoError(b, err)
	}
}

func BenchmarkDecodeCompact(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeCompact -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")
	pay, err := EncodeCompact(val)
	require.NoError(b, err)

	for b.Loop() {
		var res dummyif.DummyStruct1
		err = DecodeCompact(pay, &res)
		require.NoError(b, err)
	}
}

func BenchmarkDecodeCompactJSON(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeCompactJSON -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")
	pay, err := EncodeCompactJSON(val)
	require.NoError(b, err)

	for b.Loop() {
		var res dummyif.DummyStruct1
		err = DecodeCompactJSON(pay, &res)
		require.NoError(b, err)
	}
}

func BenchmarkDecodeSimpleJSON(b *testing.B) {
	// Run: buck run @mode/opt thrift/lib/go/thrift:thrift_test -- --test.bench=BenchmarkDecodeSimpleJSON -test.benchtime=1000x

	val := dummyif.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")
	pay, err := EncodeSimpleJSON(val)
	require.NoError(b, err)

	for b.Loop() {
		var res dummyif.DummyStruct1
		err = DecodeSimpleJSON(pay, &res)
		require.NoError(b, err)
	}
}
