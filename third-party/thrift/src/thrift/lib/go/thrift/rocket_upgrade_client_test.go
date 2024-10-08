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
	"bytes"
	"context"
	"net"
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

func TestUpgradeToRocketClientUnix(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	errChan := make(chan error)
	path := t.TempDir() + "/test.sock"
	addr, err := net.ResolveUnixAddr("unix", path)
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	listener, err := net.ListenUnix("unix", addr)
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	server := NewSimpleServer(&rocketServerTestProcessor{}, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	proto, err := NewClient(WithConn(conn), WithUpgradeToRocket())
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := NewSerialChannel(proto)
	req := &MyTestStruct{
		St: "hello",
	}
	resp := &MyTestStruct{}
	if err := client.Call(context.Background(), "test", req, resp); err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if resp.St != "hello" {
		t.Fatalf("expected response to be a hello, got %s", resp.St)
	}
	if !bytes.Equal(resp.GetBin(), []byte(conn.LocalAddr().String())) {
		t.Fatalf("expected response to be an address %s, got %s", conn.LocalAddr().String(), resp.GetBin())
	}
	cancel()
	<-errChan
}
