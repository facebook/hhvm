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

package gotest

import (
	"encoding/json"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

func TestReflectCodec(t *testing.T) {
	// This test ensures code coverage of the reflection encoder/decoder

	// TODO: field51 and field53 result in non-buildable code.
	// Use manual workaround below for now.
	// ....
	// "field51": {{"field1": 123}: "value1"},
	// "field53": {["hello1", "hello2", "hello3"]: "value1"},
	// ....

	writeTarget := thrifttest.NewVariousFieldsStruct()
	err := thrift.DeepCopy(thrifttest.VariousFieldsStructConst1, writeTarget)
	require.NoError(t, err)
	writeTarget.Field51 = map[thrifttest.ComparableStruct]string{
		{Field1: 123}: "value1",
	}
	writeTarget.Field53 = map[*[]string]string{
		{"hello1", "hello2", "hello3"}: "value1",
	}

	originalBuf, err := thrift.EncodeCompact(writeTarget)
	require.NoError(t, err)

	readTarget := &thrifttest.VariousFieldsStruct{}
	err = thrift.DecodeCompact(originalBuf, readTarget)
	require.NoError(t, err)

	// TODO: field53 is not printed properly using String() method, so use json for now.
	writeData, _ := json.Marshal(writeTarget)
	readData, _ := json.Marshal(readTarget)
	writeStr := string(writeData)
	readStr := string(readData)
	require.Equal(t, writeStr, readStr)
}
