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
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
)

const BinaryVersionMask uint32 = 0xffff0000
const BinaryVersion1 uint32 = 0x80010000

type BinaryProtocol struct {
	trans         RichTransport
	origTransport Transport
	reader        io.Reader
	writer        io.Writer
	strictRead    bool
	strictWrite   bool
	buffer        [64]byte
}

type BinaryProtocolFactory struct {
	strictRead  bool
	strictWrite bool
}

func NewBinaryProtocolTransport(t Transport) *BinaryProtocol {
	return NewBinaryProtocol(t, false, true)
}

func NewBinaryProtocol(t Transport, strictRead, strictWrite bool) *BinaryProtocol {
	p := &BinaryProtocol{origTransport: t, strictRead: strictRead, strictWrite: strictWrite}
	if et, ok := t.(RichTransport); ok {
		p.trans = et
	} else {
		p.trans = NewRichTransport(t)
	}
	p.reader = p.trans
	p.writer = p.trans
	return p
}

func NewBinaryProtocolFactoryDefault() *BinaryProtocolFactory {
	return NewBinaryProtocolFactory(false, true)
}

func NewBinaryProtocolFactory(strictRead, strictWrite bool) *BinaryProtocolFactory {
	return &BinaryProtocolFactory{strictRead: strictRead, strictWrite: strictWrite}
}

func (p *BinaryProtocolFactory) GetProtocol(t Transport) Protocol {
	return NewBinaryProtocol(t, p.strictRead, p.strictWrite)
}

/**
 * Writing Methods
 */

func (p *BinaryProtocol) WriteMessageBegin(name string, typeId MessageType, seqId int32) error {
	if p.strictWrite {
		version := uint32(VERSION_1) | uint32(typeId)
		e := p.WriteI32(int32(version))
		if e != nil {
			return e
		}
		e = p.WriteString(name)
		if e != nil {
			return e
		}
		e = p.WriteI32(seqId)
		return e
	} else {
		e := p.WriteString(name)
		if e != nil {
			return e
		}
		e = p.WriteByte(byte(typeId))
		if e != nil {
			return e
		}
		e = p.WriteI32(seqId)
		return e
	}
}

func (p *BinaryProtocol) WriteMessageEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteStructBegin(name string) error {
	return nil
}

func (p *BinaryProtocol) WriteStructEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteFieldBegin(name string, typeId Type, id int16) error {
	e := p.WriteByte(byte(typeId))
	if e != nil {
		return e
	}
	e = p.WriteI16(id)
	return e
}

func (p *BinaryProtocol) WriteFieldEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteFieldStop() error {
	e := p.WriteByte(STOP)
	return e
}

func (p *BinaryProtocol) WriteMapBegin(keyType Type, valueType Type, size int) error {
	e := p.WriteByte(byte(keyType))
	if e != nil {
		return e
	}
	e = p.WriteByte(byte(valueType))
	if e != nil {
		return e
	}
	e = p.WriteI32(int32(size))
	return e
}

func (p *BinaryProtocol) WriteMapEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteListBegin(elemType Type, size int) error {
	e := p.WriteByte(byte(elemType))
	if e != nil {
		return e
	}
	e = p.WriteI32(int32(size))
	return e
}

func (p *BinaryProtocol) WriteListEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteSetBegin(elemType Type, size int) error {
	e := p.WriteByte(byte(elemType))
	if e != nil {
		return e
	}
	e = p.WriteI32(int32(size))
	return e
}

func (p *BinaryProtocol) WriteSetEnd() error {
	return nil
}

func (p *BinaryProtocol) WriteBool(value bool) error {
	if value {
		return p.WriteByte(1)
	}
	return p.WriteByte(0)
}

func (p *BinaryProtocol) WriteByte(value byte) error {
	e := p.trans.WriteByte(value)
	return NewProtocolException(e)
}

func (p *BinaryProtocol) WriteI16(value int16) error {
	v := p.buffer[0:2]
	binary.BigEndian.PutUint16(v, uint16(value))
	_, e := p.writer.Write(v)
	return NewProtocolException(e)
}

