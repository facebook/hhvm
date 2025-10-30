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
	"runtime"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func TestDefaultClientConfig(t *testing.T) {
	config := newClientConfig()
	require.Equal(t, TransportIDUnknown, config.transport)
	require.Equal(t, types.ProtocolIDCompact, config.protocol)
	require.Equal(t, NoTimeout, config.ioTimeout)
	require.NotNil(t, config.persistentHeaders)
	require.Empty(t, config.persistentHeaders)
	require.Nil(t, config.dialerFn)
	require.Nil(t, config.tlsConfig)
}

func TestCustomClientOptions(t *testing.T) {
	t.Run("WithProtocolID", func(t *testing.T) {
		config := newClientConfig(WithProtocolID(types.ProtocolIDBinary))
		require.Equal(t, types.ProtocolIDBinary, config.protocol)
	})
	t.Run("WithHeader", func(t *testing.T) {
		config := newClientConfig(WithHeader())
		require.Equal(t, TransportIDHeader, config.transport)
	})
	t.Run("WithUpgradeToRocket", func(t *testing.T) {
		config := newClientConfig(WithUpgradeToRocket())
		require.Equal(t, TransportIDUpgradeToRocket, config.transport)
	})
	t.Run("WithRocket", func(t *testing.T) {
		config := newClientConfig(WithRocket())
		require.Equal(t, TransportIDRocket, config.transport)
	})
	t.Run("WithPersistentHeader", func(t *testing.T) {
		config := newClientConfig(WithPersistentHeader("foo", "bar"))
		require.Subset(t, config.persistentHeaders, map[string]string{"foo": "bar"})
	})
	t.Run("WithPersistentHeaders", func(t *testing.T) {
		config := newClientConfig(WithPersistentHeaders(map[string]string{"foo": "123", "bar": "456"}))
		require.Subset(t, config.persistentHeaders, map[string]string{"foo": "123", "bar": "456"})
	})
	t.Run("WithIdentity", func(t *testing.T) {
		config := newClientConfig(WithIdentity("foo"))
		require.Subset(t, config.persistentHeaders, map[string]string{IdentityHeader: "foo"})
	})
	t.Run("WithIoTimeout", func(t *testing.T) {
		config := newClientConfig(WithIoTimeout(time.Second))
		require.Equal(t, time.Second, config.ioTimeout)
	})
	t.Run("WithDialer", func(t *testing.T) {
		dialerFn := func() (net.Conn, error) { return nil, errors.New("foo") }
		config := newClientConfig(WithDialer(dialerFn))
		// Functions cannot be compared for equality, so use the following workaround:
		res, err := config.dialerFn()
		require.Nil(t, res)
		require.EqualError(t, err, "foo")
	})
	t.Run("WithTLS", func(t *testing.T) {
		clientConfig, _, err := generateSelfSignedCerts()
		require.NoError(t, err)
		config := newClientConfig(WithTLS("foo", time.Second, clientConfig))
		require.Equal(t, clientConfig, config.tlsConfig)
		require.NotNil(t, config.dialerFn)
	})
}

func TestNewClientConnectionScenarios(t *testing.T) {
	listener, err := net.Listen("tcp", ":0")
	require.NoError(t, err)
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	addr := listener.Addr()

	// Testing successful client connection
	client, err := NewClient(
		WithRocket(),
		WithDialer(func() (net.Conn, error) {
			return net.Dial(addr.Network(), addr.String())
		}),
	)
	require.NoError(t, err)
	err = client.Close()
	require.NoError(t, err)

	// Testing unsuccessful client connection (cannot connect at all)
	client, err = NewClient(
		WithRocket(),
		WithDialer(func() (net.Conn, error) {
			return net.Dial("unix", "/tmp/non_existent_garbage_socket_12345")
		}),
	)
	expectedErrMsg := "dial unix /tmp/non_existent_garbage_socket_12345: connect: no such file or directory"
	require.ErrorContains(t, err, expectedErrMsg)

	// Testing unsuccessful client connection (can connect, but cannot create client)
	fdCountBefore := getNumFileDesciptors(t)
	client, err = NewClient(
		WithRocket(),
		// Invalid protocol that intentionally breaks client creation
		WithProtocolID(types.ProtocolID(12345)),
		WithDialer(func() (net.Conn, error) {
			return net.Dial(addr.Network(), addr.String())
		}),
	)
	expectedErrMsg = "unsupported ProtocolID: 12345"
	require.ErrorContains(t, err, expectedErrMsg)
	// IMPORTANT!
	// Even though we perform an explicit call to Close() in NewClient()
	// upon protocol error, actual FD closing is done by the garbage collector.
	// Without the GC call below - the FD may still linger and affect test results.
	runtime.GC()
	fdCountAfter := getNumFileDesciptors(t)
	require.LessOrEqual(t, fdCountAfter, fdCountBefore, "FDs got leaked: %d (before), %d (after)", fdCountBefore, fdCountAfter)

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}

func TestNewClientCreation(t *testing.T) {
	listener, err := net.Listen("tcp", ":0")
	require.NoError(t, err)
	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	addr := listener.Addr()

	t.Run("Rocket", func(t *testing.T) {
		channel, err := NewClient(
			WithRocket(),
			WithDialer(func() (net.Conn, error) {
				return net.Dial(addr.Network(), addr.String())
			}),
		)
		require.NoError(t, err)
		require.IsType(t, &rocketClient{}, channel)

		err = channel.Close()
		require.NoError(t, err)
	})

	t.Run("UpgradeToRocket", func(t *testing.T) {
		channel, err := NewClient(
			WithUpgradeToRocket(),
			WithDialer(func() (net.Conn, error) {
				return net.Dial(addr.Network(), addr.String())
			}),
		)
		require.NoError(t, err)
		require.IsType(t, &upgradeToRocketClient{}, channel)
		err = channel.Close()
		require.NoError(t, err)
	})

	t.Run("Header", func(t *testing.T) {
		channel, err := NewClient(
			WithHeader(),
			WithDialer(func() (net.Conn, error) {
				return net.Dial(addr.Network(), addr.String())
			}),
		)
		require.NoError(t, err)
		require.IsType(t, &serialChannel{}, channel)
		err = channel.Close()
		require.NoError(t, err)
	})

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}

func TestValidTransportRequired(t *testing.T) {
	require.PanicsWithError(t, "no transport specified! Please use thrift.WithUpgradeToRocket() in the thrift.NewClient call", func() {
		NewClient( /* no transport option given */ )
	})
}
