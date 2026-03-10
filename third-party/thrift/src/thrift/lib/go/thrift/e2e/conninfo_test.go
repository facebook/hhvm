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

func TestEchoConnInfo(t *testing.T) {
	t.Run("UpgradeToRocket", func(t *testing.T) {
		runConnInfoTest(t, thrift.TransportIDUpgradeToRocket)
	})
	t.Run("Rocket", func(t *testing.T) {
		runConnInfoTest(t, thrift.TransportIDRocket)
	})
}

func runConnInfoTest(t *testing.T, serverTransport thrift.TransportID) {
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
			return net.DialTimeout("tcp", addr.String(), 5*time.Second)
		}),
		thrift.WithIoTimeout(5*time.Second),
	)
	require.NoError(t, err)
	client := service.NewE2EChannelClient(channel)
	defer client.Close()

	connInfo, err := client.EchoConnInfo(context.Background())
	require.NoError(t, err)
	require.NotEmpty(t, connInfo["remote_address"])
}
