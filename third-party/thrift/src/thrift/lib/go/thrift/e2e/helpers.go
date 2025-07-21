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
	"testing"
	"time"

	"golang.org/x/sync/errgroup"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"thrift/lib/go/thrift/e2e/handler"
	"thrift/lib/go/thrift/e2e/service"

	"github.com/stretchr/testify/require"
)

func startE2EServer(t *testing.T, serverTransport thrift.TransportID, options ...thrift.ServerOption) (net.Addr, func()) {
	listener, err := net.Listen("unix", fmt.Sprintf("/tmp/thrift_e2e_test_%d.sock", time.Now().UnixNano()))
	require.NoError(t, err)
	addr := listener.Addr()
	t.Logf("Server listening on %v", addr)

	processor := service.NewE2EProcessor(&handler.E2EHandler{})
	server := thrift.NewServer(processor, listener, serverTransport, options...)

	serverCtx, serverCancel := context.WithCancel(context.Background())
	var serverEG errgroup.Group
	serverEG.Go(func() error {
		return server.ServeContext(serverCtx)
	})

	stopServer := func() {
		// Shut down server.
		serverCancel()
		err = serverEG.Wait()
		require.ErrorIs(t, err, context.Canceled)
	}
	return addr, stopServer
}
