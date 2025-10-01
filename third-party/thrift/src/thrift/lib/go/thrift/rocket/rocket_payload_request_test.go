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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rpcmetadata"
	"github.com/rsocket/rsocket-go/payload"
	"github.com/stretchr/testify/require"
)

func TestRequestRPCMetadata(t *testing.T) {
	wantName := "test123"
	wantType := types.CALL
	wantProto := types.ProtocolIDCompact
	wantOther := map[string]string{"header": "1"}
	data, err := EncodeRequestPayload(wantName, wantProto, rpcmetadata.RpcKind_SINGLE_REQUEST_SINGLE_RESPONSE, wantOther, rpcmetadata.CompressionAlgorithm_NONE, nil)
	require.NoError(t, err)
	_, got, err := DecodeRequestPayload(data)
	require.NoError(t, err)
	require.Equal(t, wantName, got.Name())
	require.Equal(t, wantType, got.TypeID())
	require.Equal(t, wantProto, got.ProtoID())
	require.Equal(t, wantOther, got.Headers())

	payloadNoMetadata := payload.New([]byte("data_bytes"), nil /* metadata bytes */)
	_, _, err = DecodeRequestPayload(payloadNoMetadata)
	require.ErrorContains(t, err, "request payload is missing metadata")
}
