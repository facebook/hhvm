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
	"fmt"
	"io"
	"math"
)

const (
	COMPACT_PROTOCOL_ID       = 0x082
	COMPACT_VERSION           = 0x01
	COMPACT_VERSION_BE        = 0x02
	COMPACT_VERSION_MASK      = 0x1f
	COMPACT_TYPE_MASK         = 0x0E0
	COMPACT_TYPE_BITS         = 0x07
	COMPACT_TYPE_SHIFT_AMOUNT = 5
)

type compactType byte

const (
	COMPACT_BOOLEAN_TRUE  = 0x01
	COMPACT_BOOLEAN_FALSE = 0x02
	COMPACT_BYTE          = 0x03
	COMPACT_I16           = 0x04
	COMPACT_I32           = 0x05
	COMPACT_I64           = 0x06
	COMPACT_DOUBLE        = 0x07
	COMPACT_BINARY        = 0x08
	COMPACT_LIST          = 0x09
	COMPACT_SET           = 0x0A
	COMPACT_MAP           = 0x0B
	COMPACT_STRUCT        = 0x0C
	COMPACT_FLOAT         = 0x0D
)

var (
	typeToCompactType map[Type]compactType
)

func init() {
	typeToCompactType = map[Type]compactType{
		STOP:   STOP,
		BOOL:   COMPACT_BOOLEAN_TRUE,
		BYTE:   COMPACT_BYTE,
		I16:    COMPACT_I16,
		I32:    COMPACT_I32,
		I64:    COMPACT_I64,
		DOUBLE: COMPACT_DOUBLE,
		FLOAT:  COMPACT_FLOAT,
		STRING: COMPACT_BINARY,
		LIST:   COMPACT_LIST,
		SET:    COMPACT_SET,
		MAP:    COMPACT_MAP,
		STRUCT: COMPACT_STRUCT,
	}
}

type compactProtocol struct {
	trans RichTransport
	compactEncoder
	compactDecoder
}

// NewCompactProtocol creates a CompactProtocol
func NewCompactProtocol(trans io.ReadWriteCloser) Format {
	p := &compactProtocol{}
	if et, ok := trans.(RichTransport); ok {
		p.trans = et
	} else {
		p.trans = newRichTransport(trans)
	}
	p.compactDecoder.version = COMPACT_VERSION_BE
	p.compactDecoder.reader = p.trans
	p.compactEncoder.version = COMPACT_VERSION_BE
	p.compactEncoder.writer = p.trans
	return p
}

func newCompactEncoder(writer io.Writer) Encoder {
	return &compactEncoder{writer: writer, version: COMPACT_VERSION_BE}
}

func newCompactDecoder(reader io.Reader) Decoder {
	return &compactDecoder{reader: reader, version: COMPACT_VERSION_BE}
}

func (p *compactProtocol) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	// The version can be updated in compactEncoder's ReadMessageBegin, so we need to update it in p.compactEncoder before writing a new message.
	p.compactEncoder.version = p.compactDecoder.version
	return p.compactEncoder.WriteMessageBegin(name, typeID, seqid)
}

type compactDecoder struct {
	reader io.Reader
	// Used to keep track of the last field for the current and previous structs,
	// so we can do the delta stuff.
	lastFieldRead   []int
	lastFieldIDRead int

	// If we read a field header, and it's a boolean field, save the boolean
	// value here so that readBool can use it. This is only used for reads.
	boolValue          bool
	boolValueIsNotNull bool

	rBuffer [64]byte // reading

	version int
}

type compactEncoder struct {
	writer io.Writer
	// Used to keep track of the last field for the current and previous structs,
	// so we can do the delta stuff.
	lastFieldWritten   []int
	lastFieldIDWritten int

	// If we encounter a boolean field begin, save the Field here so it can
	// have the value incorporated. This is only used for writes.
	booleanFieldName    string
	booleanFieldID      int16
	booleanFieldPending bool

	wBuffer [64]byte // writing

	version int
}

//
// Public Writing methods.
//

// Write a message header to the wire. Compact Protocol messages contain the
// protocol version so we can migrate forwards in the future if need be.
func (p *compactEncoder) WriteMessageBegin(name string, typeID MessageType, seqid int32) error {
	err := p.writeByteDirect(COMPACT_PROTOCOL_ID)
	if err != nil {
		return NewProtocolException(err)
	}
	err = p.writeByteDirect((byte(p.version) & COMPACT_VERSION_MASK) | ((byte(typeID) << COMPACT_TYPE_SHIFT_AMOUNT) & COMPACT_TYPE_MASK))
	if err != nil {
		return NewProtocolException(err)
	}
	_, err = p.writeVarint32(seqid)
	if err != nil {
		return NewProtocolException(err)
	}
	e := p.WriteString(name)
	return e

}

