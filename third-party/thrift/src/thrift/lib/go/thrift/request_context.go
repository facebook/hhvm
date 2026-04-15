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

// ClientIdentityHook is a function that extracts peer identities from a TLS connection.
// This mirrors the C++ ClientIdentityHook type in Cpp2ConnContext.h.
// The hook is called once per connection during connection setup.
// The return value is stored as-is on ConnInfo.PeerIdentities (type-erased via any).
// Consumers (e.g. aclchecker) are responsible for type-asserting the result.
type ClientIdentityHook func(tlsState *tls.ConnectionState, peerAddr net.Addr) any

// ConnInfo contains connection information from clients of the Server.
type ConnInfo struct {
	LocalAddr        net.Addr
	RemoteAddr       net.Addr
	SecurityProtocol string
	PeerCommonName   string
	PeerIdentities   any
	tlsState         tlsConnectionStater // set by thrift tcp servers
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
	ServiceName    string
	MethodName     string
	SequenceID     int32
	InteractionID  int64
	Priority       Priority

	PeerIdentities []Identity

	ConnInfo ConnInfo

	// For sending and receiving headers.
	contextHeaders
}

// withConnInfo adds connection info (from a thrift.Transport) to context, if applicable.
// If a ClientIdentityHook is provided, it is called to extract peer identities,
// mirroring the C++ Cpp2ConnContext constructor behavior.
func withConnInfo(ctx context.Context, conn net.Conn, hook ClientIdentityHook) context.Context {
	var tlsState tlsConnectionStater
	if t, ok := conn.(tlsConnectionStater); ok {
		tlsState = t
	}
	securityProtocol := ""
	peerCommonName := ""
	if tlsState != nil {
		securityProtocol = "TLS"
		cs := tlsState.ConnectionState()
		if len(cs.PeerCertificates) > 0 {
			peerCommonName = cs.PeerCertificates[0].Subject.CommonName
		}
	}
	connInfo := ConnInfo{
		LocalAddr:        conn.LocalAddr(),
		RemoteAddr:       conn.RemoteAddr(),
		SecurityProtocol: securityProtocol,
		PeerCommonName:   peerCommonName,
		tlsState:         tlsState,
	}
	if hook != nil {
		connInfo.PeerIdentities = hook(connInfo.TLS(), connInfo.RemoteAddr)
	}
	return context.WithValue(ctx, connInfoKey, connInfo)
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
