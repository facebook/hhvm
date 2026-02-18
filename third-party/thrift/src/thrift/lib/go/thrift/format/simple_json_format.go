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

package format

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

var (
	JSON_COMMA                   = []byte{','}
	JSON_COLON                   = []byte{':'}
	JSON_LBRACE                  = []byte{'{'}
	JSON_RBRACE                  = []byte{'}'}
	JSON_LBRACKET                = []byte{'['}
	JSON_RBRACKET                = []byte{']'}
	JSON_QUOTE                   = byte('"')
	JSON_QUOTE_BYTES             = []byte{'"'}
	JSON_NULL                    = []byte{'n', 'u', 'l', 'l'}
	JSON_TRUE                    = []byte{'t', 'r', 'u', 'e'}
	JSON_FALSE                   = []byte{'f', 'a', 'l', 's', 'e'}
	JSON_INFINITY                = "Infinity"
	JSON_NEGATIVE_INFINITY       = "-Infinity"
	JSON_NAN                     = "NaN"
	JSON_INFINITY_BYTES          = []byte{'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NEGATIVE_INFINITY_BYTES = []byte{'-', 'I', 'n', 'f', 'i', 'n', 'i', 't', 'y'}
	JSON_NAN_BYTES               = []byte{'N', 'a', 'N'}
	json_nonbase_map_elem_bytes  = []byte{']', ',', '['}
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

	containerBugFix bool
}

var _ types.Format = (*simpleJSONFormat)(nil)

// NewSimpleJSONFormat creates a new simpleJSONFormat
func NewSimpleJSONFormat(readWriter io.ReadWriter) types.Format {
	return newSimpleJSONFormat(readWriter)
}

func newSimpleJSONFormat(readWriter io.ReadWriter) *simpleJSONFormat {
	v := &simpleJSONFormat{
		buffer: readWriter,
		writer: bufio.NewWriter(readWriter),
		reader: bufio.NewReader(readWriter),
	}
	v.resetContextStack()
	return v
}

func newSimpleJSONFormatV2(readWriter io.ReadWriter) *simpleJSONFormat {
	v := &simpleJSONFormat{
		buffer:          readWriter,
		writer:          bufio.NewWriter(readWriter),
		reader:          bufio.NewReader(readWriter),
		containerBugFix: true,
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
	if err := p.OutputListBegin(); err != nil {
		return err
	}
	if err := p.WriteString(name); err != nil {
		return err
	}
	if err := p.WriteByte(byte(typeID)); err != nil {
		return err
	}
	if err := p.WriteI32(seqID); err != nil {
		return err
	}
	return nil
}

func (p *simpleJSONFormat) WriteMessageEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteStructBegin(name string) error {
	if err := p.OutputObjectBegin(); err != nil {
		return err
	}
	return nil
}

func (p *simpleJSONFormat) WriteStructEnd() error {
	return p.OutputObjectEnd()
}

func (p *simpleJSONFormat) WriteFieldBegin(name string, typeID types.Type, id int16) error {
	if err := p.WriteString(name); err != nil {
		return err
	}
	return nil
}

func (p *simpleJSONFormat) WriteFieldEnd() error {
	//return p.OutputListEnd()
	return nil
}

func (p *simpleJSONFormat) WriteFieldStop() error { return nil }

func (p *simpleJSONFormat) WriteMapBegin(keyType types.Type, valueType types.Type, size int) error {
	if p.containerBugFix {
		return p.OutputObjectBegin()
	}
	if err := p.OutputListBegin(); err != nil {
		return err
	}
	if err := p.WriteByte(byte(keyType)); err != nil {
		return err
	}
	if err := p.WriteByte(byte(valueType)); err != nil {
		return err
	}
	return p.WriteI32(int32(size))
}

func (p *simpleJSONFormat) WriteMapEnd() error {
	if p.containerBugFix {
		return p.OutputObjectEnd()
	}
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteListBegin(elemType types.Type, size int) error {
	if p.containerBugFix {
		return p.OutputListBegin()
	}
	return p.OutputElemListBegin(elemType, size)
}

func (p *simpleJSONFormat) WriteListEnd() error {
	return p.OutputListEnd()
}

func (p *simpleJSONFormat) WriteSetBegin(elemType types.Type, size int) error {
	if p.containerBugFix {
		return p.OutputListBegin()
	}
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
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	if _, err := p.write(JSON_QUOTE_BYTES); err != nil {
		return types.NewProtocolException(err)
	}
	if len(v) > 0 {
		writer := base64.NewEncoder(base64.StdEncoding, p.writer)
		if _, err := writer.Write(v); err != nil {
			p.writer.Reset(p.buffer) // THRIFT-3735
			return types.NewProtocolException(err)
		}
		if err := writer.Close(); err != nil {
			return types.NewProtocolException(err)
		}
	}
	if _, err := p.write(JSON_QUOTE_BYTES); err != nil {
		return types.NewProtocolException(err)
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

func (p *simpleJSONFormat) ReadStructBegin() (string, error) {
	_, err := p.ParseObjectStart()
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
		case JSON_RBRACE[0]:
			return "", types.STOP, 0, nil
		case JSON_QUOTE:
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

func (p *simpleJSONFormat) ReadMapBegin() (types.Type /* kType */, types.Type /* vType */, int /* size */, error) {
	if p.containerBugFix {
		isNull, err := p.ParseObjectStart()
		if isNull || err != nil {
			return types.STOP, types.STOP, 0, err
		}
		// SimpleJSON does not encode key/value types or size; return placeholders
		return types.STOP, types.STOP, -1, nil
	}
	isNull, err := p.ParseListBegin()
	if isNull || err != nil {
		return 0, 0, 0, err
	}

	// read kType
	kTypeByte, err := p.ReadByte()
	kType := types.Type(kTypeByte)
	if err != nil {
		return 0, 0, 0, err
	}

	// read vType
	vTypeByte, err := p.ReadByte()
	vType := types.Type(vTypeByte)
	if err != nil {
		return 0, 0, 0, err
	}

	// read size
	size64, err := p.ReadI64()
	size := int(size64)
	return kType, vType, size, err
}

func (p *simpleJSONFormat) ReadMapEnd() error {
	if p.containerBugFix {
		return p.ParseObjectEnd()
	}
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadListBegin() (types.Type /* elemType */, int /* size */, error) {
	if p.containerBugFix {
		isNull, err := p.ParseListBegin()
		if isNull || err != nil {
			return types.STOP, 0, err
		}
		// SimpleJSON does not encode element type or size; return placeholders
		return types.STOP, -1, nil
	}
	return p.ParseElemListBegin()
}

func (p *simpleJSONFormat) ReadListEnd() error {
	return p.ParseListEnd()
}

func (p *simpleJSONFormat) ReadSetBegin() (types.Type /* elemType */, int /* size */, error) {
	if p.containerBugFix {
		isNull, err := p.ParseListBegin()
		if isNull || err != nil {
			return types.STOP, 0, err
		}
		// SimpleJSON does not encode element type or size; return placeholders
		return types.STOP, -1, nil
	}
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
		case JSON_TRUE[0]:
			b := make([]byte, len(JSON_TRUE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(JSON_TRUE) {
				value = true
			} else {
				e := fmt.Errorf("Expected \"true\" but found: %s", string(b))
				return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case JSON_FALSE[0]:
			b := make([]byte, len(JSON_FALSE))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(JSON_FALSE) {
				value = false
			} else {
				e := fmt.Errorf("Expected \"false\" but found: %s", string(b))
				return value, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case JSON_NULL[0]:
			b := make([]byte, len(JSON_NULL))
			_, err := p.reader.Read(b)
			if err != nil {
				return false, types.NewProtocolException(err)
			}
			if string(b) == string(JSON_NULL) {
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

func (p *simpleJSONFormat) ReadBinary() ([]byte, error) {
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

func (p *simpleJSONFormat) Flush() error {
	return types.NewProtocolException(p.writer.Flush())
}

func (p *simpleJSONFormat) Skip(fieldType types.Type) error {
	return types.SkipDefaultDepth(p, fieldType)
}

func (p *simpleJSONFormat) OutputPreValue() error {
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_LIST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		if _, err := p.write(JSON_COMMA); err != nil {
			return types.NewProtocolException(err)
		}
	case _CONTEXT_IN_OBJECT_NEXT_VALUE:
		if _, err := p.write(JSON_COLON); err != nil {
			return types.NewProtocolException(err)
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
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	var v string
	if value {
		v = string(JSON_TRUE)
	} else {
		v = string(JSON_FALSE)
	}
	cxt, ok := p.dumpContext.peek()
	if !ok {
		return errEmptyJSONContextStack
	}
	switch cxt {
	case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
		v = jsonQuote(v)
	}
	if err := p.OutputStringData(v); err != nil {
		return err
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputNull() error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	if _, err := p.write(JSON_NULL); err != nil {
		return types.NewProtocolException(err)
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputF64(value float64) error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	var v string
	if math.IsNaN(value) {
		v = string(JSON_QUOTE) + JSON_NAN + string(JSON_QUOTE)
	} else if math.IsInf(value, 1) {
		v = string(JSON_QUOTE) + JSON_INFINITY + string(JSON_QUOTE)
	} else if math.IsInf(value, -1) {
		v = string(JSON_QUOTE) + JSON_NEGATIVE_INFINITY + string(JSON_QUOTE)
	} else {
		cxt, ok := p.dumpContext.peek()
		if !ok {
			return errEmptyJSONContextStack
		}
		v = strconv.FormatFloat(value, 'g', -1, 64)
		switch cxt {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(JSON_QUOTE) + v + string(JSON_QUOTE)
		}
	}
	if err := p.OutputStringData(v); err != nil {
		return err
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputF32(value float32) error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	var v string
	if math.IsNaN(float64(value)) {
		v = string(JSON_QUOTE) + JSON_NAN + string(JSON_QUOTE)
	} else if math.IsInf(float64(value), 1) {
		v = string(JSON_QUOTE) + JSON_INFINITY + string(JSON_QUOTE)
	} else if math.IsInf(float64(value), -1) {
		v = string(JSON_QUOTE) + JSON_NEGATIVE_INFINITY + string(JSON_QUOTE)
	} else {
		cxt, ok := p.dumpContext.peek()
		if !ok {
			return errEmptyJSONContextStack
		}
		v = strconv.FormatFloat(float64(value), 'g', -1, 32)
		switch cxt {
		case _CONTEXT_IN_OBJECT_FIRST, _CONTEXT_IN_OBJECT_NEXT_KEY:
			v = string(JSON_QUOTE) + v + string(JSON_QUOTE)
		}
	}
	if err := p.OutputStringData(v); err != nil {
		return err
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputI64(value int64) error {
	if err := p.OutputPreValue(); err != nil {
		return err
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
	if err := p.OutputStringData(v); err != nil {
		return err
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputString(s string) error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	if err := p.OutputStringData(jsonQuote(s)); err != nil {
		return err
	}
	return p.OutputPostValue()
}

func (p *simpleJSONFormat) OutputStringData(s string) error {
	_, err := p.write([]byte(s))
	return types.NewProtocolException(err)
}

func (p *simpleJSONFormat) OutputObjectBegin() error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	if _, e := p.write(JSON_LBRACE); e != nil {
		return types.NewProtocolException(e)
	}
	p.dumpContext.push(_CONTEXT_IN_OBJECT_FIRST)
	return nil
}

func (p *simpleJSONFormat) OutputObjectEnd() error {
	if _, err := p.write(JSON_RBRACE); err != nil {
		return types.NewProtocolException(err)
	}
	_, ok := p.dumpContext.pop()
	if !ok {
		return errEmptyJSONContextStack
	}
	if err := p.OutputPostValue(); err != nil {
		return err
	}
	return nil
}

func (p *simpleJSONFormat) OutputListBegin() error {
	if err := p.OutputPreValue(); err != nil {
		return err
	}
	if _, err := p.write(JSON_LBRACKET); err != nil {
		return types.NewProtocolException(err)
	}
	p.dumpContext.push(_CONTEXT_IN_LIST_FIRST)
	return nil
}

func (p *simpleJSONFormat) OutputListEnd() error {
	if _, err := p.write(JSON_RBRACKET); err != nil {
		return types.NewProtocolException(err)
	}
	_, ok := p.dumpContext.pop()
	if !ok {
		return errEmptyJSONContextStack
	}
	if err := p.OutputPostValue(); err != nil {
		return err
	}
	return nil
}

func (p *simpleJSONFormat) OutputElemListBegin(elemType types.Type, size int) error {
	if err := p.OutputListBegin(); err != nil {
		return err
	}
	if err := p.WriteByte(byte(elemType)); err != nil {
		return err
	}
	if err := p.WriteI64(int64(size)); err != nil {
		return err
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
			case JSON_RBRACKET[0]:
				return nil
			case JSON_COMMA[0]:
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
			case JSON_RBRACE[0]:
				return nil
			case JSON_COMMA[0]:
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
			case JSON_COLON[0]:
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
	if err := p.readNonSignificantWhitespace(); err != nil {
		return types.NewProtocolException(err)
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
	line, err := p.reader.ReadString(JSON_QUOTE)
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
		v, ok := jsonUnquote(string(JSON_QUOTE) + line)
		if !ok {
			return "", types.NewProtocolException(err)
		}
		return v, nil
	}
	s, err := p.ParseQuotedStringBody()
	if err != nil {
		return "", types.NewProtocolException(err)
	}
	str := string(JSON_QUOTE) + line + s
	v, ok := jsonUnquote(str)
	if !ok {
		e := fmt.Errorf("Unable to parse as JSON string %s", str)
		return "", types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return v, nil
}

func (p *simpleJSONFormat) ParseQuotedStringBody() (string, error) {
	line, err := p.reader.ReadString(JSON_QUOTE)
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
	line, err := p.reader.ReadBytes(JSON_QUOTE)
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

func (p *simpleJSONFormat) ParseF64() (float64, bool, error) {
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

func (p *simpleJSONFormat) ParseF32() (float32, bool, error) {
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

func (p *simpleJSONFormat) ParseObjectStart() (bool, error) {
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
		p.parseContextStack.push(_CONTEXT_IN_OBJECT_FIRST)
		return false, nil
	} else if p.safePeekContains(JSON_NULL) {
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
	line, err := p.reader.ReadString(JSON_RBRACE[0])
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
	if len(b) >= 1 && b[0] == JSON_LBRACKET[0] {
		p.parseContextStack.push(_CONTEXT_IN_LIST_FIRST)
		p.reader.ReadByte()
		isNull = false
	} else if p.safePeekContains(JSON_NULL) {
		isNull = true
	} else {
		err = fmt.Errorf("Expected \"null\" or \"[\", received %q", b)
	}
	return isNull, types.NewProtocolExceptionWithType(types.INVALID_DATA, err)
}

func (p *simpleJSONFormat) ParseElemListBegin() (elemType types.Type, size int, err error) {
	if isNull, err := p.ParseListBegin(); isNull || err != nil {
		return types.VOID, 0, err
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
	line, err := p.reader.ReadString(JSON_RBRACKET[0])
	if err != nil {
		return types.NewProtocolException(err)
	}
	for _, char := range line {
		switch char {
		default:
			e := fmt.Errorf("Expecting end of list \"]\", but found: %q", line)
			return types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		case ' ', '\n', '\r', '\t', rune(JSON_RBRACKET[0]):
			// do nothing
		}
	}
	p.parseContextStack.pop()
	if _ParseContext(p.parseContextStack[len(p.parseContextStack)-1]) == _CONTEXT_IN_TOPLEVEL {
		return nil
	}
	return p.ParsePostValue()
}

func (p *simpleJSONFormat) readSingleValue() (any, types.Type, error) {
	e := p.readNonSignificantWhitespace()
	if e != nil {
		return nil, types.VOID, types.NewProtocolException(e)
	}
	b, e := p.reader.Peek(1)
	if len(b) > 0 {
		c := b[0]
		switch c {
		case JSON_NULL[0]:
			buf := make([]byte, len(JSON_NULL))
			_, e := p.reader.Read(buf)
			if e != nil {
				return nil, types.VOID, types.NewProtocolException(e)
			}
			if string(JSON_NULL) != string(buf) {
				e = mismatch(string(JSON_NULL), string(buf))
				return nil, types.VOID, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return nil, types.VOID, nil
		case JSON_QUOTE:
			p.reader.ReadByte()
			v, e := p.ParseStringBody()
			if e != nil {
				return v, types.UTF8, types.NewProtocolException(e)
			}
			if v == JSON_INFINITY {
				return INFINITY, types.DOUBLE, nil
			} else if v == JSON_NEGATIVE_INFINITY {
				return NEGATIVE_INFINITY, types.DOUBLE, nil
			} else if v == JSON_NAN {
				return NAN, types.DOUBLE, nil
			}
			return v, types.UTF8, nil
		case JSON_TRUE[0]:
			buf := make([]byte, len(JSON_TRUE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return true, types.BOOL, types.NewProtocolException(e)
			}
			if string(JSON_TRUE) != string(buf) {
				e := mismatch(string(JSON_TRUE), string(buf))
				return true, types.BOOL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return true, types.BOOL, nil
		case JSON_FALSE[0]:
			buf := make([]byte, len(JSON_FALSE))
			_, e := p.reader.Read(buf)
			if e != nil {
				return false, types.BOOL, types.NewProtocolException(e)
			}
			if string(JSON_FALSE) != string(buf) {
				e := mismatch(string(JSON_FALSE), string(buf))
				return false, types.BOOL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			return false, types.BOOL, nil
		case JSON_LBRACKET[0]:
			_, e := p.reader.ReadByte()
			return make([]any, 0), types.LIST, types.NewProtocolException(e)
		case JSON_LBRACE[0]:
			_, e := p.reader.ReadByte()
			return make(map[string]any), types.STRUCT, types.NewProtocolException(e)
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'e', 'E', '.', '+', '-', JSON_INFINITY[0], JSON_NAN[0]:
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
		case JSON_NULL[0]:
			cont = false
		case ' ', '\n', '\r', '\t':
			p.reader.ReadByte()
		}
	}
	if p.safePeekContains(JSON_NULL) {
		p.reader.Read(make([]byte, len(JSON_NULL)))
		return true, nil
	}
	return false, nil
}

func (p *simpleJSONFormat) readQuoteIfNext() {
	b, _ := p.reader.Peek(1)
	if len(b) > 0 && b[0] == JSON_QUOTE {
		p.reader.ReadByte()
	}
}

func (p *simpleJSONFormat) readNumeric() (Numeric, error) {
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
			return NUMERIC_NULL, types.NewProtocolException(err)
		}
		switch c {
		case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9':
			buf.WriteByte(c)
			nextCanBeSign = false
		case '.':
			if hasDecimalPoint {
				e := fmt.Errorf("Unable to parse number with multiple decimal points '%s.'", buf.String())
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			if hasE {
				e := fmt.Errorf("Unable to parse number with decimal points in the exponent '%s.'", buf.String())
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasDecimalPoint, nextCanBeSign = true, false
		case 'e', 'E':
			if hasE {
				e := fmt.Errorf("Unable to parse number with multiple exponents '%s%c'", buf.String(), c)
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
			buf.WriteByte(c)
			hasE, nextCanBeSign = true, true
		case '-', '+':
			if !nextCanBeSign {
				e := fmt.Errorf("Negative sign within number")
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
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
					return NUMERIC_NULL, types.NewProtocolException(e)
				}
				if JSON_NAN != string(buffer) {
					e := mismatch(JSON_NAN, string(buffer))
					return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return NAN, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case JSON_INFINITY[0]:
			if buf.Len() == 0 || (buf.Len() == 1 && buf.Bytes()[0] == '+') {
				buffer := make([]byte, len(JSON_INFINITY))
				buffer[0] = c
				_, e := p.reader.Read(buffer[1:])
				if e != nil {
					return NUMERIC_NULL, types.NewProtocolException(e)
				}
				if JSON_INFINITY != string(buffer) {
					e := mismatch(JSON_INFINITY, string(buffer))
					return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
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
					return NUMERIC_NULL, types.NewProtocolException(e)
				}
				if JSON_NEGATIVE_INFINITY != string(buffer) {
					e := mismatch(JSON_NEGATIVE_INFINITY, string(buffer))
					return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
				}
				if inQuotes {
					p.readQuoteIfNext()
				}
				return NEGATIVE_INFINITY, nil
			} else {
				e := fmt.Errorf("Unable to parse number starting with character '%c' due to existing buffer %s", c, buf.String())
				return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
			}
		case JSON_QUOTE:
			if !inQuotes {
				inQuotes = true
			}
		default:
			e := fmt.Errorf("Unable to parse number starting with character '%c'", c)
			return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
		}
	}
	if buf.Len() == 0 {
		e := fmt.Errorf("Unable to parse number from empty string ''")
		return NUMERIC_NULL, types.NewProtocolExceptionWithType(types.INVALID_DATA, e)
	}
	return NewNumericFromJSONString(buf.String(), false), nil
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
