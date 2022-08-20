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

type SSLServerSocket struct {
	listener      net.Listener
	addr          net.Addr
	clientTimeout time.Duration
	interrupted   bool
	cfg           *tls.Config
}

func NewSSLServerSocket(listenAddr string, cfg *tls.Config) (*SSLServerSocket, error) {
	return NewSSLServerSocketTimeout(listenAddr, cfg, 0)
}

func NewSSLServerSocketTimeout(listenAddr string, cfg *tls.Config, clientTimeout time.Duration) (*SSLServerSocket, error) {
	addr, err := net.ResolveTCPAddr("tcp", listenAddr)
	if err != nil {
		return nil, err
	}
	return &SSLServerSocket{addr: addr, clientTimeout: clientTimeout, cfg: cfg}, nil
}

func (p *SSLServerSocket) Listen() error {
	if p.IsListening() {
		return nil
	}
	l, err := tls.Listen(p.addr.Network(), p.addr.String(), p.cfg)
	if err != nil {
		return err
	}
	p.listener = l
	return nil
}

func (p *SSLServerSocket) Accept() (Transport, error) {
	if p.interrupted {
		return nil, errTransportInterrupted
	}
	if p.listener == nil {
		return nil, NewTransportException(NOT_OPEN, "No underlying server socket")
	}
	conn, err := p.listener.Accept()
	if err != nil {
		return nil, NewTransportExceptionFromError(err)
	}
	return NewSSLSocketFromConnTimeout(conn, p.cfg, p.clientTimeout), nil
}

// Checks whether the socket is listening.
func (p *SSLServerSocket) IsListening() bool {
	return p.listener != nil
}

// Connects the socket, creating a new socket object if necessary.
func (p *SSLServerSocket) Open() error {
	if p.IsListening() {
		return NewTransportException(ALREADY_OPEN, "Server socket already open")
	}
	if l, err := tls.Listen(p.addr.Network(), p.addr.String(), p.cfg); err != nil {
		return err
	} else {
		p.listener = l
	}
	return nil
}

func (p *SSLServerSocket) Addr() net.Addr {
	return p.addr
}

func (p *SSLServerSocket) Close() error {
	defer func() {
		p.listener = nil
	}()
	if p.IsListening() {
		return p.listener.Close()
	}
	return nil
}

func (p *SSLServerSocket) Interrupt() error {
	p.interrupted = true
	return nil
}
