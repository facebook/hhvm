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
	"fmt"
	"net"
	"os"
	"runtime"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"

	"golang.org/x/sync/errgroup"
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
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	require.NoError(t, err)

	cconn := &closeConn{Conn: conn, closed: make(chan struct{}, 10)}
	channel, err := newRocketClient(cconn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(channel)
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

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
	path := fmt.Sprintf("/tmp/test%s.sock", rand.Text())
	defer os.Remove(path)
	listener, err := net.Listen("unix", path)
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	addr := listener.Addr()
	conn, err := net.Dial(addr.Network(), addr.String())
	require.NoError(t, err)

	channel, err := newRocketClient(conn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(channel)
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

	cancel()
	<-errChan
}

func TestFDRelease(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	require.NoError(t, err)

	addr := listener.Addr().(*net.TCPAddr)
	t.Logf("Server listening on %v", addr)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	for range 10000 {
		channel, err := NewClient(
			WithRocket(),
			WithIoTimeout(60*time.Second),
			WithDialer(func() (net.Conn, error) {
				return net.DialTimeout("tcp", addr.String(), 60*time.Second)
			}),
		)
		require.NoError(t, err)
		// NOTE!!!!!!
		// Close() call is missing intentionally!!!!
		// We are testing that the FD is released when the client is GCed.
		client := dummyif.NewDummyChannelClient(channel)
		_, err = client.Echo(context.Background(), "hello")
		require.NoError(t, err)
	}

	// Run GC to ensure it releases the underlying FDs
	for range 10 {
		runtime.GC()
	}
	fdCount := getNumFileDesciptors(t)

	// Because we run alongside other tests concurrently - we cannot assert zero.
	// But it is sufficient to assert that we are down to below 200 FDs.
	// This is a very solid assertion, given that we opened 10,000 connections (FDs)
	// in the for-loop above.
	require.Less(t, fdCount, 200)

	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}
