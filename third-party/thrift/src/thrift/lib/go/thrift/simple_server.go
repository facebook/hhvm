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
	"context"
	"errors"
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
	processorFactoryContext      ProcessorFactoryContext
	configurableRequestProcessor func(ctx context.Context, client Transport) error
	*ServerOptions
}

// NewSimpleServer create a new server
func NewSimpleServer(processor Processor, serverTransport ServerTransport, options ...func(*ServerOptions)) *SimpleServer {
	return NewSimpleServerContext(NewProcessorContextAdapter(processor), serverTransport, options...)
}

// NewSimpleServerContext creates a new server that supports contexts
func NewSimpleServerContext(processor ProcessorContext, serverTransport ServerTransport, options ...func(*ServerOptions)) *SimpleServer {
	return NewSimpleServerFactoryContext(NewProcessorFactoryContext(processor), serverTransport, options...)
}

// NewSimpleServer2 is deprecated, used NewSimpleServer instead
func NewSimpleServer2(processor Processor, serverTransport ServerTransport) *SimpleServer {
	return NewSimpleServerFactory(NewProcessorFactory(processor), serverTransport)
}

// NewSimpleServer4 is deprecated, used NewSimpleServer instead
func NewSimpleServer4(processor Processor, serverTransport ServerTransport, transportFactory TransportFactory, protocolFactory ProtocolFactory) *SimpleServer {
	return NewSimpleServerFactory(
		NewProcessorFactory(processor),
		serverTransport,
		TransportFactories(transportFactory),
		ProtocolFactories(protocolFactory),
	)
}

// NewSimpleServer6 is deprecated, used NewSimpleServer instead
func NewSimpleServer6(processor Processor, serverTransport ServerTransport, inputTransportFactory TransportFactory, outputTransportFactory TransportFactory, inputProtocolFactory ProtocolFactory, outputProtocolFactory ProtocolFactory) *SimpleServer {
	return NewSimpleServerFactory(
		NewProcessorFactory(processor),
		serverTransport,
		InputTransportFactory(inputTransportFactory),
		OutputTransportFactory(outputTransportFactory),
		InputProtocolFactory(inputProtocolFactory),
		OutputProtocolFactory(outputProtocolFactory),
	)
}

// NewSimpleServerFactory create a new server factory
func NewSimpleServerFactory(processorFactory ProcessorFactory, serverTransport ServerTransport, options ...func(*ServerOptions)) *SimpleServer {
	return NewSimpleServerFactoryContext(NewProcessorFactoryContextAdapter(processorFactory), serverTransport, options...)
}

// NewSimpleServerFactoryContext creates a new server factory that supports contexts.
func NewSimpleServerFactoryContext(processorFactoryContext ProcessorFactoryContext, serverTransport ServerTransport, options ...func(*ServerOptions)) *SimpleServer {
	return &SimpleServer{
		processorFactoryContext: processorFactoryContext,
		ServerOptions:           simpleServerOptions(serverTransport, options...),
	}
}

func simpleServerOptions(t ServerTransport, options ...func(*ServerOptions)) *ServerOptions {
	opts := defaultServerOptions(t)
	for _, option := range options {
		option(opts)
	}
	return opts
}

// NewSimpleServerFactory2 is deprecated, used NewSimpleServerFactory instead
func NewSimpleServerFactory2(processorFactory ProcessorFactory, serverTransport ServerTransport) *SimpleServer {
	return NewSimpleServerFactory(processorFactory, serverTransport)
}

// NewSimpleServerFactory4 is deprecated, used NewSimpleServerFactory instead
func NewSimpleServerFactory4(processorFactory ProcessorFactory, serverTransport ServerTransport, transportFactory TransportFactory, protocolFactory ProtocolFactory) *SimpleServer {
	return NewSimpleServerFactory(
		processorFactory,
		serverTransport,
		TransportFactories(transportFactory),
		ProtocolFactories(protocolFactory),
	)
}

