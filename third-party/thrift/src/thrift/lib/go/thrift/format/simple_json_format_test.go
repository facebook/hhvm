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
	"bytes"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"math"
	"strconv"
	"strings"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

func TestWriteSimpleJSONProtocolBool(t *testing.T) {
	thetype := "boolean"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range boolValues {
		if e := p.WriteBool(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s != fmt.Sprint(value) {
			t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
		}
		v := false
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolBool(t *testing.T) {
	thetype := "boolean"
	for _, value := range boolValues {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		if value {
			trans.Write(JSON_TRUE)
		} else {
			trans.Write(JSON_FALSE)
		}

		s := trans.String()
		v, e := p.ReadBool()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolByte(t *testing.T) {
	thetype := "byte"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range byteValues {
		if e := p.WriteByte(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s != fmt.Sprint(value) {
			t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
		}
		v := byte(0)
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolByte(t *testing.T) {
	thetype := "byte"
	for _, value := range byteValues {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		trans.WriteString(strconv.Itoa(int(value)))

		s := trans.String()
		v, e := p.ReadByte()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolI16(t *testing.T) {
	thetype := "int16"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range int16Values {
		if e := p.WriteI16(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s != fmt.Sprint(value) {
			t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
		}
		v := int16(0)
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolI16(t *testing.T) {
	thetype := "int16"
	for _, value := range int16Values {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		trans.WriteString(strconv.Itoa(int(value)))

		s := trans.String()
		v, e := p.ReadI16()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolI32(t *testing.T) {
	thetype := "int32"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range int32Values {
		if e := p.WriteI32(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s != fmt.Sprint(value) {
			t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
		}
		v := int32(0)
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolI32(t *testing.T) {
	thetype := "int32"
	for _, value := range int32Values {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		trans.WriteString(strconv.Itoa(int(value)))

		s := trans.String()
		v, e := p.ReadI32()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolI32Null(t *testing.T) {
	thetype := "int32"
	value := "null"

	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	trans.WriteString(value)
	s := trans.String()
	v, e := p.ReadI32()

	if e != nil {
		t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
	}
	if v != 0 {
		t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
	}
	trans.Reset()
}

func TestWriteSimpleJSONProtocolI64(t *testing.T) {
	thetype := "int64"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range int64Values {
		if e := p.WriteI64(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s != fmt.Sprint(value) {
			t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
		}
		v := int64(0)
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolI64(t *testing.T) {
	thetype := "int64"
	for _, value := range int64Values {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		trans.WriteString(strconv.FormatInt(value, 10))

		s := trans.String()
		v, e := p.ReadI64()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolI64Null(t *testing.T) {
	thetype := "int32"
	value := "null"

	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	trans.WriteString(value)
	s := trans.String()
	v, e := p.ReadI64()

	if e != nil {
		t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
	}
	if v != 0 {
		t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
	}
	trans.Reset()
}

func TestWriteSimpleJSONProtocolDouble(t *testing.T) {
	thetype := "double"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range doubleValues {
		if e := p.WriteDouble(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if math.IsInf(value, 1) {
			if s != jsonQuote(JSON_INFINITY) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_INFINITY))
			}
		} else if math.IsInf(value, -1) {
			if s != jsonQuote(JSON_NEGATIVE_INFINITY) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_NEGATIVE_INFINITY))
			}
		} else if math.IsNaN(value) {
			if s != jsonQuote(JSON_NAN) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_NAN))
			}
		} else {
			if s != fmt.Sprint(value) {
				t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
			}
			v := float64(0)
			if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
			}
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolDouble(t *testing.T) {
	thetype := "double"
	for _, value := range doubleValues {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		n := NewNumericFromDouble(value)
		trans.WriteString(n.String())

		s := trans.String()
		v, e := p.ReadDouble()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if math.IsInf(value, 1) {
			if !math.IsInf(v, 1) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else if math.IsInf(value, -1) {
			if !math.IsInf(v, -1) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else if math.IsNaN(value) {
			if !math.IsNaN(v) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else {
			if v != value {
				t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
			}
			if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
			}
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolFloat(t *testing.T) {
	thetype := "float"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range floatValues {
		if e := p.WriteFloat(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if math.IsInf(float64(value), 1) {
			if s != jsonQuote(JSON_INFINITY) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_INFINITY))
			}
		} else if math.IsInf(float64(value), -1) {
			if s != jsonQuote(JSON_NEGATIVE_INFINITY) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_NEGATIVE_INFINITY))
			}
		} else if math.IsNaN(float64(value)) {
			if s != jsonQuote(JSON_NAN) {
				t.Fatalf("Bad value for %s %v, wrote: %v, expected: %v", thetype, value, s, jsonQuote(JSON_NAN))
			}
		} else {
			if s != fmt.Sprint(value) {
				t.Fatalf("Bad value for %s %v: %s", thetype, value, s)
			}
			v := float32(0)
			if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
			}
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolFloat(t *testing.T) {
	thetype := "float"
	for _, value := range floatValues {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		n := NewNumericFromFloat(value)
		trans.WriteString(n.String())

		s := trans.String()
		v, e := p.ReadFloat()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if math.IsInf(float64(value), 1) {
			if !math.IsInf(float64(v), 1) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else if math.IsInf(float64(value), -1) {
			if !math.IsInf(float64(v), -1) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else if math.IsNaN(float64(value)) {
			if !math.IsNaN(float64(v)) {
				t.Fatalf("Bad value for %s %v, wrote: %v, received: %v", thetype, value, s, v)
			}
		} else {
			if v != value {
				t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
			}
			if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
			}
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolString(t *testing.T) {
	thetype := "string"
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	for _, value := range stringValues {
		if e := p.WriteString(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
		if e := p.Flush(); e != nil {
			t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
		}
		s := trans.String()
		if s[0] != '"' || s[len(s)-1] != '"' {
			t.Fatalf("Bad value for %s '%v', wrote '%v', expected: %v", thetype, value, s, fmt.Sprint("\"", value, "\""))
		}
		v := new(string)
		if err := json.Unmarshal([]byte(s), v); err != nil || *v != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, *v)
		}
		trans.Reset()
	}
}

func TestReadSimpleJSONProtocolString(t *testing.T) {
	thetype := "string"
	for _, value := range stringValues {
		trans := new(bytes.Buffer)
		p := NewSimpleJSONFormat(trans)
		trans.WriteString(jsonQuote(value))

		s := trans.String()
		v, e := p.ReadString()
		if e != nil {
			t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
		}
		if v != value {
			t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
		}
		v1 := new(string)
		if err := json.Unmarshal([]byte(s), v1); err != nil || *v1 != value {
			t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, *v1)
		}
		trans.Reset()
	}
}
func TestReadSimpleJSONProtocolStringNull(t *testing.T) {
	thetype := "string"
	value := "null"

	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	trans.WriteString(value)
	s := trans.String()
	v, e := p.ReadString()
	if e != nil {
		t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
	}
	if v != "" {
		t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
	}
	trans.Reset()
}

func TestWriteSimpleJSONProtocolBinary(t *testing.T) {
	thetype := "binary"
	value := protocolBdata
	b64value := make([]byte, base64.StdEncoding.EncodedLen(len(protocolBdata)))
	base64.StdEncoding.Encode(b64value, value)
	b64String := string(b64value)
	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	if e := p.WriteBinary(value); e != nil {
		t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
	}
	if e := p.Flush(); e != nil {
		t.Fatalf("Unable to write %s value %v due to error flushing: %s", thetype, value, e.Error())
	}
	s := trans.String()
	if s != fmt.Sprint("\"", b64String, "\"") {
		t.Fatalf("Bad value for %s %v\n  wrote: %v\nexpected: %v", thetype, value, s, "\""+b64String+"\"")
	}
	v1 := new(string)
	if err := json.Unmarshal([]byte(s), v1); err != nil || *v1 != b64String {
		t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, *v1)
	}
}

func TestReadSimpleJSONProtocolBinary(t *testing.T) {
	thetype := "binary"
	value := protocolBdata
	b64value := make([]byte, base64.StdEncoding.EncodedLen(len(protocolBdata)))
	base64.StdEncoding.Encode(b64value, value)
	b64String := string(b64value)
	trans := new(bytes.Buffer)
	p := newSimpleJSONFormatV2(trans)
	trans.WriteString(jsonQuote(b64String))
	s := trans.String()
	v, e := p.ReadBinary()
	if e != nil {
		t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
	}
	if len(v) != len(value) {
		t.Fatalf("Bad value for %s value length %v, wrote: %v, received length: %v", thetype, len(value), s, len(v))
	}
	for i := 0; i < len(v); i++ {
		if v[i] != value[i] {
			t.Fatalf("Bad value for %s at index %d value %v, wrote: %v, received: %v", thetype, i, value[i], s, v[i])
		}
	}
	v1 := new(string)
	if err := json.Unmarshal([]byte(s), v1); err != nil || *v1 != b64String {
		t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, *v1)
	}
	trans.Reset()
}

func TestReadSimpleJSONProtocolBinaryNull(t *testing.T) {
	thetype := "binary"
	value := "null"

	trans := new(bytes.Buffer)
	p := NewSimpleJSONFormat(trans)
	trans.WriteString(value)
	s := trans.String()
	b, e := p.ReadBinary()
	v := string(b)

	if e != nil {
		t.Fatalf("Unable to read %s value %v due to error: %s", thetype, value, e.Error())
	}
	if v != "" {
		t.Fatalf("Bad value for %s value %v, wrote: %v, received: %v", thetype, value, s, v)
	}
	trans.Reset()
}

func TestWriteSimpleJSONProtocolList(t *testing.T) {
	thetype := "list"
	trans := new(bytes.Buffer)
	p := newSimpleJSONFormatV2(trans)
	p.WriteListBegin(types.DOUBLE, len(doubleValues))
	for _, value := range doubleValues {
		if e := p.WriteDouble(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
	}
	p.WriteListEnd()
	if e := p.Flush(); e != nil {
		t.Fatalf("Unable to write %s due to error flushing: %s", thetype, e.Error())
	}
	str := trans.String()
	str1 := new([]any)
	err := json.Unmarshal([]byte(str), str1)
	if err != nil {
		t.Fatalf("Unable to decode %s, wrote: %s", thetype, str)
	}
	l := *str1
	if len(l) < 2 {
		t.Fatalf("List must be at least of length two to include metadata")
	}
	for k, value := range doubleValues {
		s := l[k]
		if math.IsInf(value, 1) {
			if s.(string) != JSON_INFINITY {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_INFINITY), str)
			}
		} else if math.IsInf(value, 0) {
			if s.(string) != JSON_NEGATIVE_INFINITY {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_NEGATIVE_INFINITY), str)
			}
		} else if math.IsNaN(value) {
			if s.(string) != JSON_NAN {
				t.Fatalf("Bad value for %s at index %v  %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_NAN), str)
			}
		} else {
			if s.(float64) != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s'", thetype, value, s)
			}
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolSet(t *testing.T) {
	thetype := "set"
	trans := new(bytes.Buffer)
	p := newSimpleJSONFormatV2(trans)
	p.WriteSetBegin(types.DOUBLE, len(doubleValues))
	for _, value := range doubleValues {
		if e := p.WriteDouble(value); e != nil {
			t.Fatalf("Unable to write %s value %v due to error: %s", thetype, value, e.Error())
		}
	}
	p.WriteSetEnd()
	if e := p.Flush(); e != nil {
		t.Fatalf("Unable to write %s due to error flushing: %s", thetype, e.Error())
	}
	str := trans.String()
	str1 := new([]any)
	err := json.Unmarshal([]byte(str), str1)
	if err != nil {
		t.Fatalf("Unable to decode %s, wrote: %s", thetype, str)
	}
	l := *str1
	if len(l) < 2 {
		t.Fatalf("Set must be at least of length two to include metadata")
	}
	for k, value := range doubleValues {
		s := l[k]
		if math.IsInf(value, 1) {
			if s.(string) != JSON_INFINITY {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_INFINITY), str)
			}
		} else if math.IsInf(value, 0) {
			if s.(string) != JSON_NEGATIVE_INFINITY {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_NEGATIVE_INFINITY), str)
			}
		} else if math.IsNaN(value) {
			if s.(string) != JSON_NAN {
				t.Fatalf("Bad value for %s at index %v  %v, wrote: %q, expected: %q, originally wrote: %q", thetype, k, value, s, jsonQuote(JSON_NAN), str)
			}
		} else {
			if s.(float64) != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s'", thetype, value, s)
			}
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolMap(t *testing.T) {
	thetype := "map"
	trans := new(bytes.Buffer)
	p := newSimpleJSONFormatV2(trans)
	p.WriteMapBegin(types.I32, types.DOUBLE, len(doubleValues))
	for k, value := range doubleValues {
		if e := p.WriteI32(int32(k)); e != nil {
			t.Fatalf("Unable to write %s key int32 value %v due to error: %s", thetype, k, e.Error())
		}
		if e := p.WriteDouble(value); e != nil {
			t.Fatalf("Unable to write %s value float64 value %v due to error: %s", thetype, value, e.Error())
		}
	}
	p.WriteMapEnd()
	if e := p.Flush(); e != nil {
		t.Fatalf("Unable to write %s due to error flushing: %s", thetype, e.Error())
	}
	str := trans.String()
	if str[0] != '{' || str[len(str)-1] != '}' {
		t.Fatalf("Bad value for %s, wrote: %q, in go: %v", thetype, str, doubleValues)
	}
	l := strings.Split(str[1:len(str)-1], ",")
	if len(l) < 3 {
		t.Fatal("Expected list of at least length 3 for map for metadata, but was of length ", len(l))
	}
	for k, value := range doubleValues {
		kvStrs := strings.Split(l[k], ":")
		strk := strings.Trim(kvStrs[0], "\"")
		strv := kvStrs[1]
		ik, err := strconv.Atoi(strk)
		if err != nil {
			t.Fatalf("Bad value for %s index %v, wrote: %v, expected: %v, error: %s", thetype, k, strk, strconv.Itoa(k), err.Error())
		}
		if ik != k {
			t.Fatalf("Bad value for %s index %v, wrote: %v, expected: %v", thetype, k, strk, k)
		}
		s := strv
		if math.IsInf(value, 1) {
			if s != jsonQuote(JSON_INFINITY) {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %v, expected: %v", thetype, k, value, s, jsonQuote(JSON_INFINITY))
			}
		} else if math.IsInf(value, 0) {
			if s != jsonQuote(JSON_NEGATIVE_INFINITY) {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %v, expected: %v", thetype, k, value, s, jsonQuote(JSON_NEGATIVE_INFINITY))
			}
		} else if math.IsNaN(value) {
			if s != jsonQuote(JSON_NAN) {
				t.Fatalf("Bad value for %s at index %v  %v, wrote: %v, expected: %v", thetype, k, value, s, jsonQuote(JSON_NAN))
			}
		} else {
			expected := strconv.FormatFloat(value, 'g', 10, 64)
			if s != expected {
				t.Fatalf("Bad value for %s at index %v %v, wrote: %v, expected %v", thetype, k, value, s, expected)
			}
			v := float64(0)
			if err := json.Unmarshal([]byte(s), &v); err != nil || v != value {
				t.Fatalf("Bad json-decoded value for %s %v, wrote: '%s', expected: '%v'", thetype, value, s, v)
			}
		}
		trans.Reset()
	}
}

func TestWriteSimpleJSONProtocolSafePeek(t *testing.T) {
	trans := new(bytes.Buffer)
	p := newSimpleJSONFormat(trans)
	trans.Write([]byte{'a', 'b'})

	test1 := p.safePeekContains([]byte{'a', 'b'})
	if !test1 {
		t.Fatalf("Should match at test 1")
	}

	test2 := p.safePeekContains([]byte{'a', 'b', 'c', 'd'})
	if test2 {
		t.Fatalf("Should not match at test 2")
	}

	test3 := p.safePeekContains([]byte{'x', 'y'})
	if test3 {
		t.Fatalf("Should not match at test 3")
	}
}

func TestJSONContextStack(t *testing.T) {
	var stack jsonContextStack
	t.Run("empty-peek", func(t *testing.T) {
		v, ok := stack.peek()
		if ok {
			t.Error("peek() on empty should return ok: false")
		}
		expected := _CONTEXT_INVALID
		if v != expected {
			t.Errorf("Expected value from peek() to be %v(%d), got %v(%d)", expected, expected, v, v)
		}
	})
	t.Run("empty-pop", func(t *testing.T) {
		v, ok := stack.pop()
		if ok {
			t.Error("pop() on empty should return ok: false")
		}
		expected := _CONTEXT_INVALID
		if v != expected {
			t.Errorf("Expected value from pop() to be %v(%d), got %v(%d)", expected, expected, v, v)
		}
	})
	t.Run("push-peek-pop", func(t *testing.T) {
		expected := _CONTEXT_INVALID
		stack.push(expected)
		if len(stack) != 1 {
			t.Errorf("Expected stack to be as size 1 after push, got %#v", stack)
		}
		v, ok := stack.peek()
		if !ok {
			t.Error("peek() on non-empty should return ok: true")
		}
		if v != expected {
			t.Errorf("Expected value from peek() to be %v(%d), got %v(%d)", expected, expected, v, v)
		}
		if len(stack) != 1 {
			t.Errorf("Expected peek() to be read-only, got %#v", stack)
		}
		v, ok = stack.pop()
		if !ok {
			t.Error("pop() on non-empty should return ok: true")
		}
		if v != expected {
			t.Errorf("Expected value from pop() to be %v(%d), got %v(%d)", expected, expected, v, v)
		}
		if len(stack) != 0 {
			t.Errorf("Expected pop() to empty the stack, got %#v", stack)
		}
	})
}

func TestSimpleJSONProtocolUnmatchedBeginEnd(t *testing.T) {
	UnmatchedBeginEndProtocolTest(t, NewSimpleJSONFormat)
}
