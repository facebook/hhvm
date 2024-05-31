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
	"net"
)

type contextKey int

const (
	connInfoKey contextKey = iota
	protocolKey
)

// ConnInfo contains connection information from clients of the SimpleServer.
type ConnInfo struct {
	RemoteAddr net.Addr
	tlsState   tlsConnectionStater // set by thrift tcp servers
}

// TLS returns the TLS connection state.
func (c ConnInfo) TLS() *tls.ConnectionState {
	cs := c.tlsState.ConnectionState()
	// See the caveat in tlsConnectionStater.
	if cs.Version == 0 {
		return nil
	}
	return &cs
}

// WithConnInfo adds connection info (from a thrift.Transport) to context, if applicable
func WithConnInfo(ctx context.Context, client Transport) context.Context {
	s, ok := client.(*socket)
	if !ok {
		return ctx
	}
	ctx = context.WithValue(ctx, connInfoKey, ConnInfo{
		RemoteAddr: s.RemoteAddr(),
		tlsState:   s,
	})
	return ctx
}

// ConnInfoFromContext extracts and returns ConnInfo from context.
func ConnInfoFromContext(ctx context.Context) (ConnInfo, bool) {
	v, ok := ctx.Value(connInfoKey).(ConnInfo)
	return v, ok
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
	t, ok := ctx.Value(protocolKey).(Protocol)
	if !ok {
		// A nil map behaves like an empty map for reading.
		return nil
	}
	return t.GetResponseHeaders()
}
