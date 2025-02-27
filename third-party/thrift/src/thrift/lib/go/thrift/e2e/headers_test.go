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

	"thrift/lib/go/thrift"
	"thrift/lib/go/thrift/e2e/service"
)

// TestServicePersistentHeaders ensures that persistent headers can be retrieved
// from the context passed to the server handler.
func TestServicePersistentHeaders(t *testing.T) {
	t.Run("Header", func(t *testing.T) {
		runHeaderTest(t, thrift.TransportIDHeader)
	})
	t.Run("UpgradeToRocket", func(t *testing.T) {
		runHeaderTest(t, thrift.TransportIDUpgradeToRocket)
	})
	t.Run("Rocket", func(t *testing.T) {
		runHeaderTest(t, thrift.TransportIDRocket)
	})
}

func runHeaderTest(t *testing.T, serverTransport thrift.TransportID) {
	const (
		headerKey   = "X-My-Test-Header"
		headerValue = "Success!"
	)

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

	addr, stopServer := startE2EServer(t, serverTransport, thrift.WithNumWorkers(10))
	defer stopServer()

	conn, err := thrift.NewClient(
		clientTransportOption,
		thrift.WithDialer(func() (net.Conn, error) {
			return net.DialTimeout("unix", addr.String(), 5*time.Second)
		}),
		thrift.WithIoTimeout(5*time.Second),
		thrift.WithPersistentHeader(headerKey, headerValue),
	)
	if err != nil {
		t.Fatalf("failed to create client: %v", err)
	}
	client := service.NewE2EChannelClient(thrift.NewSerialChannel(conn))
	defer client.Close()
	echoedHeaders, err := client.EchoHeaders(context.Background())
	if err != nil {
		t.Fatalf("failed to make RPC: %v", err)
	}
	if echoedHeaders[headerKey] != headerValue {
		t.Fatalf("unexpected or missing header value: '%v'", echoedHeaders)
	}
}
