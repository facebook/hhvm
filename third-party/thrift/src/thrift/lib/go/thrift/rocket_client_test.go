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
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

type closeConn struct {
	net.Conn
	closed chan struct{}
}

func (c *closeConn) Close() error {
	c.closed <- struct{}{}
	return c.Conn.Close()
}

// Check that rocket client does close the connection
func TestRocketClientClose(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	errChan := make(chan error)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	server := NewSimpleServer(&rocketServerTestProcessor{}, listener, TransportIDRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	cconn := &closeConn{Conn: conn, closed: make(chan struct{}, 10)}
	proto, err := newRocketClient(cconn, types.ProtocolIDCompact, 0, nil)
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
	go client.Close()
	select {
	case <-cconn.closed:
	case <-time.After(3 * time.Second):
		panic("connection was not closed")
	}
	cancel()
	<-errChan
}

func TestRocketClientUnix(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	errChan := make(chan error)
	path := t.TempDir() + "/test.sock"
	listener, err := net.Listen("unix", path)
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	server := NewSimpleServer(&rocketServerTestProcessor{}, listener, TransportIDRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	proto, err := newRocketClient(conn, types.ProtocolIDCompact, 0, nil)
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
