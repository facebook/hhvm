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
	"net"
)

type socket struct {
	conn net.Conn
}

// SocketOption is the type used to set options on the socket
type SocketOption func(*socket) error

func resolveAddr(hostPort string) (net.Addr, error) {
	addr, err := net.ResolveTCPAddr("tcp6", hostPort)
	if err != nil {
		return net.ResolveTCPAddr("tcp", hostPort)
	}
	return addr, nil
}

// DialHostPort creates a net.Conn, given a host and port combination, for example "127.0.0.1:8080".
func DialHostPort(hostPort string) (net.Conn, error) {
	addr, err := resolveAddr(hostPort)
	if err != nil {
		return nil, err
	}
	if len(addr.Network()) == 0 {
		return nil, NewTransportException(NOT_OPEN, "Cannot open bad network name.")
	}
	if len(addr.String()) == 0 {
		return nil, NewTransportException(NOT_OPEN, "Cannot open bad address.")
	}
	conn, err := net.Dial(addr.Network(), addr.String())
	if err != nil {
		return nil, NewTransportException(NOT_OPEN, err.Error())
	}
	return conn, nil
}

// SocketConn sets the socket connection
func SocketConn(conn net.Conn) SocketOption {
	return func(socket *socket) error {
		socket.conn = conn
		return nil
	}
}

// NewSocket creates a net.Conn, given a host and port or an existing connection.
//
//	conn, err := thrift.NewSocket(thrift.SocketAddr("localhost:9090"))
func NewSocket(options ...SocketOption) (net.Conn, error) {
	socket := &socket{}
	for _, option := range options {
		err := option(socket)
		if err != nil {
			return nil, err
		}
	}
	return socket.conn, nil
}
