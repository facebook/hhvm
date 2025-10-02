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
	"time"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/stats"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
	"github.com/facebook/fbthrift/thrift/lib/thrift/metadata"
)

// Processor exposes access to processor functions which
// manage I/O and processing of a input message for a specific
// server function
type Processor interface {
	// GetProcessorFunctionMap is given the name of a thrift function
	// of the inbound thrift message.  It is expected to return
	// a non-nil GetProcessorFunction when the function can be successfully
	// found.
	//
	// If GetProcessorFunctionMap is nil or a value in the map is nil, a generic error will be
	// sent which explains that no processor function exists with the specified
	// name on this server.
	ProcessorFunctionMap() map[string]types.ProcessorFunction
	GetThriftMetadata() *metadata.ThriftMetadata
	FunctionServiceMap() map[string]string
}

func errorType(err error) string {
	// get type name without package or pointer information
	fqet := strings.Replace(fmt.Sprintf("%T", err), "*", "", -1)
	et := strings.Split(fqet, ".")
	return et[len(et)-1]
}

func getProcessorFunction(processor Processor, messageType types.MessageType, name string) (types.ProcessorFunction, error) {
	if messageType != types.CALL && messageType != types.ONEWAY {
		// case one: invalid message type
		return nil, fmt.Errorf("unexpected message type: %d", messageType)
	}
	pmap := processor.ProcessorFunctionMap()
	if pmap != nil {
		if pf := pmap[name]; pf != nil {
			return pf, nil
		}
	}
	return nil, fmt.Errorf("no such function: %q", name)
}

func skipMessage(protocol Protocol) error {
	if err := protocol.Skip(types.STRUCT); err != nil {
		return err
	}
	return protocol.ReadMessageEnd()
}

func setRequestHeadersForError(protocol Protocol, err error) {
	protocol.setRequestHeader("uex", errorType(err))
	protocol.setRequestHeader("uexw", err.Error())
}

func setRequestHeadersForResult(protocol Protocol, result types.WritableStruct) {
	if rr, ok := result.(types.WritableResult); ok && rr.Exception() != nil {
		// If we got a structured exception back, write metadata about it into headers
		terr := rr.Exception()
		protocol.setRequestHeader("uex", errorType(terr))
		protocol.setRequestHeader("uexw", terr.Error())
	}
}

// sendException is a utility function to send the exception for the specified
// method.
func sendException(prot types.Encoder, name string, seqID int32, appEx *types.ApplicationException) error {
	if err := prot.WriteMessageBegin(name, types.EXCEPTION, seqID); err != nil {
		return err
	} else if err := appEx.Write(prot); err != nil {
		return err
	} else if err := prot.WriteMessageEnd(); err != nil {
		return err
	} else if err := prot.Flush(); err != nil {
		return err
	}
	return nil
}

// process is a utility function to take a processor and a protocol, and fully process a message.
// It is broken up into 3 steps:
// 1. Read the message from the protocol.
// 2. Process the message.
// 3. Write the message to the protocol.
func process(ctx context.Context, processor Processor, prot Protocol, processorStats map[string]*stats.TimingSeries, observer ServerObserver) error {
	// Step 1: Decode message only using Decoder interface and GetResponseHeaders method on the protocol.

	// Step 1a: find the processor function for the message.
	name, messageType, seqID, readErr := prot.ReadMessageBegin()
	if readErr != nil {
		// close connection on read failure
		return readErr
	}
	var argStruct types.Struct
	var appException *types.ApplicationException

	pfunc, pfuncErr := getProcessorFunction(processor, messageType, name)
	if pfuncErr != nil {
		appException = NewApplicationException(types.UNKNOWN_METHOD, pfuncErr.Error())
	}

	// Step 1b: finish reading the message.
	if pfunc == nil {
		// attempt to skip the rest of the invalid message but keep the connection open.
		readErr = skipMessage(prot)
		if readErr != nil {
			// close connection on read failure
			return readErr
		}
	} else {
		argStruct, readErr = pfunc.Read(prot)
		if readErr != nil {
			// close connection on read failure
			return readErr
		}
		// Track successful read/deserialization of function arguments
		observer.ReceivedReadForFunction(name)
	}

	// Step 1c: Use Protocol interface to retrieve headers.
	ctx = WithRequestHeaders(ctx, prot.getResponseHeaders())

	// Step 2: Processing the message without using the Protocol.
	var result types.WritableStruct
	var runError error
	if pfunc != nil {
		pfuncStartTime := time.Now()
		result, runError = pfunc.RunContext(ctx, argStruct)
		pfuncDuration := time.Since(pfuncStartTime)
		if timingSeries := processorStats[name]; timingSeries != nil {
			timingSeries.RecordWithStatus(pfuncDuration, runError == nil)
		}
		if runError != nil {
			appException = maybeWrapApplicationException(runError)
		}
		// Record function-level process timing for stats collection
		observer.TimeProcessUsForFunction(name, pfuncDuration)
	}

	// Often times oneway calls do not even have msgType ONEWAY.
	// Then we detect a oneway call with a result that is nil.
	isOneWay := messageType == types.ONEWAY || (appException == nil && result == nil)
	if isOneWay {
		// for ONEWAY messages, never send a response and never throw an exception.
		return nil
	}

	// Step 3: Write the message using only the Encoder interface and SetRequestHeader method.

	// Step 3a: Write the headers to the protocol.
	if appException != nil {
		setRequestHeadersForError(prot, appException)
	} else {
		setRequestHeadersForResult(prot, result)
	}

	// Step 3b: Write the message using only the Decoder interface on the protocol.
	if pfunc == nil {
		if writeErr := sendException(prot, name, seqID, appException); writeErr != nil {
			// close connection on write failure
			return writeErr
		}
		// Track undeclared exception only after successful write
		observer.UndeclaredException()
		observer.UndeclaredExceptionForFunction(name)
		observer.AnyExceptionForFunction(name)
		return nil
	}
	if appException != nil {
		// it's an application generated error, so serialize it to the client
		if writeErr := pfunc.Write(seqID, appException, prot); writeErr != nil {
			// close connection on write failure
			return writeErr
		}
		// Track undeclared exception only after successful write
		observer.UndeclaredException()
		observer.UndeclaredExceptionForFunction(name)
		observer.AnyExceptionForFunction(name)
	} else {
		if writeErr := pfunc.Write(seqID, result, prot); writeErr != nil {
			// close connection on write failure
			return writeErr
		}
		// For REPLY, check for declared exceptions via "uex" header for structured exceptions
		if protocolBuffer, ok := prot.(*protocolBuffer); ok {
			if uexHeader := protocolBuffer.getRequestHeaders()["uex"]; uexHeader != "" {
				observer.DeclaredException()
				observer.AnyExceptionForFunction(name)
			}
		}
	}
	// if we got here, we successfully processed the message
	return nil
}