func (p *compactEncoder) WriteMessageEnd() error { return nil }

// Write a struct begin. This doesn't actually put anything on the wire. We
// use it as an opportunity to put special placeholder markers on the field
// stack so we can get the field id deltas correct.
func (p *compactEncoder) WriteStructBegin(name string) error {
	p.lastFieldWritten = append(p.lastFieldWritten, p.lastFieldIDWritten)
	p.lastFieldIDWritten = 0
	return nil
}

// Write a struct end. This doesn't actually put anything on the wire. We use
// this as an opportunity to pop the last field from the current struct off
// of the field stack.
func (p *compactEncoder) WriteStructEnd() error {
	p.lastFieldIDWritten = p.lastFieldWritten[len(p.lastFieldWritten)-1]
	p.lastFieldWritten = p.lastFieldWritten[:len(p.lastFieldWritten)-1]
	return nil
}

func (p *compactEncoder) WriteFieldBegin(name string, typeID Type, id int16) error {
	if typeID == BOOL {
		// we want to possibly include the value, so we'll wait.
		p.booleanFieldName, p.booleanFieldID, p.booleanFieldPending = name, id, true
		return nil
	}
	_, err := p.writeFieldBeginInternal(name, typeID, id, 0xFF)
	return NewProtocolException(err)
}

// The workhorse of writeFieldBegin. It has the option of doing a
// 'type override' of the type header. This is used specifically in the
// boolean field case.
func (p *compactEncoder) writeFieldBeginInternal(name string, typeID Type, id int16, typeOverride byte) (int, error) {
	// short lastFieldWritten = lastFieldWritten_.pop();

	// if there's a type override, use that.
	var typeToWrite byte
	if typeOverride == 0xFF {
		typeToWrite = byte(typeToCompactType[typeID])
	} else {
		typeToWrite = typeOverride
	}
	// check if we can use delta encoding for the field id
	fieldID := int(id)
	written := 0
	if fieldID > p.lastFieldIDWritten && fieldID-p.lastFieldIDWritten <= 15 {
		// write them together
		err := p.writeByteDirect(byte((fieldID-p.lastFieldIDWritten)<<4) | typeToWrite)
		if err != nil {
			return 0, err
		}
	} else {
		// write them separate
		err := p.writeByteDirect(typeToWrite)
		if err != nil {
			return 0, err
		}
		err = p.WriteI16(id)
		written = 1 + 2
		if err != nil {
			return 0, err
		}
	}

	p.lastFieldIDWritten = fieldID
	return written, nil
}

func (p *compactEncoder) WriteFieldEnd() error { return nil }

func (p *compactEncoder) WriteFieldStop() error {
	err := p.writeByteDirect(STOP)
	return NewProtocolException(err)
}

func (p *compactEncoder) WriteMapBegin(keyType Type, valueType Type, size int) error {
	if size == 0 {
		err := p.writeByteDirect(0)
		return NewProtocolException(err)
	}
	_, err := p.writeVarint32(int32(size))
	if err != nil {
		return NewProtocolException(err)
	}
	err = p.writeByteDirect(byte(typeToCompactType[keyType])<<4 | byte(typeToCompactType[valueType]))
	return NewProtocolException(err)
}

func (p *compactEncoder) WriteMapEnd() error { return nil }

// Write a list header.
func (p *compactEncoder) WriteListBegin(elemType Type, size int) error {
	_, err := p.writeCollectionBegin(elemType, size)
	return NewProtocolException(err)
}

func (p *compactEncoder) WriteListEnd() error { return nil }

// Write a set header.
func (p *compactEncoder) WriteSetBegin(elemType Type, size int) error {
	_, err := p.writeCollectionBegin(elemType, size)
	return NewProtocolException(err)
}

func (p *compactEncoder) WriteSetEnd() error { return nil }

func (p *compactEncoder) WriteBool(value bool) error {
	v := byte(COMPACT_BOOLEAN_FALSE)
	if value {
		v = byte(COMPACT_BOOLEAN_TRUE)
	}
	if p.booleanFieldPending {
		// we haven't written the field header yet
		_, err := p.writeFieldBeginInternal(p.booleanFieldName, BOOL, p.booleanFieldID, v)
		p.booleanFieldPending = false
		return NewProtocolException(err)
	}
	// we're not part of a field, so just write the value.
	err := p.writeByteDirect(v)
	return NewProtocolException(err)
}

