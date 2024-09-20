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
)

func TestCloseWithoutSendingMessages(t *testing.T) {
	serverSocket, err := NewListener("[::]:0")
	if err != nil {
		t.Fatalf("could not create server socket: %s", err)
	}
	addr := serverSocket.Addr()
	conn, err := DialHostPort(addr.String())
	if err != nil {
		t.Fatalf("could not create client socket: %s", err)
	}
	clientSocket, err := NewSocket(SocketConn(conn))
	if err != nil {
		t.Fatalf("could not create client socket: %s", err)
	}
	proto, err := newUpgradeToRocketClient(clientSocket, types.ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	proto.Close()
}
