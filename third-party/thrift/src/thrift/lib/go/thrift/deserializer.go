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
	"io"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type Deserializer struct {
	Transport io.ReadWriteCloser
	Protocol  types.Decoder
}

func NewDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewBinaryProtocolTransport(transport)
	return &Deserializer{transport, protocol}
}

// NewCompactDeserializer creates a new deserializer using the compact protocol
func NewCompactDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewCompactProtocol(transport)
	return &Deserializer{transport, protocol}
}

func deserializeCompact(data []byte, msg types.Struct) error {
	buffer := NewMemoryBufferWithData(data)
	format := NewCompactProtocol(buffer)
	return msg.Read(format)
}

// NewJSONDeserializer creates a new deserializer using the JSON protocol
func NewJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewJSONProtocol(transport)
	return &Deserializer{transport, protocol}
}

func NewSimpleJSONDeserializer() *Deserializer {
	transport := NewMemoryBufferLen(1024)
	protocol := NewSimpleJSONProtocol(transport)
	return &Deserializer{transport, protocol}
}

func (t *Deserializer) ReadString(msg types.Struct, s string) (err error) {
	err = nil
	if _, err = t.Transport.Write([]byte(s)); err != nil {
		return
	}
	if err = msg.Read(t.Protocol); err != nil {
		return
	}
	return
}

func (t *Deserializer) Read(msg types.Struct, b []byte) (err error) {
	err = nil
	if _, err = t.Transport.Write(b); err != nil {
		return
	}
	if err = msg.Read(t.Protocol); err != nil {
		return
	}
	return
}
