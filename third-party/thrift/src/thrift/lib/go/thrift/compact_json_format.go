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
	"io"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

const (
	THRIFT_JSON_PROTOCOL_VERSION = 1
)

// for references to _ParseContext see simple_json_protocol.go

// compactJSONFormat is the Compact JSON format implementation for thrift.
//
// This format produces/consumes a compact JSON output with field numbers as
// object keys and field values lightly encoded.
//
// Example: With the Message definition
//
//	struct Message {
//	  1: bool aBool
//	  2: map<string, bool> aBoolStringMap
//	},
//
//	Message(aBool=True, aBoolStringMap={"key1": True, "key2": False})
//
// will be encoded as:
//
//	{"1":{"tf":1},"2":{"map":["str","tf",2,{"key1": 1,"key2":0}]}}'
type compactJSONFormat struct {
	*simpleJSONFormat
}

var _ types.Format = (*compactJSONFormat)(nil)

// NewCompactJSONFormat creates a new compact JSON Format.
func NewCompactJSONFormat(readWriter io.ReadWriter) types.Format {
	v := &compactJSONFormat{simpleJSONFormat: newSimpleJSONFormat(readWriter)}
	v.resetContextStack()
	return v
}

func (p *compactJSONFormat) WriteMessageBegin(name string, typeID types.MessageType, seqID int32) error {
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

func (p *compactJSONFormat) WriteMessageEnd() error {
	return p.OutputListEnd()
}

func (p *compactJSONFormat) WriteStructBegin(name string) error {
	if e := p.OutputObjectBegin(); e != nil {
		return e
	}
	return nil
}

func (p *compactJSONFormat) WriteStructEnd() error {
	return p.OutputObjectEnd()
}

func (p *compactJSONFormat) WriteFieldBegin(name string, typeID types.Type, id int16) error {
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

func (p *compactJSONFormat) WriteFieldEnd() error {
	return p.OutputObjectEnd()
}

func (p *compactJSONFormat) WriteFieldStop() error { return nil }

func (p *compactJSONFormat) WriteMapBegin(keyType types.Type, valueType types.Type, size int) error {
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

func (p *compactJSONFormat) WriteMapEnd() error {
	if e := p.OutputObjectEnd(); e != nil {
		return e
	}
	return p.OutputListEnd()
}

func (p *compactJSONFormat) WriteListBegin(elemType types.Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *compactJSONFormat) WriteListEnd() error {
	return p.OutputListEnd()
}

func (p *compactJSONFormat) WriteSetBegin(elemType types.Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *compactJSONFormat) WriteSetEnd() error {
	return p.OutputListEnd()
}

func (p *compactJSONFormat) WriteBool(b bool) error {
	if b {
		return p.WriteI32(1)
	}
	return p.WriteI32(0)
}

func (p *compactJSONFormat) WriteByte(b byte) error {
	return p.WriteI32(int32(b))
}

func (p *compactJSONFormat) WriteI16(v int16) error {
	return p.WriteI32(int32(v))
}

func (p *compactJSONFormat) WriteI32(v int32) error {
	return p.OutputI64(int64(v))
}

func (p *compactJSONFormat) WriteI64(v int64) error {
	return p.OutputI64(int64(v))
}

func (p *compactJSONFormat) WriteDouble(v float64) error {
	return p.OutputF64(v)
}

func (p *compactJSONFormat) WriteFloat(v float32) error {
	return p.OutputF32(v)
}

func (p *compactJSONFormat) WriteString(v string) error {
	return p.OutputString(v)
}

func (p *compactJSONFormat) WriteBinary(v []byte) error {
	// JSON library only takes in a string,
	// not an arbitrary byte array, to ensure bytes are transmitted
	// efficiently we must convert this into a valid JSON string
	// therefore we use base64 encoding to avoid excessive escaping/quoting
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(JSON_QUOTE_BYTES); e != nil {
		return types.NewProtocolException(e)
	}
	if len(v) > 0 {
		writer := base64.NewEncoder(base64.StdEncoding, p.writer)
		if _, e := writer.Write(v); e != nil {
			p.writer.Reset(p.buffer) // THRIFT-3735
			return types.NewProtocolException(e)
		}
		if e := writer.Close(); e != nil {
			return types.NewProtocolException(e)
		}
	}
	if _, e := p.write(JSON_QUOTE_BYTES); e != nil {
		return types.NewProtocolException(e)
	}
	return p.OutputPostValue()
}

// Reading methods.
func (p *compactJSONFormat) ReadMessageBegin() (name string, typeID types.MessageType, seqID int32, err error) {
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
		return name, typeID, seqID, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)

	}
	if name, err = p.ReadString(); err != nil {
		return name, typeID, seqID, err
	}
	bTypeID, err := p.ReadByte()
	typeID = types.MessageType(bTypeID)
	if err != nil {
		return name, typeID, seqID, err
	}
	if seqID, err = p.ReadI32(); err != nil {
		return name, typeID, seqID, err
	}
	return name, typeID, seqID, nil
}

func (p *compactJSONFormat) ReadMessageEnd() error {
	err := p.ParseListEnd()
	return err
}

func (p *compactJSONFormat) ReadStructBegin() (string, error) {
	_, err := p.ParseObjectStart()
	return "", err
}

func (p *compactJSONFormat) ReadStructEnd() error {
	return p.ParseObjectEnd()
}

func (p *compactJSONFormat) ReadFieldBegin() (string, types.Type, int16, error) {
	b, _ := p.reader.Peek(1)
	if len(b) < 1 || b[0] == JSON_RBRACE[0] || b[0] == JSON_RBRACKET[0] {
		return "", types.STOP, -1, nil
	}
	fieldID, err := p.ReadI16()
	if err != nil {
		return "", types.STOP, fieldID, err
	}
	if _, err = p.ParseObjectStart(); err != nil {
		return "", types.STOP, fieldID, err
	}
	sType, err := p.ReadString()
	if err != nil {
		return "", types.STOP, fieldID, err
	}
	fType, err := p.StringToTypeId(sType)
	return "", fType, fieldID, err
}

func (p *compactJSONFormat) ReadFieldEnd() error {
	return p.ParseObjectEnd()
}

func (p *compactJSONFormat) ReadMapBegin() (types.Type /* kType */, types.Type /* vType */, int /* size */, error) {
	isNull, err := p.ParseListBegin()
	if isNull || err != nil {
		return 0, 0, 0, err
	}

	// read kType
	kTypeStr, err := p.ReadString()
	if err != nil {
		return 0, 0, 0, err
	}
	kType, err := p.StringToTypeId(kTypeStr)
	if err != nil {
		return 0, 0, 0, err
	}

	// read vType
	vTypeStr, err := p.ReadString()
	if err != nil {
		return 0, 0, 0, err
	}
	vType, err := p.StringToTypeId(vTypeStr)
	if err != nil {
		return 0, 0, 0, err
	}

	// read size
	size64, err := p.ReadI64()
	if err != nil {
		return 0, 0, 0, err
	}
	size := int(size64)

	_, err = p.ParseObjectStart()
	return kType, vType, size, err
}

func (p *compactJSONFormat) ReadMapEnd() error {
	e := p.ParseObjectEnd()
	if e != nil {
		return e
	}
	return p.ParseListEnd()
}

func (p *compactJSONFormat) ReadListBegin() (types.Type /* elemType */, int /* size */, error) {
	return p.ParseElemListBegin()
}

func (p *compactJSONFormat) ReadListEnd() error {
	return p.ParseListEnd()
}

func (p *compactJSONFormat) ReadSetBegin() (types.Type /* elemType */, int /* size */, error) {
	return p.ParseElemListBegin()
}

func (p *compactJSONFormat) ReadSetEnd() error {
	return p.ParseListEnd()
}

func (p *compactJSONFormat) ReadBool() (bool, error) {
	value, err := p.ReadI32()
	return (value != 0), err
}

func (p *compactJSONFormat) ReadByte() (byte, error) {
	v, err := p.ReadI64()
	return byte(v), err
}

func (p *compactJSONFormat) ReadI16() (int16, error) {
	v, err := p.ReadI64()
	return int16(v), err
}

func (p *compactJSONFormat) ReadI32() (int32, error) {
	v, err := p.ReadI64()
	return int32(v), err
}

func (p *compactJSONFormat) ReadI64() (int64, error) {
	v, _, err := p.ParseI64()
	return v, err
}

func (p *compactJSONFormat) ReadDouble() (float64, error) {
	v, _, err := p.ParseF64()
	return v, err
}

func (p *compactJSONFormat) ReadFloat() (float32, error) {
	v, _, err := p.ParseF32()
	return v, err
}

func (p *compactJSONFormat) ReadString() (string, error) {
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
			return v, types.NewProtocolException(err)
		}
		if string(b) != string(JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return v, p.ParsePostValue()
}

func (p *compactJSONFormat) ReadBinary() ([]byte, error) {
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
			return v, types.NewProtocolException(err)
		}
		if string(b) != string(JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}

	return v, p.ParsePostValue()
}

