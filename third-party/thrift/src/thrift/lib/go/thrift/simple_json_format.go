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
	"errors"
	"fmt"
	"io"
	"math"
	"strconv"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type _ParseContext int

const (
	_CONTEXT_INVALID              _ParseContext = iota
	_CONTEXT_IN_TOPLEVEL                        // 1
	_CONTEXT_IN_LIST_FIRST                      // 2
	_CONTEXT_IN_LIST                            // 3
	_CONTEXT_IN_OBJECT_FIRST                    // 4
	_CONTEXT_IN_OBJECT_NEXT_KEY                 // 5
	_CONTEXT_IN_OBJECT_NEXT_VALUE               // 6
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

type jsonContextStack []_ParseContext

func (s *jsonContextStack) push(v _ParseContext) {
	*s = append(*s, v)
}

func (s jsonContextStack) peek() (v _ParseContext, ok bool) {
	l := len(s)
	if l <= 0 {
		return
	}
	return s[l-1], true
}

func (s *jsonContextStack) pop() (v _ParseContext, ok bool) {
	l := len(*s)
	if l <= 0 {
		return
	}
	v = (*s)[l-1]
	*s = (*s)[0 : l-1]
	return v, true
}

var errEmptyJSONContextStack = types.NewProtocolExceptionWithType(types.INVALID_DATA, errors.New("Unexpected empty json protocol context stack"))

// JSON protocol implementation for thrift.
//
// This protocol produces/consumes a simple output format
// suitable for parsing by scripting languages.  It should not be
// confused with the full-featured JSONProtocol.
type simpleJSONFormat struct {
	buffer io.ReadWriter

	parseContextStack jsonContextStack
	dumpContext       jsonContextStack

	writer *bufio.Writer
	reader *bufio.Reader
}

var _ types.Format = (*simpleJSONFormat)(nil)

// NewSimpleJSONFormat creates a new simpleJSONFormat
func NewSimpleJSONFormat(buffer io.ReadWriter) types.Format {
	return newSimpleJSONFormat(buffer)
}

func newSimpleJSONFormat(buffer io.ReadWriter) *simpleJSONFormat {
	v := &simpleJSONFormat{
		buffer: buffer,
		writer: bufio.NewWriter(buffer),
		reader: bufio.NewReader(buffer),
	}
	v.resetContextStack()
	return v
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

func (p *simpleJSONFormat) WriteMessageBegin(name string, typeID types.MessageType, seqID int32) error {
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

func (p *simpleJSONFormat) WriteMessageEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteStructBegin(name string) error {
	if e := p.OutputObjectBegin(); e != nil {
		return e
	}
	return nil
}

func (p *simpleJSONFormat) WriteStructEnd() error {
	return p.OutputObjectEnd()
}

func (p *simpleJSONFormat) WriteFieldBegin(name string, typeID types.Type, id int16) error {
	if e := p.WriteString(name); e != nil {
		return e
	}
	return nil
}

func (p *simpleJSONFormat) WriteFieldEnd() error {
	//return p.OutputListEnd()
	return nil
}

func (p *simpleJSONFormat) WriteFieldStop() error { return nil }

func (p *simpleJSONFormat) WriteMapBegin(keyType types.Type, valueType types.Type, size int) error {
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

func (p *simpleJSONFormat) WriteMapEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteListBegin(elemType types.Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *simpleJSONFormat) WriteListEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteSetBegin(elemType types.Type, size int) error {
	return p.OutputElemListBegin(elemType, size)
}

func (p *simpleJSONFormat) WriteSetEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteBool(b bool) error {
	return p.OutputBool(b)
}

func (p *simpleJSONFormat) WriteByte(b byte) error {
	return p.WriteI32(int32(b))
}

func (p *simpleJSONFormat) WriteI16(v int16) error {
	return p.WriteI32(int32(v))
}

func (p *simpleJSONFormat) WriteI32(v int32) error {
	return p.OutputI64(int64(v))
}

func (p *simpleJSONFormat) WriteI64(v int64) error {
	return p.OutputI64(int64(v))
}

func (p *simpleJSONFormat) WriteDouble(v float64) error {
	return p.OutputF64(v)
}

func (p *simpleJSONFormat) WriteFloat(v float32) error {
	return p.OutputF32(v)
}

func (p *simpleJSONFormat) WriteString(v string) error {
	return p.OutputString(v)
}

func (p *simpleJSONFormat) WriteBinary(v []byte) error {
	// JSON library only takes in a string,
	// not an arbitrary byte array, to ensure bytes are transmitted
	// efficiently we must convert this into a valid JSON string
	// therefore we use base64 encoding to avoid excessive escaping/quoting
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(types.JSON_QUOTE_BYTES); e != nil {
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
	if _, e := p.write(types.JSON_QUOTE_BYTES); e != nil {
		return types.NewProtocolException(e)
	}
	return p.OutputPostValue()
}

// Reading methods.
func (p *simpleJSONFormat) ReadMessageBegin() (name string, typeID types.MessageType, seqID int32, err error) {
	p.resetContextStack() // THRIFT-3735
	if isNull, err := p.ParseListBegin(); isNull || err != nil {
		return name, typeID, seqID, err
	}
	if name, err = p.ReadString(); err != nil {
		return name, typeID, seqID, err
	}
	bTypeId, err := p.ReadByte()
	typeID = types.MessageType(bTypeId)
	if err != nil {
		return name, typeID, seqID, err
	}
	if seqID, err = p.ReadI32(); err != nil {
		return name, typeID, seqID, err
	}
	return name, typeID, seqID, nil
}

func (p *simpleJSONFormat) ReadMessageEnd() error {
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadStructBegin() (name string, err error) {
	_, err = p.ParseObjectStart()
	return "", err
}

func (p *simpleJSONFormat) ReadStructEnd() error {
	return p.ParseObjectEnd()
}

func (p *simpleJSONFormat) ReadFieldBegin() (string, types.Type, int16, error) {
	if err := p.ParsePreValue(); err != nil {
		return "", types.STOP, 0, err
	}
	b, _ := p.reader.Peek(1)
	if len(b) > 0 {
		switch b[0] {
		case types.JSON_RBRACE[0]:
			return "", types.STOP, 0, nil
		case types.JSON_QUOTE:
			p.reader.ReadByte()
			name, err := p.ParseStringBody()
			if err != nil {
				return name, types.STOP, 0, err
			}
			// In SimpleJSON - we only have field name when reading.
			// For field type - use VOID as placeholder.
			// For field ID - use NO_FIELD_ID as placeholder.
			return name, types.VOID, types.NO_FIELD_ID, p.ParsePostValue()
		}
		e := fmt.Errorf("Expected \"}\" or '\"', but found: '%s'", string(b))
		return "", types.STOP, 0, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return "", types.STOP, 0, types.NewProtocolException(io.EOF)
}

func (p *simpleJSONFormat) ReadFieldEnd() error {
	return nil
}

func (p *simpleJSONFormat) ReadMapBegin() (keyType types.Type, valueType types.Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return types.VOID, types.VOID, 0, e
	}

	// read keyType
	bKeyType, e := p.ReadByte()
	keyType = types.Type(bKeyType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read valueType
	bValueType, e := p.ReadByte()
	valueType = types.Type(bValueType)
	if e != nil {
		return keyType, valueType, size, e
	}

	// read size
	iSize, err := p.ReadI64()
	size = int(iSize)
	return keyType, valueType, size, err
}

func (p *simpleJSONFormat) ReadMapEnd() error {
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadListBegin() (elemType types.Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *simpleJSONFormat) ReadListEnd() error {
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadSetBegin() (elemType types.Type, size int, e error) {
	return p.ParseElemListBegin()
}

func (p *simpleJSONFormat) ReadSetEnd() error {
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadBool() (bool, error) {
	var value bool

	if err := p.ParsePreValue(); err != nil {
		return value, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 {
		switch f[0] {
		case types.JSON_TRUE[0]:
			b := make([]byte, len(types.JSON_TRUE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(types.JSON_TRUE) {
				value = true
			} else {
				e := fmt.Errorf("Expected \"true\" but found: %s", string(b))
				return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case types.JSON_FALSE[0]:
			b := make([]byte, len(types.JSON_FALSE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(types.JSON_FALSE) {
				value = false
			} else {
				e := fmt.Errorf("Expected \"false\" but found: %s", string(b))
				return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case types.JSON_NULL[0]:
			b := make([]byte, len(types.JSON_NULL))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(types.JSON_NULL) {
				value = false
			} else {
				e := fmt.Errorf("Expected \"null\" but found: %s", string(b))
				return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		default:
			e := fmt.Errorf("Expected \"true\", \"false\", or \"null\" but found: %s", string(f))
			return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	}
	return value, p.ParsePostValue()
}

func (p *simpleJSONFormat) ReadByte() (byte, error) {
	v, err := p.ReadI64()
	return byte(v), err
}

func (p *simpleJSONFormat) ReadI16() (int16, error) {
	v, err := p.ReadI64()
	return int16(v), err
}

func (p *simpleJSONFormat) ReadI32() (int32, error) {
	v, err := p.ReadI64()
	return int32(v), err
}

func (p *simpleJSONFormat) ReadI64() (int64, error) {
	v, _, err := p.ParseI64()
	return v, err
}

func (p *simpleJSONFormat) ReadDouble() (float64, error) {
	v, _, err := p.ParseF64()
	return v, err
}

func (p *simpleJSONFormat) ReadFloat() (float32, error) {
	v, _, err := p.ParseF32()
	return v, err
}

func (p *simpleJSONFormat) ReadString() (string, error) {
	var v string
	if err := p.ParsePreValue(); err != nil {
		return v, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 && f[0] == types.JSON_QUOTE {
		p.reader.ReadByte()
		value, err := p.ParseStringBody()
		v = value
		if err != nil {
			return v, err
		}
	} else if len(f) > 0 && f[0] == types.JSON_NULL[0] {
		b := make([]byte, len(types.JSON_NULL))
		_, err := p.reader.Read(b)
		if err != nil {
			return v, types.NewProtocolException(err)
		}
		if string(b) != string(types.JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return v, p.ParsePostValue()
}

func (p *simpleJSONFormat) ReadBinary() ([]byte, error) {
	var v []byte
	if err := p.ParsePreValue(); err != nil {
		return nil, err
	}
	f, _ := p.reader.Peek(1)
	if len(f) > 0 && f[0] == types.JSON_QUOTE {
		p.reader.ReadByte()
		value, err := p.ParseBase64EncodedBody()
		v = value
		if err != nil {
			return v, err
		}
	} else if len(f) > 0 && f[0] == types.JSON_NULL[0] {
		b := make([]byte, len(types.JSON_NULL))
		_, err := p.reader.Read(b)
		if err != nil {
			return v, types.NewProtocolException(err)
		}
		if string(b) != string(types.JSON_NULL) {
			e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(b))
			return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	} else {
		e := fmt.Errorf("Expected a JSON string, found unquoted data started with %s", string(f))
		return v, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}

	return v, p.ParsePostValue()
}

func (p *simpleJSONFormat) Flush() (err error) {
	return types.NewProtocolException(p.writer.Flush())
}

func (p *simpleJSONFormat) Skip(fieldType types.Type) (err error) {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *simpleJSONFormat) OutputPreValue() error {
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_LIST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		if _, e := p.write(types.JSON_COMMA); e != nil {
			return types.NewProtocolException(e)
		}
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		if _, e := p.write(types.JSON_COLON); e != nil {
			return types.NewProtocolException(e)
		}
	}
	return nil
}

func (p *simpleJSONFormat) OutputPostValue() error {
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_LIST_FIRST:
		p.dumpContext.pop()
		p.dumpContext.push(_CONTEXT_IN_LIST)
	case _CONTEXT_IN_OBJECT_FIRST:
		p.dumpContext.pop()
		p.dumpContext.push(_CONTEXT_IN_OBJECT_NEXT_VALUE)
	case _CONTEXT_IN_OBJECT_NEXT_KEY:
		p.dumpContext.pop()
		p.dumpContext.push(_CONTEXT_IN_OBJECT_NEXT_VALUE)
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		p.dumpContext.pop()
		p.dumpContext.push(_CONTEXT_IN_OBJECT_NEXT_KEY)
	}
	return nil
}

func (p *simpleJSONFormat) OutputBool(value bool) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if value {
		v = string(types.JSON_TRUE)
	} else {
		v = string(types.JSON_FALSE)
	}
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		v = jsonQuote(v)
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputNull() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(types.JSON_NULL); e != nil {
		return types.NewProtocolException(e)
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputF64(value float64) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if math.IsNaN(value) {
		v = string(types.JSON_QUOTE) + types.JSON_NAN + string(types.JSON_QUOTE)
	} else if math.IsInf(value, 1) {
		v = string(types.JSON_QUOTE) + types.JSON_INFINITY + string(types.JSON_QUOTE)
	} else if math.IsInf(value, -1) {
		v = string(types.JSON_QUOTE) + types.JSON_NEGATIVE_INFINITY + string(types.JSON_QUOTE)
	} else {
		cxt, ok := p.dumpContext.peek()
		if !ok {
			return errEmptyJSONContextStack
		}
		v = strconv.FormatFloat(value, 'g', -1, 64)
		switch cxt {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(types.JSON_QUOTE) + v + string(types.JSON_QUOTE)
		}
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputF32(value float32) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	var v string
	if math.IsNaN(float64(value)) {
		v = string(types.JSON_QUOTE) + types.JSON_NAN + string(types.JSON_QUOTE)
	} else if math.IsInf(float64(value), 1) {
		v = string(types.JSON_QUOTE) + types.JSON_INFINITY + string(types.JSON_QUOTE)
	} else if math.IsInf(float64(value), -1) {
		v = string(types.JSON_QUOTE) + types.JSON_NEGATIVE_INFINITY + string(types.JSON_QUOTE)
	} else {
		cxt, ok := p.dumpContext.peek()
		if !ok {
			return errEmptyJSONContextStack
		}
		v = strconv.FormatFloat(float64(value), 'g', -1, 32)
		switch cxt {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(types.JSON_QUOTE) + v + string(types.JSON_QUOTE)
		}
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputI64(value int64) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	v := strconv.FormatInt(value, 10)
	switch cxt {
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		v = jsonQuote(v)
	}
	if e := p.OutputStringData(v); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputString(s string) error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if e := p.OutputStringData(jsonQuote(s)); e != nil {
		return e
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputStringData(s string) error {
	_, e := p.write([]byte(s))
	return types.NewProtocolException(e)
}

func (p *simpleJSONFormat) OutputObjectBegin() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(types.JSON_LBRACE); e != nil {
		return types.NewProtocolException(e)
	}
	p.dumpContext.push(_CONTEXT_IN_OBJECT_FIRST)
	return nil
}

func (p *simpleJSONFormat) OutputObjectEnd() error {
	if _, e := p.write(types.JSON_RBRACE); e != nil {
		return types.NewProtocolException(e)
	}
	_, ok := p.dumpContext.pop()
	if !ok {
		return errEmptyJSONContextStack
	}
	if e := p.OutputPostValue(); e != nil {
		return e
	}
	return nil
}

func (p *simpleJSONFormat) OutputListBegin() error {
	if e := p.OutputPreValue(); e != nil {
		return e
	}
	if _, e := p.write(types.JSON_LBRACKET); e != nil {
		return types.NewProtocolException(e)
	}
	p.dumpContext.push(_CONTEXT_IN_LIST_FIRST)
	return nil
}

func (p *simpleJSONFormat) OutputListEnd() error {
	if _, e := p.write(types.JSON_RBRACKET); e != nil {
		return types.NewProtocolException(e)
	}
	_, ok := p.dumpContext.pop()
	if !ok {
		return errEmptyJSONContextStack
	}
	if e := p.OutputPostValue(); e != nil {
		return e
	}
	return nil
}

func (p *simpleJSONFormat) OutputElemListBegin(elemType types.Type, size int) error {
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

func (p *simpleJSONFormat) ParsePreValue() error {
	if e := p.readNonSignificantWhitespace(); e != nil {
		return types.NewProtocolException(e)
	}
	cxt, ok := p.parseContextStack.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	b, _ := p.reader.Peek(1)
	switch cxt {
	case _CONTEXT_IN_LIST:
		if len(b) > 0 {
			switch b[0] {
			case types.JSON_RBRACKET[0]:
				return nil
			case types.JSON_COMMA[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return types.NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \"]\" or \",\" in list context, but found \"%s\"", string(b))
				return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		}
	case _CONTEXT_IN_OBJECT_NEXT_KEY:
		if len(b) > 0 {
			switch b[0] {
			case types.JSON_RBRACE[0]:
				return nil
			case types.JSON_COMMA[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return types.NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \"}\" or \",\" in object context, but found \"%s\"", string(b))
				return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		}
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		if len(b) > 0 {
			switch b[0] {
			case types.JSON_COLON[0]:
				p.reader.ReadByte()
				if e := p.readNonSignificantWhitespace(); e != nil {
					return types.NewProtocolException(e)
				}
				return nil
			default:
				e := fmt.Errorf("Expected \":\" in object context, but found \"%s\"", string(b))
				return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		}
	}
	return nil
}

func (p *simpleJSONFormat) ParsePostValue() error {
	if e := p.readNonSignificantWhitespace(); e != nil {
		return types.NewProtocolException(e)
	}
	cxt, ok := p.parseContextStack.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_LIST_FIRST:
		p.parseContextStack.pop()
		p.parseContextStack.push(_CONTEXT_IN_LIST)
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		p.parseContextStack.pop()
		p.parseContextStack.push(_CONTEXT_IN_OBJECT_NEXT_VALUE)
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		p.parseContextStack.pop()
		p.parseContextStack.push(_CONTEXT_IN_OBJECT_NEXT_KEY)
	}
	return nil
}

func (p *simpleJSONFormat) readNonSignificantWhitespace() error {
	for {
		b, _ := p.reader.Peek(1)
		if len(b) < 1 {
			return nil
		}
		switch b[0] {
		case ' ', '\r', '\n', '\t':
			p.reader.ReadByte()
			continue
		}
		break
	}
	return nil
}

func (p *simpleJSONFormat) ParseStringBody() (string, error) {
	line, err := p.reader.ReadString(types.JSON_QUOTE)
	if err != nil {
		return "", types.NewProtocolException(err)
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
		v, ok := jsonUnquote(string(types.JSON_QUOTE) + line)
		if !ok {
			return "", types.NewProtocolException(err)
		}
		return v, nil
	}
	s, err := p.ParseQuotedStringBody()
	if err != nil {
		return "", types.NewProtocolException(err)
	}
	str := string(types.JSON_QUOTE) + line + s
	v, ok := jsonUnquote(str)
	if !ok {
		e := fmt.Errorf("Unable to parse as JSON string %s", str)
		return "", types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return v, nil
}

func (p *simpleJSONFormat) ParseQuotedStringBody() (string, error) {
	line, err := p.reader.ReadString(types.JSON_QUOTE)
	if err != nil {
		return "", types.NewProtocolException(err)
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
		return "", types.NewProtocolException(err)
	}
	v := line + s
	return v, nil
}

func (p *simpleJSONFormat) ParseBase64EncodedBody() ([]byte, error) {
	line, err := p.reader.ReadBytes(types.JSON_QUOTE)
	if err != nil {
		return line, types.NewProtocolException(err)
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
	return output[0:n], types.NewProtocolException(err)
}

func (p *simpleJSONFormat) ParseI64() (int64, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value int64
	var isnull bool
	if p.safePeekContains(types.JSON_NULL) {
		p.reader.Read(make([]byte, len(types.JSON_NULL)))
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

func (p *simpleJSONFormat) ParseF64() (float64, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value float64
	var isnull bool
	if p.safePeekContains(types.JSON_NULL) {
		p.reader.Read(make([]byte, len(types.JSON_NULL)))
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

func (p *simpleJSONFormat) ParseF32() (float32, bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return 0, false, err
	}
	var value float32
	var isnull bool
	if p.safePeekContains(types.JSON_NULL) {
		p.reader.Read(make([]byte, len(types.JSON_NULL)))
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

func (p *simpleJSONFormat) ParseObjectStart() (bool, error) {
	if err := p.ParsePreValue(); err != nil {
		return false, err
	}
	var b []byte
	b, err := p.reader.Peek(1)
	if err != nil {
		return false, err
	}
	if len(b) > 0 && b[0] == types.JSON_LBRACE[0] {
		p.reader.ReadByte()
		p.parseContextStack.push(_CONTEXT_IN_OBJECT_FIRST)
		return false, nil
	} else if p.safePeekContains(types.JSON_NULL) {
		return true, nil
	}
	e := fmt.Errorf("Expected '{' or null, but found '%s'", string(b))
	return false, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
}

func (p *simpleJSONFormat) ParseObjectEnd() error {
	if isNull, err := p.readIfNull(); isNull || err != nil {
		return err
	}
	cxt, ok := p.parseContextStack.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	if (cxt != _CONTEXT_IN_OBJECT_FIRST) && (cxt != _CONTEXT_IN_OBJECT_NEXT_KEY) {
		e := fmt.Errorf("Expected to be in the Object Context, but not in Object Context (%d)", cxt)
		return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	line, err := p.reader.ReadString(types.JSON_RBRACE[0])
	if err != nil {
		return types.NewProtocolException(err)
	}
	for _, char := range line {
		switch char {
		default:
			e := fmt.Errorf("Expecting end of object \"}\", but found: \"%s\"", line)
			return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		case ' ', '\n', '\r', '\t', '}':
			// do nothing
		}
	}
	p.parseContextStack.pop()
	return p.ParsePostValue()
}

func (p *simpleJSONFormat) ParseListBegin() (isNull bool, err error) {
	if e := p.ParsePreValue(); e != nil {
		return false, e
	}
	var b []byte
	b, err = p.reader.Peek(1)
	if err != nil {
		return false, err
	}
	if len(b) >= 1 && b[0] == types.JSON_LBRACKET[0] {
		p.parseContextStack.push(_CONTEXT_IN_LIST_FIRST)
		p.reader.ReadByte()
		isNull = false
	} else if p.safePeekContains(types.JSON_NULL) {
		isNull = true
	} else {
		err = fmt.Errorf("Expected \"null\" or \"[\", received %q", b)
	}
	return isNull, types.NewProtocolExceptionWithType(types.INVALID_DATA, err)
}

func (p *simpleJSONFormat) ParseElemListBegin() (elemType types.Type, size int, e error) {
	if isNull, e := p.ParseListBegin(); isNull || e != nil {
		return types.VOID, 0, e
	}
	bElemType, err := p.ReadByte()
	elemType = types.Type(bElemType)
	if err != nil {
		return elemType, size, err
	}
	nSize, err2 := p.ReadI64()
	size = int(nSize)
	return elemType, size, err2
}

func (p *simpleJSONFormat) ParseListEnd() error {
	if isNull, err := p.readIfNull(); isNull || err != nil {
		return err
	}
	cxt, _ := p.parseContextStack.peek()
	if cxt != _CONTEXT_IN_LIST {
		e := fmt.Errorf("Expected to be in the List Context, but not in List Context (%d)", cxt)
		return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	line, err := p.reader.ReadString(types.JSON_RBRACKET[0])
	if err != nil {
		return types.NewProtocolException(err)
	}
	for _, char := range line {
		switch char {
		default:
			e := fmt.Errorf("Expecting end of list \"]\", but found: %q", line)
			return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		case ' ', '\n', '\r', '\t', rune(types.JSON_RBRACKET[0]):
			// do nothing
		}
	}
	p.parseContextStack.pop()
	if _ParseContext(p.parseContextStack[len(p.parseContextStack)-1]) == _CONTEXT_IN_TOPLEVEL {
		return nil
	}
	return p.ParsePostValue()
}

func (p *simpleJSONFormat) readSingleValue() (interface{}, types.Type, error) {
	e := p.readNonSignificantWhitespace()
	if e != nil {
		return nil, types.VOID, types.NewProtocolException(e)
	}
	b, e := p.reader.Peek(1)
	if len(b) > 0 {
		c := b[0]
		switch c {
		case types.JSON_NULL[0]:
			buf := make([]byte, len(types.JSON_NULL))
			_, e := p.reader.Read(buf)
			if e != nil {
				return nil, types.VOID, types.NewProtocolException(e)
			}
			if string(types.JSON_NULL) != string(buf) {
				e = mismatch(string(types.JSON_NULL), string(buf))
				return nil, types.VOID, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return nil, types.VOID, nil
		case types.JSON_QUOTE:
			p.reader.ReadByte()
			v, e := p.ParseStringBody()
			if e != nil {
				return v, types.UTF8, types.NewProtocolException(e)
			}
			if v == types.JSON_INFINITY {
				return types.INFINITY, types.DOUBLE, nil
			} else if v == types.JSON_NEGATIVE_INFINITY {
				return types.NEGATIVE_INFINITY, types.DOUBLE, nil
			} else if v == types.JSON_NAN {
				return types.NAN, types.DOUBLE, nil
			}
			return v, types.UTF8, nil
		case types.JSON_TRUE[0]:
			buf := make([]byte, len(types.JSON_TRUE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return true, types.BOOL, types.NewProtocolException(e)
			}
			if string(types.JSON_TRUE) != string(buf) {
				e := mismatch(string(types.JSON_TRUE), string(buf))
				return true, types.BOOL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return true, types.BOOL, nil
		case types.JSON_FALSE[0]:
			buf := make([]byte, len(types.JSON_FALSE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return false, types.BOOL, types.NewProtocolException(e)
			}
			if string(types.JSON_FALSE) != string(buf) {
				e := mismatch(string(types.JSON_FALSE), string(buf))
				return false, types.BOOL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return false, types.BOOL, nil
		case types.JSON_LBRACKET[0]:
			_, e := p.reader.ReadByte()
			return make([]interface{}, 0), types.LIST, types.NewProtocolException(e)
		case types.JSON_LBRACE[0]:
			_, e := p.reader.ReadByte()
			return make(map[string]interface{}), types.STRUCT, types.NewProtocolException(e)
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'e', 'E', '.', '+', '-', types.JSON_INFINITY[0], types.JSON_NAN[0]:
			// assume numeric
			v, e := p.readNumeric()
			return v, types.DOUBLE, e
		default:
			e := fmt.Errorf("Expected element in list but found '%s' while parsing JSON.", string(c))
			return nil, types.VOID, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	}
	e = fmt.Errorf("Cannot read a single element while parsing JSON.")
	return nil, types.VOID, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)

}

func (p *simpleJSONFormat) readIfNull() (bool, error) {
	cont := true
	for cont {
		b, _ := p.reader.Peek(1)
		if len(b) < 1 {
			return false, nil
		}
		switch b[0] {
		default:
			return false, nil
		case types.JSON_NULL[0]:
			cont = false
		case ' ', '\n', '\r', '\t':
			p.reader.ReadByte()
		}
	}
	if p.safePeekContains(types.JSON_NULL) {
		p.reader.Read(make([]byte, len(types.JSON_NULL)))
		return true, nil
	}
	return false, nil
}

func (p *simpleJSONFormat) readQuoteIfNext() {
	b, _ := p.reader.Peek(1)
	if len(b) > 0 && b[0] == types.JSON_QUOTE {
		p.reader.ReadByte()
	}
}

func (p *simpleJSONFormat) readNumeric() (types.Numeric, error) {
	isNull, err := p.readIfNull()
	if isNull || err != nil {
		return types.NUMERIC_NULL, err
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
			return types.NUMERIC_NULL, types.NewProtocolException(err)
		}
		switch c {
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			buf.WriteByte(c)
			nextCanBeSign = false
		case '.':
			if hasDecimalPoint {
				e := fmt.Errorf("Unable to parse number with multiple decimal points '%s.'", buf.String())
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			if hasE {
				e := fmt.Errorf("Unable to parse number with decimal points in the exponent '%s.'", buf.String())
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasDecimalPoint, nextCanBeSign = true, false
		case 'e', 'E':
			if hasE {
				e := fmt.Errorf("Unable to parse number with multiple exponents '%s%c'", buf.String(), c)
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasE, nextCanBeSign = true, true
		case '-', '+':
			if !nextCanBeSign {
				e := fmt.Errorf("Negative sign within number")
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			buf.WriteByte(c)
			nextCanBeSign = false
		case ' ', 0, '\t', '\n', '\r', types.JSON_RBRACE[0], types.JSON_RBRACKET[0], types.JSON_COMMA[0], types.JSON_COLON[0]:
			p.reader.UnreadByte()
			continueFor = false
		case types.JSON_NAN[0]:
			if buf.Len() == 0 {
				buffer := make([]byte, len(types.JSON_NAN))
				buffer[0] = c
				_, e := p.reader.Read(buffer[1:])
				if e != nil {
					return types.NUMERIC_NULL, types.NewProtocolException(e)
				}
				if types.JSON_NAN != string(buffer) {
					e := mismatch(types.JSON_NAN, string(buffer))
					return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return types.NAN, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case types.JSON_INFINITY[0]:
			if buf.Len() == 0 || (buf.Len() == 1 && buf.Bytes()[0] == '+') {
				buffer := make([]byte, len(types.JSON_INFINITY))
				buffer[0] = c
				_, e := p.reader.Read(buffer[1:])
				if e != nil {
					return types.NUMERIC_NULL, types.NewProtocolException(e)
				}
				if types.JSON_INFINITY != string(buffer) {
					e := mismatch(types.JSON_INFINITY, string(buffer))
					return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return types.INFINITY, nil
			} else if buf.Len() == 1 && buf.Bytes()[0] == types.JSON_NEGATIVE_INFINITY[0] {
				buffer := make([]byte, len(types.JSON_NEGATIVE_INFINITY))
				buffer[0] = types.JSON_NEGATIVE_INFINITY[0]
				buffer[1] = c
				_, e := p.reader.Read(buffer[2:])
				if e != nil {
					return types.NUMERIC_NULL, types.NewProtocolException(e)
				}
				if types.JSON_NEGATIVE_INFINITY != string(buffer) {
					e := mismatch(types.JSON_NEGATIVE_INFINITY, string(buffer))
					return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return types.NEGATIVE_INFINITY, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c' due to existing buffer %s", c, buf.String())
				return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case types.JSON_QUOTE:
			if !inQuotes {
				inQuotes = true
			}
		default:
			e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
			return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	}
	if buf.Len() == 0 {
		e := fmt.Errorf("Unable to parse number from empty string ''")
		return types.NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return types.NewNumericFromJSONString(buf.String(), false), nil
}

// Safely peeks into the buffer, reading only what is necessary
func (p *simpleJSONFormat) safePeekContains(b []byte) bool {
	for i := 0; i < len(b); i++ {
		a, _ := p.reader.Peek(i + 1)
		if len(a) < (i+1) || a[i] != b[i] {
			return false
		}
	}
	return true
}

// Reset the context stack to its initial state.
func (p *simpleJSONFormat) resetContextStack() {
	p.parseContextStack = jsonContextStack{_CONTEXT_IN_TOPLEVEL}
	p.dumpContext = jsonContextStack{_CONTEXT_IN_TOPLEVEL}
}

func (p *simpleJSONFormat) write(b []byte) (int, error) {
	n, err := p.writer.Write(b)
	if err != nil {
		p.writer.Reset(p.buffer) // THRIFT-3735
	}
	return n, err
}
