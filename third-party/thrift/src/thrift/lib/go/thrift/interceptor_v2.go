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

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// ServiceInterceptor allows introspecting incoming connections, requests, and
// outgoing responses for a server. An interceptor hooks into the connection and
// request lifecycle through the callbacks below and can thread user-defined
// state across the paired callbacks (OnConnectionEstablished/OnConnectionClosed
// and OnRequest/OnResponse).
//
// User-defined interceptors MUST embed BaseServiceInterceptor. Embedding is the
// only way to satisfy this interface: BaseServiceInterceptor provides no-op
// defaults for every callback and supplies the unexported marker method, so the
// runtime will refuse to register any interceptor that does not embed it.
//
// The expected pattern is to embed BaseServiceInterceptor and override only the
// callbacks of interest. For example, an interceptor that only inspects requests
// overrides just OnRequest and inherits the no-op defaults for the rest, which
// avoids boilerplate.
type ServiceInterceptor interface {
	// OnRequest is called when a new request arrives on an existing connection,
	// after the incoming request has been deserialized. userConnState is the
	// value returned by the corresponding OnConnectionEstablished call for this
	// connection.
	//
	// The returned value is the user request state: it is persisted for the
	// duration of the request and passed unchanged into the matching OnResponse
	// call as userReqState. The runtime only guarantees this hand-off; the caller
	// is responsible for type asserting the value back to its concrete type.
	OnRequest(ctx context.Context, req types.ReadableStruct, userConnState any) (any /* user req state */, error)

	// OnResponse is called when the response is about to be sent back, before the
	// outgoing response is serialized. userReqState is the value returned by the
	// corresponding OnRequest call. The caller is responsible for type asserting
	// it back to its concrete type.
	OnResponse(ctx context.Context, resp types.WritableStruct, userReqState any) error

	// OnConnectionAttempted is called when a client makes a new connection
	// attempt, before the connection is fully established.
	OnConnectionAttempted(connInfo int64 /* placeholder type for now - unused */) error

	// OnConnectionEstablished is called when a new connection is established by
	// the server. The returned value is the user connection state: it is
	// persisted for the duration of the connection and passed unchanged into the
	// matching OnConnectionClosed call as userConnState. The caller is responsible
	// for type asserting it back to its concrete type.
	OnConnectionEstablished(connInfo int64 /* placeholder type for now - unused */) (any /* user conn state */, error)

	// OnConnectionClosed is called when an existing connection is closed by the
	// server. userConnState is the value returned by the corresponding
	// OnConnectionEstablished call.
	OnConnectionClosed(connInfo int64 /* placeholder type for now - unused */, userConnState any)

	// serviceInterceptorMarker is an unexported marker method implemented only by
	// BaseServiceInterceptor. It forces user-defined interceptors to embed
	// BaseServiceInterceptor in order to satisfy ServiceInterceptor.
	serviceInterceptorMarker()
}

// BaseServiceInterceptor is the required base for every user-defined
// ServiceInterceptor. Embed it to satisfy the ServiceInterceptor interface: it
// provides no-op defaults for all callbacks and the unexported marker method.
// Override only the callbacks you need.
type BaseServiceInterceptor struct{}

var _ ServiceInterceptor = (*BaseServiceInterceptor)(nil)

// OnRequest implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnRequest(ctx context.Context, req types.ReadableStruct, userConnState any) (any, error) {
	return nil, nil
}

// OnResponse implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnResponse(ctx context.Context, resp types.WritableStruct, userReqState any) error {
	return nil
}

// OnConnectionAttempted implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnConnectionAttempted(connInfo int64) error {
	return nil
}

// OnConnectionEstablished implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnConnectionEstablished(connInfo int64) (any, error) {
	return nil, nil
}

// OnConnectionClosed implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnConnectionClosed(connInfo int64, userConnState any) {
}

func (si *BaseServiceInterceptor) serviceInterceptorMarker() {}