func (p *compactJSONFormat) Flush() error {
	err := p.writer.Flush()
	if err == nil {
		return flush(p.buffer)
	}
	return types.NewProtocolException(err)
}

func (p *compactJSONFormat) Skip(fieldType types.Type) error {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *compactJSONFormat) OutputElemListBegin(elemType types.Type, size int) error {
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

func (p *compactJSONFormat) ParseElemListBegin() (elemType types.Type, size int, err error) {
	if isNull, err := p.ParseListBegin(); isNull || err != nil {
		return types.VOID, 0, err
	}
	sElemType, err := p.ReadString()
	if err != nil {
		return types.VOID, size, err
	}
	elemType, err = p.StringToTypeId(sElemType)
	if err != nil {
		return elemType, size, err
	}
	nSize, err2 := p.ReadI64()
	size = int(nSize)
	return elemType, size, err2
}

func (p *compactJSONFormat) TypeIdToString(fieldType types.Type) (string, error) {
	switch fieldType {
	case types.BOOL:
		return "tf", nil
	case types.BYTE:
		return "i8", nil
	case types.I16:
		return "i16", nil
	case types.I32:
		return "i32", nil
	case types.I64:
		return "i64", nil
	case types.DOUBLE:
		return "dbl", nil
	case types.FLOAT:
		return "flt", nil
	case types.STRING:
		return "str", nil
	case types.STRUCT:
		return "rec", nil
	case types.MAP:
		return "map", nil
	case types.SET:
		return "set", nil
	case types.LIST:
		return "lst", nil
	}

	e := fmt.Errorf("Unknown fieldType: %d", int(fieldType))
	return "", types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
}

func (p *compactJSONFormat) StringToTypeId(fieldType string) (types.Type, error) {
	switch fieldType {
	case "tf":
		return types.BOOL, nil
	case "i8":
		return types.BYTE, nil
	case "i16":
		return types.I16, nil
	case "i32":
		return types.I32, nil
	case "i64":
		return types.I64, nil
	case "dbl":
		return types.DOUBLE, nil
	case "flt":
		return types.FLOAT, nil
	case "str":
		return types.STRING, nil
	case "rec":
		return types.STRUCT, nil
	case "map":
		return types.MAP, nil
	case "set":
		return types.SET, nil
	case "lst":
		return types.LIST, nil
	}

	e := fmt.Errorf("Unknown type identifier: %s", fieldType)
	return types.STOP, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
}
