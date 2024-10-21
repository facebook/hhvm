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
	"log"
	"net"
	"os"
)

// ConnContextFunc is the type for connection context modifier functions.
type ConnContextFunc func(context.Context, net.Conn) context.Context

// ServerOptions is options needed to run a thrift server
type ServerOptions struct {
	interceptor Interceptor
	connContext ConnContextFunc
	log         func(format string, args ...interface{})
}

// Deprecated: use WrapInterceptor
func WithInterceptor(interceptor Interceptor) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.interceptor = interceptor
	}
}

// WithConnContext adds connContext option
// that specifies a function that modifies the context passed to procedures per connection.
func WithConnContext(connContext ConnContextFunc) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.connContext = func(ctx context.Context, conn net.Conn) context.Context {
			ctx = WithConnInfo(ctx, conn)
			return connContext(ctx, conn)
		}
	}
}

// WithLog allows you to over-ride the location that exceptional server events are logged.
// The default is stderr.
func WithLog(log func(format string, args ...interface{})) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.log = log
	}
}

func defaultServerOptions() *ServerOptions {
	logger := log.New(os.Stderr, "", log.LstdFlags)
	return &ServerOptions{
		connContext: WithConnInfo,
		log:         logger.Printf,
	}
}

func simpleServerOptions(options ...func(*ServerOptions)) *ServerOptions {
	opts := defaultServerOptions()
	for _, option := range options {
		option(opts)
	}
	return opts
}
