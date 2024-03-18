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

import "testing"

func TestHeaderProtocolHeaders(t *testing.T) {
	tmb := NewMemoryBuffer()
	proto1 := NewHeaderProtocol(tmb)
	proto2 := NewHeaderProtocol(tmb)

	proto1.(RequestHeaders).SetRequestHeader("preferred_cheese", "cheddar")
	if v, _ := proto1.(RequestHeaders).GetRequestHeader("preferred_cheese"); v != "cheddar" {
		t.Fatalf("failed to set header")
	}
	if len(proto1.(RequestHeaders).GetRequestHeaders()) != 1 {
		t.Fatalf("wrong number of headers")
	}

	proto1.(PersistentHeaders).SetPersistentHeader("preferred_cheese", "gouda")
	if v, _ := proto1.(PersistentHeaders).GetPersistentHeader("preferred_cheese"); v != "gouda" {
		t.Fatalf("failed to set persistent header")
	}
	if len(proto1.(PersistentHeaders).GetPersistentHeaders()) != 1 {
		t.Fatalf("wrong number of headers")
	}

	proto1.(*headerProtocol).SetIdentity("batman")
	if proto1.(*headerProtocol).Identity() != "batman" {
		t.Fatalf("failed to set identity")
	}

	proto1.WriteMessageBegin("", CALL, 1)
	proto1.WriteMessageEnd()
	proto1.Flush()

	_, _, _, err := proto2.ReadMessageBegin()
	if err != nil {
		t.Fatalf("failed to read message from proto1 in proto2")
	}

	if v, _ := proto2.(ResponseHeaderGetter).GetResponseHeader("preferred_cheese"); v != "gouda" {
		t.Fatalf("failed to read header, got: %s", v)
	}

	if proto2.(*headerProtocol).peerIdentity() != "batman" {
		t.Fatalf("failed to peer identity")
	}
}
