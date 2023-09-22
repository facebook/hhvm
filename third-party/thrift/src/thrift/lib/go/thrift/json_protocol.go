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
	"encoding/base64"
	"fmt"
)

const (
	THRIFT_JSON_PROTOCOL_VERSION = 1
)

// for references to _ParseContext see simple_json_protocol.go

// JSONProtocol is the Compact JSON protocol implementation for thrift.
//
// This protocol produces/consumes a compact JSON output with field numbers as
// object keys and field values lightly encoded.
//
// Example: With the Message definition
//
//   struct Message {
//     1: bool aBool
//     2: map<string, bool> aBoolStringMap
//   },
//
//   Message(aBool=True, aBoolStringMap={"key1": True, "key2": False})
//
// will be encoded as:
//
//   {"1":{"tf":1},"2":{"map":["str","tf",2,{"key1": 1,"key2":0}]}}'
type JSONProtocol struct {
	*SimpleJSONProtocol
}

// Constructor
func NewJSONProtocol(t Transport) *JSONProtocol {
	v := &JSONProtocol{SimpleJSONProtocol: NewSimpleJSONProtocol(t)}
	v.parseContextStack = append(v.parseContextStack, int(_CONTEXT_IN_TOPLEVEL))
	v.dumpContext = append(v.dumpContext, int(_CONTEXT_IN_TOPLEVEL))
	return v
}

// Factory
type JSONProtocolFactory struct{}

func (p *JSONProtocolFactory) GetProtocol(trans Transport) Protocol {
	return NewJSONProtocol(trans)
}

func NewJSONProtocolFactory() *JSONProtocolFactory {
	return &JSONProtocolFactory{}
}

func (p *JSONProtocol) WriteMessageBegin(name string, typeID MessageType, seqID int32) error {
	p.resetContextStack() // THRIFT-3735
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	if e := p.WriteI32(THRIFT_JSON_PROTOCOL_VERSION); e != nil {
		return e
	}
	if e := p.WriteString(name); e != nil {
		return e
	}
	if e := p.WriteByte(byte(typeID)); e != nil {
		return e
	}
	if e := p.WriteI32(seqID); e != nil {
		return e
	}
	return nil
}

func (p *JSONProtocol) WriteMessageEnd() error {
	return p.OutputListEnd()
}

func (p *JSONProtocol) WriteStructBegin(name string) error {
	if e := p.OutputObjectBegin(); e != nil {
		return e
	}
	return nil
}

func (p *JSONProtocol) WriteStructEnd() error {
	return p.OutputObjectEnd()
}

func (p *JSONProtocol) WriteFieldBegin(name string, typeID Type, id int16) error {
	if e := p.WriteI16(id); e != nil {
		return e
	}
	if e := p.OutputObjectBegin(); e != nil {
		return e
	}
	s, e1 := p.TypeIdToString(typeID)
	if e1 != nil {
		return e1
	}
	if e := p.WriteString(s); e != nil {
		return e
	}
	return nil
}

func (p *JSONProtocol) WriteFieldEnd() error {
	return p.OutputObjectEnd()
}

func (p *JSONProtocol) WriteFieldStop() error { return nil }

func (p *JSONProtocol) WriteMapBegin(keyType Type, valueType Type, size int) error {
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	s, e1 := p.TypeIdToString(keyType)
	if e1 != nil {
		return e1
	}
	if e := p.WriteString(s); e != nil {
		return e
	}
	s, e1 = p.TypeIdToString(valueType)
	if e1 != nil {
		return e1
	}
	if e := p.WriteString(s); e != nil {
		return e
	}
	if e := p.WriteI64(int64(size)); e != nil {
		return e
	}
	return p.OutputObjectBegin()
}

func (p *JSONProtocol) WriteMapEnd() error {
	if e := p.OutputObjectEnd(); e != nil {
		return e
	}
	return p.OutputListEnd()
}

func (p *JSONProtocol) WriteListBegin(elemType Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *JSONProtocol) WriteListEnd() error {
	return p.OutputListEnd()
}

func (p *JSONProtocol) WriteSetBegin(elemType Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *JSONProtocol) WriteSetEnd() error {
	return p.OutputListEnd()
}

func (p *JSONProtocol) WriteBool(b bool) error {
	if b {
		return p.WriteI32(1)
	}
	return p.WriteI32(0)
}

func (p *JSONProtocol) WriteByte(b byte) error {
	return p.WriteI32(int32(b))
}

func (p *JSONProtocol) WriteI16(v int16) error {
	return p.WriteI32(int32(v))
}

func (p *JSONProtocol) WriteI32(v int32) error {
	return p.OutputI64(int64(v))
}

func (p *JSONProtocol) WriteI64(v int64) error {
	return p.OutputI64(int64(v))
}

func (p *JSONProtocol) WriteDouble(v float64) error {
	return p.OutputF64(v)
}

func (p *JSONProtocol) WriteFloat(v float32) error {
	return p.OutputF32(v)
}

func (p *JSONProtocol) WriteString(v string) error {
	return p.OutputString(v)
}

