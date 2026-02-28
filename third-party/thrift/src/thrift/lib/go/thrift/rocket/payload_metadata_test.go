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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/stretchr/testify/require"
)

func TestDecodePayloadMetadata(t *testing.T) {
	metadata := rpcmetadata.NewRequestRpcMetadata()
	payloadNoMetadata := payload.New([]byte("data_bytes"), nil /* metadata bytes */)
	err := DecodePayloadMetadata(payloadNoMetadata, metadata)
	require.ErrorContains(t, err, "payload is missing metadata")

	originalMetadata := rpcmetadata.NewRequestRpcMetadata().
		SetName(types.Pointerize("hello"))
	actualMetadata := rpcmetadata.NewRequestRpcMetadata()
	metadataBytes, err := format.EncodeCompact(originalMetadata)
	require.NoError(t, err)
	payloadWithMetadata := payload.New([]byte("data_bytes"), metadataBytes)
	err = DecodePayloadMetadata(payloadWithMetadata, actualMetadata)
	require.NoError(t, err)
	require.Equal(t, originalMetadata, actualMetadata)
}
