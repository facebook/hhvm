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
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

const BinaryVersionMask uint32 = 0xffff0000
const BinaryVersion1 uint32 = 0x80010000

type binaryFormat struct {
	binaryEncoder
	binaryDecoder
}

var _ types.Format = (*binaryFormat)(nil)

// NewBinaryFormat creates a new Format handler using a buffer
func NewBinaryFormat(readWriter io.ReadWriter) types.Format {
	return NewBinaryFormatOptions(readWriter, false /* strict read */, true /* strict write */)
}

// NewBinaryFormatOptions creates a new Format handler using a buffer
func NewBinaryFormatOptions(readWriter io.ReadWriter, strictRead, strictWrite bool) types.Format {
	return &binaryFormat{
		binaryDecoder: binaryDecoder{reader: readWriter, strictRead: strictRead},
		binaryEncoder: binaryEncoder{writer: readWriter, strictWrite: strictWrite},
	}
}

func newBinaryEncoder(writer io.Writer) types.Encoder {
	// This default for binaryEncoder's strictWrite matches cpp implementation
	return &binaryEncoder{writer: writer, strictWrite: true}
}

func newBinaryDecoder(reader io.Reader) types.Decoder {
	// This default for binaryEncoder's strictRead matches cpp implementation
	return &binaryDecoder{reader: reader, strictRead: false}
}

// IsBinaryFramed checks if the magic value corresponds to binary proto
func IsBinaryFramed(magic uint32) bool {
	return (magic & BinaryVersionMask) == BinaryVersion1
}

/**
 * Writing Methods
 */

type binaryEncoder struct {
	writer      io.Writer
	strictWrite bool
	buffer      [64]byte
}

func (p *binaryEncoder) WriteMessageBegin(name string, typeID types.MessageType, seqID int32) error {
	if p.strictWrite {
		version := uint32(types.VERSION_1) | uint32(typeID)
		e := p.WriteI32(int32(version))
		if e != nil {
			return e
		}
		e = p.WriteString(name)
		if e != nil {
			return e
		}
		return p.WriteI32(seqID)
	} else {
		e := p.WriteString(name)
		if e != nil {
			return e
		}
		e = p.WriteByte(byte(typeID))
		if e != nil {
			return e
		}
		return p.WriteI32(seqID)
	}
}

func (p *binaryEncoder) WriteMessageEnd() error {
	return nil
}

func (p *binaryEncoder) WriteStructBegin(name string) error {
	return nil
}

func (p *binaryEncoder) WriteStructEnd() error {
	return nil
}

func (p *binaryEncoder) WriteFieldBegin(name string, typeID types.Type, id int16) error {
	e := p.WriteByte(byte(typeID))
	if e != nil {
		return e
	}
	return p.WriteI16(id)
}

func (p *binaryEncoder) WriteFieldEnd() error {
	return nil
}

func (p *binaryEncoder) WriteFieldStop() error {
	return p.WriteByte(byte(types.STOP))
}

func (p *binaryEncoder) WriteMapBegin(keyType types.Type, valueType types.Type, size int) error {
	e := p.WriteByte(byte(keyType))
	if e != nil {
		return e
	}
	e = p.WriteByte(byte(valueType))
	if e != nil {
		return e
	}
	return p.WriteI32(int32(size))
}

func (p *binaryEncoder) WriteMapEnd() error {
	return nil
}

func (p *binaryEncoder) WriteListBegin(elemType types.Type, size int) error {
	e := p.WriteByte(byte(elemType))
	if e != nil {
		return e
	}
	return p.WriteI32(int32(size))
}

func (p *binaryEncoder) WriteListEnd() error {
	return nil
}

func (p *binaryEncoder) WriteSetBegin(elemType types.Type, size int) error {
	e := p.WriteByte(byte(elemType))
	if e != nil {
		return e
	}
	return p.WriteI32(int32(size))
}

func (p *binaryEncoder) WriteSetEnd() error {
	return nil
}

func (p *binaryEncoder) WriteBool(value bool) error {
	if value {
		return p.WriteByte(1)
	}
	return p.WriteByte(0)
}

func (p *binaryEncoder) WriteByte(value byte) error {
	e := writeByte(p.writer, value)
	return types.NewProtocolException(e)
}

func (p *binaryEncoder) WriteI16(value int16) error {
	v := p.buffer[0:2]
	binary.BigEndian.PutUint16(v, uint16(value))
	_, err := p.writer.Write(v)
	return types.NewProtocolException(err)
}

