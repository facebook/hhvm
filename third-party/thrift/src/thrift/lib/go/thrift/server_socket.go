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

type ServerSocket struct {
	listener net.Listener
	addr     net.Addr
}

func NewServerSocket(listenAddr string) (*ServerSocket, error) {
	addr, err := net.ResolveTCPAddr("tcp", listenAddr)
	if err != nil {
		return nil, err
	}
	l, err := net.Listen(addr.Network(), addr.String())
	if err != nil {
		return nil, err
	}
	return &ServerSocket{addr: addr, listener: l}, nil
}

func (p *ServerSocket) Listen() error {
	return nil
}

func (p *ServerSocket) Accept() (net.Conn, error) {
	return p.listener.Accept()
}

func (p *ServerSocket) Addr() net.Addr {
	return p.listener.Addr()
}

func (p *ServerSocket) Close() error {
	return p.listener.Close()
}
