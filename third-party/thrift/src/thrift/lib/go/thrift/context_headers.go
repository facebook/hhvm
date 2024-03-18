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
	"fmt"
)

// The headersKeyType type is unexported to prevent collisions with context keys.
type headersKeyType int

const headersKey headersKeyType = 0

// AddHeader adds a header to the context, which will be sent as part of the request.
// AddHeader can be called multiple times to add multiple headers.
// These headers are not persistent and will only be sent with the current request.
func AddHeader(ctx context.Context, key string, value string) (context.Context, error) {
	headersMap := make(map[string]string)
	if headers := ctx.Value(headersKey); headers != nil {
		var ok bool
		headersMap, ok = headers.(map[string]string)
		if !ok {
			return nil, NewTransportException(INVALID_HEADERS_TYPE, "Headers key in context value is not map[string]string")
		}
	}
	headersMap[key] = value
	ctx = context.WithValue(ctx, headersKey, headersMap)
	return ctx, nil
}

// Deprecated: RequestHeaders will eventually be private.
type RequestHeaders interface {
	SetRequestHeader(key, value string)
	GetRequestHeader(key string) (value string, ok bool)
	GetRequestHeaders() map[string]string
}

// Compile time interface enforcer
var _ RequestHeaders = (*headerProtocol)(nil)
var _ RequestHeaders = (*rocketProtocol)(nil)
var _ RequestHeaders = (*upgradeToRocketProtocol)(nil)
var _ RequestHeaders = (*httpProtocol)(nil)

// setRequestHeaders sets the Headers in the protocol to send with the request.
// These headers will be written via the Write method, inside the Call method for each generated request.
// These Headers will be cleared with Flush, as they are not persistent.
func setRequestHeaders(ctx context.Context, protocol Protocol) error {
	if ctx == nil {
		return nil
	}
	headers := ctx.Value(headersKey)
	if headers == nil {
		return nil
	}
	headersMap, ok := headers.(map[string]string)
	if !ok {
		return NewTransportException(INVALID_HEADERS_TYPE, "Headers key in context value is not map[string]string")
	}
	p, ok := protocol.(RequestHeaders)
	if !ok {
		return NewTransportException(NOT_IMPLEMENTED, fmt.Sprintf("requestHeaders not implemented for transport type %T", protocol))
	}
	for k, v := range headersMap {
		p.SetRequestHeader(k, v)
	}
	return nil
}
