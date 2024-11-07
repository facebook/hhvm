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
	"context"
	"net"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// Test that upgrade to rocket server accepts one way calls.
func TestUpgradeToRocketServerOneWay(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	received := make(chan *MyTestStruct)
	server := NewSimpleServer(&rocketServerTestProcessor{received}, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	proto, err := newUpgradeToRocketClient(conn, types.ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := NewSerialChannel(proto)
	req := &MyTestStruct{
		St: "hello",
	}
	if err := client.Oneway(context.Background(), "test", req); err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	<-received
	cancel()
	<-errChan
}