// NewSimpleServerFactory6 is deprecated, used NewSimpleServerFactory instead
func NewSimpleServerFactory6(processorFactory ProcessorFactory, serverTransport ServerTransport, inputTransportFactory TransportFactory, outputTransportFactory TransportFactory, inputProtocolFactory ProtocolFactory, outputProtocolFactory ProtocolFactory) *SimpleServer {
	return NewSimpleServerFactory(
		processorFactory,
		serverTransport,
		InputTransportFactory(inputTransportFactory),
		OutputTransportFactory(outputTransportFactory),
		InputProtocolFactory(inputProtocolFactory),
		OutputProtocolFactory(outputProtocolFactory),
	)
}

// ProcessorFactoryContext returns the processor factory that supports contexts
func (p *SimpleServer) ProcessorFactoryContext() ProcessorFactoryContext {
	return p.processorFactoryContext
}

// ServerTransport returns the server transport
func (p *SimpleServer) ServerTransport() ServerTransport {
	return p.serverTransport
}

// InputTransportFactory returns the input transport factory
func (p *SimpleServer) InputTransportFactory() TransportFactory {
	return p.inputTransportFactory
}

// OutputTransportFactory returns the output transport factory
func (p *SimpleServer) OutputTransportFactory() TransportFactory {
	return p.outputTransportFactory
}

// InputProtocolFactory returns the input protocolfactory
func (p *SimpleServer) InputProtocolFactory() ProtocolFactory {
	return p.inputProtocolFactory
}

// OutputProtocolFactory returns the output protocol factory
func (p *SimpleServer) OutputProtocolFactory() ProtocolFactory {
	return p.outputProtocolFactory
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
		go func(ctx context.Context, client Transport) {
			ctx = p.addConnInfo(ctx, client)
			if err := p.processRequests(ctx, client); err != nil {
				p.log.Println("thrift: error processing request:", err)
			}
		}(ctx, client)
	}
}

func (p *SimpleServer) addConnInfo(ctx context.Context, client Transport) context.Context {
	if p.processorFactoryContext == nil {
		return ctx
	}
	return WithConnInfo(ctx, client)
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

func (p *SimpleServer) processRequests(ctx context.Context, client Transport) error {
	if p.configurableRequestProcessor != nil {
		return p.configurableRequestProcessor(ctx, client)
	}

	processor := p.processorFactoryContext.GetProcessorContext(client)
	var (
		inputTransport, outputTransport Transport
		inputProtocol, outputProtocol   Protocol
	)

	inputTransport = p.inputTransportFactory.GetTransport(client)

	// Special case for Header, it requires that the transport/protocol for
	// input/output is the same object (to track session state).
	if _, ok := inputTransport.(*HeaderTransport); ok {
		outputTransport = nil
		inputProtocol = p.inputProtocolFactory.GetProtocol(inputTransport)
		outputProtocol = inputProtocol
	} else {
		outputTransport = p.outputTransportFactory.GetTransport(client)
		inputProtocol = p.inputProtocolFactory.GetProtocol(inputTransport)
		outputProtocol = p.outputProtocolFactory.GetProtocol(outputTransport)
	}

	// Store the input protocol on the context so handlers can query headers.
	// See HeadersFromContext.
	ctx = WithProtocol(ctx, inputProtocol)

	defer func() {
		if err := recover(); err != nil {
			p.log.Printf("panic in processor: %v: %s", err, debug.Stack())
		}
	}()
	if inputTransport != nil {
		defer inputTransport.Close()
	}
	if outputTransport != nil {
		defer outputTransport.Close()
	}
	intProcessor := WrapInterceptorContext(p.interceptor, processor)
	for {
		keepOpen, exc := ProcessContext(ctx, intProcessor, inputProtocol, outputProtocol)
		if exc != nil {
			outputProtocol.Flush()
			return exc
		}
		if !keepOpen {
			break
		}
	}

	// graceful exit.  client closed connection
	return nil
}
