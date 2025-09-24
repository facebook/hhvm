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

package stress

import (
	"context"
	"fmt"
	"net"
	"os"
	"runtime"
	"sync/atomic"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"

	"github.com/stretchr/testify/require"
)

func TestServerStress(t *testing.T) {
	t.Run("UpgradeToRocket", func(t *testing.T) {
		runStressTest(t, thrift.TransportIDUpgradeToRocket)
	})
	t.Run("Rocket", func(t *testing.T) {
		runStressTest(t, thrift.TransportIDRocket)
	})
}

func runStressTest(t *testing.T, serverTransport thrift.TransportID) {
	listener, err := net.Listen("unix", fmt.Sprintf("/tmp/thrift_go_stress_server_test_%d.sock", os.Getpid()))
	require.NoError(t, err)
	addr := listener.Addr()
	t.Logf("Server listening on %v", addr)

	var clientTransportOption thrift.ClientOption
	switch serverTransport {
	case thrift.TransportIDHeader:
		clientTransportOption = thrift.WithHeader()
	case thrift.TransportIDUpgradeToRocket:
		clientTransportOption = thrift.WithUpgradeToRocket()
	case thrift.TransportIDRocket:
		clientTransportOption = thrift.WithRocket()
	default:
		panic("unsupported transport!")
	}

	// A special server option to allocate 10KB for each incoming connection,
	// for the purposes of stress testing and exposing memory leaks.
	connContextOption := thrift.WithConnContext(
		func(ctx context.Context, conn net.Conn) context.Context {
			type dummContextKey int
			const dummyKey dummContextKey = 12345
			const dummyAllocSize = 10 * 1024 // 10KB
			return context.WithValue(ctx, dummyKey, make([]byte, dummyAllocSize, dummyAllocSize))
		},
	)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := thrift.NewServer(processor, listener, serverTransport, connContextOption, thrift.WithNumWorkers(10))

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	var successRequestCount atomic.Uint64

	makeRequestFunc := func() error {
		channel, err := thrift.NewClient(
			clientTransportOption,
			thrift.WithDialer(func() (net.Conn, error) {
				return net.DialTimeout("unix", addr.String(), 60*time.Second)
			}),
			thrift.WithIoTimeout(60*time.Second),
		)
		if err != nil {
			errRes := fmt.Errorf("failed to create client: %w", err)
			t.Log(errRes.Error())
			return errRes
		}
		client := dummyif.NewDummyChannelClient(channel)
		defer client.Close()
		result, err := client.Echo(context.Background(), "hello")
		if err != nil {
			errRes := fmt.Errorf("failed to make RPC: %w", err)
			t.Log(errRes.Error())
			return errRes
		}
		if result != "hello" {
			return fmt.Errorf("unexpected RPC result: %s", result)
		}
		successRequestCount.Add(1)
		return nil
	}

	runtime.GC()
	fdCountBefore, err := getNumFileDesciptors()
	require.NoError(t, err)

	const requestCount = 100_000
	const parallelism = 100

	var clientsEG errgroup.Group
	clientsEG.SetLimit(parallelism) // Max 100 Go-routines running at once
	startTime := time.Now()
	for range requestCount {
		clientsEG.Go(makeRequestFunc)
	}
	err = clientsEG.Wait()
	timeElapsed := time.Since(startTime)
	timePerRequest := timeElapsed / requestCount
	t.Logf("successful requests: %d/%d", successRequestCount.Load(), requestCount)
	require.NoError(t, err)

	goroutinesAfterRequests := runtime.NumGoroutine()
	var memStatsAfter runtime.MemStats
	runtime.GC()
	runtime.ReadMemStats(&memStatsAfter)
	fdCountAfter, err := getNumFileDesciptors()
	require.NoError(t, err)

	// Go routine check (while server is still running)
	// We shouldn't exceed 100 Go-routines, if we do - server is likely leaking.
	require.Less(t, goroutinesAfterRequests, 100)

	// Mem alloc check (while server is still running)
	require.Less(t, memStatsAfter.HeapAlloc, uint64(50*1024*1024) /* 50MB */)

	// FD count check (against FD leaks)
	require.LessOrEqual(t, fdCountAfter, fdCountBefore)

	// Latency per-request
	require.Less(t, timePerRequest, 500*time.Microsecond)

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)

	// A tiny sleep to allow for some lingering goroutines to finish up.
	time.Sleep(10 * time.Millisecond)

	// Go routine check (after server shutdown)
	// We shouldn't exceed 10 Go-routines, if we do - something didn't get cleaned up properly.
	goroutinesAfterServerStop := runtime.NumGoroutine()
	require.LessOrEqual(t, goroutinesAfterServerStop, 10)
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
