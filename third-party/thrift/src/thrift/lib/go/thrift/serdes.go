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
	"io"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// Default initial size for serializer/deserializer memory buffer.
// The buffer grows automatically when needed during ser/des.
// The initial bufer size is meant to fit most messages, in order to
// completely avoid buffer growth/reallocation and improve ser/des
// performance for the most common usecase (a.k.a. a <1KB message).
const defaultBufferSize = 1024 // 1KB

// A Serializer is used to turn a Struct in to a byte stream
type Serializer struct {
	format types.Format
}

// NewBinarySerializer creates a new serializer using the binary format
func NewBinarySerializer(readWriter types.ReadWriteSizer) *Serializer {
	return &Serializer{format: NewBinaryFormat(readWriter)}
}

// NewCompactSerializer creates a new serializer using the compact format
func NewCompactSerializer(readWriter types.ReadWriteSizer) *Serializer {
	return &Serializer{format: NewCompactFormat(readWriter)}
}

// NewCompactJSONSerializer creates a new serializer using the CompactJSON format
func NewCompactJSONSerializer(readWriter io.ReadWriter) *Serializer {
	return &Serializer{format: NewCompactJSONFormat(readWriter)}
}

// NewSimpleJSONSerializer creates a new serializer using the SimpleJSON format
func NewSimpleJSONSerializer(readWriter io.ReadWriter) *Serializer {
	return &Serializer{format: NewSimpleJSONFormat(readWriter)}
}

// EncodeCompact serializes msg using the compact format
func EncodeCompact(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := NewCompactSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeBinary serializes msg using the binary format
func EncodeBinary(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := NewBinarySerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeCompactJSON serializes msg using the compact JSON format
func EncodeCompactJSON(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := NewCompactJSONSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeSimpleJSON serializes msg using the simple JSON format
func EncodeSimpleJSON(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := NewSimpleJSONSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// Encode encodes a Thrift struct into the underlying format/writer.
func (s *Serializer) Encode(msg types.WritableStruct) error {
	if err := msg.Write(s.format); err != nil {
		return err
	}
	return s.format.Flush()
}

// A Deserializer is used to turn a byte stream into a Struct
type Deserializer struct {
	format types.Format
}

// NewBinaryDeserializer creates a new deserializer using the binary format
func NewBinaryDeserializer(readWriter types.ReadWriteSizer) *Deserializer {
	return &Deserializer{format: NewBinaryFormat(readWriter)}
}

// NewCompactDeserializer creates a new deserializer using the compact format
func NewCompactDeserializer(readWriter types.ReadWriteSizer) *Deserializer {
	return &Deserializer{format: NewCompactFormat(readWriter)}
}

// NewCompactJSONDeserializer creates a new deserializer using the JSON format
func NewCompactJSONDeserializer(readWriter io.ReadWriter) *Deserializer {
	return &Deserializer{format: NewCompactJSONFormat(readWriter)}
}

// NewSimpleJSONDeserializer creates a new deserializer using the simple JSON format
func NewSimpleJSONDeserializer(readWriter io.ReadWriter) *Deserializer {
	return &Deserializer{format: NewSimpleJSONFormat(readWriter)}
}

// DecodeCompact deserializes a compact format message
func DecodeCompact(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return NewCompactDeserializer(reader).Decode(msg)
}

// DecodeBinary deserializes a binary format message
func DecodeBinary(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return NewBinaryDeserializer(reader).Decode(msg)
}

// DecodeCompactJSON deserializes a compact JSON format message
func DecodeCompactJSON(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return NewCompactJSONDeserializer(reader).Decode(msg)
}

// DecodeSimpleJSON deserializes a simple JSON format message
func DecodeSimpleJSON(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return NewSimpleJSONDeserializer(reader).Decode(msg)
}

// Decode deserializes a Thrift struct from the underlying format
func (d *Deserializer) Decode(msg types.ReadableStruct) error {
	return msg.Read(d.format)
}
