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

type BufferedTransport struct {
	buf    bufio.ReadWriter
	socket Socket
}

var _ Socket = (*BufferedTransport)(nil)

func NewBufferedTransport(socket Socket, bufferSize int) *BufferedTransport {
	return &BufferedTransport{
		buf: bufio.ReadWriter{
			Reader: bufio.NewReaderSize(socket, bufferSize),
			Writer: bufio.NewWriterSize(socket, bufferSize),
		},
		socket: socket,
	}
}

func (p *BufferedTransport) Open() error {
	return p.socket.Open()
}

func (p *BufferedTransport) Close() (err error) {
	if err = p.buf.Flush(); err != nil {
		return err
	}
	return p.socket.Close()
}

func (p *BufferedTransport) Read(b []byte) (int, error) {
	n, err := p.buf.Read(b)
	if err != nil {
		p.buf.Reader.Reset(p.socket)
	}
	return n, err
}

func (p *BufferedTransport) Write(b []byte) (int, error) {
	n, err := p.buf.Write(b)
	if err != nil {
		p.buf.Writer.Reset(p.socket)
	}
	return n, err
}

func (p *BufferedTransport) Flush() error {
	if err := p.buf.Flush(); err != nil {
		p.buf.Writer.Reset(p.socket)
		return err
	}
	return nil
}

// LocalAddr returns the local network address, if known.
func (s *BufferedTransport) LocalAddr() net.Addr {
	return s.socket.LocalAddr()
}

// RemoteAddr returns the remote network address, if known.
func (s *BufferedTransport) RemoteAddr() net.Addr {
	return s.socket.RemoteAddr()
}

func (s *BufferedTransport) SetDeadline(t time.Time) error {
	return s.socket.SetDeadline(t)
}

func (s *BufferedTransport) SetReadDeadline(t time.Time) error {
	return s.socket.SetReadDeadline(t)
}

func (s *BufferedTransport) SetWriteDeadline(t time.Time) error {
	return s.socket.SetWriteDeadline(t)
}
