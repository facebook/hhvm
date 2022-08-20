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
	"errors"
	"net"
	"time"
)

type Socket struct {
	conn    net.Conn
	addr    net.Addr
	timeout time.Duration
}

// SocketOption is the type used to set options on the socket
type SocketOption func(*Socket) error

// SocketTimeout sets the timeout
func SocketTimeout(timeout time.Duration) SocketOption {
	return func(socket *Socket) error {
		socket.timeout = timeout
		return nil
	}
}

// SocketAddr sets the socket address
func SocketAddr(hostPort string) SocketOption {
	return func(socket *Socket) error {
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
	return func(socket *Socket) error {
		socket.conn = conn
		socket.addr = conn.RemoteAddr()
		return nil
	}
}

// NewSocket creates a net.Conn-backed Transport, given a host and port,
// or an existing connection.
// 	trans, err := thrift.NewSocket(thrift.SocketAddr("localhost:9090"))
func NewSocket(options ...SocketOption) (*Socket, error) {
	socket := &Socket{}

	for _, option := range options {
		err := option(socket)
		if err != nil {
			return nil, err
		}
	}

	if socket.addr.String() == "" && socket.conn.RemoteAddr().String() == "" {
		return nil, errors.New("must supply either an address or a connection")
	}

	return socket, nil
}

// Sets the socket timeout
func (p *Socket) SetTimeout(timeout time.Duration) error {
	p.timeout = timeout
	return nil
}

func (p *Socket) pushDeadline(read, write bool) {
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
func (p *Socket) Open() error {
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
	var err error
	if p.conn, err = net.DialTimeout(p.addr.Network(), p.addr.String(), p.timeout); err != nil {
		return NewTransportException(NOT_OPEN, err.Error())
	}
	return nil
}

// Retrieve the underlying net.Conn
func (p *Socket) Conn() net.Conn {
	return p.conn
}

// Returns true if the connection is open
func (p *Socket) IsOpen() bool {
	if p.conn == nil {
		return false
	}
	return true
}

// Closes the socket.
func (p *Socket) Close() error {
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

//Returns the remote address of the socket.
func (p *Socket) Addr() net.Addr {
	return p.addr
}

func (p *Socket) Read(buf []byte) (int, error) {
	if !p.IsOpen() {
		return 0, NewTransportException(NOT_OPEN, "Connection not open")
	}
	p.pushDeadline(true, false)
	n, err := p.conn.Read(buf)
	return n, NewTransportExceptionFromError(err)
}

func (p *Socket) Write(buf []byte) (int, error) {
	if !p.IsOpen() {
		return 0, NewTransportException(NOT_OPEN, "Connection not open")
	}
	p.pushDeadline(false, true)
	return p.conn.Write(buf)
}

func (p *Socket) Flush() error {
	return nil
}

func (p *Socket) Interrupt() error {
	if !p.IsOpen() {
		return nil
	}
	return p.conn.Close()
}

func (p *Socket) RemainingBytes() (num_bytes uint64) {
	return UnknownRemaining // the truth is, we just don't know unless framed is used
}
