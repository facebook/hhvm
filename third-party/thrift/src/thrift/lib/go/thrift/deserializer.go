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

type Deserializer struct {
	Transport *MemoryBuffer
	Protocol  types.Decoder
}

// NewBinaryDeserializer creates a new deserializer using the binary protocol
func NewBinaryDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewBinaryFormat(transport)
	return &Deserializer{Transport: transport, Protocol: protocol}
}

// NewCompactDeserializer creates a new deserializer using the compact protocol
func NewCompactDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewCompactFormat(transport)
	return &Deserializer{Transport: transport, Protocol: protocol}
}

// NewCompactJSONDeserializer creates a new deserializer using the JSON protocol
func NewCompactJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewCompactJSONFormat(transport)
	return &Deserializer{Transport: transport, Protocol: protocol}
}

// NewSimpleJSONDeserializer creates a new deserializer using the simple JSON protocol
func NewSimpleJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewSimpleJSONFormat(transport)
	return &Deserializer{Transport: transport, Protocol: protocol}
}

// DecodeCompact deserializes a compact protocol message
func DecodeCompact(data []byte, msg types.Struct) error {
	return NewCompactDeserializer().Read(msg, data)
}

// DecodeBinary deserializes a binary protocol message
func DecodeBinary(data []byte, msg types.Struct) error {
	return NewBinaryDeserializer().Read(msg, data)
}

// ReadString deserializes a Thrift struct from a string
func (t *Deserializer) ReadString(msg types.Struct, s string) error {
	if _, err := t.Transport.Write([]byte(s)); err != nil {
		return err
	}
	if err := msg.Read(t.Protocol); err != nil {
		return err
	}
	return nil
}

// Read deserializes a Thrift struct from a byte slice
func (t *Deserializer) Read(msg types.Struct, b []byte) error {
	if _, err := t.Transport.Write(b); err != nil {
		return err
	}
	if err := msg.Read(t.Protocol); err != nil {
		return err
	}
	return nil
}
