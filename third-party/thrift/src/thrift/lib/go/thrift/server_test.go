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
	"math"
	"net"
	"os"
	"runtime"
	"strconv"
	"sync"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/assert"
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

func TestALPN(t *testing.T) {
	clientConfig, serverConfig, err := generateSelfSignedCerts()
	require.NoError(t, err)

	// Apply ALPN to server config:
	WithALPNUpgradeToRocket()(serverConfig)
	listener, err := tls.Listen("tcp", "[::]:0", serverConfig)
	require.NoError(t, err)
	addr := listener.Addr()
	t.Logf("Server listening on %v", addr)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	tlsDialerOption := WithTLS(
		addr.String(), 1*time.Second, clientConfig,
	)
	plaintextDialerOption := WithDialer(func() (net.Conn, error) {
		return net.Dial("tcp", addr.String())
	})

	createAlpnClient := func(opts ...ClientOption) (dummyif.DummyClient, error) {
		channel, err := NewClient(opts...)
		if err != nil {
			return nil, fmt.Errorf("could not create client: %w", err)
		}
		return dummyif.NewDummyChannelClient(channel), nil
	}

	// Rocket
	client1, err := createAlpnClient(WithRocket(), tlsDialerOption)
	require.NoError(t, err)
	err = client1.Ping(context.TODO())
	require.NoError(t, err)

	// UpgradeToRocket
	client2, err := createAlpnClient(WithUpgradeToRocket(), tlsDialerOption)
	require.NoError(t, err)
	err = client2.Ping(context.TODO())
	require.NoError(t, err)

	// Header
	client3, err := createAlpnClient(WithHeader(), tlsDialerOption)
	require.NoError(t, err)
	err = client3.Ping(context.TODO())
	require.NoError(t, err)

	// Plaintext
	client4, err := createAlpnClient(WithUpgradeToRocket(), plaintextDialerOption)
	require.NoError(t, err)
	err = client4.Ping(context.TODO())
	// Server is configured in TLS-only mode. We expect an error when we try to speak plaintext.
	require.Error(t, err, "read: connection reset by peer")

	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}

func TestUexHeaderFunctionality(t *testing.T) {
	runUexTestFunc := func(t *testing.T, serverTransport TransportID) {
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
		ctx := NewResponseHeadersContext(context.Background())
		err = client.GetDeclaredException(ctx)
		require.Error(t, err)
		err = client.Close()
		require.NoError(t, err)

		responseHeaders := ResponseHeadersFromContext(ctx)
		// read uex header for the exception
		require.Contains(t, responseHeaders, "uex")
		require.Contains(t, responseHeaders, "uexw")
		require.Equal(t, "DummyException", responseHeaders["uex"])
		require.Equal(t, "DummyException({Message:hello})", responseHeaders["uexw"])

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	t.Run("NewServer/Header", func(t *testing.T) {
		runUexTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runUexTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runUexTestFunc(t, TransportIDRocket)
	})
}

func TestPanics(t *testing.T) {
	runPanicTestFunc := func(t *testing.T, serverTransport TransportID) {
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
		// Should receive a graceful error upon panic
		err = client.Panic(context.Background())
		require.Error(t, err)
		// Should be able to make another request after a panic
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
		runPanicTestFunc(t, TransportIDHeader)
	})
	t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
		runPanicTestFunc(t, TransportIDUpgradeToRocket)
	})
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runPanicTestFunc(t, TransportIDRocket)
	})
}

