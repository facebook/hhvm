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
		{"Header", ApplyALPNHeader, []string{"thrift"}},
		{"Rocket", ApplyALPNRocket, []string{"rs"}},
		{"UpgradeToRocket", ApplyALPNUpgradeToRocket, []string{"rs", "thrift"}},
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
	defaultConfig := newServerConfig()
	require.Equal(t, GoroutinePerRequest, defaultConfig.numWorkers)

	customConfig := newServerConfig(WithNumWorkers(12345))
	require.Equal(t, 12345, customConfig.numWorkers)
}

func TestWithConnContext(t *testing.T) {
	dummyConn, _ := net.Pipe()

	defaultConfig := newServerConfig()
	require.NotNil(t, defaultConfig.connContext)
	defaultConfig.connContext(context.TODO(), dummyConn)

	invoked := false
	customConnContextFn := func(ctx context.Context, conn net.Conn) context.Context { invoked = true; return ctx }
	customConfig := newServerConfig(WithConnContext(customConnContextFn))
	require.NotNil(t, customConfig.connContext)
	customConfig.connContext(context.TODO(), dummyConn)
	require.True(t, invoked)
}

func TestWithLog(t *testing.T) {
	defaultConfig := newServerConfig()
	require.NotNil(t, defaultConfig.log)
	defaultConfig.log("test")

	invoked := false
	customLogFn := func(format string, args ...any) { invoked = true }
	customConfig := newServerConfig(WithLog(customLogFn))
	require.NotNil(t, customConfig.log)
	customConfig.log("test")
	require.True(t, invoked)
}

func TestWithServerStats(t *testing.T) {
	defaultConfig := newServerConfig()
	require.NotNil(t, defaultConfig.serverStats)

	customServerStats := stats.NewServerStats(stats.NewTimingConfig(60), 60)
	customConfig := newServerConfig(WithServerStats(customServerStats))
	require.Equal(t, customServerStats, customConfig.serverStats)
}

func TestWithProcessorStats(t *testing.T) {
	defaultConfig := newServerConfig()
	require.NotNil(t, defaultConfig.processorStats)
	require.Len(t, defaultConfig.processorStats, 0)

	customProcessorStats := map[string]*stats.TimingSeries{"test": {}}
	customConfig := newServerConfig(WithProcessorStats(customProcessorStats))
	require.Equal(t, customProcessorStats, customConfig.processorStats)
}

func TestWithServerObserver(t *testing.T) {
	defaultConfig := newServerConfig()
	require.NotNil(t, defaultConfig.serverObserver)
	require.IsType(t, &noopServerObserver{}, defaultConfig.serverObserver)

	type customServerObserver = noopServerObserver
	customObserver := &customServerObserver{}
	customConfig := newServerConfig(WithServerObserver(customObserver))
	require.Equal(t, customObserver, customConfig.serverObserver)
}

func TestWithLoadFn(t *testing.T) {
	defaultConfig := newServerConfig()
	require.Nil(t, defaultConfig.loadFn)

	customConfig := newServerConfig(WithLoadFn(func() uint32 { return 123 }))
	require.NotNil(t, customConfig.loadFn)
}
