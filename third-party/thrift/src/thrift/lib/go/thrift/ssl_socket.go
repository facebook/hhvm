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
	"crypto/tls"
	"net"
	"time"
)

type SSLSocket struct {
	conn net.Conn
	// hostPort contains host:port (e.g. "asdf.com:12345"). The field is
	// only valid if addr is nil.
	hostPort string
	// addr is nil when hostPort is not "", and is only used when the
	// SSLSocket is constructed from a net.Addr.
	addr    net.Addr
	timeout time.Duration
	cfg     *tls.Config
}

// NewSSLSocket creates a net.Conn-backed Transport, given a host and port and tls Configuration
//
// Example:
// 	trans, err := thrift.NewSSLSocket("localhost:9090", nil)
func NewSSLSocket(hostPort string, cfg *tls.Config) (*SSLSocket, error) {
	return NewSSLSocketTimeout(hostPort, cfg, 0)
}

// NewSSLSocketTimeout creates a net.Conn-backed Transport, given a host and port
// it also accepts a tls Configuration and a timeout as a time.Duration
func NewSSLSocketTimeout(hostPort string, cfg *tls.Config, timeout time.Duration) (*SSLSocket, error) {
	return &SSLSocket{hostPort: hostPort, timeout: timeout, cfg: cfg}, nil
}

// Creates a SSLSocket from a net.Addr
func NewSSLSocketFromAddrTimeout(addr net.Addr, cfg *tls.Config, timeout time.Duration) *SSLSocket {
	return &SSLSocket{addr: addr, timeout: timeout, cfg: cfg}
}

// Creates a SSLSocket from an existing net.Conn
func NewSSLSocketFromConnTimeout(conn net.Conn, cfg *tls.Config, timeout time.Duration) *SSLSocket {
	return &SSLSocket{conn: conn, addr: conn.RemoteAddr(), timeout: timeout, cfg: cfg}
}

// Sets the socket timeout
func (p *SSLSocket) SetTimeout(timeout time.Duration) error {
	p.timeout = timeout
	return nil
}

func (p *SSLSocket) pushDeadline(read, write bool) {
	var t time.Time
	if p.timeout > 0 {
		t = time.Now().Add(time.Duration(p.timeout))
	}
	if read && write {
		p.conn.SetDeadline(t)
	} else if read {
		p.conn.SetReadDeadline(t)
	} else if write {
		p.conn.SetWriteDeadline(t)
	}
}

// Connects the socket, creating a new socket object if necessary.
func (p *SSLSocket) Open() error {
	var err error
	// If we have a hostname, we need to pass the hostname to tls.Dial for
	// certificate hostname checks.
	if p.hostPort != "" {
		if p.conn, err = tls.Dial("tcp", p.hostPort, p.cfg); err != nil {
			return NewTransportException(NOT_OPEN, err.Error())
		}
	} else {
		if p.IsOpen() {
			return NewTransportException(ALREADY_OPEN, "Socket already connected.")
		}
		if p.addr == nil {
			return NewTransportException(NOT_OPEN, "Cannot open nil address.")
		}
		if len(p.addr.Network()) == 0 {
			return NewTransportException(NOT_OPEN, "Cannot open bad network name.")
		}
		if len(p.addr.String()) == 0 {
			return NewTransportException(NOT_OPEN, "Cannot open bad address.")
		}
		if p.conn, err = tls.Dial(p.addr.Network(), p.addr.String(), p.cfg); err != nil {
			return NewTransportException(NOT_OPEN, err.Error())
		}
	}
	return nil
}

// Retrieve the underlying net.Conn
func (p *SSLSocket) Conn() net.Conn {
	return p.conn
}

// Returns true if the connection is open
func (p *SSLSocket) IsOpen() bool {
	if p.conn == nil {
		return false
	}
	return true
}

// Closes the socket.
func (p *SSLSocket) Close() error {
	// Close the socket
	if p.conn != nil {
		err := p.conn.Close()
		if err != nil {
			return err
		}
		p.conn = nil
	}
	return nil
}

func (p *SSLSocket) Read(buf []byte) (int, error) {
	if !p.IsOpen() {
		return 0, NewTransportException(NOT_OPEN, "Connection not open")
	}
	p.pushDeadline(true, false)
	n, err := p.conn.Read(buf)
	return n, NewTransportExceptionFromError(err)
}

func (p *SSLSocket) Write(buf []byte) (int, error) {
	if !p.IsOpen() {
		return 0, NewTransportException(NOT_OPEN, "Connection not open")
	}
	p.pushDeadline(false, true)
	return p.conn.Write(buf)
}

func (p *SSLSocket) Flush() error {
	return nil
}

func (p *SSLSocket) Interrupt() error {
	if !p.IsOpen() {
		return nil
	}
	return p.conn.Close()
}

func (p *SSLSocket) RemainingBytes() (num_bytes uint64) {
	return UnknownRemaining // the truth is, we just don't know unless framed is used
}
