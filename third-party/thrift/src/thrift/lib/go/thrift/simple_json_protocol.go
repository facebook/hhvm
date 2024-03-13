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
	"bytes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"io"
	"math"
	"strconv"
)

type _ParseContext int

const (
	_CONTEXT_IN_TOPLEVEL          _ParseContext = 1
	_CONTEXT_IN_LIST_FIRST        _ParseContext = 2
	_CONTEXT_IN_LIST              _ParseContext = 3
	_CONTEXT_IN_OBJECT_FIRST      _ParseContext = 4
	_CONTEXT_IN_OBJECT_NEXT_KEY   _ParseContext = 5
	_CONTEXT_IN_OBJECT_NEXT_VALUE _ParseContext = 6
)

func (p _ParseContext) String() string {
	switch p {
	case _CONTEXT_IN_TOPLEVEL:
		return "TOPLEVEL"
	case _CONTEXT_IN_LIST_FIRST:
		return "LIST-FIRST"
	case _CONTEXT_IN_LIST:
		return "LIST"
	case _CONTEXT_IN_OBJECT_FIRST:
		return "OBJECT-FIRST"
	case _CONTEXT_IN_OBJECT_NEXT_KEY:
		return "OBJECT-NEXT-KEY"
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		return "OBJECT-NEXT-VALUE"
	}
	return "UNKNOWN-PARSE-CONTEXT"
}

// JSON protocol implementation for thrift.
//
// This protocol produces/consumes a simple output format
// suitable for parsing by scripting languages.  It should not be
// confused with the full-featured JSONProtocol.
type SimpleJSONProtocol struct {
	trans Transport

	parseContextStack []int
	dumpContext       []int

	writer *bufio.Writer
	reader *bufio.Reader
}

// Constructor
func NewSimpleJSONProtocol(t Transport) *SimpleJSONProtocol {
	v := &SimpleJSONProtocol{trans: t,
		writer: bufio.NewWriter(t),
		reader: bufio.NewReader(t),
	}
	v.parseContextStack = append(v.parseContextStack, int(_CONTEXT_IN_TOPLEVEL))
	v.dumpContext = append(v.dumpContext, int(_CONTEXT_IN_TOPLEVEL))
	return v
}

// Factory
type SimpleJSONProtocolFactory struct{}

func (p *SimpleJSONProtocolFactory) GetProtocol(trans Transport) Protocol {
	return NewSimpleJSONProtocol(trans)
}

func NewSimpleJSONProtocolFactory() *SimpleJSONProtocolFactory {
	return &SimpleJSONProtocolFactory{}
}

var (
	JSON_COMMA                   []byte
	JSON_COLON                   []byte
	JSON_LBRACE                  []byte
	JSON_RBRACE                  []byte
	JSON_LBRACKET                []byte
	JSON_RBRACKET                []byte
	JSON_QUOTE                   byte
	JSON_QUOTE_BYTES             []byte
	JSON_NULL                    []byte
	JSON_TRUE                    []byte
	JSON_FALSE                   []byte
	JSON_INFINITY                string
	JSON_NEGATIVE_INFINITY       string
	JSON_NAN                     string
	JSON_INFINITY_BYTES          []byte
	JSON_NEGATIVE_INFINITY_BYTES []byte
	JSON_NAN_BYTES               []byte
	json_nonbase_map_elem_bytes  []byte
)