// Write a byte. Nothing to see here!
func (p *compactEncoder) WriteByte(value byte) error {
	err := p.writeByteDirect(value)
	return NewProtocolException(err)
}

// Write an I16 as a zigzag varint.
func (p *compactEncoder) WriteI16(value int16) error {
	_, err := p.writeVarint32(p.int32ToZigzag(int32(value)))
	return NewProtocolException(err)
}

// Write an i32 as a zigzag varint.
func (p *compactEncoder) WriteI32(value int32) error {
	_, err := p.writeVarint32(p.int32ToZigzag(value))
	return NewProtocolException(err)
}

// Write an i64 as a zigzag varint.
func (p *compactEncoder) WriteI64(value int64) error {
	_, err := p.writeVarint64(p.int64ToZigzag(value))
	return NewProtocolException(err)
}

// Write a double to the wire as 8 bytes.
func (p *compactEncoder) WriteDouble(value float64) error {
	buf := p.wBuffer[0:8]
	if p.version == COMPACT_VERSION {
		binary.LittleEndian.PutUint64(buf, math.Float64bits(value))
	} else {
		binary.BigEndian.PutUint64(buf, math.Float64bits(value))
	}
	_, err := p.writer.Write(buf)
	return NewProtocolException(err)
}

// Write a float to the wire as 4 bytes.
func (p *compactEncoder) WriteFloat(value float32) error {
	buf := p.wBuffer[0:4]
	binary.BigEndian.PutUint32(buf, math.Float32bits(value))
	_, err := p.writer.Write(buf)
	return NewProtocolException(err)
}

// Write a string to the wire with a varint size preceding.
func (p *compactEncoder) WriteString(value string) error {
	_, err := p.writeVarint32(int32(len(value)))
	if err != nil {
		return NewProtocolException(err)
	}
	_, err = p.writer.Write([]byte(value))
	return err
}

// Write a byte array, using a varint for the size.
func (p *compactEncoder) WriteBinary(bin []byte) error {
	_, e := p.writeVarint32(int32(len(bin)))
	if e != nil {
		return NewProtocolException(e)
	}
	if len(bin) > 0 {
		_, e = p.writer.Write(bin)
		return NewProtocolException(e)
	}
	return nil
}

//
// Reading methods.
//

// Read a message header.
func (p *compactDecoder) ReadMessageBegin() (name string, typeID MessageType, seqID int32, err error) {

	protocolID, err := p.ReadByte()
	if err != nil {
		return
	}

	if protocolID != COMPACT_PROTOCOL_ID {
		e := fmt.Errorf("Expected protocol id %02x but got %02x", COMPACT_PROTOCOL_ID, protocolID)
		return "", typeID, seqID, NewProtocolExceptionWithType(BAD_VERSION, e)
	}

	versionAndType, err := p.ReadByte()
	if err != nil {
		return
	}

	version := versionAndType & COMPACT_VERSION_MASK
	typeID = MessageType((versionAndType >> COMPACT_TYPE_SHIFT_AMOUNT) & COMPACT_TYPE_BITS)
	if version == COMPACT_VERSION || version == COMPACT_VERSION_BE {
		p.version = int(version)
	} else {
		e := fmt.Errorf("Expected version %02x or %02x but got %02x", COMPACT_VERSION, COMPACT_VERSION_BE, version)
		err = NewProtocolExceptionWithType(BAD_VERSION, e)
		return
	}
	seqID, e := p.readVarint32()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	name, err = p.ReadString()
	return
}

func (p *compactDecoder) ReadMessageEnd() error { return nil }

// Read a struct begin. There's nothing on the wire for this, but it is our
// opportunity to push a new struct begin marker onto the field stack.
func (p *compactDecoder) ReadStructBegin() (name string, err error) {
	p.lastFieldRead = append(p.lastFieldRead, p.lastFieldIDRead)
	p.lastFieldIDRead = 0
	return
}

// Doesn't actually consume any wire data, just removes the last field for
// this struct from the field stack.
func (p *compactDecoder) ReadStructEnd() error {
	// consume the last field we read off the wire.
	p.lastFieldIDRead = p.lastFieldRead[len(p.lastFieldRead)-1]
	p.lastFieldRead = p.lastFieldRead[:len(p.lastFieldRead)-1]
	return nil
}

