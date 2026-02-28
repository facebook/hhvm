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

package rocket

import (
	"testing"

	"github.com/stretchr/testify/require"
)

func compressDecompressTestHelper(t *testing.T, compress, decompress func([]byte) ([]byte, error)) {
	want := []byte{0x28, 0xb5, 0x2f, 0xfd}

	compressed, err := compress(want)
	require.NoError(t, err)
	require.NotEqual(t, want, compressed)

	got, err := decompress(compressed)
	require.NoError(t, err)
	require.Equal(t, want, got)
}

func TestZstd(t *testing.T) {
	t.Run("default", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZstd, decompressZstd)
	})
	t.Run("less", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZstdLess, decompressZstd)
	})
	t.Run("more", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZstdMore, decompressZstd)
	})
}

func TestZlib(t *testing.T) {
	t.Run("default", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZlib, decompressZlib)
	})
	t.Run("less", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZlibLess, decompressZlib)
	})
	t.Run("more", func(t *testing.T) {
		compressDecompressTestHelper(t, compressZlibMore, decompressZlib)
	})
}
