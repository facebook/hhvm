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
	"github.com/stretchr/testify/require"
)

// This tests the upgradeToRocket client against a header server.
func TestUpgradeToRocketFallbackAgainstHeaderServer(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	errChan := make(chan error)
	defer close(errChan)
	listener, err := net.Listen("tcp", ":0")
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDHeader)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	require.NoError(t, err)

	proto, err := newUpgradeToRocketClient(conn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(newSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

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
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	require.NoError(t, err)

	proto, err := newHeaderProtocol(conn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(newSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

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
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	require.NoError(t, err)

	proto, err := newUpgradeToRocketClient(conn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(newSerialChannel(proto))
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

	// check if client was really upgraded to rocket and is not using header
	upgradeToRocketClient := proto.(*upgradeToRocketClient)
	require.IsType(t, &rocketClient{}, upgradeToRocketClient.Protocol)

	cancel()
	<-errChan
}