// Read a field header off the wire.
func (p *compactDecoder) ReadFieldBegin() (name string, typeID Type, id int16, err error) {
	t, err := p.ReadByte()
	if err != nil {
		return
	}

	// if it's a stop, then we can return immediately, as the struct is over.
	if (t & 0x0f) == STOP {
		return "", STOP, 0, nil
	}
	// mask off the 4 MSB of the type header. it could contain a field id delta.
	modifier := int16((t & 0xf0) >> 4)
	if modifier == 0 {
		// not a delta. look ahead for the zigzag varint field id.
		id, err = p.ReadI16()
		if err != nil {
			return
		}
	} else {
		// has a delta. add the delta to the last read field id.
		id = int16(p.lastFieldIDRead) + modifier
	}
	typeID, e := compactToThriftType(compactType(t & 0x0f))
	if e != nil {
		err = NewProtocolException(e)
		return
	}

	// if this happens to be a boolean field, the value is encoded in the type
	if p.isBoolType(t) {
		// save the boolean value in a special instance variable.
		p.boolValue = (byte(t)&0x0f == COMPACT_BOOLEAN_TRUE)
		p.boolValueIsNotNull = true
	}

	// push the new field onto the field stack so we can keep the deltas going.
	p.lastFieldIDRead = int(id)
	return
}

func (p *compactDecoder) ReadFieldEnd() error { return nil }

