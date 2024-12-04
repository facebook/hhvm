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

	"thrift/lib/go/thrift"
	"thrift/test/go/if/reflecttest"
)

// Run this benchmark:
// buck2 run thrift/test/go/:gobench -- -test.bench=. -test.benchtime=100000x

func BenchmarkStructRead(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := reflecttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[reflecttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	buffer := thrift.NewMemoryBufferLen(10 * 1024 * 1024)
	proto := thrift.NewBinaryFormat(buffer)
	err := originalStruct.Write(proto)
	if err != nil {
		b.Fatalf("failed to write struct: %v", err)
	}

	dataBytes := buffer.Bytes()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		buffer.Init(dataBytes)
		readTargetStruct := reflecttest.NewVariousFieldsStruct()
		err = readTargetStruct.Read(proto)
		if err != nil {
			b.Fatalf("failed to read struct: %v", err)
		}
	}
}

func BenchmarkStructWrite(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := reflecttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[reflecttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	buffer := thrift.NewMemoryBufferLen(10 * 1024 * 1024)
	proto := thrift.NewBinaryFormat(buffer)

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		buffer.Reset()
		err := originalStruct.Write(proto)
		if err != nil {
			b.Fatalf("failed to write struct: %v", err)
		}
	}
}

func BenchmarkStructToString(b *testing.B) {
	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	originalStruct := reflecttest.VariousFieldsStructConst1
	originalStruct.Field51 = map[reflecttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	originalStruct.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = originalStruct.String()
	}
}
