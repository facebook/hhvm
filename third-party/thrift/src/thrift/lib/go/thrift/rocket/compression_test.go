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
	"bytes"
	"encoding/binary"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
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

func TestLz4(t *testing.T) {
	t.Run("default", func(t *testing.T) {
		compressDecompressTestHelper(t, compressLz4, decompressLz4)
	})
	t.Run("less", func(t *testing.T) {
		compressDecompressTestHelper(t, compressLz4Less, decompressLz4)
	})
	t.Run("more", func(t *testing.T) {
		compressDecompressTestHelper(t, compressLz4More, decompressLz4)
	})

	// Larger, highly compressible payload exercises real LZ4 block compression
	// (not just literal-block encoding) and verifies the varint size prefix
	// for sizes that span more than one byte.
	t.Run("compressible payload round-trips across all levels", func(t *testing.T) {
		want := bytes.Repeat([]byte("the quick brown fox jumps over the lazy dog "), 256)
		for _, c := range []struct {
			name     string
			compress func([]byte) ([]byte, error)
		}{
			{"default", compressLz4},
			{"less", compressLz4Less},
			{"more", compressLz4More},
		} {
			t.Run(c.name, func(t *testing.T) {
				compressed, err := c.compress(want)
				require.NoError(t, err)
				require.Less(t, len(compressed), len(want), "compression should shrink repetitive data")
				got, err := decompressLz4(compressed)
				require.NoError(t, err)
				require.Equal(t, want, got)
			})
		}
	})
}

func TestMaybeCompressDecompressLz4(t *testing.T) {
	want := bytes.Repeat([]byte("hello lz4 world "), 64)
	for _, algo := range []rpcmetadata.CompressionAlgorithm{
		rpcmetadata.CompressionAlgorithm_LZ4,
		rpcmetadata.CompressionAlgorithm_LZ4_LESS,
		rpcmetadata.CompressionAlgorithm_LZ4_MORE,
	} {
		t.Run(algo.String(), func(t *testing.T) {
			compressed, err := MaybeCompress(want, algo)
			require.NoError(t, err)
			require.NotEqual(t, want, compressed)

			got, err := MaybeDecompress(compressed, algo)
			require.NoError(t, err)
			require.Equal(t, want, got)
		})
	}
}

// Reject crafted inputs whose varint header declares an implausibly large
// uncompressed size, which would otherwise trigger an oversized allocation.
func TestDecompressLz4RejectsOversizedSizePrefix(t *testing.T) {
	var buf [binary.MaxVarintLen64]byte
	// Declare an absurd uncompressed size with an empty payload.
	n := binary.PutUvarint(buf[:], 1<<40)
	_, err := decompressLz4(buf[:n])
	require.Error(t, err)
	require.Contains(t, err.Error(), "plausible bound")
}
