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
	"errors"
	"net"
	"time"
)

// Socket is a Transport that can be opened and reopened.
type Socket interface {
	net.Conn
	// Opens the socket for communication
	Open() error
}

var _ net.Conn = (*socket)(nil)

type socket struct {
	conn    net.Conn
	addr    net.Addr
	timeout time.Duration
}

// SocketOption is the type used to set options on the socket
type SocketOption func(*socket) error

// SocketTimeout sets the timeout
func SocketTimeout(timeout time.Duration) SocketOption {
	return func(socket *socket) error {
		socket.timeout = timeout
		return nil
	}
}

// SocketAddr sets the socket address
func SocketAddr(hostPort string) SocketOption {
	return func(socket *socket) error {
		addr, err := net.ResolveTCPAddr("tcp6", hostPort)
		if err != nil {
			addr, err = net.ResolveTCPAddr("tcp", hostPort)
			if err != nil {
				return err
			}
		}
		socket.addr = addr
		return nil
	}
}

// SocketConn sets the socket connection
func SocketConn(conn net.Conn) SocketOption {
	return func(socket *socket) error {
		socket.conn = conn
		socket.addr = conn.RemoteAddr()
		return nil
	}
}

// NewSocket creates a net.Conn-backed Transport, given a host and port,
// or an existing connection.
//
//	trans, err := thrift.NewSocket(thrift.SocketAddr("localhost:9090"))
func NewSocket(options ...SocketOption) (Socket, error) {
	socket := &socket{}

	for _, option := range options {
		err := option(socket)
		if err != nil {
			return nil, err
		}
	}

	if socket.addr.String() == "" && socket.conn.RemoteAddr().String() == "" && socket.addr.Network() != "unix" {
		return nil, errors.New("must supply either an address or a connection")
	}

	return socket, nil
}

// Sets the socket timeout
func (s *socket) SetTimeout(timeout time.Duration) error {
	s.timeout = timeout
	return nil
}

func (s *socket) pushDeadline(read, write bool) {
	var t time.Time
	if s.timeout > 0 {
		t = time.Now().Add(time.Duration(s.timeout))
	}
	if read && write {
		s.conn.SetDeadline(t)
	} else if read {
		s.conn.SetReadDeadline(t)
	} else if write {
		s.conn.SetWriteDeadline(t)
	}
}

// Open connects the socket to a server, creating a new socket object if necessary.
func (s *socket) Open() error {
	if s.conn != nil {
		return NewTransportException(ALREADY_OPEN, "Socket already connected.")
	}
	if s.addr == nil {
		return NewTransportException(NOT_OPEN, "Cannot open nil address.")
	}
	if len(s.addr.Network()) == 0 {
		return NewTransportException(NOT_OPEN, "Cannot open bad network name.")
	}
	if len(s.addr.String()) == 0 {
		return NewTransportException(NOT_OPEN, "Cannot open bad address.")
	}
	var err error
	if s.conn, err = net.DialTimeout(s.addr.Network(), s.addr.String(), s.timeout); err != nil {
		return NewTransportException(NOT_OPEN, err.Error())
	}
	return nil
}

// Addr returns the address the Socket is listening on.
func (s *socket) Addr() net.Addr {
	return s.addr
}

// LocalAddr returns the local network address, if known.
func (s *socket) LocalAddr() net.Addr {
	return s.conn.LocalAddr()
}

// RemoteAddr returns the remote network address, if known.
func (s *socket) RemoteAddr() net.Addr {
	return s.conn.RemoteAddr()
}

func (s *socket) SetDeadline(t time.Time) error {
	return s.conn.SetDeadline(t)
}

func (s *socket) SetReadDeadline(t time.Time) error {
	return s.conn.SetReadDeadline(t)
}

func (s *socket) SetWriteDeadline(t time.Time) error {
	return s.conn.SetWriteDeadline(t)
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

func (s *socket) ConnectionState() tls.ConnectionState {
	tlsConn, ok := s.conn.(tlsConnectionStater)
	if !ok {
		return tls.ConnectionState{}
	}
	return tlsConn.ConnectionState()
}

// Conn retrieves the underlying net.Conn
func (s *socket) Conn() net.Conn {
	return s.conn
}

// Close cleans up all resources used by the Socket.
func (s *socket) Close() error {
	// Close the socket
	if s.conn != nil {
		err := s.conn.Close()
		if err != nil {
			return err
		}
		s.conn = nil
	}
	return nil
}

func (s *socket) Read(buf []byte) (int, error) {
	if s.conn == nil {
		return 0, NewTransportException(NOT_OPEN, "connection not open")
	}
	s.pushDeadline(true, false)
	n, err := s.conn.Read(buf)
	return n, NewTransportExceptionFromError(err)
}

func (s *socket) Write(buf []byte) (int, error) {
	if s.conn == nil {
		return 0, NewTransportException(NOT_OPEN, "connection not open")
	}
	s.pushDeadline(false, true)
	return s.conn.Write(buf)
}
