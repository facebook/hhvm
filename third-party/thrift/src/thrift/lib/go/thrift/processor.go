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
	// a non-nil GetProcessorFunction when the function can be successfully
	// found.
	//
	// If ProcessorFunction is nil, a generic error will be
	// sent which explains that no processor function exists with the specified
	// name on this server.
	GetProcessorFunction(name string) ProcessorFunction
}

// ProcessorFunctionContext is the interface that must be implemented in
// order to perform io and message processing
type ProcessorFunction interface {
	// Read a serializable message from the protocol.
	Read(prot Decoder) (Struct, Exception)
	// RunContext processes a message handing it to the client handler.
	RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException)
	// Write a serializable response
	Write(seqID int32, result WritableStruct, prot Encoder) Exception
}

func errorType(err error) string {
	// get type name without package or pointer information
	fqet := strings.Replace(fmt.Sprintf("%T", err), "*", "", -1)
	et := strings.Split(fqet, ".")
	return et[len(et)-1]
}

func getProcessorFunction(processor Processor, messageType MessageType, name string) (ProcessorFunction, ApplicationException) {
	if messageType != CALL && messageType != ONEWAY {
		// case one: invalid message type
		return nil, NewApplicationException(UNKNOWN_METHOD, fmt.Sprintf("unexpected message type: %d", messageType))
	}
	if pf := processor.GetProcessorFunction(name); pf != nil {
		return pf, nil
	}
	return nil, NewApplicationException(UNKNOWN_METHOD, fmt.Sprintf("no such function: %q", name))
}

func skipMessage(protocol Protocol) error {
	if err := protocol.Skip(STRUCT); err != nil {
		return err
	}
	return protocol.ReadMessageEnd()
}

func setRequestHeadersForErrors(protocol Protocol, result WritableStruct, err error) {
	if err != nil {
		protocol.SetRequestHeader("uex", errorType(err))
		protocol.SetRequestHeader("uexw", err.Error())
	}
	if rr, ok := result.(WritableResult); ok && rr.Exception() != nil {
		// If we got a structured exception back, write metadata about it into headers
		terr := rr.Exception()
		protocol.SetRequestHeader("uex", errorType(terr))
		protocol.SetRequestHeader("uexw", terr.Error())
	}
}

// processContext is a utility function to take a processor and a protocol,
// and fully process a message. It understands the thrift protocol.
// A framework could be written outside of the thrift library but would need to
// duplicate this logic.
func processContext(ctx context.Context, processor Processor, prot Protocol) (ext Exception) {
	name, messageType, seqID, rerr := prot.ReadMessageBegin()
	if rerr != nil {
		return rerr
	}
	ctx = WithHeaders(ctx, prot.GetResponseHeaders())
	pfunc, err := getProcessorFunction(processor, messageType, name)
	if err != nil {
		// attempt to skip the rest of the invalid message but keep the connection open.
		if e2 := skipMessage(prot); e2 != nil {
			return e2
		}
		// for ONEWAY, we have no way to report that the processing failed.
		if messageType != ONEWAY {
			// error should be sent, connection should stay open if successful.
			if e2 := sendException(prot, name, seqID, err); e2 != nil {
				return e2
			}
		}
		return nil
	}

	argStruct, e2 := pfunc.Read(prot)
	if e2 != nil {
		// close connection on read failure
		return e2
	}
	result, err := pfunc.RunContext(ctx, argStruct)
	if messageType == ONEWAY {
		// for ONEWAY messages, never send a response
		return nil
	}
	if result == nil && err == nil {
		// if result was nil, call was oneway
		// often times oneway calls do not even have msgType ONEWAY
		return nil
	}
	if err != nil {
		// it's an application generated error, so serialize it
		// to the client
		result = err
	}
	setRequestHeadersForErrors(prot, result, err)

	if e2 := pfunc.Write(seqID, result, prot); e2 != nil {
		// close connection on write failure
		return err
	}
	// keep the connection open and ignore errors
	// if type was CALL, error has already been serialized to client
	// if type was ONEWAY, no exception is to be thrown
	return nil
}
