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
	"crypto/tls"
	"net"
	"testing"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
	"github.com/stretchr/testify/require"
)

func TestListenerALPNOptions(t *testing.T) {
	testCases := []struct {
		name               string
		option             func(*tls.Config)
		expectedNextProtos []string
	}{
		{"Header", WithALPNHeader(), []string{"thrift"}},
		{"Rocket", WithALPNRocket(), []string{"rs"}},
		{"UpgradeToRocket", WithALPNUpgradeToRocket(), []string{"rs", "thrift"}},
	}

	for _, tc := range testCases {
		t.Run(tc.name, func(t *testing.T) {
			tlsConfig := &tls.Config{}
			tc.option(tlsConfig)
			require.Equal(t, tc.expectedNextProtos, tlsConfig.NextProtos)
		})
	}
}

func TestWithNumWorkers(t *testing.T) {
	defaultOptions := newServerOptions()
	require.Equal(t, GoroutinePerRequest, defaultOptions.numWorkers)

	customOptions := newServerOptions(WithNumWorkers(12345))
	require.Equal(t, 12345, customOptions.numWorkers)
}

func TestWithConnContext(t *testing.T) {
	dummyConn, _ := net.Pipe()

	defaultOptions := newServerOptions()
	require.NotNil(t, defaultOptions.connContext)
	defaultOptions.connContext(context.TODO(), dummyConn)

	invoked := false
	customConnContextFn := func(ctx context.Context, conn net.Conn) context.Context { invoked = true; return ctx }
	customOptions := newServerOptions(WithConnContext(customConnContextFn))
	require.NotNil(t, customOptions.connContext)
	customOptions.connContext(context.TODO(), dummyConn)
	require.True(t, invoked)
}

func TestWithLog(t *testing.T) {
	defaultOptions := newServerOptions()
	require.NotNil(t, defaultOptions.log)
	defaultOptions.log("test")

	invoked := false
	customLogFn := func(format string, args ...any) { invoked = true }
	customOptions := newServerOptions(WithLog(customLogFn))
	require.NotNil(t, customOptions.log)
	customOptions.log("test")
	require.True(t, invoked)
}

func TestWithServerStats(t *testing.T) {
	defaultOptions := newServerOptions()
	require.NotNil(t, defaultOptions.serverStats)

	customServerStats := stats.NewServerStats(stats.NewTimingConfig(60), 60)
	customOptions := newServerOptions(WithServerStats(customServerStats))
	require.Equal(t, customServerStats, customOptions.serverStats)
}

func TestWithProcessorStats(t *testing.T) {
	defaultOptions := newServerOptions()
	require.NotNil(t, defaultOptions.processorStats)
	require.Len(t, defaultOptions.processorStats, 0)

	customProcessorStats := map[string]*stats.TimingSeries{"test": {}}
	customOptions := newServerOptions(WithProcessorStats(customProcessorStats))
	require.Equal(t, customProcessorStats, customOptions.processorStats)
}

func TestWithServerObserver(t *testing.T) {
	defaultOptions := newServerOptions()
	require.NotNil(t, defaultOptions.serverObserver)
	require.IsType(t, &noopServerObserver{}, defaultOptions.serverObserver)

	type customServerObserver = noopServerObserver
	customObserver := &customServerObserver{}
	customOptions := newServerOptions(WithServerObserver(customObserver))
	require.Equal(t, customObserver, customOptions.serverObserver)
}