func (p *JSONProtocol) WriteBinary(v []byte) error {
	// JSON library only takes in a string,
	// not an arbitrary byte array, to ensure bytes are transmitted
	// efficiently we must convert this into a valid JSON string
	// therefore we use base64 encoding to avoid excessive escaping/quoting
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(JSON_QUOTE_BYTES); e != nil {
		return NewProtocolException(e)
	}
	if len(v) > 0 {
		writer := base64.NewEncoder(base64.StdEncoding, p.writer)
		if _, e := writer.Write(v); e != nil {
			p.writer.Reset(p.trans) // THRIFT-3735
			return NewProtocolException(e)
		}
		if e := writer.Close(); e != nil {
			return NewProtocolException(e)
		}
	}
	if _, e := p.write(JSON_QUOTE_BYTES); e != nil {
		return NewProtocolException(e)
	}
	return p.OutputPostValue()
}

// Reading methods.
func (p *JSONProtocol) ReadMessageBegin() (name string, typeID MessageType, seqID int32, err error) {
	p.resetContextStack() // THRIFT-3735
	if isNull, err := p.ParseListBegin(); isNull || err != nil {
		return name, typeID, seqID, err
	}
	version, err := p.ReadI32()
	if err != nil {
		return name, typeID, seqID, err
	}
	if version != THRIFT_JSON_PROTOCOL_VERSION {
		e := fmt.Errorf("Unknown Protocol version %d, expected version %d", version, THRIFT_JSON_PROTOCOL_VERSION)
		return name, typeID, seqID, NewProtocolExceptionWithType(INVALID_DATA, e)

	}
	if name, err = p.ReadString(); err != nil {
		return name, typeID, seqID, err
	}
	bTypeID, err := p.ReadByte()
	typeID = MessageType(bTypeID)
	if err != nil {
		return name, typeID, seqID, err
	}
	if seqID, err = p.ReadI32(); err != nil {
		return name, typeID, seqID, err
	}
	return name, typeID, seqID, nil
}

func (p *JSONProtocol) ReadMessageEnd() error {
	err := p.ParseListEnd()
	return err
}

func (p *JSONProtocol) ReadStructBegin() (name string, err error) {
	_, err = p.ParseObjectStart()
	return "", err
}

func (p *JSONProtocol) ReadStructEnd() error {
	return p.ParseObjectEnd()
}

func (p *JSONProtocol) ReadFieldBegin() (string, Type, int16, error) {
	b, _ := p.reader.Peek(1)
	if len(b) < 1 || b[0] == JSON_RBRACE[0] || b[0] == JSON_RBRACKET[0] {
		return "", STOP, -1, nil
	}
	fieldID, err := p.ReadI16()
	if err != nil {
		return "", STOP, fieldID, err
	}
	if _, err = p.ParseObjectStart(); err != nil {
		return "", STOP, fieldID, err
	}
	sType, err := p.ReadString()
	if err != nil {
		return "", STOP, fieldID, err
	}
	fType, err := p.StringToTypeId(sType)
	return "", fType, fieldID, err
}

func (p *JSONProtocol) ReadFieldEnd() error {
	return p.ParseObjectEnd()
}

