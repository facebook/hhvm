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
)

// MemoryBuffer is a buffer-based implementation with RemainingBytes method.
type MemoryBuffer struct {
	*bytes.Buffer
}

// NewMemoryBuffer returns a new MemoryBuffer.
func NewMemoryBuffer() *MemoryBuffer {
	return &MemoryBuffer{Buffer: &bytes.Buffer{}}
}

// NewMemoryBufferWithData returns a new MemoryBuffer with data.
func NewMemoryBufferWithData(data []byte) *MemoryBuffer {
	return &MemoryBuffer{Buffer: bytes.NewBuffer(data)}
}

// NewMemoryBufferLen returns a new empty MemoryBuffer with a given size.
func NewMemoryBufferLen(size int) *MemoryBuffer {
	buf := make([]byte, 0, size)
	return &MemoryBuffer{Buffer: bytes.NewBuffer(buf)}
}

// Init initializes the buffer with data.
func (m *MemoryBuffer) Init(data []byte) {
	m.Buffer = bytes.NewBuffer(data)
}

// Close resets the buffer.
func (m *MemoryBuffer) Close() error {
	m.Buffer.Reset()
	return nil
}

// RemainingBytes returns the number of bytes remaining in the buffer.
func (m *MemoryBuffer) RemainingBytes() uint64 {
	return uint64(m.Buffer.Len())
}
