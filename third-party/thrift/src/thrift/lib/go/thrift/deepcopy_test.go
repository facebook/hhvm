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
	"testing"

	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func TestDeepCopy(t *testing.T) {
	val := dummyif.NewDummyStruct1().
		SetField1(int8(12)).
		SetField2(true).
		SetField3(int16(12345)).
		SetField4(int32(123456)).
		SetField5(int64(1234567)).
		SetField6(float32(1234.567)).
		SetField7(float64(123456.7)).
		SetField8([]byte{1, 2, 3, 4}).
		SetField9("hello")

	var valCopy dummyif.DummyStruct1

	err := DeepCopy(val, &valCopy)
	require.NoError(t, err)
	require.Equal(t, val, &valCopy)
}
