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
	"bytes"
	"fmt"
	"strings"
	"testing"
	"time"
)

func TestReadWriteBinaryProtocol(t *testing.T) {
	ReadWriteProtocolTest(t, NewBinaryProtocolFactoryDefault())
}

func TestSkipUnknownTypeBinaryProtocol(t *testing.T) {
	var m MyTestStruct
	d := NewDeserializer()
	f := NewBinaryProtocolFactoryDefault()
	d.Protocol = f.GetProtocol(d.Transport)
	// skip over a map with invalid key/value type and ~550M entries
	data := make([]byte, 1100000000)
	copy(data[:], []byte("\n\x10\rO\t6\x03\n\n\n\x10\r\n\tsl ce\x00"))
	transport, _ := d.Transport.(*MemoryBuffer)
	transport.Buffer = bytes.NewBuffer(data)
	start := time.Now()
	err := m.Read(d.Protocol)
	if err == nil {
		t.Fatalf("Parsed invalid message correctly")
	} else if !strings.Contains(err.Error(), "unknown type") {
		t.Fatalf("Failed for reason besides unknown type")
	}

	if time.Now().Sub(start).Seconds() > 5 {
		t.Fatalf("It should not take seconds to parse a small message")
	}
}

func TestInitialAllocationMapBinaryProtocol(t *testing.T) {
	var m MyTestStruct
	d := NewDeserializer()
	f := NewBinaryProtocolFactoryDefault()
	d.Protocol = f.GetProtocol(d.Transport)
	// attempts to allocate a map with 1.8B elements for a 20 byte message
	data := []byte("\n\x10\rO\t6\x03\n\n\n\x10\r\n\tslice\x00")
	err := d.Read(&m, data)
	if err == nil {
		t.Fatalf("Parsed invalid message correctly")
	} else if !strings.Contains(err.Error(), "Invalid data length") {
		t.Fatalf("Failed for reason besides Invalid data length")
	}
}

func TestInitialAllocationListBinaryProtocol(t *testing.T) {
	var m MyTestStruct
	d := NewDeserializer()
	f := NewBinaryProtocolFactoryDefault()
	d.Protocol = f.GetProtocol(d.Transport)
	// attempts to allocate a list with 1.8B elements for a 20 byte message
	data := []byte("\n\x10\rO\t6\x03\n\n\n\x10\x0f\n\tslice\x00")
	err := d.Read(&m, data)
	if err == nil {
		t.Fatalf("Parsed invalid message correctly")
	} else if !strings.Contains(err.Error(), "Invalid data length") {
		t.Fatalf("Failed for reason besides Invalid data length")
	}
}

func TestInitialAllocationSetBinaryProtocol(t *testing.T) {
	var m MyTestStruct
	d := NewDeserializer()
	f := NewBinaryProtocolFactoryDefault()
	d.Protocol = f.GetProtocol(d.Transport)
	// attempts to allocate a set with 1.8B elements for a 20 byte message
	data := []byte("\n\x12\rO\t6\x03\n\n\n\x10\x0e\n\tslice\x00")
	err := d.Read(&m, data)
	if err == nil {
		t.Fatalf("Parsed invalid message correctly")
	} else if !strings.Contains(err.Error(), "Invalid data length") {
		fmt.Printf("Got %+v", err)
		t.Fatalf("Failed for reason besides Invalid data length")
	}
}
