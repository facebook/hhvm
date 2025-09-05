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
	"sync"
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

type testServerConfig struct {
	maxRequests     int64
	timeoutDuration time.Duration
}

type testHarness struct {
	server       Server
	client       dummyif.DummyClientInterface
	serverCtx    context.Context
	serverCancel context.CancelFunc
	serverEG     *errgroup.Group
}

// setupTestServer creates a rocket server with configurable overload protection
func setupTestServer(t *testing.T, config testServerConfig) *testHarness {
	listener, err := net.Listen("tcp", "[::]:0")
	require.NoError(t, err)
	addr := listener.Addr()

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	var serverOptions []ServerOption
	if config.maxRequests > 0 {
		serverOptions = append(serverOptions, WithMaxRequests(config.maxRequests))
	}

	server := NewServer(processor, listener, TransportIDRocket, serverOptions...)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	channel, err := NewClient(
		WithRocket(),
		WithIoTimeout(config.timeoutDuration),
		WithDialer(func() (net.Conn, error) {
			return net.DialTimeout(addr.Network(), addr.String(), config.timeoutDuration)
		}),
	)
	require.NoError(t, err)
	client := dummyif.NewDummyChannelClient(channel)

	return &testHarness{
		server:       server,
		client:       client,
		serverCtx:    serverCtx,
		serverCancel: serverCancel,
		serverEG:     &serverEG,
	}
}

func (h *testHarness) teardown(t *testing.T) {
	err := h.client.Close()
	require.NoError(t, err)
	h.serverCancel()
	err = h.serverEG.Wait()
	require.ErrorIs(t, err, context.Canceled)
}

type concurrentRequestResult struct {
	successCount  int
	overloadCount int
	totalCount    int
	results       []error
}

// executeConcurrentRequests tests server behavior under concurrent load
func executeConcurrentRequests(client dummyif.DummyClientInterface, numRequests int, sleepDuration int64) *concurrentRequestResult {
	var wg sync.WaitGroup
	results := make([]error, numRequests)

	for i := range numRequests {
		wg.Go(func() {
			results[i] = client.Sleep(context.Background(), sleepDuration)
		})
	}
	wg.Wait()

	var successCount, overloadCount int
	for _, result := range results {
		if result == nil {
			successCount++
		} else if result.Error() == "APPLICATION_ERROR: load shedding due to max request limit" {
			overloadCount++
		}
	}

	return &concurrentRequestResult{
		successCount:  successCount,
		overloadCount: overloadCount,
		totalCount:    numRequests,
		results:       results,
	}
}

func TestRocketServerOverload(t *testing.T) {
	config := testServerConfig{
		maxRequests:     1,
		timeoutDuration: 5 * time.Second,
	}
	harness := setupTestServer(t, config)
	defer harness.teardown(t)

	err := harness.client.Ping(context.Background())
	require.NoError(t, err)

	const numRequests = 10
	const sleepDuration = 3000
	result := executeConcurrentRequests(harness.client, numRequests, sleepDuration)

	assert.Greater(t, result.overloadCount, 0)
	assert.Greater(t, result.successCount, 0)
	assert.Equal(t, numRequests, result.successCount+result.overloadCount)

	for _, err := range result.results {
		if err != nil {
			assert.Equal(t, "APPLICATION_ERROR: load shedding due to max request limit", err.Error())
		}
	}
}

func TestRocketServerOverloadDisabled(t *testing.T) {
	config := testServerConfig{
		maxRequests:     0,
		timeoutDuration: 10 * time.Second,
	}
	harness := setupTestServer(t, config)
	defer harness.teardown(t)

	const numRequests = 10
	const sleepDuration = 500
	result := executeConcurrentRequests(harness.client, numRequests, sleepDuration)

	for _, err := range result.results {
		assert.NoError(t, err)
	}
	assert.Equal(t, numRequests, result.successCount)
	assert.Equal(t, 0, result.overloadCount)
}

func TestLoadSheddingExceptionType(t *testing.T) {
	loadSheddingErr := NewApplicationException(LOADSHEDDING, "test message")
	assert.Equal(t, int32(8), LOADSHEDDING)
	assert.Equal(t, LOADSHEDDING, loadSheddingErr.TypeID())
	assert.Equal(t, "test message", loadSheddingErr.Error())
}

func TestLoadSheddingErrorConstant(t *testing.T) {
	assert.Equal(t, LOADSHEDDING, loadSheddingError.TypeID())
	assert.Equal(t, "load shedding due to max request limit", loadSheddingError.Error())
}
