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
	types.Processor
	GetThriftMetadata() *metadata.ThriftMetadata
}

func errorType(err error) string {
	// get type name without package or pointer information
	fqet := strings.ReplaceAll(fmt.Sprintf("%T", err), "*", "")
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

func sendWritableStruct(prot types.Encoder, name string, messageType types.MessageType, seqID int32, strct WritableStruct) error {
	if err := prot.WriteMessageBegin(name, messageType, seqID); err != nil {
		return err
	}
	if err := strct.Write(prot); err != nil {
		return err
	}
	if err := prot.WriteMessageEnd(); err != nil {
		return err
	}
	return prot.Flush()
}

// process is a utility function to take a processor and a protocol, and fully process a message.
// It is broken up into 3 steps:
// 1. Read the message from the protocol.
// 2. Process the message.
// 3. Write the message to the protocol.
func process(ctx context.Context, processor Processor, prot Protocol, processorStats map[string]*stats.TimingSeries, observer ServerObserver) (*ApplicationException, error) {
	// Step 1: Decode message only using Decoder interface and GetResponseHeaders method on the protocol.

	// Step 1a: find the processor function for the message.
	name, messageType, seqID, readErr := prot.ReadMessageBegin()
	if readErr != nil {
		// close connection on read failure
		return nil, readErr
	}
	var argStruct types.ReadableStruct
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
			return nil, readErr
		}
	} else {
		argStruct = pfunc.NewReqArgs()
		if readErr := argStruct.Read(prot); readErr != nil {
			// close connection on read failure
			return nil, readErr
		}
		if readErr := prot.ReadMessageEnd(); readErr != nil {
			// close connection on read failure
			return nil, readErr
		}
	}

	// Step 1c: Use Protocol interface to retrieve headers.
	requestHeaders := prot.getResponseHeaders()
	reqCtx := RequestContext{
		Method:     name,
		SequenceID: seqID,
	}
	reqCtx.SetReadHeaders(requestHeaders)
	ctx = WithRequestContext(ctx, &reqCtx)

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
		// Record function-level process timing for stats collection
		observer.TimeProcessUsForFunction(name, pfuncDuration)
		if runError != nil {
			appException = maybeWrapApplicationException(runError)
		}
	}

	// Often times oneway calls do not even have msgType ONEWAY.
	// Then we detect a oneway call with a result that is nil.
	isOneWay := messageType == types.ONEWAY || (appException == nil && result == nil)
	if isOneWay {
		// for ONEWAY messages, never send a response and never throw an exception.
		return nil, nil
	}

	// Step 3: Write the message using only the Encoder interface and SetRequestHeader method.

	// Step 3a: Write the headers to the protocol.
	if appException != nil {
		// ApplicationException (a.k.a. undeclared exception)
		setRequestHeadersForError(prot, appException)
	} else if wr, ok := result.(types.WritableResult); ok && wr.Exception() != nil {
		// Declared exception
		setRequestHeadersForError(prot, wr.Exception())
	}

	// Step 3b: Write the message using only the Decoder interface on the protocol.
	if appException != nil {
		// it's an application generated error, so serialize it to the client
		if writeErr := sendWritableStruct(prot, name, types.EXCEPTION, seqID, appException); writeErr != nil {
			// close connection on write failure
			return nil, writeErr
		}
		// Track undeclared exception only after successful write
		observer.UndeclaredException()
		observer.AnyExceptionForFunction(name)
	} else {
		if writeErr := sendWritableStruct(prot, name, types.REPLY, seqID, result); writeErr != nil {
			// close connection on write failure
			return nil, writeErr
		}
		// Track declared exceptions
		if wr, ok := result.(types.WritableResult); ok && wr.Exception() != nil {
			observer.DeclaredException()
			observer.AnyExceptionForFunction(name)
		}
	}
	// if we got here, we successfully processed the message
	return appException, nil
}
