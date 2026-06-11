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
	"net"
	"testing"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/lib/go/thrift/e2e/service"

	"github.com/stretchr/testify/require"
)

// TestEchoHeadersStream verifies that RequestContext headers are populated on
// the streaming server path: the first response of the stream echoes back the
// request headers retrieved via thrift.GetRequestContext.
func TestEchoHeadersStream(t *testing.T) {
	const (
		persistentHeaderKey   = "persistentHeader"
		persistentHeaderValue = "persistentHeaderValue"
		rpcHeaderKey          = "rpcHeader"
		rpcHeaderValue        = "rpcHeaderValue"
	)

	addr, stopServer := startE2EServer(t, thrift.TransportIDRocket, thrift.WithNumWorkers(10))
	defer stopServer()

	channel, err := thrift.NewClient(
		thrift.WithRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.DialTimeout("tcp", addr.String(), 5*time.Second)
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

	streamCtx, streamCancel := context.WithCancel(ctx)
	defer streamCancel()
	echoedHeaders, streamSeq, err := client.EchoHeadersStream(streamCtx)
	require.NoError(t, err)
	require.Equal(t, persistentHeaderValue, echoedHeaders[persistentHeaderKey])
	require.Equal(t, rpcHeaderValue, echoedHeaders[rpcHeaderKey])

	// The stream is empty; draining it should yield no elements and no error.
	for _, err := range streamSeq {
		require.NoError(t, err)
	}
}

// TestEchoConnInfoStream verifies that RequestContext connection info is
// populated on the streaming server path: the first response of the stream
// echoes back the client's remote address.
func TestEchoConnInfoStream(t *testing.T) {
	addr, stopServer := startE2EServer(t, thrift.TransportIDRocket, thrift.WithNumWorkers(10))
	defer stopServer()

	channel, err := thrift.NewClient(
		thrift.WithRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.DialTimeout("tcp", addr.String(), 5*time.Second)
		}),
		thrift.WithIoTimeout(5*time.Second),
	)
	require.NoError(t, err)
	client := service.NewE2EChannelClient(channel)
	defer client.Close()

	streamCtx, streamCancel := context.WithCancel(context.Background())
	defer streamCancel()
	connInfo, streamSeq, err := client.EchoConnInfoStream(streamCtx)
	require.NoError(t, err)
	require.NotEmpty(t, connInfo["remote_address"])

	// The stream is empty; draining it should yield no elements and no error.
	for _, err := range streamSeq {
		require.NoError(t, err)
	}
}
