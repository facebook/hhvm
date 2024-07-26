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

// ProcessorContext exposes access to processor functions which
// manage I/O and processing of a input message for a specific
// server function
type ProcessorContext interface {
	// GetProcessorFunctionContext is given the name of a thrift function and
	// the type of the inbound thrift message.  It is expected to return
	// a non-nil GetProcessorFunctionContext when the function can be successfully
	// found.
	//
	// If an error is returned, it will be wrapped in an application level
	// thrift exception and returned.
	//
	// If ProcessorFunctionContext and error are both nil, a generic error will be
	// sent which explains that no processor function exists with the specified
	// name on this server.
	GetProcessorFunctionContext(name string) (ProcessorFunctionContext, error)
}

// ProcessorFunctionContext is the interface that must be implemented in
// order to perform io and message processing
type ProcessorFunctionContext interface {
	// Read a serializable message from the protocol.
	Read(prot Format) (Struct, Exception)
	// RunContext processes a message handing it to the client handler.
	RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException)
	// Write a serializable response
	Write(seqID int32, result WritableStruct, prot Format) Exception
}

func errorType(err error) string {
	// get type name without package or pointer information
	fqet := strings.Replace(fmt.Sprintf("%T", err), "*", "", -1)
	et := strings.Split(fqet, ".")
	return et[len(et)-1]
}

// processContext is a utility function to take a processor and a protocol,
// and fully process a message. It understands the thrift protocol.
// A framework could be written outside of the thrift library but would need to
// duplicate this logic.
func processContext(ctx context.Context, processor ProcessorContext, prot Protocol) (ext Exception) {
	name, messageType, seqID, rerr := prot.ReadMessageBegin()
	if rerr != nil {
		return rerr
	}
	ctx = WithHeaders(ctx, prot.GetResponseHeaders())
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
		if e2 := prot.Skip(STRUCT); e2 != nil {
			return e2
		} else if e2 := prot.ReadMessageEnd(); e2 != nil {
			return e2
		}
		// for ONEWAY, we have no way to report that the processing failed.
		if messageType != ONEWAY {
			if e2 := sendException(prot, name, seqID, err); e2 != nil {
				return e2
			}
		}
		return nil
	}

	if pfunc == nil {
		panic("logic error in thrift.Process() handler.  processor function may not be nil")
	}

	argStruct, e2 := pfunc.Read(prot)
	if e2 != nil {
		// close connection on read failure
		return e2
	}
	var result WritableStruct
	result, err = pfunc.RunContext(ctx, argStruct)

	// for ONEWAY messages, never send a response
	if messageType == CALL {
		// protect message writing
		if err != nil {
			prot.SetRequestHeader("uex", errorType(err))
			prot.SetRequestHeader("uexw", err.Error())
			// it's an application generated error, so serialize it
			// to the client
			result = err
		}

		// If we got a structured exception back, write metadata about it into headers
		if rr, ok := result.(WritableResult); ok && rr.Exception() != nil {
			terr := rr.Exception()
			prot.SetRequestHeader("uex", errorType(terr))
			prot.SetRequestHeader("uexw", terr.Error())
		}

		// if result was nil, call was oneway
		// often times oneway calls do not even have msgType ONEWAY
		if result != nil {
			if e2 := pfunc.Write(seqID, result, prot); e2 != nil {
				// close connection on write failure
				return err
			}
		}
	}

	// keep the connection open and ignore errors
	// if type was CALL, error has already been serialized to client
	// if type was ONEWAY, no exception is to be thrown
	return nil
}
