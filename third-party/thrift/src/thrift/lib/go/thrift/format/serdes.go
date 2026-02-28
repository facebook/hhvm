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

// newBinarySerializer creates a new serializer using the binary format
func newBinarySerializer(readWriter types.ReadWriteSizer) *Serializer {
	return &Serializer{format: NewBinaryFormat(readWriter)}
}

// newCompactSerializer creates a new serializer using the compact format
func newCompactSerializer(readWriter types.ReadWriteSizer) *Serializer {
	return &Serializer{format: NewCompactFormat(readWriter)}
}

// newCompactJSONSerializer creates a new serializer using the CompactJSON format
func newCompactJSONSerializer(readWriter io.ReadWriter) *Serializer {
	return &Serializer{format: NewCompactJSONFormat(readWriter)}
}

// newSimpleJSONSerializer creates a new serializer using the SimpleJSON format
func newSimpleJSONSerializer(readWriter io.ReadWriter) *Serializer {
	return &Serializer{format: NewSimpleJSONFormat(readWriter)}
}

// newSimpleJSONSerializerV2 creates a new serializer using the SimpleJSON format
func newSimpleJSONSerializerV2(readWriter io.ReadWriter) *Serializer {
	return &Serializer{format: newSimpleJSONFormatV2(readWriter)}
}

// EncodeCompact serializes msg using the compact format
func EncodeCompact(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := newCompactSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeBinary serializes msg using the binary format
func EncodeBinary(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := newBinarySerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeCompactJSON serializes msg using the compact JSON format
func EncodeCompactJSON(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := newCompactJSONSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeSimpleJSON serializes msg using the simple JSON format
func EncodeSimpleJSON(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := newSimpleJSONSerializer(buffer).Encode(msg)
	if err != nil {
		return nil, err
	}
	return buffer.Bytes(), nil
}

// EncodeSimpleJSONV2 serializes msg using the simple JSON format
func EncodeSimpleJSONV2(msg types.WritableStruct) ([]byte, error) {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	err := newSimpleJSONSerializerV2(buffer).Encode(msg)
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

// newBinaryDeserializer creates a new deserializer using the binary format
func newBinaryDeserializer(readWriter types.ReadWriteSizer) *Deserializer {
	return &Deserializer{format: NewBinaryFormat(readWriter)}
}

// newCompactDeserializer creates a new deserializer using the compact format
func newCompactDeserializer(readWriter types.ReadWriteSizer) *Deserializer {
	return &Deserializer{format: NewCompactFormat(readWriter)}
}

// newCompactJSONDeserializer creates a new deserializer using the JSON format
func newCompactJSONDeserializer(readWriter io.ReadWriter) *Deserializer {
	return &Deserializer{format: NewCompactJSONFormat(readWriter)}
}

// newSimpleJSONDeserializer creates a new deserializer using the simple JSON format
func newSimpleJSONDeserializer(readWriter io.ReadWriter) *Deserializer {
	return &Deserializer{format: NewSimpleJSONFormat(readWriter)}
}

// newSimpleJSONDeserializerV2 creates a new deserializer using the simple JSON format
func newSimpleJSONDeserializerV2(readWriter io.ReadWriter) *Deserializer {
	return &Deserializer{format: newSimpleJSONFormatV2(readWriter)}
}

// DecodeCompact deserializes a compact format message
func DecodeCompact(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return newCompactDeserializer(reader).Decode(msg)
}

// DecodeBinary deserializes a binary format message
func DecodeBinary(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return newBinaryDeserializer(reader).Decode(msg)
}

// DecodeCompactJSON deserializes a compact JSON format message
func DecodeCompactJSON(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return newCompactJSONDeserializer(reader).Decode(msg)
}

// DecodeSimpleJSON deserializes a simple JSON format message
func DecodeSimpleJSON(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return newSimpleJSONDeserializer(reader).Decode(msg)
}

// DecodeSimpleJSONV2 deserializes a simple JSON format message
func DecodeSimpleJSONV2(data []byte, msg types.ReadableStruct) error {
	reader := bytes.NewBuffer(data)
	return newSimpleJSONDeserializerV2(reader).Decode(msg)
}

// Decode deserializes a Thrift struct from the underlying format
func (d *Deserializer) Decode(msg types.ReadableStruct) error {
	return msg.Read(d.format)
}
