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
	"errors"
	"net"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// TestSimpleServer is a simple tests that simple sends an empty message to a server and receives an empty result.
func TestSimpleServer(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		t.Fatalf("could not create listener: %s", err)
	}
	addr := listener.Addr()
	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewSimpleServer(processor, listener, TransportIDHeader)
	errChan := make(chan error)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := server.ServeContext(ctx)
		errChan <- err
		close(errChan)
	}()
	conn, err := net.Dial("tcp", addr.String())
	if err != nil {
		t.Fatalf("could not create client socket: %s", err)
	}
	proto, err := newHeaderProtocol(conn, types.ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := dummy.NewDummyChannelClient(NewSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	if err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if result != "hello" {
		t.Fatalf("expected response to be a hello, got %s", result)
	}
	cancel()
	err = <-errChan
	if !errors.Is(err, context.Canceled) {
		t.Fatalf("expected %v, got %v", context.Canceled, err)
	}
}

// This tests that S425600 does not happen again.
// The client is allowed to set a serializaton format to the non default and the server should adjust accordingly.
func TestSimpleServerClientSetsDifferentProtocol(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		t.Fatalf("could not create listener: %s", err)
	}
	addr := listener.Addr()
	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewSimpleServer(processor, listener, TransportIDHeader)
	errChan := make(chan error)
	ctx, cancel := context.WithCancel(context.Background())
	go func() {
		err := server.ServeContext(ctx)
		errChan <- err
		close(errChan)
	}()
	// Sets the client serialization format to a non default.
	proto, err := NewClient(
		WithHeader(),
		WithDialer(func() (net.Conn, error) {
			return net.Dial("tcp", addr.String())
		}),
		WithProtocolID(types.ProtocolIDBinary))
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := dummy.NewDummyChannelClient(NewSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	if err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if result != "hello" {
		t.Fatalf("expected response to be a hello, got %s", result)
	}
	cancel()
	<-errChan
}
