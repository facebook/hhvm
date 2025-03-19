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
	"fmt"
	"net"
	"os"
	"runtime"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

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
		WithIoTimeout(time.Second),
		WithProtocolID(types.ProtocolIDCompact),
		WithPersistentHeader("foo", "bar"),
		WithIdentity("client_test"),
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
	fdCountBefore, err := getNumFileDesciptors()
	require.NoError(t, err)
	client, err = NewClient(
		WithRocket(),
		// Invalid protocol that intentionally breaks client creation
		WithProtocolID(types.ProtocolID(12345)),
		WithDialer(func() (net.Conn, error) {
			return net.Dial(addr.Network(), addr.String())
		}),
	)
	expectedErrMsg = "Unknown protocol id: 12345"
	require.ErrorContains(t, err, expectedErrMsg)
	// IMPORTANT!
	// Even though we perform an explicit call to Close() in NewClient()
	// upon protocol error, actual FD closing is done by the garbage collector.
	// Without the GC call below - the FD may still linger and affect test results.
	runtime.GC()
	fdCountAfter, err := getNumFileDesciptors()
	require.NoError(t, err)
	require.LessOrEqual(t, fdCountAfter, fdCountBefore, "FDs got leaked: %d (before), %d (after)", fdCountBefore, fdCountAfter)

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}

func getNumFileDesciptors() (int, error) {
	fdDir, err := os.Open(fmt.Sprintf("/proc/%d/fd", os.Getpid()))
	if err != nil {
		return -1, err
	}
	defer fdDir.Close()

	files, err := fdDir.Readdirnames(-1)
	if err != nil {
		return -1, err
	}

	return len(files), nil
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
		require.IsType(t, &SerialChannel{}, channel)
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
		require.IsType(t, &SerialChannel{}, channel)
		err = channel.Close()
		require.NoError(t, err)
	})

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}
