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
	"log"
	"net"
	"os"
	"runtime/debug"
)

// ErrServerClosed is returned by the Serve methods after a call to Stop
var ErrServerClosed = errors.New("thrift: Server closed")

// SimpleServer is a functional but unoptimized server that is easy to
// understand.  In its accept loop, it performs an accept on an
// underlying socket, wraps the socket in the net.Listener, and
// then spins up a gofunc to process requests.
//
// There is one gofunc per active connection that handles all requests
// on the connection.  multiple simultaneous requests over a single
// connection are not supported, as the per-connection gofunc reads
// the request, processes it, and writes the response serially
type SimpleServer struct {
	processorContext ProcessorContext
	listener         net.Listener
	newProtocol      func(net.Conn) (Protocol, error)
	quit             chan struct{}
	log              *log.Logger
	*ServerOptions
}

// NewSimpleServer creates a new server that only supports Header Transport.
func NewSimpleServer(processor ProcessorContext, listener net.Listener, transportType TransportID, options ...func(*ServerOptions)) *SimpleServer {
	if transportType != TransportIDHeader {
		panic(fmt.Sprintf("SimpleServer only supports Header Transport and not %d", transportType))
	}
	return &SimpleServer{
		quit:             make(chan struct{}, 1),
		processorContext: processor,
		listener:         listener,
		newProtocol:      NewHeaderProtocol,
		log:              log.New(os.Stderr, "", log.LstdFlags),
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

// Listen returns the server transport listener
func (p *SimpleServer) Listen() (net.Addr, error) {
	return p.listener.Addr(), nil
}

// acceptLoopContext takes a context that will be decorated with ConnInfo and passed down to new clients.
func (p *SimpleServer) acceptLoopContext(ctx context.Context) error {
	for {
		client, err := p.listener.Accept()
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
	_, err := p.Listen()
	if err != nil {
		return err
	}
	go func() {
		<-ctx.Done()
		p.Stop()
	}()
	err = p.acceptLoopContext(ctx)
	if ctx.Err() != nil {
		return ctx.Err()
	}
	return err
}

// Stop stops the server
func (p *SimpleServer) Stop() error {
	p.quit <- struct{}{}
	p.listener.Close()
	return nil
}

func (p *SimpleServer) processRequests(ctx context.Context, client net.Conn) error {
	processor := p.processorContext

	protocol, err := p.newProtocol(client)
	if err != nil {
		return err
	}

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
