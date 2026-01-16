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

package gotest

import (
	"context"
	"net"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift"
	"github.com/stretchr/testify/require"
	"thrift/test/go/if/thrifttest"
)

func TestInternalConstructClientFromRegistry(t *testing.T) {
	handler := &testHandler{}
	cancel, addr, err := createTestServer(handler)
	require.NoError(t, err)
	defer cancel()

	channel, err := thrift.NewClient(
		thrift.WithUpgradeToRocket(),
		thrift.WithDialer(func() (net.Conn, error) {
			return net.Dial("tcp", addr.String())
		}),
		thrift.WithIoTimeout(localConnTimeout),
	)
	require.NoError(t, err)

	client, err := thrift.InternalConstructClientFromRegistry[thrifttest.ThriftTestClient](channel)
	require.NoError(t, err, "ThriftTestClient should be registered in the client registry")
	require.NotNil(t, client)
	defer client.Close()

	res, err := client.DoTestString(context.Background(), testCallString)
	require.NoError(t, err)
	require.Equal(t, testCallString, res)
}
