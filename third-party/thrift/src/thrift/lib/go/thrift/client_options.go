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
	conn              net.Conn
	transport         TransportID
	protocol          types.ProtocolID
	ioTimeout         time.Duration // Read/Write timeout
	persistentHeaders map[string]string
}

// ClientOption is a single configuration setting for the thrift client
type ClientOption func(*clientOptions) error

// NoTimeout is a special value for WithTimeout that disables timeouts.
const NoTimeout = time.Duration(0)

// WithProtocolID sets protocol to given protocolID
func WithProtocolID(id types.ProtocolID) ClientOption {
	return func(opts *clientOptions) error {
		opts.protocol = id
		return nil
	}
}

// WithHeader sets the transport to Header, protocol Header is implied here.
func WithHeader() ClientOption {
	return func(opts *clientOptions) error {
		opts.transport = TransportIDHeader
		return nil
	}
}

// WithUpgradeToRocket sets the protocol UpgradeToRocket is implied here.
func WithUpgradeToRocket() ClientOption {
	return func(opts *clientOptions) error {
		opts.transport = TransportIDUpgradeToRocket
		return nil
	}
}

// Deprecated: Use WithUpgradeToRocket. This is only used for testing purposes.
func WithRocket() ClientOption {
	return func(opts *clientOptions) error {
		opts.transport = TransportIDRocket
		return nil
	}
}

// WithPersistentHeader sets a Header persistent info value
func WithPersistentHeader(name, value string) ClientOption {
	return func(opts *clientOptions) error {
		opts.persistentHeaders[name] = value
		return nil
	}
}

// WithIdentity sets the Header identity field
func WithIdentity(name string) ClientOption {
	return func(opts *clientOptions) error {
		opts.persistentHeaders[IdentityHeader] = name
		return nil
	}
}

// WithDialer specifies the remote connection that the thrift
// client should connect to should be resolved via the given function
func WithDialer(d func() (net.Conn, error)) ClientOption {
	return func(opts *clientOptions) error {
		conn, err := d()
		if err != nil {
			return err
		}
		opts.conn = conn
		return nil
	}
}

// WithTimeout sets deadline duration for I/O operations
// (see https://golang.org/pkg/net/#Conn). Note that
// this timeout is not a connection timeout as it is
// not honored during Dial operation.
func WithTimeout(ioTimeout time.Duration) ClientOption {
	return func(opts *clientOptions) error {
		opts.ioTimeout = ioTimeout
		return nil
	}
}

// WithConn sets the connection to use for the thrift client.
func WithConn(conn net.Conn) ClientOption {
	return func(opts *clientOptions) error {
		opts.conn = conn
		return nil
	}
}

// WithTLS is a creates a TLS connection to the given address, including ALPN for thrift.
func WithTLS(addr string, dialTimeout time.Duration, tlsConfig *tls.Config) ClientOption {
	return func(opts *clientOptions) error {
		conn, err := DialTLS(addr, dialTimeout, tlsConfig)
		if err != nil {
			return err
		}
		opts.conn = conn
		return nil
	}
}

// DialTLS dials and returns a TLS connection to the given address, including ALPN for thrift.
func DialTLS(addr string, dialTimeout time.Duration, tlsConfig *tls.Config) (net.Conn, error) {
	dialer := net.Dialer{Timeout: dialTimeout}
	conn, err := dialer.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}
	config := tlsConfig.Clone()
	AddALPNForTransport(config)
	return tls.Client(conn, config), nil
}

func newDefaultPersistentHeaders() map[string]string {
	// set Identity Headers
	return map[string]string{
		IDVersionHeader: IDVersion,
		IdentityHeader:  "",
	}
}

// newOptions creates a new options objects and inits it
func newOptions(opts ...ClientOption) (*clientOptions, error) {
	res := &clientOptions{
		protocol:          types.ProtocolIDCompact,
		transport:         TransportIDUnknown,
		ioTimeout:         NoTimeout,
		persistentHeaders: newDefaultPersistentHeaders(),
	}
	for _, opt := range opts {
		if err := opt(res); err != nil {
			return nil, err
		}
	}
	if res.transport == TransportIDUnknown {
		panic(NewTransportException(types.NOT_SUPPORTED, "no transport specified! Please use thrift.WithHeader() or thrift.WithUpgradeToRocket() in the thrift.NewClient call"))
	}
	return res, nil
}

// NewClient will return a connected thrift protocol object.
// Effectively, this is an open thrift connection to a server.
// A thrift client can use this connection to communicate with a server.
func NewClient(opts ...ClientOption) (types.Protocol, error) {
	options, err := newOptions(opts...)
	if err != nil {
		return nil, err
	}
	switch options.transport {
	case TransportIDHeader:
		return newHeaderProtocol(options.conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	case TransportIDRocket:
		return newRocketClient(options.conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	case TransportIDUpgradeToRocket:
		return newUpgradeToRocketClient(options.conn, options.protocol, options.ioTimeout, options.persistentHeaders)
	default:
		panic("framed and unframed transport are not supported")
	}
}
