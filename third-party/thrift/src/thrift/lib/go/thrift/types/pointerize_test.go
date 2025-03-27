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

package types

import (
	"testing"

	"github.com/stretchr/testify/require"
)

func TestPointerize(t *testing.T) {
	require.Equal(t, bool(true), *Pointerize(bool(true)))
	require.Equal(t, int8(2), *Pointerize(int8(2)))
	require.Equal(t, int16(3), *Pointerize(int16(3)))
	require.Equal(t, int32(4), *Pointerize(int32(4)))
	require.Equal(t, int64(5), *Pointerize(int64(5)))
	require.Equal(t, float32(123.4), *Pointerize(float32(123.4)))
	require.Equal(t, float64(567.8), *Pointerize(float64(567.8)))
	require.Equal(t, string("hello"), *Pointerize(string("hello")))
}
