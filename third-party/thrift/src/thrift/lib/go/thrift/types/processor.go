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

package types

import (
	"context"
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
	ProcessorFunctionMap() map[string]ProcessorFunction
}

// ProcessorFunction is the interface that must be implemented in
// order to perform io and message processing
type ProcessorFunction interface {
	// Read a serializable message from the protocol.
	Read(prot Decoder) (Struct, Exception)
	// RunContext processes a message handing it to the client handler.
	RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationException)
	// Write a serializable response
	Write(seqID int32, result WritableStruct, prot Encoder) Exception
}
