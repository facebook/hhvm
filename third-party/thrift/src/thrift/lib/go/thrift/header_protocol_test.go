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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/stretchr/testify/require"
)

func TestHeaderProtocolHeaders(t *testing.T) {
	mockSocket := newMockSocket()
	proto1, err := newHeaderProtocol(mockSocket, types.ProtocolIDCompact, 0, map[string]string{
		"preferred_cheese": "gouda",
		IDVersionHeader:    IDVersion,
		IdentityHeader:     "batman",
	})
	require.NoError(t, err)

	proto2, err := NewHeaderProtocol(mockSocket)
	require.NoError(t, err)

	proto1.setRequestHeader("preferred_cheese", "cheddar")
	require.Equal(t, "cheddar", proto1.(*headerProtocol).trans.writeInfoHeaders["preferred_cheese"])
	require.Len(t, proto1.(*headerProtocol).trans.writeInfoHeaders, 1)

	proto1.WriteMessageBegin("", types.CALL, 1)
	proto1.WriteMessageEnd()
	proto1.Flush()

	_, _, _, err = proto2.ReadMessageBegin()
	require.NoError(t, err)

	require.Equal(t, "gouda", proto2.getResponseHeaders()["preferred_cheese"])
	require.Equal(t, "batman", peerIdentity(proto2.(*headerProtocol).trans))
}
