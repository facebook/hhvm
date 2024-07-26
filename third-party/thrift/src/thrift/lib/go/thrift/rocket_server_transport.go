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
	"errors"
	"fmt"
	"io"
	"net"

	"github.com/rsocket/rsocket-go/core/transport"
)

type rocketServerTransport struct {
	listener net.Listener
	acceptor transport.ServerTransportAcceptor
}

func newRocketServerTransport(listener net.Listener) transport.ServerTransport {
	return &rocketServerTransport{
		listener: listener,
	}
}

// Accept register incoming connection handler.
func (r *rocketServerTransport) Accept(acceptor transport.ServerTransportAcceptor) {
	r.acceptor = acceptor
}

// Listen listens on the network address addr and handles requests on incoming connections.
// You can specify notifier chan, it'll be sent true/false when server listening success/failed.
func (r *rocketServerTransport) Listen(ctx context.Context, notifier chan<- bool) error {
	notifier <- true
	go func() {
		<-ctx.Done()
		r.listener.Close()
	}()
	err := r.acceptLoop(ctx)
	if ctx.Err() != nil {
		return ctx.Err()
	}
	return err
}

// acceptLoop takes a context that will be decorated with ConnInfo and passed down to new clients.
func (r *rocketServerTransport) acceptLoop(ctx context.Context) error {
	for {
		conn, err := r.listener.Accept()
		if err != nil {
			if errors.Is(err, io.EOF) {
				return nil
			}
			select {
			case <-ctx.Done():
				return nil
			default:
				err = fmt.Errorf("accept next conn failed: %v", err)
			}
		}
		if conn == nil {
			continue
		}
		go func(ctx context.Context, conn net.Conn) {
			ctx = WithConnInfo(ctx, conn)
			go r.acceptor(ctx, transport.NewTransport(transport.NewTCPConn(conn)), func(tp *transport.Transport) {})
		}(ctx, conn)
	}
}

func (r *rocketServerTransport) Close() (err error) {
	return r.listener.Close()
}
