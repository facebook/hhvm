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
	"errors"
	"fmt"
	"net"
	"os"
	"runtime"
	"sync/atomic"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"thrift/lib/go/thrift"
)

func TestServerStress(t *testing.T) {
	t.Run("Header", func(t *testing.T) {
		runStressTest(t, thrift.TransportIDHeader)
	})
	// t.Run("UpgradeToRocket", func(t *testing.T) {
	// 	runStressTest(t, thrift.TransportIDUpgradeToRocket)
	// })
	// t.Run("Rocket", func(t *testing.T) {
	// 	runStressTest(t, thrift.TransportIDRocket)
	// })
}

func runStressTest(t *testing.T, serverTransport thrift.TransportID) {
	listener, err := net.Listen("unix", fmt.Sprintf("/tmp/thrift_go_stress_server_test_%d.sock", os.Getpid()))
	if err != nil {
		t.Fatalf("could not create listener: %s", err)
	}
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

	processor := dummy.NewDummyProcessor(&dummy.DummyHandler{})
	server := thrift.NewServer(processor, listener, serverTransport, connContextOption, thrift.WithNumWorkers(10))

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	var successRequestCount atomic.Uint64

	makeRequestFunc := func() error {
		conn, err := thrift.NewClient(
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
		client := dummy.NewDummyChannelClient(thrift.NewSerialChannel(conn))
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
	if err != nil {
		t.Fatalf("failed to get FD count: %v", err)
	}

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
	if err != nil {
		t.Logf("successful requests: %d/%d", successRequestCount.Load(), requestCount)
		t.Fatalf("failed to make request: %v", err)
	}

	goroutinesAfterRequests := runtime.NumGoroutine()
	var memStatsAfter runtime.MemStats
	runtime.GC()
	runtime.ReadMemStats(&memStatsAfter)
	fdCountAfter, err := getNumFileDesciptors()
	if err != nil {
		t.Fatalf("failed to get FD count: %v", err)
	}

	// Go routine check (while server is still running)
	// We shouldn't exceed 100 Go-routines, if we do - server is likely leaking.
	if goroutinesAfterRequests > 100 {
		t.Fatalf("unexpected large goroutine count: %d", goroutinesAfterRequests)
	}
	// Mem alloc check (while server is still running)
	if memStatsAfter.HeapAlloc > 50*1024*1024 /* 50MB */ {
		t.Fatalf("unexpectedly large memory alloc: %d", memStatsAfter.HeapAlloc)
	}
	// FD count check (against FD leaks)
	if fdCountAfter > fdCountBefore {
		t.Fatalf("unexpected FD count increase: %d (before), %d (after)", fdCountBefore, fdCountAfter)
	}
	// Latency per-request
	if timePerRequest > 500*time.Microsecond {
		t.Fatalf("unexpected per-request latency: %v", timePerRequest)
	}

	// Shut down server.
	serverCancel()
	err = serverEG.Wait()
	if err != nil && !errors.Is(err, context.Canceled) {
		t.Fatalf("unexpected error in ServeContext: %v", err)
	}

	// Go routine check (after server shutdown)
	// We shouldn't exceed 10 Go-routines, if we do - something didn't get cleaned up properly.
	goroutinesAfterServerStop := runtime.NumGoroutine()
	if goroutinesAfterServerStop > 10 {
		t.Fatalf("unexpected large goroutine count: %d", goroutinesAfterServerStop)
	}
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
