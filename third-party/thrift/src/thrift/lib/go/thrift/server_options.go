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
type ServerOption func(*serverOptions)

// ConnContextFunc is the type for connection context modifier functions.
type ConnContextFunc func(context.Context, net.Conn) context.Context

// serverOptions is options needed to run a thrift server
type serverOptions struct {
	pipeliningEnabled bool
	numWorkers        int
	log               func(format string, args ...any)
	connContext       ConnContextFunc
	serverStats       *stats.ServerStats
	processorStats    map[string]*stats.TimingSeries
	serverObserver    ServerObserver
}

func defaultServerOptions() *serverOptions {
	logger := log.New(os.Stderr, "", log.LstdFlags)
	return &serverOptions{
		pipeliningEnabled: true,
		numWorkers:        GoroutinePerRequest,
		log:               logger.Printf,
		connContext:       WithConnInfo,
		processorStats:    make(map[string]*stats.TimingSeries),
		serverStats:       stats.NewServerStats(stats.NewTimingConfig(defaultStatsPeriod), defaultStatsPeriod),
		serverObserver:    newNoopServerObserver(),
	}
}

func newServerOptions(options ...ServerOption) *serverOptions {
	opts := defaultServerOptions()
	for _, option := range options {
		option(opts)
	}
	return opts
}

// WithoutPipelining disables pipelining for the thrift server.
func WithoutPipelining() ServerOption {
	return func(server *serverOptions) {
		server.pipeliningEnabled = false
	}
}

// WithNumWorkers sets the number of concurrent workers for the thrift server.
// These workers are responsible for executing client requests.
// This should be tuned based on the nature of the application using the framework.
// if special value of thrift.GoroutinePerRequest (-1) is passed, thrift will not use
// a pool of workers and instead launch a goroutine per request.
func WithNumWorkers(num int) ServerOption {
	return func(server *serverOptions) {
		server.numWorkers = num
	}
}

// WithConnContext adds connContext option
// that specifies a function that modifies the context passed to procedures per connection.
func WithConnContext(connContext ConnContextFunc) ServerOption {
	return func(server *serverOptions) {
		server.connContext = func(ctx context.Context, conn net.Conn) context.Context {
			ctx = WithConnInfo(ctx, conn)
			return connContext(ctx, conn)
		}
	}
}

// WithLog allows you to over-ride the location that exceptional server events are logged.
// The default is stderr.
func WithLog(log func(format string, args ...any)) ServerOption {
	return func(server *serverOptions) {
		server.log = log
	}
}

// WithServerStats allows the user to provide stats for the server to update.
func WithServerStats(serverStats *stats.ServerStats) ServerOption {
	return func(server *serverOptions) {
		server.serverStats = serverStats
	}
}

// WithProcessorStats allows the user to provide stats for the server to update for each processor function.
func WithProcessorStats(processorStats map[string]*stats.TimingSeries) ServerOption {
	return func(server *serverOptions) {
		server.processorStats = processorStats
	}
}

// WithServerObserver allows the user to provide a custom ServerObserver for the server.
func WithServerObserver(serverObserver ServerObserver) ServerOption {
	return func(server *serverOptions) {
		server.serverObserver = serverObserver
	}
}

// WithALPNHeader adds the ALPN value "thrift" to the provided tls.Config
func WithALPNHeader() func(*tls.Config) {
	return func(tlsConfig *tls.Config) {
		tlsConfig.NextProtos = []string{"thrift"}
	}
}

// WithALPNRocket adds the ALPN value "rs" to the provided tls.Config
func WithALPNRocket() func(*tls.Config) {
	return func(tlsConfig *tls.Config) {
		tlsConfig.NextProtos = []string{"rs"}
	}
}

// WithALPNUpgradeToRocket adds the ALPN value "rs" and "thrift" to the provided tls.Config
func WithALPNUpgradeToRocket() func(*tls.Config) {
	return func(tlsConfig *tls.Config) {
		tlsConfig.NextProtos = []string{"rs" /* preferred */, "thrift" /* fallback */}
	}
}