func (p *binaryEncoder) WriteI32(value int32) error {
	v := p.buffer[0:4]
	binary.BigEndian.PutUint32(v, uint32(value))
	_, err := p.writer.Write(v)
	return types.NewProtocolException(err)
}

func (p *binaryEncoder) WriteI64(value int64) error {
	v := p.buffer[0:8]
	binary.BigEndian.PutUint64(v, uint64(value))
	_, err := p.writer.Write(v)
	return types.NewProtocolException(err)
}

func (p *binaryEncoder) WriteDouble(value float64) error {
	return p.WriteI64(int64(math.Float64bits(value)))
}

func (p *binaryEncoder) WriteFloat(value float32) error {
	return p.WriteI32(int32(math.Float32bits(value)))
}

func (p *binaryEncoder) WriteString(value string) error {
	e := p.WriteI32(int32(len(value)))
	if e != nil {
		return e
	}
	_, err := p.writer.Write([]byte(value))
	return types.NewProtocolException(err)
}

func (p *binaryEncoder) WriteBinary(value []byte) error {
	err := p.WriteI32(int32(len(value)))
	if err != nil {
		return err
	}
	if len(value) > 0 {
		_, err = p.writer.Write(value)
	}
	return types.NewProtocolException(err)
}

func (p *binaryEncoder) Flush() error {
	return flush(p.writer)
}

/**
 * Reading methods
 */

type binaryDecoder struct {
	reader     io.Reader
	strictRead bool
	buffer     [64]byte
}

func (p *binaryDecoder) ReadMessageBegin() (name string, typeID types.MessageType, seqID int32, err error) {
	size, e := p.ReadI32()
	if e != nil {
		return "", typeID, 0, types.NewProtocolException(e)
	}
	if size < 0 {
		typeID = types.MessageType(size & 0x0ff)
		version := int64(int64(size) & types.VERSION_MASK)
		if version != types.VERSION_1 {
			return name, typeID, seqID, types.NewProtocolExceptionWithType(types.BAD_VERSION, fmt.Errorf("Bad version in ReadMessageBegin"))
		}
		name, e = p.ReadString()
		if e != nil {
			return name, typeID, seqID, types.NewProtocolException(e)
		}
		seqID, e = p.ReadI32()
		if e != nil {
			return name, typeID, seqID, types.NewProtocolException(e)
		}
		return name, typeID, seqID, nil
	}
	if p.strictRead {
		return name, typeID, seqID, types.NewProtocolExceptionWithType(types.BAD_VERSION, fmt.Errorf("Missing version in ReadMessageBegin"))
	}
	name, e2 := p.readStringBody(size)
	if e2 != nil {
		return name, typeID, seqID, e2
	}
	b, e3 := p.ReadByte()
	if e3 != nil {
		return name, typeID, seqID, e3
	}
	typeID = types.MessageType(b)
	seqID, e4 := p.ReadI32()
	if e4 != nil {
		return name, typeID, seqID, e4
	}
	return name, typeID, seqID, nil
}

func (p *binaryDecoder) ReadMessageEnd() error {
	return nil
}

func (p *binaryDecoder) ReadStructBegin() (string, error) {
	return "", nil
}

func (p *binaryDecoder) ReadStructEnd() error {
	return nil
}

func (p *binaryDecoder) ReadFieldBegin() (name string, typeID types.Type, seqID int16, err error) {
	t, err := p.ReadByte()
	typeID = types.Type(t)
	if err != nil {
		return name, typeID, seqID, err
	}
	if typeID != types.STOP {
		seqID, err = p.ReadI16()
	}
	return name, typeID, seqID, err
}

func (p *binaryDecoder) ReadFieldEnd() error {
	return nil
}

var invalidDataLength = types.NewProtocolExceptionWithType(types.INVALID_DATA, errors.New("Invalid data length"))