// Read a map header off the wire. If the size is zero, skip reading the key
// and value type. This means that 0-length maps will yield Maps without the
// "correct" types.
func (p *compactDecoder) ReadMapBegin() (keyType Type, valueType Type, size int, err error) {
	size32, e := p.readVarint32()
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	if size32 < 0 {
		err = invalidDataLength
		return
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(size32*2) > remainingBytes || remainingBytes == UnknownRemaining {
		err = invalidDataLength
		return
	}
	size = int(size32)

	keyAndValueType := byte(STOP)
	if size != 0 {
		keyAndValueType, err = p.ReadByte()
		if err != nil {
			return
		}
	}
	keyType, _ = compactToThriftType(compactType(keyAndValueType >> 4))
	valueType, _ = compactToThriftType(compactType(keyAndValueType & 0xf))
	return
}

func (p *compactDecoder) ReadMapEnd() error { return nil }

// Read a list header off the wire. If the list size is 0-14, the size will
// be packed into the element type header. If it's a longer list, the 4 MSB
// of the element type header will be 0xF, and a varint will follow with the
// true size.
func (p *compactDecoder) ReadListBegin() (elemType Type, size int, err error) {
	size_and_type, err := p.ReadByte()
	if err != nil {
		return
	}
	size = int((size_and_type >> 4) & 0x0f)
	if size == 15 {
		size2, e := p.readVarint32()
		if e != nil {
			err = NewProtocolException(e)
			return
		}
		if size2 < 0 {
			err = invalidDataLength
			return
		}
		size = int(size2)
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(size) > remainingBytes || remainingBytes == UnknownRemaining {
		err = invalidDataLength
		return
	}

	elemType, e := compactToThriftType(compactType(size_and_type))
	if e != nil {
		err = NewProtocolException(e)
		return
	}
	return
}

func (p *compactDecoder) ReadListEnd() error { return nil }

// Read a set header off the wire. If the set size is 0-14, the size will
// be packed into the element type header. If it's a longer set, the 4 MSB
// of the element type header will be 0xF, and a varint will follow with the
// true size.
func (p *compactDecoder) ReadSetBegin() (elemType Type, size int, err error) {
	return p.ReadListBegin()
}

func (p *compactDecoder) ReadSetEnd() error { return nil }

// Read a boolean off the wire. If this is a boolean field, the value should
// already have been read during readFieldBegin, so we'll just consume the
// pre-stored value. Otherwise, read a byte.
func (p *compactDecoder) ReadBool() (value bool, err error) {
	if p.boolValueIsNotNull {
		p.boolValueIsNotNull = false
		return p.boolValue, nil
	}
	v, err := p.ReadByte()
	return v == COMPACT_BOOLEAN_TRUE, err
}

// Read a single byte off the wire. Nothing interesting here.
func (p *compactDecoder) ReadByte() (byte, error) {
	return readByte(p.reader)
}

// Read an i16 from the wire as a zigzag varint.
func (p *compactDecoder) ReadI16() (value int16, err error) {
	v, err := p.ReadI32()
	return int16(v), err
}

// Read an i32 from the wire as a zigzag varint.
func (p *compactDecoder) ReadI32() (value int32, err error) {
	v, e := p.readVarint32()
	if e != nil {
		return 0, NewProtocolException(e)
	}
	value = p.zigzagToInt32(v)
	return value, nil
}

// Read an i64 from the wire as a zigzag varint.
func (p *compactDecoder) ReadI64() (value int64, err error) {
	v, e := p.readVarint64()
	if e != nil {
		return 0, NewProtocolException(e)
	}
	value = p.zigzagToInt64(v)
	return value, nil
}

// No magic here - just read a double off the wire.
func (p *compactDecoder) ReadDouble() (value float64, err error) {
	longBits := p.rBuffer[0:8]
	_, e := io.ReadFull(p.reader, longBits)
	if e != nil {
		return 0.0, NewProtocolException(e)
	}
	if p.version == COMPACT_VERSION {
		return math.Float64frombits(binary.LittleEndian.Uint64(longBits)), nil
	}
	return math.Float64frombits(binary.BigEndian.Uint64(longBits)), nil
}

// No magic here - just read a float off the wire.
func (p *compactDecoder) ReadFloat() (value float32, err error) {
	bits := p.rBuffer[0:4]
	_, e := io.ReadFull(p.reader, bits)
	if e != nil {
		return 0.0, NewProtocolException(e)
	}
	return math.Float32frombits(binary.BigEndian.Uint32(bits)), nil
}

// Reads a []byte (via readBinary), and then UTF-8 decodes it.
func (p *compactDecoder) ReadString() (value string, err error) {
	length, e := p.readVarint32()
	if e != nil {
		return "", NewProtocolException(e)
	}
	if length < 0 {
		return "", invalidDataLength
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(length) > remainingBytes || remainingBytes == UnknownRemaining {
		return "", invalidDataLength
	}

	if length == 0 {
		return "", nil
	}
	var buf []byte
	if length <= int32(len(p.rBuffer)) {
		buf = p.rBuffer[0:length]
	} else {
		buf = make([]byte, length)
	}
	_, e = io.ReadFull(p.reader, buf)
	return string(buf), NewProtocolException(e)
}

// Read a []byte from the wire.
func (p *compactDecoder) ReadBinary() (value []byte, err error) {
	length, e := p.readVarint32()
	if e != nil {
		return nil, NewProtocolException(e)
	}
	if length == 0 {
		return []byte{}, nil
	}
	if length < 0 {
		return nil, invalidDataLength
	}
	remainingBytes := remainingBytes(p.reader)
	if uint64(length) > remainingBytes || remainingBytes == UnknownRemaining {
		return nil, invalidDataLength
	}

	buf := make([]byte, length)
	_, e = io.ReadFull(p.reader, buf)
	return buf, NewProtocolException(e)
}

func (p *compactEncoder) Flush() (err error) {
	flusher, ok := p.writer.(Flusher)
	if !ok {
		return nil
	}
	return NewProtocolException(flusher.Flush())
}

func (p *compactDecoder) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

//
// Internal writing methods
//

// Abstract method for writing the start of lists and sets. List and sets on
// the wire differ only by the type indicator.
func (p *compactEncoder) writeCollectionBegin(elemType Type, size int) (int, error) {
	if size <= 14 {
		return 1, p.writeByteDirect(byte(int32(size<<4) | int32(typeToCompactType[elemType])))
	}
	err := p.writeByteDirect(0xf0 | byte(typeToCompactType[elemType]))
	if err != nil {
		return 0, err
	}
	m, err := p.writeVarint32(int32(size))
	return 1 + m, err
}

// Write an i32 as a varint. Results in 1-5 bytes on the wire.
// TODO(pomack): make a permanent buffer like writeVarint64?
func (p *compactEncoder) writeVarint32(n int32) (int, error) {
	i32buf := p.wBuffer[0:5]
	idx := binary.PutUvarint(i32buf, uint64(uint32(n)))
	return p.writer.Write(i32buf[0:idx])
}

// Write an i64 as a varint. Results in 1-10 bytes on the wire.
func (p *compactEncoder) writeVarint64(n int64) (int, error) {
	varint64out := p.wBuffer[0:10]
	idx := binary.PutUvarint(varint64out, uint64(n))
	return p.writer.Write(varint64out[0:idx])
}

// Convert l into a zigzag long. This allows negative numbers to be
// represented compactly as a varint.
func (p *compactEncoder) int64ToZigzag(l int64) int64 {
	return (l << 1) ^ (l >> 63)
}

// Convert l into a zigzag long. This allows negative numbers to be
// represented compactly as a varint.
func (p *compactEncoder) int32ToZigzag(n int32) int32 {
	return (n << 1) ^ (n >> 31)
}

func (p *compactEncoder) fixedUint64ToBytes(n uint64, buf []byte) {
	binary.LittleEndian.PutUint64(buf, n)
}

func (p *compactEncoder) fixedInt64ToBytes(n int64, buf []byte) {
	binary.LittleEndian.PutUint64(buf, uint64(n))
}

// Writes a byte without any possibility of all that field header nonsense.
// Used internally by other writing methods that know they need to write a byte.
func (p *compactEncoder) writeByteDirect(b byte) error {
	return writeByte(p.writer, b)
}

// Writes a byte without any possibility of all that field header nonsense.
func (p *compactEncoder) writeIntAsByteDirect(n int) (int, error) {
	return 1, p.writeByteDirect(byte(n))
}

//
// Internal reading methods
//

// Read an i32 from the wire as a varint. The MSB of each byte is set
// if there is another byte to follow. This can read up to 5 bytes.
func (p *compactDecoder) readVarint32() (int32, error) {
	// if the wire contains the right stuff, this will just truncate the i64 we
	// read and get us the right sign.
	v, err := p.readVarint64()
	return int32(v), err
}

// Read an i64 from the wire as a proper varint. The MSB of each byte is set
// if there is another byte to follow. This can read up to 10 bytes.
func (p *compactDecoder) readVarint64() (int64, error) {
	ux, err := binary.ReadUvarint(p)
	x := int64(ux) // ok to continue in presence of error
	return x, err
}

//
// encoding helpers
//

// Convert from zigzag int to int.
func (p *compactDecoder) zigzagToInt32(n int32) int32 {
	u := uint32(n)
	return int32(u>>1) ^ -(n & 1)
}

// Convert from zigzag long to long.
func (p *compactDecoder) zigzagToInt64(n int64) int64 {
	u := uint64(n)
	return int64(u>>1) ^ -(n & 1)
}

func (p *compactDecoder) bytesToInt32(b []byte) int32 {
	return int32(binary.LittleEndian.Uint32(b))
}

func (p *compactDecoder) bytesToUint32(b []byte) uint32 {
	return binary.LittleEndian.Uint32(b)

}

// Note that it's important that the mask bytes are long literals,
// otherwise they'll default to ints, and when you shift an int left 56 bits,
// you just get a messed up int.
func (p *compactDecoder) bytesToInt64(b []byte) int64 {
	return int64(binary.LittleEndian.Uint64(b))
}

// Note that it's important that the mask bytes are long literals,
// otherwise they'll default to ints, and when you shift an int left 56 bits,
// you just get a messed up int.
func (p *compactDecoder) bytesToUint64(b []byte) uint64 {
	return binary.LittleEndian.Uint64(b)
}

//
// type testing and converting
//

func (p *compactDecoder) isBoolType(b byte) bool {
	return (b&0x0f) == COMPACT_BOOLEAN_TRUE || (b&0x0f) == COMPACT_BOOLEAN_FALSE
}

// Given a compactType constant, convert it to its corresponding
// Type value.
func compactToThriftType(t compactType) (Type, error) {
	switch byte(t) & 0x0f {
	case STOP:
		return STOP, nil
	case COMPACT_BOOLEAN_FALSE, COMPACT_BOOLEAN_TRUE:
		return BOOL, nil
	case COMPACT_BYTE:
		return BYTE, nil
	case COMPACT_I16:
		return I16, nil
	case COMPACT_I32:
		return I32, nil
	case COMPACT_I64:
		return I64, nil
	case COMPACT_DOUBLE:
		return DOUBLE, nil
	case COMPACT_FLOAT:
		return FLOAT, nil
	case COMPACT_BINARY:
		return STRING, nil
	case COMPACT_LIST:
		return LIST, nil
	case COMPACT_SET:
		return SET, nil
	case COMPACT_MAP:
		return MAP, nil
	case COMPACT_STRUCT:
		return STRUCT, nil
	}
	return STOP, Exception(fmt.Errorf("don't know what type: %#x", t&0x0f))
}
