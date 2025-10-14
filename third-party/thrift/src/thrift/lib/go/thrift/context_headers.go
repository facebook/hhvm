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
	"maps"
)

// The headersKeyType type is unexported to prevent collisions with context keys.
type headersKeyType int

const (
	// requestHeadersKey is a context key.
	requestHeadersKey headersKeyType = 0
)

// WithRequestHeader adds a header to the context, which will be sent as part of the request.
// WithRequestHeader can be called multiple times to add multiple headers.
// These headers are not persistent and will only be sent with the current request.
func WithRequestHeader(ctx context.Context, key string, value string) context.Context {
	return WithRequestHeaders(ctx, map[string]string{key: value})
}

// WithRequestHeaders attaches thrift headers to a ctx.
func WithRequestHeaders(ctx context.Context, headers map[string]string) context.Context {
	storedHeaders := ctx.Value(requestHeadersKey)
	if storedHeaders == nil {
		return context.WithValue(ctx, requestHeadersKey, headers)
	}
	headersMap := storedHeaders.(map[string]string)
	maps.Copy(headersMap, headers)
	return context.WithValue(ctx, requestHeadersKey, headersMap)
}

// SetHeaders replaces all the current headers.
func SetHeaders(ctx context.Context, headers map[string]string) context.Context {
	return context.WithValue(ctx, requestHeadersKey, headers)
}

// GetRequestHeadersFromContext returns the request headers from the context.
func GetRequestHeadersFromContext(ctx context.Context) map[string]string {
	if ctx == nil {
		return nil
	}
	requestHeaders := ctx.Value(requestHeadersKey)
	if requestHeaders == nil {
		return nil
	}
	return requestHeaders.(map[string]string)
}
