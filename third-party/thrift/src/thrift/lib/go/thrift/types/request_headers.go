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

package types

import (
	"context"
)

// The headersKeyType type is unexported to prevent collisions with context keys.
type headersKeyType int

// RequestHeadersKey is a context key.
const RequestHeadersKey headersKeyType = 0

// SetRequestHeaders sets the Headers in the protocol to send with the request.
// These headers will be written via the Write method, inside the Call method for each generated request.
// These Headers will be cleared with Flush, as they are not persistent.
func SetRequestHeaders(ctx context.Context, protocol Protocol) error {
	if ctx == nil {
		return nil
	}
	headers := ctx.Value(RequestHeadersKey)
	if headers == nil {
		return nil
	}
	headersMap, ok := headers.(map[string]string)
	if !ok {
		return NewTransportException(INVALID_HEADERS_TYPE, "Headers key in context value is not map[string]string")
	}
	for k, v := range headersMap {
		protocol.SetRequestHeader(k, v)
	}
	return nil
}
