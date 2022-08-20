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
	"fmt"
	"log"
	"runtime/debug"
	"sync"
)

// ConcurrentServer is the concurrent counterpart of SimpleServer
// It is able to process out-of-order requests on the same transport
type ConcurrentServer struct {
	*SimpleServer
}

// NewConcurrentServer create a new NewConcurrentServer
func NewConcurrentServer(processor Processor, serverTransport ServerTransport, options ...func(*ServerOptions)) *ConcurrentServer {
	return NewConcurrentServerFactory(NewProcessorFactory(processor), serverTransport, options...)
}

// NewConcurrentServerFactory create a new server factory
func NewConcurrentServerFactory(processorFactory ProcessorFactory, serverTransport ServerTransport, options ...func(*ServerOptions)) *ConcurrentServer {
	return NewConcurrentServerFactoryContext(NewProcessorFactoryContextAdapter(processorFactory), serverTransport, options...)
}

// NewConcurrentServerContext is a version of the ConcurrentServer that supports contexts.
func NewConcurrentServerContext(processor ProcessorContext, serverTransport ServerTransport, options ...func(*ServerOptions)) *ConcurrentServer {
	return NewConcurrentServerFactoryContext(NewProcessorFactoryContext(processor), serverTransport, options...)
}

// NewConcurrentServerFactoryContext is a version of the ConcurrentServerFactory that supports contexts.
func NewConcurrentServerFactoryContext(processorFactory ProcessorFactoryContext, serverTransport ServerTransport, options ...func(*ServerOptions)) *ConcurrentServer {
	srv := &ConcurrentServer{
		SimpleServer: NewSimpleServerFactoryContext(processorFactory, serverTransport, options...),
	}
	srv.SimpleServer.configurableRequestProcessor = srv.processRequests
	return srv
}

func (p *ConcurrentServer) processRequests(ctx context.Context, client Transport) error {
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
	ctx = context.WithValue(ctx, protocolKey, inputProtocol)

	// recover from any panic in the processor, so it doesn't crash the
	// thrift server
	defer func() {
		if e := recover(); e != nil {
			log.Printf("panic in processor: %s: %s", e, debug.Stack())
		}
	}()
	if inputTransport != nil {
		defer inputTransport.Close()
	}
	if outputTransport != nil {
		defer outputTransport.Close()
	}
	intProcessor := WrapInterceptorContext(p.interceptor, processor)

	// WARNING: This server implementation has a host of problems, and is included
	// to preserve previous behavior.  If you really want a production quality thrift
	// server, use simple server or write your own.
	//
	// In the concurrent server case, we wish to handle multiple concurrent requests
	// on a single transport.  To do this, we re-implement the generated Process()
	// function inline for greater control, then directly interact with the Read(),
	// Run(), and Write() functionality.
	//
	// Note, for a very high performance server, it is unclear that this unbounded
	// concurrency is productive for maintaining maximal throughput with good
	// characteristics under load.
	var writeLock sync.Mutex
	for {
		name, _, seqID, err := inputProtocol.ReadMessageBegin()
		if err != nil {
			if err, ok := err.(TransportException); ok && err.TypeID() == END_OF_FILE {
				// connection terminated because client closed connection
				break
			}
			return err
		}
		pfunc, err := intProcessor.GetProcessorFunctionContext(name)
		if pfunc == nil || err != nil {
			if err == nil {
				err = fmt.Errorf("no such function: %q", name)
			}
			inputProtocol.Skip(STRUCT)
			inputProtocol.ReadMessageEnd()
			exc := NewApplicationException(UNKNOWN_METHOD, err.Error())

			// protect message writing
			writeLock.Lock()
			defer writeLock.Unlock()

			outputProtocol.WriteMessageBegin(name, EXCEPTION, seqID)
			exc.Write(outputProtocol)
			outputProtocol.WriteMessageEnd()
			outputProtocol.Flush()
			return exc
		}
		argStruct, err := pfunc.Read(inputProtocol)
		if err != nil {
			return err
		}
		go func() {
			var result WritableStruct
			result, err = pfunc.RunContext(ctx, argStruct)
			// protect message writing
			writeLock.Lock()
			defer writeLock.Unlock()
			if err != nil && result == nil {
				// if the Run function generates an error, synthesize an application
				// error
				exc := NewApplicationException(INTERNAL_ERROR, "Internal error: "+err.Error())
				err, result = exc, exc
			}
			pfunc.Write(seqID, result, outputProtocol)
			// ignore write failures explicitly.  This emulates previous behavior
			// we hope that the read will fail and the connection will be closed
			// well.
		}()
	}
	// graceful exit
	return nil
}
