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
	"io"

	"github.com/facebook/fbthrift/thrift/lib/go/thrift/format"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/rocket"
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// These are temporary aliases to the types package. They will be removed or kept, but that is to be determined in future diffs as we refactor.

type RPCOptions = types.RPCOptions

type Priority = types.Priority

type contextHeaders = types.ContextHeaders

type RequestChannel = types.RequestChannel

type ProcessorFunction = types.ProcessorFunction

type Decoder = types.Decoder

type Struct = types.Struct

type WritableStruct = types.WritableStruct

type ReadableStruct = types.ReadableStruct

type Encoder = types.Encoder

type ApplicationException = types.ApplicationException

type Type = types.Type

type FormatID = types.ProtocolID

type Format = types.Format

type MessageType = types.MessageType

type WritableResult = types.WritableResult

type ReadableResult = types.ReadableResult

type TransportException = types.TransportException

type FormatException = types.ProtocolException

type RocketException = rocket.RocketException

const (
	RocketExceptionUnknown         = rocket.RocketExceptionUnknown
	RocketExceptionDeclared        = rocket.RocketExceptionDeclared
	RocketExceptionAppUnknown      = rocket.RocketExceptionAppUnknown
	RocketExceptionAny             = rocket.RocketExceptionAny
	RocketExceptionDeprecatedProxy = rocket.RocketExceptionDeprecatedProxy
)

var REPLY types.MessageType = types.REPLY
var CALL types.MessageType = types.CALL
var EXCEPTION types.MessageType = types.EXCEPTION
var ONEWAY types.MessageType = types.ONEWAY

var FormatIDBinary types.ProtocolID = types.ProtocolIDBinary
var FormatIDCompact types.ProtocolID = types.ProtocolIDCompact
var FormatIDJSON types.ProtocolID = types.ProtocolIDJSON
var FormatIDDebug types.ProtocolID = types.ProtocolIDDebug
var FormatIDVirtual types.ProtocolID = types.ProtocolIDVirtual
var FormatIDSimpleJSON types.ProtocolID = types.ProtocolIDSimpleJSON

var NewApplicationException = types.NewApplicationException

var INTERNAL_ERROR int32 = types.INTERNAL_ERROR
var UNKNOWN_METHOD int32 = types.UNKNOWN_METHOD
var WRONG_METHOD_NAME int32 = types.WRONG_METHOD_NAME
var UNKNOWN_APPLICATION_EXCEPTION int32 = types.UNKNOWN_APPLICATION_EXCEPTION
var INVALID_MESSAGE_TYPE_EXCEPTION int32 = types.INVALID_MESSAGE_TYPE_EXCEPTION
var LOADSHEDDING int32 = types.LOADSHEDDING

var END_OF_FILE int = types.END_OF_FILE
var UNKNOWN_TRANSPORT_EXCEPTION int = types.UNKNOWN_TRANSPORT_EXCEPTION

var STOP Type = types.STOP
var VOID Type = types.VOID
var BOOL Type = types.BOOL
var BYTE Type = types.BYTE
var DOUBLE Type = types.DOUBLE
var I16 Type = types.I16
var I32 Type = types.I32
var I64 Type = types.I64
var STRING Type = types.STRING
var STRUCT Type = types.STRUCT
var MAP Type = types.MAP
var SET Type = types.SET
var LIST Type = types.LIST
var UTF8 Type = types.UTF8
var UTF16 Type = types.UTF16
var STREAM Type = types.STREAM
var FLOAT Type = types.FLOAT

// Pointerize returns a pointer to the given value.
func Pointerize[T types.ThriftPointerizable](v T) *T {
	return types.Pointerize(v)
}

func NewCompactFormat(readWriter types.ReadWriteSizer) types.Format {
	return format.NewCompactFormat(readWriter)
}

func NewBinaryFormat(readWriter types.ReadWriteSizer) types.Format {
	return format.NewBinaryFormat(readWriter)
}

func NewSimpleJSONFormat(readWriter io.ReadWriter) types.Format {
	return format.NewSimpleJSONFormat(readWriter)
}

func EncodeCompact(msg types.WritableStruct) ([]byte, error) {
	return format.EncodeCompact(msg)
}

func EncodeBinary(msg types.WritableStruct) ([]byte, error) {
	return format.EncodeBinary(msg)
}

func EncodeCompactJSON(msg types.WritableStruct) ([]byte, error) {
	return format.EncodeCompactJSON(msg)
}

func EncodeSimpleJSON(msg types.WritableStruct) ([]byte, error) {
	return format.EncodeSimpleJSON(msg)
}

func EncodeSimpleJSONV2(msg types.WritableStruct) ([]byte, error) {
	return format.EncodeSimpleJSONV2(msg)
}

func DecodeCompact(data []byte, msg types.ReadableStruct) error {
	return format.DecodeCompact(data, msg)
}

func DecodeBinary(data []byte, msg types.ReadableStruct) error {
	return format.DecodeBinary(data, msg)
}

func DecodeCompactJSON(data []byte, msg types.ReadableStruct) error {
	return format.DecodeCompactJSON(data, msg)
}

func DecodeSimpleJSON(data []byte, msg types.ReadableStruct) error {
	return format.DecodeSimpleJSON(data, msg)
}

func DecodeSimpleJSONV2(data []byte, msg types.ReadableStruct) error {
	return format.DecodeSimpleJSONV2(data, msg)
}

func WithFrameworkMetadata(ctx context.Context, metadata []byte) context.Context {
	return rocket.WithFrameworkMetadata(ctx, metadata)
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalRegisterClientConstructor registers a constructor function for a client type.
// This is intended to be called from generated code's init() function.
func InternalRegisterClientConstructor[T io.Closer](constructor func(channel RequestChannel) T) {
	types.InternalRegisterClientConstructor(constructor)
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalConstructClientFromRegistry creates a client of type T from a channel using the registered constructor.
// Returns the client or an error if no constructor was found for the type.
func InternalConstructClientFromRegistry[T io.Closer](channel RequestChannel) (T, error) {
	return types.InternalConstructClientFromRegistry[T](channel)
}
