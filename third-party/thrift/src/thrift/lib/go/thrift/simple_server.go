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
	"net"
	"runtime/debug"
)

// ErrServerClosed is returned by the Serve methods after a call to Stop
var ErrServerClosed = errors.New("thrift: Server closed")

// SimpleServer is a functional but unoptimized server that is easy to
// understand.  In its accept loop, it performs an accept on an
// underlying socket, wraps the socket in the ServerTransport, and
// then spins up a gofunc to process requests.
//
// There is one gofunc per active connection that handles all requests
// on the connection.  multiple simultaneous requests over a single
// connection are not supported, as the per-connection gofunc reads
// the request, processes it, and writes the response serially
type SimpleServer struct {
	processorContext ProcessorContext
	serverTransport  ServerTransport
	newProtocol      func(net.Conn) Protocol
	*ServerOptions
}

// NewSimpleServer creates a new server that only supports Header Transport.
func NewSimpleServer(processor ProcessorContext, serverTransport ServerTransport, transportType TransportID, options ...func(*ServerOptions)) *SimpleServer {
	if transportType != TransportIDHeader {
		panic(fmt.Sprintf("SimpleServer only supports Header Transport and not %s", transportType))
	}
	return &SimpleServer{
		processorContext: processor,
		serverTransport:  serverTransport,
		newProtocol:      NewHeaderProtocol,
		ServerOptions:    simpleServerOptions(options...),
	}
}

func simpleServerOptions(options ...func(*ServerOptions)) *ServerOptions {
	opts := defaultServerOptions()
	for _, option := range options {
		option(opts)
	}
	return opts
}

// ServerTransport returns the server transport
func (p *SimpleServer) ServerTransport() ServerTransport {
	return p.serverTransport
}

// Listen returns the server transport listener
func (p *SimpleServer) Listen() error {
	return p.serverTransport.Listen()
}

// AcceptLoop runs the accept loop to handle requests
func (p *SimpleServer) AcceptLoop() error {
	return p.AcceptLoopContext(context.Background())
}

// AcceptLoopContext is an AcceptLoop that supports contexts.
// The context is decorated with ConnInfo and passed down to new clients.
func (p *SimpleServer) AcceptLoopContext(ctx context.Context) error {
	for {
		client, err := p.serverTransport.Accept()
		if err != nil {
			select {
			case <-p.quit:
				return ErrServerClosed
			default:
			}
			return err
		}
		if client == nil {
			continue
		}
		go func(ctx context.Context, client net.Conn) {
			ctx = p.addConnInfo(ctx, client)
			if err := p.processRequests(ctx, client); err != nil {
				p.log.Println("thrift: error processing request:", err)
			}
		}(ctx, client)
	}
}

func (p *SimpleServer) addConnInfo(ctx context.Context, conn net.Conn) context.Context {
	if p.processorContext == nil {
		return ctx
	}
	return WithConnInfo(ctx, conn)
}

// Serve starts listening on the transport and accepting new connections
// and blocks until Stop is called or an error occurs.
func (p *SimpleServer) Serve() error {
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	return p.ServeContext(ctx)
}

// ServeContext behaves like Serve but supports cancellation via context.
func (p *SimpleServer) ServeContext(ctx context.Context) error {
	err := p.Listen()
	if err != nil {
		return err
	}
	go func() {
		<-ctx.Done()
		p.Stop()
	}()
	err = p.AcceptLoopContext(ctx)
	if ctx.Err() != nil {
		return ctx.Err()
	}
	return err
}

// Stop stops the server
func (p *SimpleServer) Stop() error {
	p.quit <- struct{}{}
	p.serverTransport.Interrupt()
	return nil
}

func (p *SimpleServer) processRequests(ctx context.Context, client net.Conn) error {
	processor := p.processorContext

	protocol := p.newProtocol(client)

	// Store the protocol on the context so handlers can query headers.
	// See HeadersFromContext.
	ctx = WithProtocol(ctx, protocol)

	defer func() {
		if err := recover(); err != nil {
			p.log.Printf("panic in processor: %v: %s", err, debug.Stack())
		}
	}()
	defer protocol.Close()
	intProcessor := WrapInterceptorContext(p.interceptor, processor)
	for {
		keepOpen, exc := processContext(ctx, intProcessor, protocol)
		if exc != nil {
			protocol.Flush()
			return exc
		}
		if !keepOpen {
			break
		}
	}

	// graceful exit.  client closed connection
	return nil
}
