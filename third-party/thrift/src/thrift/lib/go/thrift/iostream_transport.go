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
	"bufio"
	"io"
)

// StreamTransport is a Transport made of an io.Reader and/or an io.Writer
type StreamTransport struct {
	io.Reader
	io.Writer
}

func newStreamTransport(r io.Reader, w io.Writer) *StreamTransport {
	return &StreamTransport{Reader: bufio.NewReader(r), Writer: bufio.NewWriter(w)}
}

// Close closes both the reader and writer streams.
func (p *StreamTransport) Close() error {
	if p.Reader != nil {
		c, ok := p.Reader.(io.Closer)
		if ok {
			e := c.Close()
			if e != nil {
				return e
			}
		}
	}
	if p.Writer != nil {
		c, ok := p.Writer.(io.Closer)
		if ok {
			e := c.Close()
			if e != nil {
				return e
			}
		}
	}
	return nil
}

// Flush flushes the underlying output stream if not null.
func (p *StreamTransport) Flush() error {
	if f, ok := p.Writer.(Flusher); ok {
		return NewTransportExceptionFromError(f.Flush())
	}
	return nil
}
