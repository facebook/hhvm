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
	"crypto/rand"
	"crypto/tls"
	"fmt"
	"net"
	"os"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func TestServerCancellation(t *testing.T) {
	runCancellationTestFunc := func(t *testing.T, serverTransport TransportID) {
		listener, err := net.Listen("tcp", "[::]:0")
		require.NoError(t, err)
		addr := listener.Addr()
		t.Logf("Server listening on %v", addr)

		processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
		server := NewServer(processor, listener, serverTransport)

		serverCtx, serverCancel := context.WithCancel(context.Background())
		var serverEG errgroup.Group
		serverEG.Go(func() error {
			return server.ServeContext(serverCtx)
		})

		// Let the server start up and get to the accept loop.
		time.Sleep(50 * time.Millisecond)

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	t.Run("NewServer/Header", func(t *testing.T) {
		runCancellationTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runCancellationTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runCancellationTestFunc(t, TransportIDRocket)
	})
}

func TestBasicServerFunctionalityTCP(t *testing.T) {
	runBasicServerTestFunc := func(t *testing.T, serverTransport TransportID) {
		var clientTransportOption ClientOption
		switch serverTransport {
		case TransportIDHeader:
			clientTransportOption = WithHeader()
		case TransportIDUpgradeToRocket:
			clientTransportOption = WithUpgradeToRocket()
		case TransportIDRocket:
			clientTransportOption = WithRocket()
		default:
			panic("unsupported transport!")
		}

		listener, err := net.Listen("tcp", "[::]:0")
		require.NoError(t, err)
		addr := listener.Addr()
		t.Logf("Server listening on %v", addr)

		processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
		server := NewServer(processor, listener, serverTransport)

		serverCtx, serverCancel := context.WithCancel(context.Background())
		var serverEG errgroup.Group
		serverEG.Go(func() error {
			return server.ServeContext(serverCtx)
		})

		channel, err := NewClient(
			clientTransportOption,
			WithIoTimeout(5*time.Second),
			WithDialer(func() (net.Conn, error) {
				return net.DialTimeout(addr.Network(), addr.String(), 5*time.Second)
			}),
		)
		require.NoError(t, err)
		client := dummyif.NewDummyChannelClient(channel)
		err = client.Ping(context.Background())
		require.NoError(t, err)
		err = client.Close()
		require.NoError(t, err)

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	t.Run("NewServer/Header", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDRocket)
	})
}

func TestBasicServerFunctionalityUDX(t *testing.T) {
	runBasicServerTestFunc := func(t *testing.T, serverTransport TransportID) {
		var clientTransportOption ClientOption
		switch serverTransport {
		case TransportIDHeader:
			clientTransportOption = WithHeader()
		case TransportIDUpgradeToRocket:
			clientTransportOption = WithUpgradeToRocket()
		case TransportIDRocket:
			clientTransportOption = WithRocket()
		default:
			panic("unsupported transport!")
		}

		path := fmt.Sprintf("/tmp/test%s.sock", rand.Text())
		defer os.Remove(path)
		addr, err := net.ResolveUnixAddr("unix", path)
		require.NoError(t, err)

		listener, err := net.ListenUnix("unix", addr)
		require.NoError(t, err)
		t.Logf("Server listening on %v", addr)

		processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
		server := NewServer(processor, listener, serverTransport)

		serverCtx, serverCancel := context.WithCancel(context.Background())
		var serverEG errgroup.Group
		serverEG.Go(func() error {
			return server.ServeContext(serverCtx)
		})

		channel, err := NewClient(
			clientTransportOption,
			WithIoTimeout(5*time.Second),
			WithDialer(func() (net.Conn, error) {
				return net.DialTimeout(addr.Network(), addr.String(), 5*time.Second)
			}),
		)
		require.NoError(t, err)
		client := dummyif.NewDummyChannelClient(channel)
		err = client.Ping(context.Background())
		require.NoError(t, err)
		err = client.Close()
		require.NoError(t, err)

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	t.Run("NewServer/Header", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDRocket)
	})
}

func TestBasicServerFunctionalityTLS(t *testing.T) {
	clientConfig, serverConfig, err := generateSelfSignedCerts()
	require.NoError(t, err)

	runBasicServerTestFunc := func(t *testing.T, serverTransport TransportID) {
		var clientTransportOption ClientOption
		switch serverTransport {
		case TransportIDHeader:
			clientTransportOption = WithHeader()
		case TransportIDUpgradeToRocket:
			clientTransportOption = WithUpgradeToRocket()
		case TransportIDRocket:
			clientTransportOption = WithRocket()
		default:
			panic("unsupported transport!")
		}

		listener, err := tls.Listen("tcp", "[::]:0", serverConfig)
		require.NoError(t, err)
		addr := listener.Addr()
		t.Logf("Server listening on %v", addr)

		processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
		server := NewServer(processor, listener, serverTransport)

		serverCtx, serverCancel := context.WithCancel(context.Background())
		var serverEG errgroup.Group
		serverEG.Go(func() error {
			return server.ServeContext(serverCtx)
		})

		channel, err := NewClient(
			clientTransportOption,
			WithIoTimeout(5*time.Second),
			WithTLS(addr.String(), 5*time.Second, clientConfig),
		)
		require.NoError(t, err)
		client := dummyif.NewDummyChannelClient(channel)
		err = client.Ping(context.Background())
		require.NoError(t, err)
		err = client.Close()
		require.NoError(t, err)

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	t.Run("NewServer/Header", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runBasicServerTestFunc(t, TransportIDRocket)
	})
}
