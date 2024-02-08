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
	"log"
	"os"
)

// ServerOptions is options needed to run a thrift server
type ServerOptions struct {
	quit        chan struct{}
	log         *log.Logger
	interceptor Interceptor
}

// Logger sets the logger used for the server
func Logger(log *log.Logger) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.log = log
	}
}

// WithInterceptor sets the interceptor for the server
func WithInterceptor(interceptor Interceptor) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.interceptor = interceptor
	}
}

func defaultServerOptions() *ServerOptions {
	return &ServerOptions{
		quit: make(chan struct{}, 1),
		log:  log.New(os.Stderr, "", log.LstdFlags),
	}
}
