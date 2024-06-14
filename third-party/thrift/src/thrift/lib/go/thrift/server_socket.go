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
	"time"
)

type ServerSocket struct {
	listener      net.Listener
	addr          net.Addr
	clientTimeout time.Duration
}

func NewServerSocket(listenAddr string) (*ServerSocket, error) {
	return NewServerSocketTimeout(listenAddr, 0)
}

func NewServerSocketTimeout(listenAddr string, clientTimeout time.Duration) (*ServerSocket, error) {
	addr, err := net.ResolveTCPAddr("tcp", listenAddr)
	if err != nil {
		return nil, err
	}
	return &ServerSocket{addr: addr, clientTimeout: clientTimeout}, nil
}

func (p *ServerSocket) Listen() error {
	if p.listener != nil {
		return nil
	}
	l, err := net.Listen(p.addr.Network(), p.addr.String())
	if err != nil {
		return err
	}
	p.listener = l
	return nil
}

func (p *ServerSocket) Accept() (net.Conn, error) {
	if p.listener == nil {
		return nil, NewTransportException(NOT_OPEN, "No underlying server socket")
	}
	conn, err := p.listener.Accept()
	if err != nil {
		return nil, NewTransportExceptionFromError(err)
	}
	return NewSocket(SocketConn(conn), SocketTimeout(p.clientTimeout))
}

func (p *ServerSocket) Addr() net.Addr {
	if p.listener != nil {
		return p.listener.Addr()
	}
	return p.addr
}

func (p *ServerSocket) Close() error {
	defer func() {
		p.listener = nil
	}()
	if p.listener != nil {
		return p.listener.Close()
	}
	return nil
}
