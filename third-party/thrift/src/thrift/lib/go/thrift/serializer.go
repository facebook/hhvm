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

// A Serializer is used to turn a Struct in to a byte stream
type Serializer struct {
	transport *MemoryBuffer
	encoder   types.Encoder
}

// NewBinarySerializer create a new serializer using the binary format
func NewBinarySerializer() *Serializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	encoder := NewBinaryFormat(transport)
	return &Serializer{transport: transport, encoder: encoder}
}

// NewCompactSerializer creates a new serializer using the compact format
func NewCompactSerializer() *Serializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	encoder := NewCompactFormat(transport)
	return &Serializer{transport: transport, encoder: encoder}
}

// NewCompactJSONSerializer creates a new serializer using the compact JSON format
func NewCompactJSONSerializer() *Serializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	encoder := NewCompactJSONFormat(transport)
	return &Serializer{transport: transport, encoder: encoder}
}

// NewSimpleJSONSerializer creates a new serializer using the SimpleJSON format
func NewSimpleJSONSerializer() *Serializer {
	transport := NewMemoryBufferLen(defaultMemoryBufferSize)
	encoder := NewSimpleJSONFormat(transport)
	return &Serializer{transport: transport, encoder: encoder}
}

// EncodeCompact serializes msg using the compact format
func EncodeCompact(msg types.WritableStruct) ([]byte, error) {
	return NewCompactSerializer().Write(msg)
}

// EncodeBinary serializes msg using the binary format
func EncodeBinary(msg types.WritableStruct) ([]byte, error) {
	return NewBinarySerializer().Write(msg)
}

// WriteString writes msg to the serializer and returns it as a string
func (s *Serializer) WriteString(msg types.WritableStruct) (string, error) {
	serBytes, err := s.Write(msg)
	if err != nil {
		return "", err
	}
	return string(serBytes), nil
}

// Write writes msg to the serializer and returns it as a byte array
func (s *Serializer) Write(msg types.WritableStruct) ([]byte, error) {
	s.transport.Reset()

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
	serBytes := make([]byte, s.transport.Len())
	copy(serBytes, s.transport.Bytes())

	return serBytes, nil
}
