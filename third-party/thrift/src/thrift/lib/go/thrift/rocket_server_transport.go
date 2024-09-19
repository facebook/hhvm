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
	"log"
	"net"
	"os"
	"runtime/debug"

	"github.com/rsocket/rsocket-go/core/transport"
)

type rocketServerTransport struct {
	listener    net.Listener
	processor   Processor
	acceptor    transport.ServerTransportAcceptor
	transportID TransportID
	log         *log.Logger
	connContext ConnContextFunc
}

func newRocketServerTransport(listener net.Listener, connContext ConnContextFunc, processor Processor, transportID TransportID) transport.ServerTransport {
	return &rocketServerTransport{
		listener:    listener,
		processor:   processor,
		log:         log.New(os.Stderr, "", log.LstdFlags),
		transportID: transportID,
		connContext: connContext,
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
				return fmt.Errorf("listener.Accept failed in rocketServerTransport.acceptLoop: %w", err)
			}
		}
		if conn == nil {
			continue
		}

		go func(ctx context.Context, conn net.Conn) {
			ctx = r.connContext(ctx, conn)
			r.processRequests(ctx, conn)
		}(ctx, conn)
	}
}

func (r *rocketServerTransport) Close() (err error) {
	return r.listener.Close()
}

func (r *rocketServerTransport) processRequests(ctx context.Context, conn net.Conn) {
	switch r.transportID {
	case TransportIDRocket:
		r.processRocketRequests(ctx, conn)
	case TransportIDUpgradeToRocket:
		processor := newRocketUpgradeProcessor(r.processor)
		headerProtocol, err := NewHeaderProtocol(conn)
		if err != nil {
			r.log.Println("thrift: error constructing header protocol: ", err)
			return
		}
		if err := r.processHeaderRequest(ctx, headerProtocol, processor); err != nil {
			r.log.Println("thrift: error processing request: ", err)
			return
		}
		if processor.upgraded {
			r.processRocketRequests(ctx, conn)
		} else {
			if err := r.processHeaderRequests(ctx, headerProtocol, processor); err != nil {
				r.log.Println("thrift: error processing request: ", err)
			}
		}
	case TransportIDHeader:
		headerProtocol, err := NewHeaderProtocol(conn)
		if err != nil {
			r.log.Println("thrift: error constructing header protocol: ", err)
			return
		}
		if err := r.processHeaderRequests(ctx, headerProtocol, r.processor); err != nil {
			r.log.Println("thrift: error processing request: ", err)
		}
	}
}

func (r *rocketServerTransport) processRocketRequests(ctx context.Context, conn net.Conn) {
	r.acceptor(ctx, transport.NewTransport(transport.NewTCPConn(conn)), func(*transport.Transport) {})
}

func (r *rocketServerTransport) processHeaderRequest(ctx context.Context, protocol Protocol, processor Processor) error {
	exc := process(ctx, processor, protocol)
	if isEOF(exc) {
		return exc
	}
	if exc != nil {
		protocol.Flush()
		return exc
	}
	return nil
}

func (r *rocketServerTransport) processHeaderRequests(ctx context.Context, protocol Protocol, processor Processor) error {
	defer func() {
		if err := recover(); err != nil {
			r.log.Printf("panic in processor: %v: %s", err, debug.Stack())
		}
	}()
	defer protocol.Close()
	var err error
	for err == nil {
		err = r.processHeaderRequest(ctx, protocol, processor)
	}
	if isEOF(err) {
		return nil
	}
	protocol.Flush()
	// graceful exit.  client closed connection
	return err
}