func (p *BinaryProtocol) WriteI32(value int32) error {
	v := p.buffer[0:4]
	binary.BigEndian.PutUint32(v, uint32(value))
	_, e := p.writer.Write(v)
	return NewProtocolException(e)
}

func (p *BinaryProtocol) WriteI64(value int64) error {
	v := p.buffer[0:8]
	binary.BigEndian.PutUint64(v, uint64(value))
	_, err := p.writer.Write(v)
	return NewProtocolException(err)
}

func (p *BinaryProtocol) WriteDouble(value float64) error {
	return p.WriteI64(int64(math.Float64bits(value)))
}

func (p *BinaryProtocol) WriteFloat(value float32) error {
	return p.WriteI32(int32(math.Float32bits(value)))
}

func (p *BinaryProtocol) WriteString(value string) error {
	e := p.WriteI32(int32(len(value)))
	if e != nil {
		return e
	}
	_, err := p.trans.WriteString(value)
	return NewProtocolException(err)
}

func (p *BinaryProtocol) WriteBinary(value []byte) error {
	e := p.WriteI32(int32(len(value)))
	if e != nil {
		return e
	}
	_, err := p.writer.Write(value)
	return NewProtocolException(err)
}

/**
 * Reading methods
 */

func (p *BinaryProtocol) ReadMessageBegin() (name string, typeId MessageType, seqId int32, err error) {
	size, e := p.ReadI32()
	if e != nil {
		return "", typeId, 0, NewProtocolException(e)
	}
	if size < 0 {
		typeId = MessageType(size & 0x0ff)
		version := int64(int64(size) & VERSION_MASK)
		if version != VERSION_1 {
			return name, typeId, seqId, NewProtocolExceptionWithType(BAD_VERSION, fmt.Errorf("Bad version in ReadMessageBegin"))
		}
		name, e = p.ReadString()
		if e != nil {
			return name, typeId, seqId, NewProtocolException(e)
		}
		seqId, e = p.ReadI32()
		if e != nil {
			return name, typeId, seqId, NewProtocolException(e)
		}
		return name, typeId, seqId, nil
	}
	if p.strictRead {
		return name, typeId, seqId, NewProtocolExceptionWithType(BAD_VERSION, fmt.Errorf("Missing version in ReadMessageBegin"))
	}
	name, e2 := p.readStringBody(size)
	if e2 != nil {
		return name, typeId, seqId, e2
	}
	b, e3 := p.ReadByte()
	if e3 != nil {
		return name, typeId, seqId, e3
	}
	typeId = MessageType(b)
	seqId, e4 := p.ReadI32()
	if e4 != nil {
		return name, typeId, seqId, e4
	}
	return name, typeId, seqId, nil
}

func (p *BinaryProtocol) ReadMessageEnd() error {
	return nil
}

func (p *BinaryProtocol) ReadStructBegin() (name string, err error) {
	return
}

func (p *BinaryProtocol) ReadStructEnd() error {
	return nil
}

func (p *BinaryProtocol) ReadFieldBegin() (name string, typeId Type, seqId int16, err error) {
	t, err := p.ReadByte()
	typeId = Type(t)
	if err != nil {
		return name, typeId, seqId, err
	}
	if t != STOP {
		seqId, err = p.ReadI16()
	}
	return name, typeId, seqId, err
}

func (p *BinaryProtocol) ReadFieldEnd() error {
	return nil
}

var invalidDataLength = NewProtocolExceptionWithType(INVALID_DATA, errors.New("Invalid data length"))

func (p *BinaryProtocol) ReadMapBegin() (kType, vType Type, size int, err error) {
	k, e := p.ReadByte()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	kType = Type(k)
	v, e := p.ReadByte()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	vType = Type(v)
	size32, e := p.ReadI32()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	if size32 < 0 {
		err = invalidDataLength
		return
	}
	if uint64(size32*2) > p.trans.RemainingBytes() || p.trans.RemainingBytes() == UnknownRemaining {
		err = invalidDataLength
		return
	}
	size = int(size32)
	return kType, vType, size, nil
}

func (p *BinaryProtocol) ReadMapEnd() error {
	return nil
}

