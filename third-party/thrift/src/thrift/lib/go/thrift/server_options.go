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
	"log"
	"net"
	"os"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
)

const (
	defaultStatsPeriod = 1 * time.Minute
	// GoroutinePerRequest is a special value to use in SetNumWorkers to enable
	// a goroutine per request (instead of a worker pool of goroutines)
	GoroutinePerRequest = -1
)

// ServerOption is the option for the thrift server.
type ServerOption func(*serverConfig)

// ConnContextFunc is the type for connection context modifier functions.
type ConnContextFunc func(context.Context, net.Conn) context.Context

// serverConfig is config needed to run a thrift server
type serverConfig struct {
	numWorkers     int
	log            func(format string, args ...any)
	connContext    ConnContextFunc
	serverStats    *stats.ServerStats
	processorStats map[string]*stats.TimingSeries
	serverObserver ServerObserver
	maxRequests    int64
}

func defaultServerConfig() *serverConfig {
	logger := log.New(os.Stderr, "", log.LstdFlags)
	return &serverConfig{
		numWorkers:     GoroutinePerRequest,
		log:            logger.Printf,
		connContext:    WithConnInfo,
		processorStats: make(map[string]*stats.TimingSeries),
		serverStats:    stats.NewServerStats(stats.NewTimingConfig(defaultStatsPeriod), defaultStatsPeriod),
		serverObserver: newNoopServerObserver(),
		maxRequests:    0, // disable
	}
}

func newServerConfig(options ...ServerOption) *serverConfig {
	config := defaultServerConfig()
	for _, option := range options {
		option(config)
	}
	return config
}

// WithNumWorkers sets the number of concurrent workers for the thrift server.
// These workers are responsible for executing client requests.
// This should be tuned based on the nature of the application using the framework.
// if special value of thrift.GoroutinePerRequest (-1) is passed, thrift will not use
// a pool of workers and instead launch a goroutine per request.
func WithNumWorkers(num int) ServerOption {
	return func(server *serverConfig) {
		server.numWorkers = num
	}
}

// WithConnContext adds connContext option
// that specifies a function that modifies the context passed to procedures per connection.
func WithConnContext(connContext ConnContextFunc) ServerOption {
	return func(server *serverConfig) {
		server.connContext = func(ctx context.Context, conn net.Conn) context.Context {
			ctx = WithConnInfo(ctx, conn)
			return connContext(ctx, conn)
		}
	}
}

// WithLog allows you to over-ride the location that exceptional server events are logged.
// The default is stderr.
func WithLog(log func(format string, args ...any)) ServerOption {
	return func(server *serverConfig) {
		server.log = log
	}
}

// WithServerStats allows the user to provide stats for the server to update.
func WithServerStats(serverStats *stats.ServerStats) ServerOption {
	return func(server *serverConfig) {
		server.serverStats = serverStats
	}
}

// WithProcessorStats allows the user to provide stats for the server to update for each processor function.
func WithProcessorStats(processorStats map[string]*stats.TimingSeries) ServerOption {
	return func(server *serverConfig) {
		server.processorStats = processorStats
	}
}

// WithServerObserver allows the user to provide a custom ServerObserver for the server.
func WithServerObserver(serverObserver ServerObserver) ServerOption {
	return func(server *serverConfig) {
		server.serverObserver = serverObserver
	}
}

// WithMaxRequests sets the maximum number of active requests before server rejects new ones.
// A value of 0 disables overload protection (default behavior).
func WithMaxRequests(maxRequests int64) ServerOption {
	return func(server *serverConfig) {
		server.maxRequests = maxRequests
	}
}

// ApplyALPNHeader adds the ALPN value "thrift" to the provided tls.Config
func ApplyALPNHeader(tlsConfig *tls.Config) {
	tlsConfig.NextProtos = []string{"thrift"}
}

// ApplyALPNRocket adds the ALPN value "rs" to the provided tls.Config
func ApplyALPNRocket(tlsConfig *tls.Config) {
	tlsConfig.NextProtos = []string{"rs"}
}

// ApplyALPNUpgradeToRocket adds the ALPN value "rs" and "thrift" to the provided tls.Config
func ApplyALPNUpgradeToRocket(tlsConfig *tls.Config) {
	tlsConfig.NextProtos = []string{"rs" /* preferred */, "thrift" /* fallback */}
}
