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
	"io"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
)

func TestHeaderZstd(t *testing.T) {
	tmb := newMockSocket()
	trans := newHeaderTransport(tmb, types.ProtocolIDCompact)
	data := []byte("ASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDFASDF")
	uncompressedlen := 30

	err := trans.AddTransform(TransformSnappy)
	require.Error(t, err, "should have failed adding unsupported transform")

	err = trans.AddTransform(TransformZstd)
	require.NoError(t, err)

	_, err = trans.Write(data)
	require.NoError(t, err)

	err = trans.Flush()
	require.NoError(t, err)

	err = trans.ResetProtocol()
	require.NoError(t, err)

	frame, err := io.ReadAll(trans)
	require.NoError(t, err)
	require.Equal(t, data, frame)
	// This is a bit of a stupid test, but make sure that the data
	// got changed somehow
	require.NotEqual(t, uncompressedlen, len(frame))
}

func TestZstd(t *testing.T) {
	want := []byte{0x28, 0xb5, 0x2f, 0xfd}
	compressed, err := compressZstd(want)
	require.NoError(t, err)
	require.NotEqual(t, want, compressed)

	got, err := decompressZstd(compressed)
	require.NoError(t, err)
	require.Equal(t, want, got)
}