func (p *BinaryProtocol) ReadListBegin() (elemType Type, size int, err error) {
	b, e := p.ReadByte()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	elemType = Type(b)
	size32, e := p.ReadI32()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	if size32 < 0 {
		err = invalidDataLength
		return
	}
	if uint64(size32) > p.trans.RemainingBytes() || p.trans.RemainingBytes() == UnknownRemaining {
		err = invalidDataLength
		return
	}
	size = int(size32)

	return
}

func (p *BinaryProtocol) ReadListEnd() error {
	return nil
}

func (p *BinaryProtocol) ReadSetBegin() (elemType Type, size int, err error) {
	b, e := p.ReadByte()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	elemType = Type(b)
	size32, e := p.ReadI32()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	if size32 < 0 {
		err = invalidDataLength
		return
	}
	if uint64(size32) > p.trans.RemainingBytes() || p.trans.RemainingBytes() == UnknownRemaining {
		err = invalidDataLength
		return
	}
	size = int(size32)
	return elemType, size, nil
}

func (p *BinaryProtocol) ReadSetEnd() error {
	return nil
}

func (p *BinaryProtocol) ReadBool() (bool, error) {
	b, e := p.ReadByte()
	v := true
	if b != 1 {
		v = false
	}
	return v, e
}

func (p *BinaryProtocol) ReadByte() (byte, error) {
	v, err := p.trans.ReadByte()
	return byte(v), err
}

func (p *BinaryProtocol) ReadI16() (value int16, err error) {
	buf := p.buffer[0:2]
	err = p.readAll(buf)
	value = int16(binary.BigEndian.Uint16(buf))
	return value, err
}

func (p *BinaryProtocol) ReadI32() (value int32, err error) {
	buf := p.buffer[0:4]
	err = p.readAll(buf)
	value = int32(binary.BigEndian.Uint32(buf))
	return value, err
}

func (p *BinaryProtocol) ReadI64() (value int64, err error) {
	buf := p.buffer[0:8]
	err = p.readAll(buf)
	value = int64(binary.BigEndian.Uint64(buf))
	return value, err
}

func (p *BinaryProtocol) ReadDouble() (value float64, err error) {
	buf := p.buffer[0:8]
	err = p.readAll(buf)
	value = math.Float64frombits(binary.BigEndian.Uint64(buf))
	return value, err
}

func (p *BinaryProtocol) ReadFloat() (value float32, err error) {
	buf := p.buffer[0:4]
	err = p.readAll(buf)
	value = math.Float32frombits(binary.BigEndian.Uint32(buf))
	return value, err
}

func (p *BinaryProtocol) ReadString() (value string, err error) {
	size, e := p.ReadI32()
	if e != nil {
		return "", e
	}
	if size < 0 {
		err = invalidDataLength
		return
	}

	return p.readStringBody(size)
}

func (p *BinaryProtocol) ReadBinary() ([]byte, error) {
	size, e := p.ReadI32()
	if e != nil {
		return nil, e
	}
	if size < 0 {
		return nil, invalidDataLength
	}
	if uint64(size) > p.trans.RemainingBytes() || p.trans.RemainingBytes() == UnknownRemaining {
		return nil, invalidDataLength
	}

	isize := int(size)
	buf := make([]byte, isize)
	_, err := io.ReadFull(p.trans, buf)
	return buf, NewProtocolException(err)
}

func (p *BinaryProtocol) Flush() (err error) {
	return NewProtocolException(p.trans.Flush())
}

func (p *BinaryProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *BinaryProtocol) Transport() Transport {
	return p.origTransport
}

func (p *BinaryProtocol) readAll(buf []byte) error {
	_, err := io.ReadFull(p.reader, buf)
	return NewProtocolException(err)
}

func (p *BinaryProtocol) readStringBody(size int32) (value string, err error) {
	if size < 0 {
		return "", nil
	}
	if uint64(size) > p.trans.RemainingBytes() || p.trans.RemainingBytes() == UnknownRemaining {
		return "", invalidDataLength
	}
	var buf []byte
	if int(size) <= len(p.buffer) {
		buf = p.buffer[0:size]
	} else {
		buf = make([]byte, size)
	}
	_, e := io.ReadFull(p.trans, buf)
	return string(buf), NewProtocolException(e)
}
