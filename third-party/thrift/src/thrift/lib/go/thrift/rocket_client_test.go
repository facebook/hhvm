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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"

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
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}
	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
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
	client := dummy.NewDummyChannelClient(NewSerialChannel(proto))
	result, err := client.Echo(context.TODO(), "hello")
	if err != nil {
		t.Fatalf("could not complete call: %v", err)
	}
	if result != "hello" {
		t.Fatalf("expected response to be a hello, got %s", result)
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
	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)
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

func TestFDRelease(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	if err != nil {
		t.Fatalf("failed to listen: %v", err)
	}

	addr := listener.Addr().(*net.TCPAddr)
	t.Logf("Server listening on %v", addr)

	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	for range 10000 {
		proto, err := DeprecatedNewClient(
			WithRocket(),
			WithIoTimeout(60*time.Second),
			WithDialer(func() (net.Conn, error) {
				return net.DialTimeout("tcp", addr.String(), 60*time.Second)
			}),
		)
		if err != nil {
			t.Fatalf("failed to get client: %v", err)
		}
		// NOTE!!!!!!
		// Close() call is missing intentionally!!!!
		// We are testing that the FD is released when the client is GCed.
		client := dummy.NewDummyChannelClient(NewSerialChannel(proto))
		_, err = client.Echo(context.Background(), "hello")
		if err != nil {
			t.Fatalf("failed to make RPC: %v", err)
		}
	}

	// Run GC to ensure it releases the underlying FDs
	runtime.GC()
	fdCount, err := getNumFileDesciptors()
	if err != nil {
		t.Fatalf("failed to get FD count: %v", err)
	}
	// Because we run alongside other tests concurrently - we cannot assert zero.
	// But it is sufficient to assert that we are down to below 200 FDs.
	// This is a very solid assertion, given that we opened 10,000 connections (FDs)
	// in the for-loop above.
	if fdCount > 200 {
		t.Fatalf("too many FDs: %d", fdCount)
	}

	serverCancel()
	err = serverEG.Wait()
	if err != nil && !errors.Is(err, context.Canceled) {
		t.Fatalf("unexpected error in ServeContext: %v", err)
	}
}
