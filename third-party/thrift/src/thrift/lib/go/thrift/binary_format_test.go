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
	"bytes"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/test/go/if/my_test_struct"
	"github.com/stretchr/testify/require"
)

type MyTestStruct = my_test_struct.MyTestStruct

func TestReadWriteBinaryFormat(t *testing.T) {
	ReadWriteProtocolTest(t, func(transport types.ReadWriteSizer) types.Format { return NewBinaryFormat(transport) })
}

func TestWriteBinaryEmptyBinaryFormat(t *testing.T) {
	m := my_test_struct.NewMyTestStruct()
	m.Bin = nil
	format := NewBinaryFormat(bytes.NewBuffer([]byte(nil)))
	err := m.Write(format)
	require.NoError(t, err)
}

func TestSkipUnknownTypeBinaryFormat(t *testing.T) {
	var m MyTestStruct
	// skip over a map with invalid key/value type and ~550M entries
	data := make([]byte, 1100000000)
	copy(data[:], []byte("\n\x10\rO\t6\x03\n\n\n\x10\r\n\tsl ce\x00"))
	format := NewBinaryFormat(bytes.NewBuffer(data))
	start := time.Now()
	err := m.Read(format)
	require.ErrorContains(t, err, "unknown type")
	require.True(t, time.Now().Sub(start).Seconds() < 5, "It should not take seconds to parse a small message")
}

func TestInitialAllocationMapBinaryFormat(t *testing.T) {
	var m MyTestStruct
	// attempts to allocate a map with 1.8B elements for a 20 byte message
	data := []byte("\n\x10\rO\t6\x03\n\n\n\x10\r\n\tslice\x00")
	format := NewBinaryFormat(bytes.NewBuffer(data))
	err := m.Read(format)
	require.ErrorIs(t, err, invalidDataLength)
}

func TestInitialAllocationListBinaryFormat(t *testing.T) {
	var m MyTestStruct
	// attempts to allocate a list with 1.8B elements for a 20 byte message
	data := []byte("\n\x10\rO\t6\x03\n\n\n\x10\x0f\n\tslice\x00")
	format := NewBinaryFormat(bytes.NewBuffer(data))
	err := m.Read(format)
	require.ErrorIs(t, err, invalidDataLength)
}

func TestInitialAllocationSetBinaryFormat(t *testing.T) {
	var m MyTestStruct
	// attempts to allocate a set with 1.8B elements for a 20 byte message
	data := []byte("\n\x12\rO\t6\x03\n\n\n\x10\x0e\n\tslice\x00")
	format := NewBinaryFormat(bytes.NewBuffer(data))
	err := m.Read(format)
	require.ErrorIs(t, err, invalidDataLength)
}