func (p *JSONProtocol) ReadMapBegin() (keyType Type, valueType Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return VOID, VOID, 0, e
	}

	// read keyType
	sKeyType, e := p.ReadString()
	if e != nil {
		return keyType, valueType, size, e
	}
	keyType, e = p.StringToTypeId(sKeyType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read valueType
	sValueType, e := p.ReadString()
	if e != nil {
		return keyType, valueType, size, e
	}
	valueType, e = p.StringToTypeId(sValueType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read size
	iSize, e := p.ReadI64()
	if e != nil {
		return keyType, valueType, size, e
	}
	size = int(iSize)

	_, e = p.ParseObjectStart()
	return keyType, valueType, size, e
}

func (p *JSONProtocol) ReadMapEnd() error {
	e := p.ParseObjectEnd()
	if e != nil {
		return e
	}
	return p.ParseListEnd()
}

func (p *JSONProtocol) ReadListBegin() (elemType Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *JSONProtocol) ReadListEnd() error {
	return p.ParseListEnd()
}

func (p *JSONProtocol) ReadSetBegin() (elemType Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *JSONProtocol) ReadSetEnd() error {
	return p.ParseListEnd()
}

func (p *JSONProtocol) ReadBool() (bool, error) {
	value, err := p.ReadI32()
	return (value != 0), err
}

func (p *JSONProtocol) ReadByte() (byte, error) {
	v, err := p.ReadI64()
	return byte(v), err
}

func (p *JSONProtocol) ReadI16() (int16, error) {
	v, err := p.ReadI64()
	return int16(v), err
}

func (p *JSONProtocol) ReadI32() (int32, error) {
	v, err := p.ReadI64()
	return int32(v), err
}

func (p *JSONProtocol) ReadI64() (int64, error) {
	v, _, err := p.ParseI64()
	return v, err
}

func (p *JSONProtocol) ReadDouble() (float64, error) {
	v, _, err := p.ParseF64()
	return v, err
}

func (p *JSONProtocol) ReadFloat() (float32, error) {
	v, _, err := p.ParseF32()
	return v, err
}

func (p *JSONProtocol) ReadString() (string, error) {
	var v string
	if err := p.ParsePreValue(); err != nil {
		return v, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 && f[0] == JSON_QUOTE {
		p.reader.ReadByte()
		value, err := p.ParseStringBody()
		v = value
		if err != nil {
			return v, err
		}
	} else if len(f) > 0 && f[0] == JSON_NULL[0] {
		b := make([]byte, len(JSON_NULL))
		_, err := p.reader.Read(b)
		if err != nil {
			return v, NewProtocolException(err)
		}
		if string(b) != string(JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, NewProtocolExceptionWithType(INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	return v, p.ParsePostValue()
}

func (p *JSONProtocol) ReadBinary() ([]byte, error) {
	var v []byte
	if err := p.ParsePreValue(); err != nil {
		return nil, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 && f[0] == JSON_QUOTE {
		p.reader.ReadByte()
		value, err := p.ParseBase64EncodedBody()
		v = value
		if err != nil {
			return v, err
		}
	} else if len(f) > 0 && f[0] == JSON_NULL[0] {
		b := make([]byte, len(JSON_NULL))
		_, err := p.reader.Read(b)
		if err != nil {
			return v, NewProtocolException(err)
		}
		if string(b) != string(JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, NewProtocolExceptionWithType(INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, NewProtocolExceptionWithType(INVALID_DATA, e)
	}

	return v, p.ParsePostValue()
}

func (p *JSONProtocol) Flush() (err error) {
	err = p.writer.Flush()
	if err == nil {
		err = p.trans.Flush()
	}
	return NewProtocolException(err)
}

func (p *JSONProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *JSONProtocol) Transport() Transport {
	return p.trans
}

func (p *JSONProtocol) OutputElemListBegin(elemType Type, size int) error {
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	s, e1 := p.TypeIdToString(elemType)
	if e1 != nil {
		return e1
	}
	if e := p.WriteString(s); e != nil {
		return e
	}
	if e := p.WriteI64(int64(size)); e != nil {
		return e
	}
	return nil
}

func (p *JSONProtocol) ParseElemListBegin() (elemType Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return VOID, 0, e
	}
	sElemType, err := p.ReadString()
	if err != nil {
		return VOID, size, err
	}
	elemType, err = p.StringToTypeId(sElemType)
	if err != nil {
		return elemType, size, err
	}
	nSize, err2 := p.ReadI64()
	size = int(nSize)
	return elemType, size, err2
}

func (p *JSONProtocol) readElemListBegin() (elemType Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return VOID, 0, e
	}
	sElemType, err := p.ReadString()
	if err != nil {
		return VOID, size, err
	}
	elemType, err = p.StringToTypeId(sElemType)
	if err != nil {
		return elemType, size, err
	}
	nSize, err2 := p.ReadI64()
	size = int(nSize)
	return elemType, size, err2
}

func (p *JSONProtocol) writeElemListBegin(elemType Type, size int) error {
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	s, e1 := p.TypeIdToString(elemType)
	if e1 != nil {
		return e1
	}
	if e := p.OutputString(s); e != nil {
		return e
	}
	if e := p.OutputI64(int64(size)); e != nil {
		return e
	}
	return nil
}

func (p *JSONProtocol) TypeIdToString(fieldType Type) (string, error) {
	switch byte(fieldType) {
	case BOOL:
		return "tf", nil
	case BYTE:
		return "i8", nil
	case I16:
		return "i16", nil
	case I32:
		return "i32", nil
	case I64:
		return "i64", nil
	case DOUBLE:
		return "dbl", nil
	case FLOAT:
		return "flt", nil
	case STRING:
		return "str", nil
	case STRUCT:
		return "rec", nil
	case MAP:
		return "map", nil
	case SET:
		return "set", nil
	case LIST:
		return "lst", nil
	}

	e := fmt.Errorf("Unknown fieldType: %d", int(fieldType))
	return "", NewProtocolExceptionWithType(INVALID_DATA, e)
}

func (p *JSONProtocol) StringToTypeId(fieldType string) (Type, error) {
	switch fieldType {
	case "tf":
		return Type(BOOL), nil
	case "i8":
		return Type(BYTE), nil
	case "i16":
		return Type(I16), nil
	case "i32":
		return Type(I32), nil
	case "i64":
		return Type(I64), nil
	case "dbl":
		return Type(DOUBLE), nil
	case "flt":
		return Type(FLOAT), nil
	case "str":
		return Type(STRING), nil
	case "rec":
		return Type(STRUCT), nil
	case "map":
		return Type(MAP), nil
	case "set":
		return Type(SET), nil
	case "lst":
		return Type(LIST), nil
	}

	e := fmt.Errorf("Unknown type identifier: %s", fieldType)
	return Type(STOP), NewProtocolExceptionWithType(INVALID_DATA, e)
}