func (p *binaryDecoder) ReadMapBegin() (types.Type /* kType */, types.Type /* vType */, int /* size */, error) {
	kTypeByte, err := p.ReadByte()
	if err != nil {
		return 0, 0, 0, types.NewProtocolException(err)
	}
	kType := types.Type(kTypeByte)

	vTypeByte, err := p.ReadByte()
	if err != nil {
		return 0, 0, 0, types.NewProtocolException(err)
	}
	vType := types.Type(vTypeByte)

	size32, err := p.ReadI32()
	if err != nil {
		return 0, 0, 0, types.NewProtocolException(err)
	}
	if size32 < 0 {
		return 0, 0, 0, invalidDataLength
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(size32*2) > remainingBytes || remainingBytes == types.UnknownRemaining {
		return 0, 0, 0, invalidDataLength
	}
	size := int(size32)
	return kType, vType, size, nil
}

func (p *binaryDecoder) ReadMapEnd() error {
	return nil
}

func (p *binaryDecoder) ReadListBegin() (types.Type /* elemType */, int /* size */, error) {
	elemTypeByte, err := p.ReadByte()
	if err != nil {
		return 0, 0, types.NewProtocolException(err)
	}
	elemType := types.Type(elemTypeByte)
	size32, err := p.ReadI32()
	if err != nil {
		return 0, 0, types.NewProtocolException(err)
	}
	if size32 < 0 {
		return 0, 0, invalidDataLength
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(size32) > remainingBytes || remainingBytes == types.UnknownRemaining {
		return 0, 0, invalidDataLength
	}
	size := int(size32)
	return elemType, size, nil
}

func (p *binaryDecoder) ReadListEnd() error {
	return nil
}

func (p *binaryDecoder) ReadSetBegin() (types.Type /* elemType */, int /* size */, error) {
	return p.ReadListBegin()
}

func (p *binaryDecoder) ReadSetEnd() error {
	return nil
}

func (p *binaryDecoder) ReadBool() (bool, error) {
	b, err := p.ReadByte()
	v := true
	if b != 1 {
		v = false
	}
	return v, err
}

func (p *binaryDecoder) ReadByte() (byte, error) {
	return readByte(p.reader)
}

func (p *binaryDecoder) ReadI16() (int16, error) {
	buf := p.buffer[0:2]
	err := p.readAll(buf)
	value := int16(binary.BigEndian.Uint16(buf))
	return value, err
}

func (p *binaryDecoder) ReadI32() (int32, error) {
	buf := p.buffer[0:4]
	err := p.readAll(buf)
	value := int32(binary.BigEndian.Uint32(buf))
	return value, err
}

func (p *binaryDecoder) ReadI64() (int64, error) {
	buf := p.buffer[0:8]
	err := p.readAll(buf)
	value := int64(binary.BigEndian.Uint64(buf))
	return value, err
}

func (p *binaryDecoder) ReadDouble() (float64, error) {
	buf := p.buffer[0:8]
	err := p.readAll(buf)
	value := math.Float64frombits(binary.BigEndian.Uint64(buf))
	return value, err
}

func (p *binaryDecoder) ReadFloat() (float32, error) {
	buf := p.buffer[0:4]
	err := p.readAll(buf)
	value := math.Float32frombits(binary.BigEndian.Uint32(buf))
	return value, err
}

func (p *binaryDecoder) ReadString() (string, error) {
	size, err := p.ReadI32()
	if err != nil {
		return "", err
	}
	if size < 0 {
		return "", invalidDataLength
	}

	return p.readStringBody(size)
}

func (p *binaryDecoder) ReadBinary() ([]byte, error) {
	size32, err := p.ReadI32()
	if err != nil {
		return nil, err
	}
	if size32 < 0 {
		return nil, invalidDataLength
	}
	size := int(size32)
	remainingBytes := remainingBytes(p.reader)
	if uint64(size32) > remainingBytes || remainingBytes == types.UnknownRemaining {
		return nil, invalidDataLength
	}

	buf := make([]byte, size)
	_, err = io.ReadFull(p.reader, buf)
	return buf, types.NewProtocolException(err)
}

func (p *binaryDecoder) Skip(fieldType types.Type) error {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *binaryDecoder) readAll(buf []byte) error {
	_, err := io.ReadFull(p.reader, buf)
	return types.NewProtocolException(err)
}

func (p *binaryDecoder) readStringBody(size int32) (string, error) {
	if size < 0 {
		return "", nil
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(size) > remainingBytes || remainingBytes == types.UnknownRemaining {
		return "", invalidDataLength
	}
	var buf []byte
	if int(size) <= len(p.buffer) {
		buf = p.buffer[0:size]
	} else {
		buf = make([]byte, size)
	}
	_, err := io.ReadFull(p.reader, buf)
	return string(buf), types.NewProtocolException(err)
}
