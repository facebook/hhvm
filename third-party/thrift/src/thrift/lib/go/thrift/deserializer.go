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
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// Default initial size for serializer/deserializer memory buffer.
// The buffer grows automatically when needed during ser/des.
// The initial bufer size is meant to fit most messages, in order to
// completely avoid buffer growth/reallocation and improve ser/des
// performance for the most common usecase (a.k.a. a <1KB message).
const defaultMemoryBufferSize = 1024 // 1KB

type Deserializer struct {
	transport *MemoryBuffer
	decoder   types.Decoder
}

// NewBinaryDeserializer creates a new deserializer using the binary format
func NewBinaryDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	decoder := NewBinaryFormat(transport)
	return &Deserializer{transport: transport, decoder: decoder}
}

// NewCompactDeserializer creates a new deserializer using the compact format
func NewCompactDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	decoder := NewCompactFormat(transport)
	return &Deserializer{transport: transport, decoder: decoder}
}

// NewCompactJSONDeserializer creates a new deserializer using the JSON format
func NewCompactJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	decoder := NewCompactJSONFormat(transport)
	return &Deserializer{transport: transport, decoder: decoder}
}

// NewSimpleJSONDeserializer creates a new deserializer using the simple JSON format
func NewSimpleJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	decoder := NewSimpleJSONFormat(transport)
	return &Deserializer{transport: transport, decoder: decoder}
}

// DecodeCompact deserializes a compact format message
func DecodeCompact(data []byte, msg types.ReadableStruct) error {
	return NewCompactDeserializer().Read(msg, data)
}

// DecodeBinary deserializes a binary format message
func DecodeBinary(data []byte, msg types.ReadableStruct) error {
	return NewBinaryDeserializer().Read(msg, data)
}

// DecodeCompactJSON deserializes a compact JSON format message
func DecodeCompactJSON(data []byte, msg types.ReadableStruct) error {
	return NewCompactJSONDeserializer().Read(msg, data)
}

// DecodeSimpleJSON deserializes a simple JSON format message
func DecodeSimpleJSON(data []byte, msg types.ReadableStruct) error {
	return NewSimpleJSONDeserializer().Read(msg, data)
}

// Read deserializes a Thrift struct from a byte slice
func (t *Deserializer) Read(msg types.ReadableStruct, b []byte) error {
	// Reset the internal buffer (while keeping the underlying storage)
	t.transport.Reset()

	if _, err := t.transport.Write(b); err != nil {
		return err
	}
	if err := msg.Read(t.decoder); err != nil {
		return err
	}
	return nil
}
