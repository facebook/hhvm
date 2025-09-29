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
	"runtime/debug"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
	"github.com/facebook/fbthrift/thrift/lib/thrift/rocket_upgrade"
	"github.com/rsocket/rsocket-go/core/transport"
)

type rocketServerTransport struct {
	listener    net.Listener
	processor   Processor
	acceptor    transport.ServerTransportAcceptor
	transportID TransportID
	connContext ConnContextFunc
	log         func(format string, args ...any)
	stats       *stats.ServerStats
	pstats      map[string]*stats.TimingSeries
	observer    ServerObserver
}

func newRocketServerTransport(
	listener net.Listener,
	connContext ConnContextFunc,
	processor Processor,
	transportID TransportID,
	log func(format string, args ...any),
	stats *stats.ServerStats,
	pstats map[string]*stats.TimingSeries,
	observer ServerObserver,
) transport.ServerTransport {
	return &rocketServerTransport{
		listener:    listener,
		processor:   processor,
		transportID: transportID,
		connContext: connContext,
		log:         log,
		stats:       stats,
		pstats:      pstats,
		observer:    observer,
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

		// Notify observer that connection was successfully accepted
		r.observer.ConnAccepted()

		go func(ctx context.Context, conn net.Conn) {
			// Explicitly force TLS handshake protocol to run (if this is a TLS connection).
			//
			// Usually, TLS handshake is done implicitly/seamlessly by 'crypto/tls' package,
			// whenever Read/Write functions are invoked on a connection for the first time.
			// However, in our case, we require the handshake to be complete ahead of any
			// Read/Write calls - so that we can access ALPN value and choose the transport.
			tlsConn, isTLS := conn.(tlsConnectionStaterHandshaker)
			if isTLS {
				r.observer.ConnTLSAccepted()

				err = tlsConn.HandshakeContext(ctx)
				if err != nil {
					r.log("thrift: error performing TLS handshake with %s: %s\n", conn.RemoteAddr(), err)
					// Notify observer that connection was dropped due to handshake failure
					r.observer.ConnDropped()
					// Handshake failed, we cannot proceed with this connection - close it and return.
					conn.Close()
					return
				}
			}

			ctxConn := r.connContext(ctx, conn)
			r.processRequests(ctxConn, conn)
		}(ctx, conn)
	}
}

func (r *rocketServerTransport) Close() error {
	return r.listener.Close()
}

func (r *rocketServerTransport) processRequests(ctx context.Context, conn net.Conn) {
	// update current connection count
	r.stats.ConnCount.Incr()
	defer func() {
		r.stats.ConnCount.Decr()
	}()

	connTransport := r.transportID
	// Use Rocket protocol right away if the server is running
	// in "UpgradeToRocket" mode and ALPN value is set to "rs".
	if r.transportID == TransportIDUpgradeToRocket {
		if connInfo, ok := ConnInfoFromContext(ctx); ok {
			tlsConnState := connInfo.TLS()
			if tlsConnState != nil && tlsConnState.NegotiatedProtocol == "rs" {
				connTransport = TransportIDRocket
			}
		}
	}

	switch connTransport {
	case TransportIDRocket:
		r.processRocketRequests(ctx, conn)
	case TransportIDUpgradeToRocket:
		ruHandler := newRocketUpgradeHandler()
		ruProcessor := rocket_upgrade.NewRocketUpgradeProcessor(ruHandler)
		ruCompProcessor := NewCompositeProcessor()
		ruCompProcessor.Include(r.processor)
		ruCompProcessor.Include(ruProcessor)

		headerProtocol, err := NewHeaderProtocol(conn)
		if err != nil {
			r.log("thrift: error constructing header protocol from %s: %s\n", conn.RemoteAddr(), err)
			r.observer.ConnDropped()
			return
		}
		if err := r.processHeaderRequest(ctx, headerProtocol, ruCompProcessor); err != nil {
			r.log("thrift: error processing first header request from %s: %s\n", conn.RemoteAddr(), err)
			// TODO: jchistyakov to ensure all other if-paths in this function close the connection properly
			r.observer.ConnDropped()
			headerProtocol.Close()
			return
		}
		if ruHandler.upgradeInvoked {
			r.processRocketRequests(ctx, conn)
		} else {
			if err := r.processHeaderRequests(ctx, headerProtocol, ruCompProcessor); err != nil {
				r.log("thrift: error processing additional header request from %s: %s\n", conn.RemoteAddr(), err)
				r.observer.ConnDropped()
			}
		}
	case TransportIDHeader:
		headerProtocol, err := NewHeaderProtocol(conn)
		if err != nil {
			r.log("thrift: error constructing header protocol from %s: %s\n", conn.RemoteAddr(), err)
			r.observer.ConnDropped()
			return
		}
		if err := r.processHeaderRequests(ctx, headerProtocol, r.processor); err != nil {
			r.log("thrift: error processing header request from %s: %s\n", conn.RemoteAddr(), err)
			r.observer.ConnDropped()
		}
	}
}

func (r *rocketServerTransport) processRocketRequests(ctx context.Context, conn net.Conn) {
	defer func() {
		if err := recover(); err != nil {
			r.log("panic in rocket processor: %v: %s", err, debug.Stack())
			// Notify observer that connection was dropped due to panic
			r.observer.ConnDropped()
		}
	}()
	r.acceptor(ctx, transport.NewTransport(transport.NewTCPConn(conn)), func(*transport.Transport) {})
}

func (r *rocketServerTransport) processHeaderRequest(ctx context.Context, protocol Protocol, processor Processor) error {
	exc := process(ctx, processor, protocol, r.pstats, r.observer)
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
			r.log("panic in processor: %v: %s", err, debug.Stack())
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
