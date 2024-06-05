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
	"net"
	"time"
)

// BufferedTransport is a buffered transport that has Flush method.
type BufferedTransport struct {
	buf  bufio.ReadWriter
	conn net.Conn
}

var _ net.Conn = (*BufferedTransport)(nil)

// NewBufferedTransport creates a new buffered transport using bufio.ReadWriter for the provided bufferSize.
func NewBufferedTransport(conn net.Conn, bufferSize int) *BufferedTransport {
	return &BufferedTransport{
		buf: bufio.ReadWriter{
			Reader: bufio.NewReaderSize(conn, bufferSize),
			Writer: bufio.NewWriterSize(conn, bufferSize),
		},
		conn: conn,
	}
}

// Close closes the underlying conn and flushes the buffer.
func (b *BufferedTransport) Close() (err error) {
	if err = b.buf.Flush(); err != nil {
		return err
	}
	return b.conn.Close()
}

// Read reads the buffer and resets the underlying buffer if there was an error.
func (b *BufferedTransport) Read(data []byte) (int, error) {
	n, err := b.buf.Read(data)
	if err != nil {
		b.buf.Reader.Reset(b.conn)
	}
	return n, err
}

// Write writes to the buffer and resets the underlying buffer if there was an error.
func (b *BufferedTransport) Write(data []byte) (int, error) {
	n, err := b.buf.Write(data)
	if err != nil {
		b.buf.Writer.Reset(b.conn)
	}
	return n, err
}

// Flush flushes the buffer and resets the underlying buffer if there was an error.
func (b *BufferedTransport) Flush() error {
	if err := b.buf.Flush(); err != nil {
		b.buf.Writer.Reset(b.conn)
		return err
	}
	return nil
}

// LocalAddr returns the local network address, if known.
func (b *BufferedTransport) LocalAddr() net.Addr {
	return b.conn.LocalAddr()
}

// RemoteAddr returns the remote network address, if known.
func (b *BufferedTransport) RemoteAddr() net.Addr {
	return b.conn.RemoteAddr()
}

// SetDeadline sets the read and write deadlines associated with the connection.
func (b *BufferedTransport) SetDeadline(t time.Time) error {
	return b.conn.SetDeadline(t)
}

// SetReadDeadline sets the deadline for future Read calls.
func (b *BufferedTransport) SetReadDeadline(t time.Time) error {
	return b.conn.SetReadDeadline(t)
}

// SetWriteDeadline sets the deadline for future Write calls.
func (b *BufferedTransport) SetWriteDeadline(t time.Time) error {
	return b.conn.SetWriteDeadline(t)
}
