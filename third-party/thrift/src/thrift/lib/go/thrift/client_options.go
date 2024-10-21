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
	"crypto/tls"
	"net"
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// clientOptions thrift and connectivity options for the thrift client
type clientOptions struct {
	transport         TransportID
	protocol          types.ProtocolID
	ioTimeout         time.Duration // Read/Write timeout
	persistentHeaders map[string]string
	dialerFn          func() (net.Conn, error)
	tlsConfig         *tls.Config
}

// ClientOption is a single configuration setting for the thrift client
type ClientOption func(*clientOptions)

// NoTimeout is a special value for WithTimeout that disables timeouts.
const NoTimeout = time.Duration(0)

// WithProtocolID sets protocol to given protocolID
func WithProtocolID(id types.ProtocolID) ClientOption {
	return func(opts *clientOptions) {
		opts.protocol = id
	}
}

// WithHeader sets the transport to Header, protocol Header is implied here.
func WithHeader() ClientOption {
	return func(opts *clientOptions) {
		opts.transport = TransportIDHeader
	}
}

// WithUpgradeToRocket sets the protocol UpgradeToRocket is implied here.
func WithUpgradeToRocket() ClientOption {
	return func(opts *clientOptions) {
		opts.transport = TransportIDUpgradeToRocket
	}
}

// Deprecated: Use WithUpgradeToRocket. This is only used for testing purposes.
func WithRocket() ClientOption {
	return func(opts *clientOptions) {
		opts.transport = TransportIDRocket
	}
}

// WithPersistentHeader sets a Header persistent info value
func WithPersistentHeader(name, value string) ClientOption {
	return func(opts *clientOptions) {
		opts.persistentHeaders[name] = value
	}
}

// WithIdentity sets the Header identity field
func WithIdentity(name string) ClientOption {
	return func(opts *clientOptions) {
		opts.persistentHeaders[IdentityHeader] = name
	}
}

// WithDialer specifies the remote connection that the thrift
// client should connect to should be resolved via the given function
func WithDialer(d func() (net.Conn, error)) ClientOption {
	return func(opts *clientOptions) {
		opts.dialerFn = d
	}
}

// WithTimeout sets deadline duration for I/O operations
// (see https://golang.org/pkg/net/#Conn). Note that
// this timeout is not a connection timeout as it is
// not honored during Dial operation.
func WithTimeout(ioTimeout time.Duration) ClientOption {
	return func(opts *clientOptions) {
		opts.ioTimeout = ioTimeout
	}
}

// WithConn sets the connection to use for the thrift client.
func WithConn(conn net.Conn) ClientOption {
	return WithDialer(func() (net.Conn, error) {
		return conn, nil
	})
}

// WithTLS is a creates a TLS connection to the given address, including ALPN for thrift.
func WithTLS(addr string, dialTimeout time.Duration, tlsConfig *tls.Config) ClientOption {
	clonedTLSConfig := tlsConfig.Clone()
	return func(opts *clientOptions) {
		opts.tlsConfig = clonedTLSConfig
		opts.dialerFn = func() (net.Conn, error) {
			return DialTLS(addr, dialTimeout, opts.tlsConfig)
		}
	}
}

// DialTLS dials and returns a TLS connection to the given address, including ALPN for thrift.
func DialTLS(addr string, dialTimeout time.Duration, tlsConfig *tls.Config) (net.Conn, error) {
	clonedTLSConfig := tlsConfig.Clone()
	AddALPNForTransport(clonedTLSConfig)
	return dialWithTLS(addr, dialTimeout, clonedTLSConfig)
}

// This is an internal helper to dial with TLS.
func dialWithTLS(addr string, dialTimeout time.Duration, tlsConfig *tls.Config) (net.Conn, error) {
	conn, err := net.DialTimeout("tcp", addr, dialTimeout)
	if err != nil {
		return nil, err
	}
	return tls.Client(conn, tlsConfig), nil
}

func newDefaultPersistentHeaders() map[string]string {
	// set Identity Headers
	return map[string]string{
		IDVersionHeader: IDVersion,
		IdentityHeader:  "",
	}
}

// newOptions creates a new options objects and inits it
func newOptions(opts ...ClientOption) *clientOptions {
	res := &clientOptions{
		protocol:          types.ProtocolIDCompact,
		transport:         TransportIDUnknown,
		ioTimeout:         NoTimeout,
		persistentHeaders: newDefaultPersistentHeaders(),
	}
	for _, opt := range opts {
		opt(res)
	}

	if res.transport == TransportIDUnknown {
		panic(NewTransportException(types.NOT_SUPPORTED, "no transport specified! Please use thrift.WithHeader() or thrift.WithUpgradeToRocket() in the thrift.NewClient call"))
	}
	return res
}

// NewClient will return a connected thrift protocol object.
// Effectively, this is an open thrift connection to a server.
// A thrift client can use this connection to communicate with a server.
func NewClient(opts ...ClientOption) (types.Protocol, error) {
	options := newOptions(opts...)

	var conn net.Conn
	var err error
	if options.dialerFn != nil {
		conn, err = options.dialerFn()
		if err != nil {
			return nil, err
		}
	}

	switch options.transport {
	case TransportIDHeader:
		return newHeaderProtocol(conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	case TransportIDRocket:
		return newRocketClient(conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	case TransportIDUpgradeToRocket:
		return newUpgradeToRocketClient(conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	default:
		panic("framed and unframed transport are not supported")
	}
}
