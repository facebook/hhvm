/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
	"net"
)

type contextKey int

const (
	connInfoKey contextKey = iota
	protocolKey
)

// ConnInfo contains connection information from clients of the SimpleServer.
type ConnInfo struct {
	LocalAddr  net.Addr
	RemoteAddr net.Addr

	netConn  net.Conn             // set by thrift tcp servers
	tlsState *tls.ConnectionState // set by thrift http servers
}

// String implements the fmt.Stringer interface.
func (c ConnInfo) String() string {
	return c.RemoteAddr.String() + " -> " + c.LocalAddr.String()
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

// TLS returns the TLS connection state.
func (c ConnInfo) TLS() *tls.ConnectionState {
	if c.tlsState != nil {
		return c.tlsState
	}
	tlsConn, ok := c.netConn.(tlsConnectionStater)
	if !ok {
		return nil
	}
	cs := tlsConn.ConnectionState()
	// See the caveat in tlsConnectionStater.
	if cs.Version == 0 {
		return nil
	}
	return &cs
}

// WithConnInfo adds connection info (from a thrift.Transport) to context, if applicable
func WithConnInfo(ctx context.Context, client Transport) context.Context {
	s, ok := client.(*Socket)
	if !ok {
		return ctx
	}
	ctx = context.WithValue(ctx, connInfoKey, ConnInfo{
		LocalAddr:  s.Conn().LocalAddr(),
		RemoteAddr: s.Conn().RemoteAddr(),
		netConn:    s.Conn(),
	})
	return ctx
}

// ConnInfoFromContext extracts and returns ConnInfo from context.
func ConnInfoFromContext(ctx context.Context) (ConnInfo, bool) {
	v, ok := ctx.Value(connInfoKey).(ConnInfo)
	return v, ok
}

// The context can be augmented with the underlying HeaderProtocol. Thrift
// handlers can then query the context for the message headers. We store the
// protocol object on the context instead of the headers directly to avoid
// copying headers at each request and only lazy-copy them when the handler
// asks for them.
func headerProtocolFromContext(ctx context.Context) *HeaderProtocol {
	v, ok := ctx.Value(protocolKey).(*HeaderProtocol)
	if !ok {
		return nil
	}
	return v
}

// WithProtocol attaches thrift protocol to a context
func WithProtocol(ctx context.Context, proto Protocol) context.Context {
	return context.WithValue(ctx, protocolKey, proto)
}

// HeadersFromContext extracts headers for this message, both per-message
// and persistent headers. When both a per-message header and a persistent
// header exist with the same name, the persistent header is returned. This is
// also the behaviour of the C++ implementation.
// This function returns nil when the underlying transport/protocol do not
// support headers.
func HeadersFromContext(ctx context.Context) map[string]string {
	t := headerProtocolFromContext(ctx)
	if t == nil {
		// A nil map behaves like an empty map for reading.
		return nil
	}
	return t.ReadHeaders()
}
