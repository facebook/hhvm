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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// The headersKeyType type is unexported to prevent collisions with context keys.
type headersKeyType int

const (
	// requestHeadersKey is a context key.
	requestHeadersKey headersKeyType = 0
	// responseHeadersKey is a context key.
	responseHeadersKey headersKeyType = 1
)

// AddHeader adds a header to the context, which will be sent as part of the request.
// AddHeader can be called multiple times to add multiple headers.
// These headers are not persistent and will only be sent with the current request.
func AddHeader(ctx context.Context, key string, value string) (context.Context, error) {
	headersMap := make(map[string]string)
	if headers := ctx.Value(requestHeadersKey); headers != nil {
		var ok bool
		headersMap, ok = headers.(map[string]string)
		if !ok {
			return nil, types.NewTransportException(types.INVALID_HEADERS_TYPE, "Headers key in context value is not map[string]string")
		}
	}
	headersMap[key] = value
	ctx = context.WithValue(ctx, requestHeadersKey, headersMap)
	return ctx, nil
}

// WithHeaders attaches thrift headers to a ctx.
func WithHeaders(ctx context.Context, headers map[string]string) context.Context {
	storedHeaders := ctx.Value(requestHeadersKey)
	if storedHeaders == nil {
		return context.WithValue(ctx, requestHeadersKey, headers)
	}
	headersMap, ok := storedHeaders.(map[string]string)
	if !ok {
		return context.WithValue(ctx, requestHeadersKey, headers)
	}
	maps.Copy(headersMap, headers)
	return context.WithValue(ctx, requestHeadersKey, headersMap)
}

// SetHeaders replaces all the current headers.
func SetHeaders(ctx context.Context, headers map[string]string) context.Context {
	return context.WithValue(ctx, requestHeadersKey, headers)
}

// GetHeaders gets thrift headers from ctx.
func GetHeaders(ctx context.Context) map[string]string {
	// check for headersKey
	v, ok := ctx.Value(requestHeadersKey).(map[string]string)
	if ok {
		return v
	}
	return nil
}

// NewResponseHeadersContext returns a new context with the response headers value.
func NewResponseHeadersContext(ctx context.Context) context.Context {
	return context.WithValue(ctx, responseHeadersKey, make(map[string]string))
}

// ResponseHeadersFromContext returns the response headers from the context.
func ResponseHeadersFromContext(ctx context.Context) map[string]string {
	if ctx == nil {
		return nil
	}
	responseHeaders := ctx.Value(responseHeadersKey)
	if responseHeaders == nil {
		return nil
	}
	responseHeadersMap, ok := responseHeaders.(map[string]string)
	if !ok {
		return nil
	}
	return responseHeadersMap
}

// RequestHeadersFromContext returns the request headers from the context.
func RequestHeadersFromContext(ctx context.Context) map[string]string {
	if ctx == nil {
		return nil
	}
	requestHeaders := ctx.Value(requestHeadersKey)
	if requestHeaders == nil {
		return nil
	}
	requestHeadersMap, ok := requestHeaders.(map[string]string)
	if !ok {
		return nil
	}
	return requestHeadersMap
}

// SetRequestHeaders sets the Headers in the protocol to send with the request.
// These headers will be written via the Write method, inside the Call method for each generated request.
// These Headers will be cleared with Flush, as they are not persistent.
func SetRequestHeaders(ctx context.Context, protocol Protocol) error {
	if ctx == nil {
		return nil
	}
	headers := ctx.Value(requestHeadersKey)
	if headers == nil {
		return nil
	}
	headersMap, ok := headers.(map[string]string)
	if !ok {
		return NewTransportException(types.INVALID_HEADERS_TYPE, "Headers key in context value is not map[string]string")
	}
	for k, v := range headersMap {
		protocol.setRequestHeader(k, v)
	}
	return nil
}

func setResponseHeaders(ctx context.Context, responseHeaders map[string]string) {
	if ctx == nil {
		return
	}
	responseHeadersMapIf := ctx.Value(responseHeadersKey)
	if responseHeadersMapIf == nil {
		return
	}
	responseHeadersMap, ok := responseHeadersMapIf.(map[string]string)
	if !ok {
		return
	}

	maps.Copy(responseHeadersMap, responseHeaders)
}
