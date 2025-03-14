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
	buffer  *bytes.Buffer
	encoder types.Encoder
}

// NewBinarySerializer create a new serializer using the binary format
func NewBinarySerializer() *Serializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	encoder := NewBinaryFormat(buffer)
	return &Serializer{buffer: buffer, encoder: encoder}
}

// NewCompactSerializer creates a new serializer using the compact format
func NewCompactSerializer() *Serializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	encoder := NewCompactFormat(buffer)
	return &Serializer{buffer: buffer, encoder: encoder}
}

// NewCompactJSONSerializer creates a new serializer using the compact JSON format
func NewCompactJSONSerializer() *Serializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	encoder := NewCompactJSONFormat(buffer)
	return &Serializer{buffer: buffer, encoder: encoder}
}

// NewSimpleJSONSerializer creates a new serializer using the SimpleJSON format
func NewSimpleJSONSerializer() *Serializer {
	buffer := bytes.NewBuffer(make([]byte, 0, defaultBufferSize))
	encoder := NewSimpleJSONFormat(buffer)
	return &Serializer{buffer: buffer, encoder: encoder}
}

// A SerializerV2 is used to turn a Struct in to a byte stream
type SerializerV2 struct {
	format types.Format
}

// NewBinarySerializerV2 creates a new serializer using the binary format
func NewBinarySerializerV2(readWriter types.ReadWriteSizer) *SerializerV2 {
	return &SerializerV2{format: NewBinaryFormat(readWriter)}
}

// NewCompactSerializerV2 creates a new serializer using the compact format
func NewCompactSerializerV2(readWriter types.ReadWriteSizer) *SerializerV2 {
	return &SerializerV2{format: NewCompactFormat(readWriter)}
}

// NewCompactJSONSerializerV2 creates a new serializer using the CompactJSON format
func NewCompactJSONSerializerV2(readWriter io.ReadWriter) *SerializerV2 {
	return &SerializerV2{format: NewCompactJSONFormat(readWriter)}
}

// NewSimpleJSONSerializerV2 creates a new serializer using the SimpleJSON format
func NewSimpleJSONSerializerV2(readWriter io.ReadWriter) *SerializerV2 {
	return &SerializerV2{format: NewSimpleJSONFormat(readWriter)}
}

// EncodeCompact serializes msg using the compact format
func EncodeCompact(msg types.WritableStruct) ([]byte, error) {
	return NewCompactSerializer().Write(msg)
}

// EncodeBinary serializes msg using the binary format
func EncodeBinary(msg types.WritableStruct) ([]byte, error) {
	return NewBinarySerializer().Write(msg)
}

// EncodeCompactJSON serializes msg using the compact JSON format
func EncodeCompactJSON(msg types.WritableStruct) ([]byte, error) {
	return NewCompactJSONSerializer().Write(msg)
}

// EncodeSimpleJSON serializes msg using the simple JSON format
func EncodeSimpleJSON(msg types.WritableStruct) ([]byte, error) {
	return NewSimpleJSONSerializer().Write(msg)
}

// Write writes msg to the serializer and returns it as a byte array
func (s *Serializer) Write(msg types.WritableStruct) ([]byte, error) {
	s.buffer.Reset()

	if err := msg.Write(s.encoder); err != nil {
		return nil, err
	}

	if err := s.encoder.Flush(); err != nil {
		return nil, err
	}

	// Copy the bytes from our internal Transport buffer into
	// a separate fresh buffer that will be fully owned by the caller.
	// The contents of the internal Transport buffer will be overwritten
	// on the next call to Write()/WriteString(). We don't want that to
	// affect the caller's buffer. Hence, the copy.
	serBytes := make([]byte, s.buffer.Len())
	copy(serBytes, s.buffer.Bytes())

	return serBytes, nil
}

// Encode encodes a Thrift struct into the underlying format/writer.
func (s *SerializerV2) Encode(msg types.WritableStruct) error {
	if err := msg.Write(s.format); err != nil {
		return err
	}
	return s.format.Flush()
}
