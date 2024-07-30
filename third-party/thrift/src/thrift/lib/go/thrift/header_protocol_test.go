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
)

func TestHeaderProtocolHeaders(t *testing.T) {
	mockSocket := newMockSocket()
	proto1, err := newHeaderProtocol(mockSocket, ProtocolIDCompact, 0, map[string]string{
		"preferred_cheese": "gouda",
		IDVersionHeader:    IDVersion,
		IdentityHeader:     "batman",
	})
	if err != nil {
		t.Fatalf("failed to create header protocol: %s", err)
	}
	proto2, err := NewHeaderProtocol(mockSocket)
	if err != nil {
		t.Fatalf("failed to create header protocol: %s", err)
	}

	proto1.(RequestHeaders).SetRequestHeader("preferred_cheese", "cheddar")
	if v, _ := proto1.(RequestHeaders).GetRequestHeaders()["preferred_cheese"]; v != "cheddar" {
		t.Fatalf("failed to set header")
	}
	if len(proto1.(RequestHeaders).GetRequestHeaders()) != 1 {
		t.Fatalf("wrong number of headers")
	}

	proto1.WriteMessageBegin("", CALL, 1)
	proto1.WriteMessageEnd()
	proto1.Flush()

	_, _, _, err = proto2.ReadMessageBegin()
	if err != nil {
		t.Fatalf("failed to read message from proto1 in proto2")
	}

	if v, _ := proto2.GetResponseHeaders()["preferred_cheese"]; v != "gouda" {
		t.Fatalf("failed to read header, got: %s", v)
	}

	if peerIdentity(proto2.(*headerProtocol).trans) != "batman" {
		t.Fatalf("failed to peer identity")
	}
}
