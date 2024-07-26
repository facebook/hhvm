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

// Deprecated: use WrapInterceptorContext
type ServerOptions struct {
	interceptor Interceptor
}

// Deprecated: use WrapInterceptorContext
func WithInterceptor(interceptor Interceptor) func(*ServerOptions) {
	return func(server *ServerOptions) {
		server.interceptor = interceptor
	}
}

func defaultServerOptions() *ServerOptions {
	return &ServerOptions{}
}

func simpleServerOptions(options ...func(*ServerOptions)) *ServerOptions {
	opts := defaultServerOptions()
	for _, option := range options {
		option(opts)
	}
	return opts
}
