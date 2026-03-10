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
	"crypto/tls"
	"maps"
	"net"
	"time"
)

type contextKey int

const (
	connInfoKey   contextKey = 0
	reqContextKey contextKey = 2
)

// Identity represents a secure peer identity
type Identity struct {
	Type string
	Data string
}

// ConnInfo contains connection information from clients of the Server.
type ConnInfo struct {
	RemoteAddr net.Addr
	tlsState   tlsConnectionStater // set by thrift tcp servers
}

// tlsConnectionStater is an abstract interface for types that can return
// the state of TLS connections. This is used to support not only tls.Conn
// but also custom wrappers such as permissive TLS/non-TLS sockets.
//
// Caveat: this interface has to support at least tls.Conn, which has
// the current signature for ConnectionState. Because of that, wrappers
// for permissive TLS/non-TLS may return an empty tls.ConnectionState.
type tlsConnectionStater interface {
	ConnectionState() tls.ConnectionState
}

// tlsConnectionStaterHandshaker is an abstract interface that allows
// custom "TLS-like" connections to be used with Thrift ALPN logic.
type tlsConnectionStaterHandshaker interface {
	tlsConnectionStater
	HandshakeContext(context.Context) error
}

// Compile time interface enforcer
var _ tlsConnectionStater = (*tls.Conn)(nil)
var _ tlsConnectionStaterHandshaker = (*tls.Conn)(nil)

// TLS returns the TLS connection state.
func (c ConnInfo) TLS() *tls.ConnectionState {
	if c.tlsState == nil {
		return nil
	}
	cs := c.tlsState.ConnectionState()
	// See the caveat in tlsConnectionStater.
	if cs.Version == 0 {
		return nil
	}
	return &cs
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

	ConnInfo ConnInfo

	// For sending and receiving headers.
	contextHeaders
}

// withConnInfo adds connection info (from a thrift.Transport) to context, if applicable
func withConnInfo(ctx context.Context, conn net.Conn) context.Context {
	var tlsState tlsConnectionStater
	if t, ok := conn.(tlsConnectionStater); ok {
		tlsState = t
	}
	ctx = context.WithValue(ctx, connInfoKey, ConnInfo{
		RemoteAddr: conn.RemoteAddr(),
		tlsState:   tlsState,
	})
	return ctx
}

// connInfoFromContext extracts and returns ConnInfo from context.
func connInfoFromContext(ctx context.Context) (ConnInfo, bool) {
	v, ok := ctx.Value(connInfoKey).(ConnInfo)
	return v, ok
}

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
