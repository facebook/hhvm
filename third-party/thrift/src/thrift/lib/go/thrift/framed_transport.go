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
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"io/ioutil"
)

const DEFAULT_MAX_LENGTH = 16384000

type FramedTransport struct {
	transport Transport
	framebuf  byteReader    // buffer for reading complete frames off the wire
	buf       bytes.Buffer  // buffers the writes
	reader    *bufio.Reader // just a buffer over the underlying transport
	frameSize uint32        // Current remaining size of the frame. if ==0 read next frame header
	rBuffer   [4]byte       // used for reading
	wBuffer   [4]byte       // used for writing
	maxLength uint32
}

type framedTransportFactory struct {
	factory   TransportFactory
	maxLength uint32
}

func NewFramedTransportFactory(factory TransportFactory) TransportFactory {
	return &framedTransportFactory{factory: factory, maxLength: DEFAULT_MAX_LENGTH}
}

func NewFramedTransportFactoryMaxLength(factory TransportFactory, maxLength uint32) TransportFactory {
	return &framedTransportFactory{factory: factory, maxLength: maxLength}
}

func (p *framedTransportFactory) GetTransport(base Transport) Transport {
	return NewFramedTransportMaxLength(p.factory.GetTransport(base), p.maxLength)
}

func NewFramedTransport(transport Transport) *FramedTransport {
	return &FramedTransport{transport: transport, reader: bufio.NewReader(transport), maxLength: DEFAULT_MAX_LENGTH}
}

func NewFramedTransportMaxLength(transport Transport, maxLength uint32) *FramedTransport {
	return &FramedTransport{transport: transport, reader: bufio.NewReader(transport), maxLength: maxLength}
}

func (p *FramedTransport) Open() error {
	return p.transport.Open()
}

func (p *FramedTransport) IsOpen() bool {
	return p.transport.IsOpen()
}

func (p *FramedTransport) Close() error {
	return p.transport.Close()
}

func (p *FramedTransport) Read(buf []byte) (l int, err error) {
	if p.frameSize == 0 {
		p.frameSize, err = p.readFrameHeader()
		if err != nil {
			return
		}
	}
	if p.frameSize < uint32(len(buf)) {
		frameSize := p.frameSize
		tmp := make([]byte, p.frameSize)
		l, err = p.Read(tmp)
		copy(buf, tmp)
		if err == nil {
			err = NewTransportExceptionFromError(fmt.Errorf("Not enough frame size %d to read %d bytes", frameSize, len(buf)))
			return
		}
	}
	got, err := p.framebuf.Read(buf)
	p.frameSize = p.frameSize - uint32(got)
	//sanity check
	if p.frameSize < 0 {
		return 0, NewTransportException(UNKNOWN_TRANSPORT_EXCEPTION, "Negative frame size")
	}
	return got, NewTransportExceptionFromError(err)
}

func (p *FramedTransport) ReadByte() (c byte, err error) {
	if p.frameSize == 0 {
		p.frameSize, err = p.readFrameHeader()
		if err != nil {
			return
		}
	}
	if p.frameSize < 1 {
		return 0, NewTransportExceptionFromError(fmt.Errorf("Not enough frame size %d to read %d bytes", p.frameSize, 1))
	}
	c, err = p.framebuf.ReadByte()
	if err == nil {
		p.frameSize--
	}
	return
}

func (p *FramedTransport) Write(buf []byte) (int, error) {
	n, err := p.buf.Write(buf)
	return n, NewTransportExceptionFromError(err)
}

func (p *FramedTransport) WriteByte(c byte) error {
	return p.buf.WriteByte(c)
}

func (p *FramedTransport) WriteString(s string) (n int, err error) {
	return p.buf.WriteString(s)
}

func (p *FramedTransport) Flush() error {
	size := p.buf.Len()
	buf := p.wBuffer[:4]
	binary.BigEndian.PutUint32(buf, uint32(size))
	_, err := p.transport.Write(buf)
	if err != nil {
		return NewTransportExceptionFromError(err)
	}
	if size > 0 {
		if n, err := p.buf.WriteTo(p.transport); err != nil {
			print("Error while flushing write buffer of size ", size, " to transport, only wrote ", n, " bytes: ", err.Error(), "\n")
			return NewTransportExceptionFromError(err)
		}
	}
	err = p.transport.Flush()
	return NewTransportExceptionFromError(err)
}

func (p *FramedTransport) readFrameHeader() (uint32, error) {
	buf := p.rBuffer[:4]
	if _, err := io.ReadFull(p.reader, buf); err != nil {
		return 0, err
	}
	size := binary.BigEndian.Uint32(buf)
	if size < 0 || size > p.maxLength {
		return 0, NewTransportException(UNKNOWN_TRANSPORT_EXCEPTION, fmt.Sprintf("Incorrect frame size (%d)", size))
	}

	framebuf := newLimitedByteReader(p.reader, int64(size))
	out, err := ioutil.ReadAll(framebuf)
	if err != nil {
		return 0, err
	}
	if uint32(len(out)) < size {
		return 0, NewTransportExceptionFromError(fmt.Errorf("Unable to read full frame of size %d", size))
	}
	p.framebuf = newLimitedByteReader(bytes.NewBuffer(out), int64(size))

	return size, nil
}

func (p *FramedTransport) RemainingBytes() (num_bytes uint64) {
	return uint64(p.frameSize)
}
