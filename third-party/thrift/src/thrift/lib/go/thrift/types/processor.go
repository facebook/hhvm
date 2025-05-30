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

// ProcessorFunction is the interface that must be implemented in
// order to perform io and message processing
type ProcessorFunction interface {
	// Read a serializable message from the protocol.
	Read(prot Decoder) (Struct, error)
	// RunContext processes a message handing it to the client handler.
	RunContext(ctx context.Context, args Struct) (WritableStruct, ApplicationExceptionIf)
	// Write a serializable response
	Write(seqID int32, result WritableStruct, prot Encoder) error
}
