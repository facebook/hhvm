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

package e2e

import (
	"context"
	"fmt"
	"net"
	"sync"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/lib/go/thrift/e2e/service"

	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
)

// TestServiceHeaders ensures that persistent headers can be retrieved
// from the context passed to the server handler.
func TestServiceHeaders(t *testing.T) {
	t.Run("UpgradeToRocket", func(t *testing.T) {
		runHeaderTest(t, thrift.TransportIDUpgradeToRocket)
	})
	t.Run("Rocket", func(t *testing.T) {
		runHeaderTest(t, thrift.TransportIDRocket)
	})
}

func runHeaderTest(t *testing.T, serverTransport thrift.TransportID) {
	const (
		persistentHeaderKey   = "persistentHeader"
		persistentHeaderValue = "persistentHeaderValue"
		rpcHeaderKey          = "rpcHeader"
		rpcHeaderValue        = "rpcHeaderValue"
	)

	var clientTransportOption thrift.ClientOption
	switch serverTransport {
	case thrift.TransportIDUpgradeToRocket:
		clientTransportOption = thrift.WithUpgradeToRocket()
	case thrift.TransportIDRocket:
		clientTransportOption = thrift.WithRocket()
	}

	addr, stopServer := startE2EServer(t, serverTransport, thrift.WithNumWorkers(10))
	defer stopServer()

	channel, err := thrift.NewClient(
		clientTransportOption,
		thrift.WithDialer(func() (net.Conn, error) {
			return net.DialTimeout("unix", addr.String(), 5*time.Second)
		}),
		thrift.WithIoTimeout(5*time.Second),
		thrift.WithPersistentHeader(persistentHeaderKey, persistentHeaderValue),
	)
	require.NoError(t, err)
	client := service.NewE2EChannelClient(channel)
	defer client.Close()
	rpcOpts := thrift.RPCOptions{}
	rpcOpts.SetWriteHeaders(map[string]string{rpcHeaderKey: rpcHeaderValue})
	ctx := thrift.WithRPCOptions(context.Background(), &rpcOpts)
	echoedHeaders, err := client.EchoHeaders(ctx)
	require.NoError(t, err)
	require.Equal(t, echoedHeaders[persistentHeaderKey], persistentHeaderValue)
	require.Equal(t, echoedHeaders[rpcHeaderKey], rpcHeaderValue)
}

func TestHeadersUnderConcurrency(t *testing.T) {
	// Ensures that headers are not mixed up under concurrency/load.
	const (
		persistentHeaderKey   = "persistentHeader"
		persistentHeaderValue = "persistentHeaderValue"
		rpcHeaderKey          = "rpcHeader"
	)

	addr, stopServer := startE2EServer(t, thrift.TransportIDRocket, thrift.WithNumWorkers(10))
	defer stopServer()

	channel, err := thrift.NewClient(
		thrift.WithRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.DialTimeout("unix", addr.String(), 5*time.Second)
		}),
		thrift.WithIoTimeout(5*time.Second),
		thrift.WithPersistentHeader(persistentHeaderKey, persistentHeaderValue),
	)
	require.NoError(t, err)

	client := service.NewE2EChannelClient(channel)
	defer client.Close()

	var clientsWG sync.WaitGroup
	for i := range 1000 {
		clientsWG.Go(
			func() {
				rpcHeaderValue := fmt.Sprintf("rpcHeaderValue-%d", i)
				rpcOpts := thrift.RPCOptions{}
				rpcOpts.SetWriteHeaders(map[string]string{rpcHeaderKey: rpcHeaderValue})
				ctx := thrift.WithRPCOptions(context.Background(), &rpcOpts)
				echoedHeaders, err := client.EchoHeaders(ctx)
				assert.NoError(t, err)
				assert.Equal(t, echoedHeaders[persistentHeaderKey], persistentHeaderValue)
				assert.Equal(t, echoedHeaders[rpcHeaderKey], rpcHeaderValue)
			},
		)
	}
	clientsWG.Wait()
}
