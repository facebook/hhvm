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
	"fmt"
	"strings"
)

// Processor exposes access to processor functions which
// manage I/O and processing of a input message for a specific
// server function
type Processor interface {
	// GetProcessorFunction is given the name of a thrift function and
	// the type of the inbound thrift message.  It is expected to return
	// a non-nil ProcessorFunction when the function can be successfully
	// found.
	//
	// If an error is returned, it will be wrapped in an application level
	// thrift exception and returned.
	//
	// If ProcessorFunction and error are both nil, a generic error will be
	// sent which explains that no processor function exists with the specified
	// name on this server.
	GetProcessorFunction(name string) (ProcessorFunction, error)
}

// ProcessorFunction is the interface that must be implemented in
// order to perform io and message processing
type ProcessorFunction interface {
	// Read a serializable message from the input protocol.
	Read(iprot Protocol) (Struct, Exception)
	// Process a message handing it to the client handler.
	Run(args Struct) (WritableStruct, ApplicationException)
	// Write a serializable responsne
	Write(seqID int32, result WritableStruct, oprot Protocol) Exception
}

// Process is a utility function to take a processor and an input and output
// protocol, and fully process a message.  It understands the thrift protocol.
// A framework could be written outside of the thrift library but would need to
// duplicate this logic.
func Process(processor Processor, iprot, oprot Protocol) (keepOpen bool, exc Exception) {
	return ProcessContext(context.Background(), NewProcessorContextAdapter(processor), iprot, oprot)
}

// ProcessorContext is a Processor that supports contexts.
type ProcessorContext interface {
	GetProcessorFunctionContext(name string) (ProcessorFunctionContext, error)
}

// NewProcessorContextAdapter creates a ProcessorContext from a regular Processor.
func NewProcessorContextAdapter(p Processor) ProcessorContext {
	return &ctxProcessorAdapter{p}
}

type ctxProcessorAdapter struct {
	Processor
}

func (p ctxProcessorAdapter) GetProcessorFunctionContext(name string) (ProcessorFunctionContext, error) {
	f, err := p.Processor.GetProcessorFunction(name)
	if err != nil {
		return nil, err
	}
	if f == nil {
		return nil, nil
	}
	return NewProcessorFunctionContextAdapter(f), nil
}

// ProcessorFunctionContext is a ProcessorFunction that supports contexts.
type ProcessorFunctionContext interface {
	Read(iprot Protocol) (Struct, Exception)
	RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException)
	Write(seqID int32, result WritableStruct, oprot Protocol) Exception
}

// NewProcessorFunctionContextAdapter creates a ProcessorFunctionContext from a regular ProcessorFunction.
func NewProcessorFunctionContextAdapter(p ProcessorFunction) ProcessorFunctionContext {
	return &ctxProcessorFunctionAdapter{p}
}

type ctxProcessorFunctionAdapter struct {
	ProcessorFunction
}

func (p ctxProcessorFunctionAdapter) RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException) {
	return p.ProcessorFunction.Run(args)
}

func errorType(err error) string {
	// get type name without package or pointer information
	fqet := strings.Replace(fmt.Sprintf("%T", err), "*", "", -1)
	et := strings.Split(fqet, ".")
	return et[len(et)-1]
}

// ProcessContext is a Process that supports contexts.
func ProcessContext(ctx context.Context, processor ProcessorContext, iprot, oprot Protocol) (keepOpen bool, ext Exception) {
	name, messageType, seqID, rerr := iprot.ReadMessageBegin()
	if rerr != nil {
		if err, ok := rerr.(TransportException); ok && err.TypeID() == END_OF_FILE {
			// connection terminated because client closed connection
			return false, nil
		}
		return false, rerr
	}
	var err ApplicationException
	var pfunc ProcessorFunctionContext
	if messageType != CALL && messageType != ONEWAY {
		// case one: invalid message type
		err = NewApplicationException(UNKNOWN_METHOD, fmt.Sprintf("unexpected message type: %d", messageType))
		// error should be sent, connection should stay open if successful
	}
	if err == nil {
		pf, e2 := processor.GetProcessorFunctionContext(name)
		if pf == nil {
			if e2 == nil {
				err = NewApplicationException(UNKNOWN_METHOD, fmt.Sprintf("no such function: %q", name))
			} else {
				err = NewApplicationException(UNKNOWN_METHOD, e2.Error())
			}
		} else {
			pfunc = pf
		}
	}

	// if there was an error before we could find the Processor function, attempt to skip the
	// rest of the invalid message but keep the connection open.
	if err != nil {
		if e2 := iprot.Skip(STRUCT); e2 != nil {
			return false, e2
		} else if e2 := iprot.ReadMessageEnd(); e2 != nil {
			return false, e2
		}
		// for ONEWAY, we have no way to report that the processing failed.
		if messageType != ONEWAY {
			if e2 := sendException(oprot, name, seqID, err); e2 != nil {
				return false, e2
			}
		}
		return true, nil
	}

	if pfunc == nil {
		panic("logic error in thrift.Process() handler.  processor function may not be nil")
	}

	argStruct, e2 := pfunc.Read(iprot)
	if e2 != nil {
		// close connection on read failure
		return false, e2
	}
	var result WritableStruct
	result, err = pfunc.RunContext(ctx, argStruct)

	// for ONEWAY messages, never send a response
	if messageType == CALL {
		// protect message writing
		if err != nil {
			switch oprotHeader := oprot.(type) {
			case *HeaderProtocol:
				// set header for ServiceRouter
				oprotHeader.SetRequestHeader("uex", errorType(err))
				oprotHeader.SetRequestHeader("uexw", err.Error())
			}
			// it's an application generated error, so serialize it
			// to the client
			result = err
		}

		// If we got a structured exception back, write metadata about it into headers
		if rr, ok := result.(WritableResult); ok && rr.Exception() != nil {
			switch oprotHeader := oprot.(type) {
			case *HeaderProtocol:
				terr := rr.Exception()
				oprotHeader.SetRequestHeader("uex", errorType(terr))
				oprotHeader.SetRequestHeader("uexw", terr.Error())
			}
		}

		// if result was nil, call was oneway
		// often times oneway calls do not even have msgType ONEWAY
		if result != nil {
			if e2 := pfunc.Write(seqID, result, oprot); e2 != nil {
				// close connection on write failure
				return false, err
			}
		}
	}

	// keep the connection open and ignore errors
	// if type was CALL, error has already been serialized to client
	// if type was ONEWAY, no exception is to be thrown
	return true, nil
}
