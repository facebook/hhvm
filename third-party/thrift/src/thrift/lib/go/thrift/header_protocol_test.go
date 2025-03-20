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
	if v, _ := proto1.(*headerProtocol).trans.writeInfoHeaders["preferred_cheese"]; v != "cheddar" {
		t.Fatalf("failed to set header")
	}
	if len(proto1.(*headerProtocol).trans.writeInfoHeaders) != 1 {
		t.Fatalf("wrong number of headers")
	}

	proto1.WriteMessageBegin("", types.CALL, 1)
	proto1.WriteMessageEnd()
	proto1.Flush()

	_, _, _, err = proto2.ReadMessageBegin()
	require.NoError(t, err)

	if v, _ := proto2.getResponseHeaders()["preferred_cheese"]; v != "gouda" {
		t.Fatalf("failed to read header, got: %s", v)
	}

	if peerIdentity(proto2.(*headerProtocol).trans) != "batman" {
		t.Fatalf("failed to peer identity")
	}
}