func TestGoroutinePerRequest(t *testing.T) {
	runTestFunc := func(t *testing.T, serverTransport TransportID) {
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
		server := NewServer(processor, listener, serverTransport, WithNumWorkers(GoroutinePerRequest))

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

		var clientsWG sync.WaitGroup
		startTime := time.Now()
		for range 1000 {
			clientsWG.Go(
				func() {
					err := client.Sleep(context.Background(), 500 /* ms */)
					assert.NoError(t, err)
				},
			)
		}
		clientsWG.Wait()
		timeElapsed := time.Since(startTime)

		err = client.Close()
		require.NoError(t, err)

		// The core assertion of this test!!!
		// We just made 1000 sleep requests in parallel (with 500ms sleep each).
		// That's 500s (seconds!!!) of total sleep time - quite a long duration.
		// If GoroutinePerRequest is working properly - all these sleeps would have been
		// done in parallel, taking about 500ms of real world time.
		// To avoid flakiness - use a 3 second buffer in the assertion.
		require.Less(t, timeElapsed, 3*time.Second)

		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}

	// Only supported in Rocket
	// t.Run("NewServer/Header", func(t *testing.T) {
	// 	runTestFunc(t, TransportIDHeader)
	// })
	// t.Run("NewServer/UpgradeToRocket", func(t *testing.T) {
	// 	runTestFunc(t, TransportIDUpgradeToRocket)
	// })
	t.Run("NewServer/Rocket", func(t *testing.T) {
		runTestFunc(t, TransportIDRocket)
	})
}

func TestLoadHeader(t *testing.T) {
	listener, err := net.Listen("tcp", "[::]:0")
	require.NoError(t, err)
	addr := listener.Addr()
	t.Logf("Server listening on %v", addr)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDRocket, WithNumWorkers(GoroutinePerRequest))

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	channel, err := NewClient(
		WithRocket(),
		WithIoTimeout(5*time.Second),
		WithDialer(func() (net.Conn, error) {
			return net.DialTimeout(addr.Network(), addr.String(), 5*time.Second)
		}),
	)
	require.NoError(t, err)
	client := dummyif.NewDummyChannelClient(channel)

	minLoadHeader := int64(math.MaxInt64)
	maxLoadHeader := int64(math.MinInt64)
	var minMaxLock sync.Mutex
	const concurrentCalls = 100

	makeRequest := func() {
		ctx := NewResponseHeadersContext(context.Background())
		err := client.Sleep(ctx, 10 /* ms */)
		assert.NoError(t, err)
		responseHeaders := ResponseHeadersFromContext(ctx)
		assert.Contains(t, responseHeaders, "load")
		loadStr := responseHeaders["load"]
		loadVal, err := strconv.ParseInt(loadStr, 10, 64)
		assert.NoError(t, err)
		minMaxLock.Lock()
		minLoadHeader = min(minLoadHeader, loadVal)
		maxLoadHeader = max(maxLoadHeader, loadVal)
		minMaxLock.Unlock()
	}
	// Make one standalone request to get the "min" baseline
	makeRequest()

	var clientsWG sync.WaitGroup
	for range concurrentCalls {
		clientsWG.Go(makeRequest)
	}
	clientsWG.Wait()

	err = client.Close()
	require.NoError(t, err)

	// The core assertion of this test!!!
	// now let's reason a bit about the range we should have seen in load headers
	// given the default implementation.  The default impl is:
	// 1000 * number of concurrent requests / number of cores
	// (the devisor allows machines of variable compute capacity to have comparable
	//  numbers).
	// thus we should expect min to be zero.
	// we should expect max to be close to:
	// (numberOfCallers / numCPU) * 1000
	//
	// We will only hit this theoretical max for the test if all callers
	// happend to be sleeping at the same time.  highly likely, but not
	// guaranteed.  Let's verify that we get to at least 50% of the
	// expected max.
	expectedMin := int64(math.Round(1000*float64(1)) / float64(runtime.NumCPU()))
	expectedMax := (1000 * float64(concurrentCalls)) / float64(runtime.NumCPU())

	assert.Equal(t, expectedMin, minLoadHeader)
	assert.Greater(t, float64(maxLoadHeader), expectedMax*0.5)
	assert.GreaterOrEqual(t, expectedMax, float64(maxLoadHeader))
	assert.Less(t, minLoadHeader, maxLoadHeader)

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}
