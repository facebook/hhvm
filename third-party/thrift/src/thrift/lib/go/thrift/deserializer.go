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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// A Deserializer is used to turn a byte stream into a Struct
type Deserializer struct {
	buffer  *bytes.Buffer
	decoder types.Decoder
}

// NewBinaryDeserializer creates a new deserializer using the binary format
func NewBinaryDeserializer() *Deserializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	decoder := NewBinaryFormat(buffer)
	return &Deserializer{buffer: buffer, decoder: decoder}
}

// NewCompactDeserializer creates a new deserializer using the compact format
func NewCompactDeserializer() *Deserializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	decoder := NewCompactFormat(buffer)
	return &Deserializer{buffer: buffer, decoder: decoder}
}

// NewCompactJSONDeserializer creates a new deserializer using the JSON format
func NewCompactJSONDeserializer() *Deserializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	decoder := NewCompactJSONFormat(buffer)
	return &Deserializer{buffer: buffer, decoder: decoder}
}

// NewSimpleJSONDeserializer creates a new deserializer using the simple JSON format
func NewSimpleJSONDeserializer() *Deserializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	decoder := NewSimpleJSONFormat(buffer)
	return &Deserializer{buffer: buffer, decoder: decoder}
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
	t.buffer.Reset()

	if _, err := t.buffer.Write(b); err != nil {
		return err
	}
	if err := msg.Read(t.decoder); err != nil {
		return err
	}
	return nil
}
