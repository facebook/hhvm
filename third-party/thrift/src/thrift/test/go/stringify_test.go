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
	"reflect"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

func TestStructStringMethod(t *testing.T) {
	// Nil ptr case
	val1 := (*thrifttest.Bonk)(nil)
	expectedVal1 := "<nil>"
	require.Equal(t, expectedVal1, val1.String())

	// Simple case
	val2 := &thrifttest.Bonk{Message: "hello", Type: 1234}
	expectedVal2 := "Bonk({Message:hello Type:1234})"
	require.Equal(t, expectedVal2, val2.String())

	// Recursive structs
	val3 := &thrifttest.Xtruct2{ByteThing: 123, I32Thing: 456, StructThing: &thrifttest.Xtruct{StringThing: "hello"}}
	expectedVal3 := "Xtruct2({ByteThing:123 StructThing:Xtruct({StringThing:hello ByteThing:0 I32Thing:0 I64Thing:0}) I32Thing:456})"
	require.Equal(t, expectedVal3, val3.String())

	// Recursive structs with nil
	val4 := &thrifttest.Xtruct2{ByteThing: 123, I32Thing: 456, StructThing: nil}
	expectedVal4 := "Xtruct2({ByteThing:123 StructThing:<nil> I32Thing:456})"
	require.Equal(t, expectedVal4, val4.String())

	// Optional field
	val5 := &thrifttest.Xtruct4{Opt64: nil}
	expectedVal5 := "Xtruct4({StringThing: IntThing:0 ListInt32Thing:[] Xtruct2:<nil> Opt64:<nil>})"
	require.Equal(t, expectedVal5, val5.String())

	// Invalid argument
	val6 := int64(1234)
	expectedVal6 := "<invalid>"
	actualVal6 := types.StructToString(reflect.ValueOf(val6))
	require.Equal(t, expectedVal6, actualVal6)
}