func init() {
	JSON_COMMA = []byte{','}
	JSON_COLON = []byte{':'}
	JSON_LBRACE = []byte{'{'}
	JSON_RBRACE = []byte{'}'}
	JSON_LBRACKET = []byte{'['}
	JSON_RBRACKET = []byte{']'}
	JSON_QUOTE = '"'
	JSON_QUOTE_BYTES = []byte{'"'}
	JSON_NULL = []byte{'n', 'u', 'l', 'l'}
	JSON_TRUE = []byte{'t', 'r', 'u', 'e'}
	JSON_FALSE = []byte{'f', 'a', 'l', 's', 'e'}
	JSON_INFINITY = "Infinity"
	JSON_NEGATIVE_INFINITY = "-Infinity"
	JSON_NAN = "NaN"
	JSON_INFINITY_BYTES = []byte{'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NEGATIVE_INFINITY_BYTES = []byte{'-', 'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NAN_BYTES = []byte{'N', 'a', 'N'}
	json_nonbase_map_elem_bytes = []byte{']', ',', '['}
}

func jsonQuote(s string) string {
	b, _ := json.Marshal(s)
	s1 := string(b)
	return s1
}

func jsonUnquote(s string) (string, bool) {
	s1 := new(string)
	err := json.Unmarshal([]byte(s), s1)
	return *s1, err == nil
}

func mismatch(expected, actual string) error {
	return fmt.Errorf("Expected '%s' but found '%s' while parsing JSON.", expected, actual)
}

func (p *SimpleJSONProtocol) WriteMessageBegin(name string, typeID MessageType, seqID int32) error {
	p.resetContextStack() // THRIFT-3735
	if e := p.OutputListBegin(); e != nil {
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

func (p *SimpleJSONProtocol) WriteMessageEnd() error {
	return p.OutputListEnd()
}

func (p *SimpleJSONProtocol) WriteStructBegin(name string) error {
	if e := p.OutputObjectBegin(); e != nil {
		return e
	}
	return nil
}

func (p *SimpleJSONProtocol) WriteStructEnd() error {
	return p.OutputObjectEnd()
}

func (p *SimpleJSONProtocol) WriteFieldBegin(name string, typeID Type, id int16) error {
	if e := p.WriteString(name); e != nil {
		return e
	}
	return nil
}

func (p *SimpleJSONProtocol) WriteFieldEnd() error {
	//return p.OutputListEnd()
	return nil
}

func (p *SimpleJSONProtocol) WriteFieldStop() error { return nil }

func (p *SimpleJSONProtocol) WriteMapBegin(keyType Type, valueType Type, size int) error {
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	if e := p.WriteByte(byte(keyType)); e != nil {
		return e
	}
	if e := p.WriteByte(byte(valueType)); e != nil {
		return e
	}
	return p.WriteI32(int32(size))
}

func (p *SimpleJSONProtocol) WriteMapEnd() error {
	return p.OutputListEnd()
}

func (p *SimpleJSONProtocol) WriteListBegin(elemType Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *SimpleJSONProtocol) WriteListEnd() error {
	return p.OutputListEnd()
}

func (p *SimpleJSONProtocol) WriteSetBegin(elemType Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *SimpleJSONProtocol) WriteSetEnd() error {
	return p.OutputListEnd()
}

func (p *SimpleJSONProtocol) WriteBool(b bool) error {
	return p.OutputBool(b)
}

func (p *SimpleJSONProtocol) WriteByte(b byte) error {
	return p.WriteI32(int32(b))
}

func (p *SimpleJSONProtocol) WriteI16(v int16) error {
	return p.WriteI32(int32(v))
}

func (p *SimpleJSONProtocol) WriteI32(v int32) error {
	return p.OutputI64(int64(v))
}

func (p *SimpleJSONProtocol) WriteI64(v int64) error {
	return p.OutputI64(int64(v))
}

func (p *SimpleJSONProtocol) WriteDouble(v float64) error {
	return p.OutputF64(v)
}

func (p *SimpleJSONProtocol) WriteFloat(v float32) error {
	return p.OutputF32(v)
}

func (p *SimpleJSONProtocol) WriteString(v string) error {
	return p.OutputString(v)
}

func (p *SimpleJSONProtocol) WriteBinary(v []byte) error {
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
func (p *SimpleJSONProtocol) ReadMessageBegin() (name string, typeID MessageType, seqID int32, err error) {
	p.resetContextStack() // THRIFT-3735
	if isNull, err := p.ParseListBegin(); isNull || err != nil {
		return name, typeID, seqID, err
	}
	if name, err = p.ReadString(); err != nil {
		return name, typeID, seqID, err
	}
	bTypeId, err := p.ReadByte()
	typeID = MessageType(bTypeId)
	if err != nil {
		return name, typeID, seqID, err
	}
	if seqID, err = p.ReadI32(); err != nil {
		return name, typeID, seqID, err
	}
	return name, typeID, seqID, nil
}

func (p *SimpleJSONProtocol) ReadMessageEnd() error {
	return p.ParseListEnd()
}

func (p *SimpleJSONProtocol) ReadStructBegin() (name string, err error) {
	_, err = p.ParseObjectStart()
	return "", err
}

func (p *SimpleJSONProtocol) ReadStructEnd() error {
	return p.ParseObjectEnd()
}

func (p *SimpleJSONProtocol) ReadFieldBegin() (string, Type, int16, error) {
	if err := p.ParsePreValue(); err != nil {
		return "", STOP, 0, err
	}
	b, _ := p.reader.Peek(1)
	if len(b) > 0 {
		switch b[0] {
		case JSON_RBRACE[0]:
			return "", STOP, 0, nil
		case JSON_QUOTE:
			p.reader.ReadByte()
			name, err := p.ParseStringBody()
			// simplejson is not meant to be read back into thrift
			// - see http://wiki.apache.org/thrift/ThriftUsageJava
			// - use JSON instead
			if err != nil {
				return name, STOP, 0, err
			}
			return name, STOP, -1, p.ParsePostValue()
			/*
			   if err = p.ParsePostValue(); err != nil {
			     return name, STOP, 0, err
			   }
			   if isNull, err := p.ParseListBegin(); isNull || err != nil {
			     return name, STOP, 0, err
			   }
			   bType, err := p.ReadByte()
			   thetype := Type(bType)
			   if err != nil {
			     return name, thetype, 0, err
			   }
			   id, err := p.ReadI16()
			   return name, thetype, id, err
			*/
		}
		e := fmt.Errorf("Expected \"}\" or '\"', but found: '%s'", string(b))
		return "", STOP, 0, NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	return "", STOP, 0, NewProtocolException(io.EOF)
}

func (p *SimpleJSONProtocol) ReadFieldEnd() error {
	return nil
	//return p.ParseListEnd()
}

func (p *SimpleJSONProtocol) ReadMapBegin() (keyType Type, valueType Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return VOID, VOID, 0, e
	}

	// read keyType
	bKeyType, e := p.ReadByte()
	keyType = Type(bKeyType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read valueType
	bValueType, e := p.ReadByte()
	valueType = Type(bValueType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read size
	iSize, err := p.ReadI64()
	size = int(iSize)
	return keyType, valueType, size, err
}

func (p *SimpleJSONProtocol) ReadMapEnd() error {
	return p.ParseListEnd()
}

func (p *SimpleJSONProtocol) ReadListBegin() (elemType Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *SimpleJSONProtocol) ReadListEnd() error {
	return p.ParseListEnd()
}

func (p *SimpleJSONProtocol) ReadSetBegin() (elemType Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *SimpleJSONProtocol) ReadSetEnd() error {
	return p.ParseListEnd()
}

func (p *SimpleJSONProtocol) ReadBool() (bool, error) {
	var value bool

	if err := p.ParsePreValue(); err != nil {
		return value, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 {
		switch f[0] {
		case JSON_TRUE[0]:
			b := make([]byte, len(JSON_TRUE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, NewProtocolException(err)
			}
			if string(b) == string(JSON_TRUE) {
				value = true
			} else {
				e := fmt.Errorf("Expected \"true\" but found: %s", string(b))
				return value, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			break
		case JSON_FALSE[0]:
			b := make([]byte, len(JSON_FALSE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, NewProtocolException(err)
			}
			if string(b) == string(JSON_FALSE) {
				value = false
			} else {
				e := fmt.Errorf("Expected \"false\" but found: %s", string(b))
				return value, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			break
		case JSON_NULL[0]:
			b := make([]byte, len(JSON_NULL))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, NewProtocolException(err)
			}
			if string(b) == string(JSON_NULL) {
				value = false
			} else {
				e := fmt.Errorf("Expected \"null\" but found: %s", string(b))
				return value, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		default:
			e := fmt.Errorf("Expected \"true\", \"false\", or \"null\" but found: %s", string(f))
			return value, NewProtocolExceptionWithType(INVALID_DATA, e)
		}
	}
	return value, p.ParsePostValue()
}

func (p *SimpleJSONProtocol) ReadByte() (byte, error) {
	v, err := p.ReadI64()
	return byte(v), err
}

func (p *SimpleJSONProtocol) ReadI16() (int16, error) {
	v, err := p.ReadI64()
	return int16(v), err
}

func (p *SimpleJSONProtocol) ReadI32() (int32, error) {
	v, err := p.ReadI64()
	return int32(v), err
}

func (p *SimpleJSONProtocol) ReadI64() (int64, error) {
	v, _, err := p.ParseI64()
	return v, err
}

func (p *SimpleJSONProtocol) ReadDouble() (float64, error) {
	v, _, err := p.ParseF64()
	return v, err
}

func (p *SimpleJSONProtocol) ReadFloat() (float32, error) {
	v, _, err := p.ParseF32()
	return v, err
}

func (p *SimpleJSONProtocol) ReadString() (string, error) {
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

func (p *SimpleJSONProtocol) ReadBinary() ([]byte, error) {
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

func (p *SimpleJSONProtocol) Flush() (err error) {
	return NewProtocolException(p.writer.Flush())
}

func (p *SimpleJSONProtocol) Skip(fieldType Type) (err error) {
	return SkipDefaultDepth(p, fieldType)
}

func (p *SimpleJSONProtocol) Close() error {
	return p.trans.Close()
}

func (p *SimpleJSONProtocol) OutputPreValue() error {
	cxt := _ParseContext(p.dumpContext[len(p.dumpContext)-1])
	switch cxt {
	case _CONTEXT_IN_LIST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		if _, e := p.write(JSON_COMMA); e != nil {
			return NewProtocolException(e)
		}
		break
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		if _, e := p.write(JSON_COLON); e != nil {
			return NewProtocolException(e)
		}
		break
	}
	return nil
}

func (p *SimpleJSONProtocol) OutputPostValue() error {
	cxt := _ParseContext(p.dumpContext[len(p.dumpContext)-1])
	switch cxt {
	case _CONTEXT_IN_LIST_FIRST:
		p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
		p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_LIST))
		break
	case _CONTEXT_IN_OBJECT_FIRST:
		p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
		p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_OBJECT_NEXT_VALUE))
		break
	case _CONTEXT_IN_OBJECT_NEXT_KEY:
		p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
		p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_OBJECT_NEXT_VALUE))
		break
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
		p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_OBJECT_NEXT_KEY))
		break
	}
	return nil
}

func (p *SimpleJSONProtocol) OutputBool(value bool) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if value {
		v = string(JSON_TRUE)
	} else {
		v = string(JSON_FALSE)
	}
	switch _ParseContext(p.dumpContext[len(p.dumpContext)-1]) {
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		v = jsonQuote(v)
	default:
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputNull() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(JSON_NULL); e != nil {
		return NewProtocolException(e)
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputF64(value float64) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if math.IsNaN(value) {
		v = string(JSON_QUOTE) + JSON_NAN + string(JSON_QUOTE)
	} else if math.IsInf(value, 1) {
		v = string(JSON_QUOTE) + JSON_INFINITY + string(JSON_QUOTE)
	} else if math.IsInf(value, -1) {
		v = string(JSON_QUOTE) + JSON_NEGATIVE_INFINITY + string(JSON_QUOTE)
	} else {
		v = strconv.FormatFloat(value, 'g', -1, 64)
		switch _ParseContext(p.dumpContext[len(p.dumpContext)-1]) {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(JSON_QUOTE) + v + string(JSON_QUOTE)
		default:
		}
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputF32(value float32) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if math.IsNaN(float64(value)) {
		v = string(JSON_QUOTE) + JSON_NAN + string(JSON_QUOTE)
	} else if math.IsInf(float64(value), 1) {
		v = string(JSON_QUOTE) + JSON_INFINITY + string(JSON_QUOTE)
	} else if math.IsInf(float64(value), -1) {
		v = string(JSON_QUOTE) + JSON_NEGATIVE_INFINITY + string(JSON_QUOTE)
	} else {
		v = strconv.FormatFloat(float64(value), 'g', -1, 32)
		switch _ParseContext(p.dumpContext[len(p.dumpContext)-1]) {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(JSON_QUOTE) + v + string(JSON_QUOTE)
		default:
		}
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputI64(value int64) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	v := strconv.FormatInt(value, 10)
	switch _ParseContext(p.dumpContext[len(p.dumpContext)-1]) {
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		v = jsonQuote(v)
	default:
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputString(s string) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if e := p.OutputStringData(jsonQuote(s)); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *SimpleJSONProtocol) OutputStringData(s string) error {
	_, e := p.write([]byte(s))
	return NewProtocolException(e)
}

func (p *SimpleJSONProtocol) OutputObjectBegin() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(JSON_LBRACE); e != nil {
		return NewProtocolException(e)
	}
	p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_OBJECT_FIRST))
	return nil
}

func (p *SimpleJSONProtocol) OutputObjectEnd() error {
	if _, e := p.write(JSON_RBRACE); e != nil {
		return NewProtocolException(e)
	}
	p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
	if e := p.OutputPostValue(); e != nil {
		return e
	}
	return nil
}

func (p *SimpleJSONProtocol) OutputListBegin() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(JSON_LBRACKET); e != nil {
		return NewProtocolException(e)
	}
	p.dumpContext = append(p.dumpContext, int(_CONTEXT_IN_LIST_FIRST))
	return nil
}

func (p *SimpleJSONProtocol) OutputListEnd() error {
	if _, e := p.write(JSON_RBRACKET); e != nil {
		return NewProtocolException(e)
	}
	p.dumpContext = p.dumpContext[:len(p.dumpContext)-1]
	if e := p.OutputPostValue(); e != nil {
		return e
	}
	return nil
}

func (p *SimpleJSONProtocol) OutputElemListBegin(elemType Type, size int) error {
	if e := p.OutputListBegin(); e != nil {
		return e
	}
	if e := p.WriteByte(byte(elemType)); e != nil {
		return e
	}
	if e := p.WriteI64(int64(size)); e != nil {
		return e
	}
	return nil
}

func (p *SimpleJSONProtocol) ParsePreValue() error {
	if e := p.readNonSignificantWhitespace(); e != nil {
		return NewProtocolException(e)
	}
	cxt := _ParseContext(p.parseContextStack[len(p.parseContextStack)-1])
	b, _ := p.reader.Peek(1)
	switch cxt {
	case _CONTEXT_IN_LIST:
		if len(b) > 0 {
			switch b[0] {
			case JSON_RBRACKET[0]:
				return nil
			case JSON_COMMA[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \"]\" or \",\" in list context, but found \"%s\"", string(b))
				return NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		}
		break
	case _CONTEXT_IN_OBJECT_NEXT_KEY:
		if len(b) > 0 {
			switch b[0] {
			case JSON_RBRACE[0]:
				return nil
			case JSON_COMMA[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \"}\" or \",\" in object context, but found \"%s\"", string(b))
				return NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		}
		break
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		if len(b) > 0 {
			switch b[0] {
			case JSON_COLON[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \":\" in object context, but found \"%s\"", string(b))
				return NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		}
		break
	}
	return nil
}

func (p *SimpleJSONProtocol) ParsePostValue() error {
	if e := p.readNonSignificantWhitespace(); e != nil {
		return NewProtocolException(e)
	}
	cxt := _ParseContext(p.parseContextStack[len(p.parseContextStack)-1])
	switch cxt {
	case _CONTEXT_IN_LIST_FIRST:
		p.parseContextStack = p.parseContextStack[:len(p.parseContextStack)-1]
		p.parseContextStack = append(p.parseContextStack, int(_CONTEXT_IN_LIST))
		break
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		p.parseContextStack = p.parseContextStack[:len(p.parseContextStack)-1]
		p.parseContextStack = append(p.parseContextStack, int(_CONTEXT_IN_OBJECT_NEXT_VALUE))
		break
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		p.parseContextStack = p.parseContextStack[:len(p.parseContextStack)-1]
		p.parseContextStack = append(p.parseContextStack, int(_CONTEXT_IN_OBJECT_NEXT_KEY))
		break
	}
	return nil
}

func (p *SimpleJSONProtocol) readNonSignificantWhitespace() error {
	for {
		b, _ := p.reader.Peek(1)
		if len(b) < 1 {
			return nil
		}
		switch b[0] {
		case ' ', '\r', '\n', '\t':
			p.reader.ReadByte()
			continue
		default:
			break
		}
		break
	}
	return nil
}

func (p *SimpleJSONProtocol) ParseStringBody() (string, error) {
	line, err := p.reader.ReadString(JSON_QUOTE)
	if err != nil {
		return "", NewProtocolException(err)
	}
	l := len(line)
	// count number of escapes to see if we need to keep going
	i := 1
	for ; i < l; i++ {
		if line[l-i-1] != '\\' {
			break
		}
	}
	if i&0x01 == 1 {
		v, ok := jsonUnquote(string(JSON_QUOTE) + line)
		if !ok {
			return "", NewProtocolException(err)
		}
		return v, nil
	}
	s, err := p.ParseQuotedStringBody()
	if err != nil {
		return "", NewProtocolException(err)
	}
	str := string(JSON_QUOTE) + line + s
	v, ok := jsonUnquote(str)
	if !ok {
		e := fmt.Errorf("Unable to parse as JSON string %s", str)
		return "", NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	return v, nil
}

func (p *SimpleJSONProtocol) ParseQuotedStringBody() (string, error) {
	line, err := p.reader.ReadString(JSON_QUOTE)
	if err != nil {
		return "", NewProtocolException(err)
	}
	l := len(line)
	// count number of escapes to see if we need to keep going
	i := 1
	for ; i < l; i++ {
		if line[l-i-1] != '\\' {
			break
		}
	}
	if i&0x01 == 1 {
		return line, nil
	}
	s, err := p.ParseQuotedStringBody()
	if err != nil {
		return "", NewProtocolException(err)
	}
	v := line + s
	return v, nil
}

func (p *SimpleJSONProtocol) ParseBase64EncodedBody() ([]byte, error) {
	line, err := p.reader.ReadBytes(JSON_QUOTE)
	if err != nil {
		return line, NewProtocolException(err)
	}
	line2 := line[0 : len(line)-1]
	l := len(line2)
	if (l % 4) != 0 {
		pad := 4 - (l % 4)
		fill := [...]byte{'=', '=', '='}
		line2 = append(line2, fill[:pad]...)
		l = len(line2)
	}
	output := make([]byte, base64.StdEncoding.DecodedLen(l))
	n, err := base64.StdEncoding.Decode(output, line2)
	return output[0:n], NewProtocolException(err)
}

func (p *SimpleJSONProtocol) ParseI64() (int64, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value int64
	var isnull bool
	if p.safePeekContains(JSON_NULL) {
		p.reader.Read(make([]byte, len(JSON_NULL)))
		isnull = true
	} else {
		num, err := p.readNumeric()
		isnull = (num == nil)
		if !isnull {
			value = num.Int64()
		}
		if err != nil {
			return value, isnull, err
		}
	}
	return value, isnull, p.ParsePostValue()
}

func (p *SimpleJSONProtocol) ParseF64() (float64, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value float64
	var isnull bool
	if p.safePeekContains(JSON_NULL) {
		p.reader.Read(make([]byte, len(JSON_NULL)))
		isnull = true
	} else {
		num, err := p.readNumeric()
		isnull = (num == nil)
		if !isnull {
			value = num.Float64()
		}
		if err != nil {
			return value, isnull, err
		}
	}
	return value, isnull, p.ParsePostValue()
}

func (p *SimpleJSONProtocol) ParseF32() (float32, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value float32
	var isnull bool
	if p.safePeekContains(JSON_NULL) {
		p.reader.Read(make([]byte, len(JSON_NULL)))
		isnull = true
	} else {
		num, err := p.readNumeric()
		isnull = (num == nil)
		if !isnull {
			value = num.Float32()
		}
		if err != nil {
			return value, isnull, err
		}
	}
	return value, isnull, p.ParsePostValue()
}

func (p *SimpleJSONProtocol) ParseObjectStart() (bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return false, err
	}
	var b []byte
	b, err := p.reader.Peek(1)
	if err != nil {
		return false, err
	}
	if len(b) > 0 && b[0] == JSON_LBRACE[0] {
		p.reader.ReadByte()
		p.parseContextStack = append(p.parseContextStack, int(_CONTEXT_IN_OBJECT_FIRST))
		return false, nil
	} else if p.safePeekContains(JSON_NULL) {
		return true, nil
	}
	e := fmt.Errorf("Expected '{' or null, but found '%s'", string(b))
	return false, NewProtocolExceptionWithType(INVALID_DATA, e)
}

func (p *SimpleJSONProtocol) ParseObjectEnd() error {
	if isNull, err := p.readIfNull(); isNull || err != nil {
		return err
	}
	cxt := _ParseContext(p.parseContextStack[len(p.parseContextStack)-1])
	if (cxt != _CONTEXT_IN_OBJECT_FIRST) && (cxt != _CONTEXT_IN_OBJECT_NEXT_KEY) {
		e := fmt.Errorf("Expected to be in the Object Context, but not in Object Context (%d)", cxt)
		return NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	line, err := p.reader.ReadString(JSON_RBRACE[0])
	if err != nil {
		return NewProtocolException(err)
	}
	for _, char := range line {
		switch char {
		default:
			e := fmt.Errorf("Expecting end of object \"}\", but found: \"%s\"", line)
			return NewProtocolExceptionWithType(INVALID_DATA, e)
		case ' ', '\n', '\r', '\t', '}':
			break
		}
	}
	p.parseContextStack = p.parseContextStack[:len(p.parseContextStack)-1]
	return p.ParsePostValue()
}

func (p *SimpleJSONProtocol) ParseListBegin() (isNull bool, err error) {
	if e := p.ParsePreValue(); e != nil {
		return false, e
	}
	var b []byte
	b, err = p.reader.Peek(1)
	if err != nil {
		return false, err
	}
	if len(b) >= 1 && b[0] == JSON_LBRACKET[0] {
		p.parseContextStack = append(p.parseContextStack, int(_CONTEXT_IN_LIST_FIRST))
		p.reader.ReadByte()
		isNull = false
	} else if p.safePeekContains(JSON_NULL) {
		isNull = true
	} else {
		err = fmt.Errorf("Expected \"null\" or \"[\", received %q", b)
	}
	return isNull, NewProtocolExceptionWithType(INVALID_DATA, err)
}

func (p *SimpleJSONProtocol) ParseElemListBegin() (elemType Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return VOID, 0, e
	}
	bElemType, err := p.ReadByte()
	elemType = Type(bElemType)
	if err != nil {
		return elemType, size, err
	}
	nSize, err2 := p.ReadI64()
	size = int(nSize)
	return elemType, size, err2
}

func (p *SimpleJSONProtocol) ParseListEnd() error {
	if isNull, err := p.readIfNull(); isNull || err != nil {
		return err
	}
	cxt := _ParseContext(p.parseContextStack[len(p.parseContextStack)-1])
	if cxt != _CONTEXT_IN_LIST {
		e := fmt.Errorf("Expected to be in the List Context, but not in List Context (%d)", cxt)
		return NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	line, err := p.reader.ReadString(JSON_RBRACKET[0])
	if err != nil {
		return NewProtocolException(err)
	}
	for _, char := range line {
		switch char {
		default:
			e := fmt.Errorf("Expecting end of list \"]\", but found: %q", line)
			return NewProtocolExceptionWithType(INVALID_DATA, e)
		case ' ', '\n', '\r', '\t', rune(JSON_RBRACKET[0]):
			break
		}
	}
	p.parseContextStack = p.parseContextStack[:len(p.parseContextStack)-1]
	if _ParseContext(p.parseContextStack[len(p.parseContextStack)-1]) == _CONTEXT_IN_TOPLEVEL {
		return nil
	}
	return p.ParsePostValue()
}

func (p *SimpleJSONProtocol) readSingleValue() (interface{}, Type, error) {
	e := p.readNonSignificantWhitespace()
	if e != nil {
		return nil, VOID, NewProtocolException(e)
	}
	b, e := p.reader.Peek(1)
	if len(b) > 0 {
		c := b[0]
		switch c {
		case JSON_NULL[0]:
			buf := make([]byte, len(JSON_NULL))
			_, e := p.reader.Read(buf)
			if e != nil {
				return nil, VOID, NewProtocolException(e)
			}
			if string(JSON_NULL) != string(buf) {
				e = mismatch(string(JSON_NULL), string(buf))
				return nil, VOID, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			return nil, VOID, nil
		case JSON_QUOTE:
			p.reader.ReadByte()
			v, e := p.ParseStringBody()
			if e != nil {
				return v, UTF8, NewProtocolException(e)
			}
			if v == JSON_INFINITY {
				return INFINITY, DOUBLE, nil
			} else if v == JSON_NEGATIVE_INFINITY {
				return NEGATIVE_INFINITY, DOUBLE, nil
			} else if v == JSON_NAN {
				return NAN, DOUBLE, nil
			}
			return v, UTF8, nil
		case JSON_TRUE[0]:
			buf := make([]byte, len(JSON_TRUE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return true, BOOL, NewProtocolException(e)
			}
			if string(JSON_TRUE) != string(buf) {
				e := mismatch(string(JSON_TRUE), string(buf))
				return true, BOOL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			return true, BOOL, nil
		case JSON_FALSE[0]:
			buf := make([]byte, len(JSON_FALSE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return false, BOOL, NewProtocolException(e)
			}
			if string(JSON_FALSE) != string(buf) {
				e := mismatch(string(JSON_FALSE), string(buf))
				return false, BOOL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			return false, BOOL, nil
		case JSON_LBRACKET[0]:
			_, e := p.reader.ReadByte()
			return make([]interface{}, 0), LIST, NewProtocolException(e)
		case JSON_LBRACE[0]:
			_, e := p.reader.ReadByte()
			return make(map[string]interface{}), STRUCT, NewProtocolException(e)
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'e', 'E', '.', '+', '-', JSON_INFINITY[0], JSON_NAN[0]:
			// assume numeric
			v, e := p.readNumeric()
			return v, DOUBLE, e
		default:
			e := fmt.Errorf("Expected element in list but found '%s' while parsing JSON.", string(c))
			return nil, VOID, NewProtocolExceptionWithType(INVALID_DATA, e)
		}
	}
	e = fmt.Errorf("Cannot read a single element while parsing JSON.")
	return nil, VOID, NewProtocolExceptionWithType(INVALID_DATA, e)

}

func (p *SimpleJSONProtocol) readIfNull() (bool, error) {
	cont := true
	for cont {
		b, _ := p.reader.Peek(1)
		if len(b) < 1 {
			return false, nil
		}
		switch b[0] {
		default:
			return false, nil
		case JSON_NULL[0]:
			cont = false
			break
		case ' ', '\n', '\r', '\t':
			p.reader.ReadByte()
			break
		}
	}
	if p.safePeekContains(JSON_NULL) {
		p.reader.Read(make([]byte, len(JSON_NULL)))
		return true, nil
	}
	return false, nil
}

func (p *SimpleJSONProtocol) readQuoteIfNext() {
	b, _ := p.reader.Peek(1)
	if len(b) > 0 && b[0] == JSON_QUOTE {
		p.reader.ReadByte()
	}
}

func (p *SimpleJSONProtocol) readNumeric() (Numeric, error) {
	isNull, err := p.readIfNull()
	if isNull || err != nil {
		return NUMERIC_NULL, err
	}
	hasDecimalPoint := false
	nextCanBeSign := true
	hasE := false
	MAX_LEN := 40
	buf := bytes.NewBuffer(make([]byte, 0, MAX_LEN))
	continueFor := true
	inQuotes := false
	for continueFor {
		c, err := p.reader.ReadByte()
		if err != nil {
			if err == io.EOF {
				break
			}
			return NUMERIC_NULL, NewProtocolException(err)
		}
		switch c {
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			buf.WriteByte(c)
			nextCanBeSign = false
		case '.':
			if hasDecimalPoint {
				e := fmt.Errorf("Unable to parse number with multiple decimal points '%s.'", buf.String())
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			if hasE {
				e := fmt.Errorf("Unable to parse number with decimal points in the exponent '%s.'", buf.String())
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasDecimalPoint, nextCanBeSign = true, false
		case 'e', 'E':
			if hasE {
				e := fmt.Errorf("Unable to parse number with multiple exponents '%s%c'", buf.String(), c)
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasE, nextCanBeSign = true, true
		case '-', '+':
			if !nextCanBeSign {
				e := fmt.Errorf("Negative sign within number")
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
			buf.WriteByte(c)
			nextCanBeSign = false
		case ' ', 0, '\t', '\n', '\r', JSON_RBRACE[0], JSON_RBRACKET[0], JSON_COMMA[0], JSON_COLON[0]:
			p.reader.UnreadByte()
			continueFor = false
		case JSON_NAN[0]:
			if buf.Len() == 0 {
				buffer := make([]byte, len(JSON_NAN))
				buffer[0] = c
				_, e := p.reader.Read(buffer[1:])
				if e != nil {
					return NUMERIC_NULL, NewProtocolException(e)
				}
				if JSON_NAN != string(buffer) {
					e := mismatch(JSON_NAN, string(buffer))
					return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return NAN, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		case JSON_INFINITY[0]:
			if buf.Len() == 0 || (buf.Len() == 1 && buf.Bytes()[0] == '+') {
				buffer := make([]byte, len(JSON_INFINITY))
				buffer[0] = c
				_, e := p.reader.Read(buffer[1:])
				if e != nil {
					return NUMERIC_NULL, NewProtocolException(e)
				}
				if JSON_INFINITY != string(buffer) {
					e := mismatch(JSON_INFINITY, string(buffer))
					return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return INFINITY, nil
			} else if buf.Len() == 1 && buf.Bytes()[0] == JSON_NEGATIVE_INFINITY[0] {
				buffer := make([]byte, len(JSON_NEGATIVE_INFINITY))
				buffer[0] = JSON_NEGATIVE_INFINITY[0]
				buffer[1] = c
				_, e := p.reader.Read(buffer[2:])
				if e != nil {
					return NUMERIC_NULL, NewProtocolException(e)
				}
				if JSON_NEGATIVE_INFINITY != string(buffer) {
					e := mismatch(JSON_NEGATIVE_INFINITY, string(buffer))
					return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return NEGATIVE_INFINITY, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c' due to existing buffer %s", c, buf.String())
				return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
			}
		case JSON_QUOTE:
			if !inQuotes {
				inQuotes = true
			} else {
				break
			}
		default:
			e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
			return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
		}
	}
	if buf.Len() == 0 {
		e := fmt.Errorf("Unable to parse number from empty string ''")
		return NUMERIC_NULL, NewProtocolExceptionWithType(INVALID_DATA, e)
	}
	return NewNumericFromJSONString(buf.String(), false), nil
}

// Safely peeks into the buffer, reading only what is necessary
func (p *SimpleJSONProtocol) safePeekContains(b []byte) bool {
	for i := 0; i < len(b); i++ {
		a, _ := p.reader.Peek(i + 1)
		if len(a) < (i+1) || a[i] != b[i] {
			return false
		}
	}
	return true
}

// Reset the context stack to its initial state.
func (p *SimpleJSONProtocol) resetContextStack() {
	p.parseContextStack = []int{int(_CONTEXT_IN_TOPLEVEL)}
	p.dumpContext = []int{int(_CONTEXT_IN_TOPLEVEL)}
}

func (p *SimpleJSONProtocol) write(b []byte) (int, error) {
	n, err := p.writer.Write(b)
	if err != nil {
		p.writer.Reset(p.trans) // THRIFT-3735
	}
	return n, err
}
