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
	"bytes"
	cryptorand "crypto/rand"
	"math/rand"
	"testing"
)

func testReadWriteNonComparable[T any](b *testing.B, genRandValue func() T, writeFunc func(T) error, readFunc func() (T, error), flushFunc func() error) {
	randomValue := genRandValue()
	if err := writeFunc(randomValue); err != nil {
		b.Fatalf("failed to write value: %v", err)
	}
	if err := flushFunc(); err != nil {
		b.Fatalf("failed to flush: %v", err)
	}
	_, err := readFunc()
	if err != nil {
		b.Fatalf("failed to read value: %v", err)
	}
}

func testReadWrite[T comparable](b *testing.B, genRandValue func() T, writeFunc func(T) error, readFunc func() (T, error), flushFunc func() error) {
	randomValue := genRandValue()
	if err := writeFunc(randomValue); err != nil {
		b.Fatalf("failed to write value: %v", err)
	}
	if err := flushFunc(); err != nil {
		b.Fatalf("failed to flush: %v", err)
	}
	readValue, err := readFunc()
	if err != nil {
		b.Fatalf("failed to read value: %v", err)
	}
	if readValue != randomValue {
		b.Fatalf("value mismatch")
	}
}

func BenchmarkBinaryBool_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() bool { return rand.Intn(2) == 0 },
			p.WriteBool,
			p.ReadBool,
			p.Flush,
		)
	}
}

func BenchmarkBinaryByte_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() byte { return byte(rand.Intn(256) - 128) },
			p.WriteByte,
			p.ReadByte,
			p.Flush,
		)
	}
}

func BenchmarkBinaryI16_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() int16 { return int16(rand.Intn(int(32767*2)) - 32767) },
			p.WriteI16,
			p.ReadI16,
			p.Flush,
		)
	}
}

func BenchmarkBinaryI32_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Int31,
			p.WriteI32,
			p.ReadI32,
			p.Flush,
		)
	}
}
func BenchmarkBinaryI64_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Int63,
			p.WriteI64,
			p.ReadI64,
			p.Flush,
		)
	}
}
func BenchmarkBinaryDouble_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Float64,
			p.WriteDouble,
			p.ReadDouble,
			p.Flush,
		)
	}
}
func BenchmarkBinaryString_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)

	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	randString := func() string {
		b := make([]rune, 16)
		for i := range b {
			b[i] = letters[rand.Intn(len(letters))]
		}
		return string(b)
	}

	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			randString,
			p.WriteString,
			p.ReadString,
			p.Flush,
		)
	}
}
func BenchmarkBinaryBinary_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewBinaryFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWriteNonComparable(
			b,
			func() []byte {
				data := make([]byte, 16)
				_, _ = cryptorand.Read(data)
				return data
			},
			p.WriteBinary,
			p.ReadBinary,
			p.Flush,
		)
	}
}

func BenchmarkCompactBool_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() bool { return rand.Intn(2) == 0 },
			p.WriteBool,
			p.ReadBool,
			p.Flush,
		)
	}
}

func BenchmarkCompactByte_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() byte { return byte(rand.Intn(256) - 128) },
			p.WriteByte,
			p.ReadByte,
			p.Flush,
		)
	}
}

func BenchmarkCompactI16_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			func() int16 { return int16(rand.Intn(int(32767*2)) - 32767) },
			p.WriteI16,
			p.ReadI16,
			p.Flush,
		)
	}
}

func BenchmarkCompactI32_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Int31,
			p.WriteI32,
			p.ReadI32,
			p.Flush,
		)
	}
}
func BenchmarkCompactI64_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Int63,
			p.WriteI64,
			p.ReadI64,
			p.Flush,
		)
	}
}
func BenchmarkCompactDouble_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			rand.Float64,
			p.WriteDouble,
			p.ReadDouble,
			p.Flush,
		)
	}
}
func BenchmarkCompactString_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)

	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
	randString := func() string {
		b := make([]rune, 16)
		for i := range b {
			b[i] = letters[rand.Intn(len(letters))]
		}
		return string(b)
	}

	for i := 0; i < b.N; i++ {
		testReadWrite(
			b,
			randString,
			p.WriteString,
			p.ReadString,
			p.Flush,
		)
	}
}
func BenchmarkCompactBinary_0(b *testing.B) {
	trans := new(bytes.Buffer)
	p := NewCompactFormat(trans)
	for i := 0; i < b.N; i++ {
		testReadWriteNonComparable(
			b,
			func() []byte {
				data := make([]byte, 16)
				_, _ = cryptorand.Read(data)
				return data
			},
			p.WriteBinary,
			p.ReadBinary,
			p.Flush,
		)
	}
}
