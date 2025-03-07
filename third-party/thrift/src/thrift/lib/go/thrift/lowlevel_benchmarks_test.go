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
	"testing"
)

func BenchmarkBinaryBool_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteBool(b, p)
	}
}

func BenchmarkBinaryByte_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteByte(b, p)
	}
}

func BenchmarkBinaryI16_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI16(b, p)
	}
}

func BenchmarkBinaryI32_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI32(b, p)
	}
}
func BenchmarkBinaryI64_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI64(b, p)
	}
}
func BenchmarkBinaryDouble_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteDouble(b, p)
	}
}
func BenchmarkBinaryString_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteString(b, p)
	}
}
func BenchmarkBinaryBinary_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteBinary(b, p)
	}
}

func BenchmarkCompactBool_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteBool(b, p)
	}
}

func BenchmarkCompactByte_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteByte(b, p)
	}
}

func BenchmarkCompactI16_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI16(b, p)
	}
}

func BenchmarkCompactI32_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI32(b, p)
	}
}
func BenchmarkCompactI64_0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteI64(b, p)
	}
}
func BenchmarkCompactDouble0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteDouble(b, p)
	}
}
func BenchmarkCompactString0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteString(b, p)
	}
}
func BenchmarkCompactBinary0(b *testing.B) {
	trans := NewMemoryBufferLen(1024)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		ReadWriteBinary(b, p)
	}
}
