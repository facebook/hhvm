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
	"net"
	"time"
)

// Identity represents a secure peer identity
type Identity struct {
	Type string
	Data string
}

// RequestContext is a mirror of C++ apache::thrift::RequestContext
// Not all options are guaranteed to be implemented by a client
type RequestContext struct {
	RequestTimeout time.Duration
	Method         string
	SequenceID     int32
	InteractionID  int64
	Priority       Priority

	LocalAddress     net.Addr
	PeerAddress      net.Addr
	SecurityProtocol string
	PeerIdentities   []Identity
	PeerCommonName   string

	// For sending and receiving headers.
	contextHeaders
}

type requestContextKey int

const (
	reqContextKey requestContextKey = 2
)

// GetRequestContext returns the RequestContext in a go context, or nil if there is nothing
func GetRequestContext(ctx context.Context) *RequestContext {
	reqCtx, ok := ctx.Value(reqContextKey).(*RequestContext)
	if ok {
		return reqCtx
	}
	return nil
}

// WithRequestContext sets the RequestContext in a given go context
func WithRequestContext(ctx context.Context, reqCtx *RequestContext) context.Context {
	return context.WithValue(ctx, reqContextKey, reqCtx)
}

// GetRequestHeadersFromContext returns the request headers from the context.
func GetRequestHeadersFromContext(ctx context.Context) map[string]string {
	if ctx == nil {
		return nil
	}

	result := make(map[string]string)
	if reqCtx := GetRequestContext(ctx); reqCtx != nil {
		maps.Copy(result, reqCtx.GetReadHeaders())
	}

	return result
}
