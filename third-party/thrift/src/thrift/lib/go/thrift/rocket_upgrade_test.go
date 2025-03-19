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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
)

// This tests the upgradeToRocket client against a header server.
func TestUpgradeToRocketFallbackAgainstHeaderServer(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDHeader)
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
	client := dummyif.NewDummyChannelClient(NewSerialChannel(proto))
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

// This tests the upgradeToRocket server against a header client.
func TestUpgradeToRocketServerAgainstHeaderClient(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		t.Fatalf("failed to dial: %v", err)
	}
	proto, err := newHeaderProtocol(conn, types.ProtocolIDCompact, 0, nil)
	if err != nil {
		t.Fatalf("could not create client protocol: %s", err)
	}
	client := dummyif.NewDummyChannelClient(NewSerialChannel(proto))
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

// This tests the upgradeToRocket client against an upgrade To Rocket server.
func TestUpgradeToRocketAgainstUpgradeToRocketServer(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)
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
	client := dummyif.NewDummyChannelClient(NewSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	if err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if result != "hello" {
		t.Fatalf("expected response to be a hello, got %s", result)
	}
	// check if client was really upgraded to rocket and is not using header
	upgradeToRocketClient := proto.(*upgradeToRocketClient)
	if _, ok := upgradeToRocketClient.Protocol.(*rocketClient); !ok {
		t.Fatalf("expected client to be rocket, got %T", upgradeToRocketClient.Protocol)
	}
	cancel()
	<-errChan
}
