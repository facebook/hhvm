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
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/dummy"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	dummyif "github.com/facebook/fbthrift/thrift/test/go/if/dummy"
	"github.com/stretchr/testify/require"
)

func TestCloseWithoutSendingMessages(t *testing.T) {
	serverSocket, err := net.Listen("tcp", "[::]:0")
	require.NoError(t, err)

	addr := serverSocket.Addr()
	conn, err := net.Dial("tcp", addr.String())
	require.NoError(t, err)

	proto, err := newUpgradeToRocketClient(conn, types.ProtocolIDCompact, 0, nil)
	require.NoError(t, err)

	proto.Close()
}

func TestUpgradeToRocketClientUnix(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	errChan := make(chan error)
	path := fmt.Sprintf("/tmp/test%s.sock", rand.Text())
	defer os.Remove(path)
	addr, err := net.ResolveUnixAddr("unix", path)
	require.NoError(t, err)

	listener, err := net.ListenUnix("unix", addr)
	require.NoError(t, err)

	processor := dummyif.NewDummyProcessor(&dummy.DummyHandler{})
	server := NewServer(processor, listener, TransportIDUpgradeToRocket)
	go func() {
		errChan <- server.ServeContext(ctx)
	}()
	channel, err := NewClient(
		WithUpgradeToRocket(),
		WithDialer(func() (net.Conn, error) {
			return net.Dial(addr.Network(), addr.String())
		}),
	)
	require.NoError(t, err)

	client := dummyif.NewDummyChannelClient(channel)
	defer client.Close()
	result, err := client.Echo(context.TODO(), "hello")
	require.NoError(t, err)
	require.Equal(t, "hello", result)

	cancel()
	<-errChan
}
