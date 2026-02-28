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

package gotest

import (
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

// Run this benchmark:
// buck2 run @mode/opt thrift/test/go/:gobench -- -test.bench=. -test.benchtime=100000x

func BenchmarkStructDecode(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := thrifttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[thrifttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	dataBytes, err := thrift.EncodeBinary(originalStruct)
	require.NoError(b, err)

	for b.Loop() {
		readTargetStruct := thrifttest.NewVariousFieldsStruct()
		err = thrift.DecodeBinary(dataBytes, readTargetStruct)
		require.NoError(b, err)
	}
}

func BenchmarkStructEncode(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := thrifttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[thrifttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	for b.Loop() {
		_, err := thrift.EncodeBinary(originalStruct)
		require.NoError(b, err)
	}
}

func BenchmarkStructToString(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := thrifttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[thrifttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	for b.Loop() {
		_ = originalStruct.String()
	}
}
