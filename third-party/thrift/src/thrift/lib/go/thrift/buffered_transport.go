/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
)

type BufferedTransportFactory struct {
	size int
}

type BufferedTransport struct {
	bufio.ReadWriter
	tp Transport
}

func (p *BufferedTransportFactory) GetTransport(trans Transport) Transport {
	return NewBufferedTransport(trans, p.size)
}

func NewBufferedTransportFactory(bufferSize int) *BufferedTransportFactory {
	return &BufferedTransportFactory{size: bufferSize}
}

func NewBufferedTransport(trans Transport, bufferSize int) *BufferedTransport {
	return &BufferedTransport{
		ReadWriter: bufio.ReadWriter{
			Reader: bufio.NewReaderSize(trans, bufferSize),
			Writer: bufio.NewWriterSize(trans, bufferSize),
		},
		tp: trans,
	}
}

func (p *BufferedTransport) IsOpen() bool {
	return p.tp.IsOpen()
}

func (p *BufferedTransport) Open() (err error) {
	return p.tp.Open()
}

func (p *BufferedTransport) Close() (err error) {
	return p.tp.Close()
}

func (p *BufferedTransport) Read(b []byte) (int, error) {
	n, err := p.ReadWriter.Read(b)
	if err != nil {
		p.ReadWriter.Reader.Reset(p.tp)
	}
	return n, err
}

func (p *BufferedTransport) Write(b []byte) (int, error) {
	n, err := p.ReadWriter.Write(b)
	if err != nil {
		p.ReadWriter.Writer.Reset(p.tp)
	}
	return n, err
}

func (p *BufferedTransport) Flush() error {
	if err := p.ReadWriter.Flush(); err != nil {
		p.ReadWriter.Writer.Reset(p.tp)
		return err
	}
	return p.tp.Flush()
}

func (p *BufferedTransport) RemainingBytes() (num_bytes uint64) {
	return p.tp.RemainingBytes()
}
