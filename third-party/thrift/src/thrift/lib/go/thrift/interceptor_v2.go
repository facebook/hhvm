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
	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// InitParams carries server-level information handed to every ServiceInterceptor
// exactly once, at server startup, before any connections are accepted. It
// mirrors the C++ ServiceInterceptorBase::InitParams: it gives interceptors
// access to the server's Thrift schema/metadata so they can precompute
// per-method state (for example, a method-keyed lookup table derived from
// method annotations) instead of recomputing it on the request hot path.
type InitParams struct {
	// ServiceMetadata is the Thrift metadata for the server's processor,
	// enumerating services, their functions, and the functions' structured
	// annotations. It may be nil if the processor does not expose metadata
	// (e.g. a raw processor), in which case schema-dependent interceptors should
	// degrade gracefully rather than fail.
	ServiceMetadata *metadata.ThriftMetadata
}

// InterceptorResult holds the outgoing response passed to
// ServiceInterceptor.OnResponse. Response is the response struct being sent back
// to the client, or nil for oneway methods.
type InterceptorResult struct {
	Response types.WritableStruct
}

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
	// OnStartServing is called exactly once, when the server starts serving and
	// before any connections are accepted. It receives the server's InitParams
	// (including the Thrift service metadata) so the interceptor can precompute
	// any per-method or per-service state it needs on the request path.
	//
	// It is the Go analog of the C++ ServiceInterceptor co_onStartServing
	// callback. Returning a non-nil error aborts server startup, so use it only
	// for failures that should prevent the server from serving.
	OnStartServing(initParams InitParams) error

	// OnRequest is called when a new request arrives on an existing connection,
	// after the incoming request has been deserialized. userConnState is the
	// value returned by the corresponding OnConnectionEstablished call for this
	// connection.
	//
	// The returned context is threaded through the rest of the request lifecycle,
	// including the matching OnResponse call. Interceptors that need to carry
	// per-request state are responsible for storing it in the returned context
	// (for example via context.WithValue) and reading it back in OnResponse.
	// Returning the unchanged ctx is fine when no state needs to be carried.
	OnRequest(ctx context.Context, req types.ReadableStruct, userConnState any) (context.Context, error)

	// OnResponse is called when the response is about to be sent back, before the
	// outgoing response is serialized. The ctx is the one returned by the
	// request-path interceptors, so any state an interceptor stored in the
	// context during OnRequest can be read back here.
	OnResponse(ctx context.Context, result InterceptorResult) error

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

// OnStartServing implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnStartServing(initParams InitParams) error {
	return nil
}

// OnRequest implements ServiceInterceptor and is a no-op by default, returning
// the context unchanged.
func (si *BaseServiceInterceptor) OnRequest(ctx context.Context, req types.ReadableStruct, userConnState any) (context.Context, error) {
	return ctx, nil
}

// OnResponse implements ServiceInterceptor and is a no-op by default.
func (si *BaseServiceInterceptor) OnResponse(ctx context.Context, result InterceptorResult) error {
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
