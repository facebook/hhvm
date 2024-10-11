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
	"github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// These are temporary aliases to the types package. They will be removed or kept, but that is to be determined in future diffs as we refactor.

type SerialChannel = types.SerialChannel

type RequestChannel = types.RequestChannel

type Protocol = types.Protocol

type ClientInterface = types.ClientInterface

type ProcessorFunction = types.ProcessorFunction

type Decoder = types.Decoder

type Struct = types.Struct

type Exception = types.Exception

type WritableStruct = types.WritableStruct

type Encoder = types.Encoder

type ApplicationException = types.ApplicationException

type Type = types.Type

type IRequest = types.IRequest

type IResponse = types.IResponse

type FormatID = types.ProtocolID

type Format = types.Format

type MessageType = types.MessageType

type WritableResult = types.WritableResult

type TransportException = types.TransportException

type ResponseHeaderGetter = types.ResponseHeaderGetter

type RequestHeaders = types.RequestHeaders

type FormatException = types.ProtocolException

var ZERO types.Numeric = types.ZERO

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

var NewApplicationExceptionCause = types.NewApplicationExceptionCause
var NewApplicationException = types.NewApplicationException

var INTERNAL_ERROR int32 = types.INTERNAL_ERROR
var UNKNOWN_METHOD int32 = types.UNKNOWN_METHOD
var WRONG_METHOD_NAME int32 = types.WRONG_METHOD_NAME
var UNKNOWN_APPLICATION_EXCEPTION int32 = types.UNKNOWN_APPLICATION_EXCEPTION
var INVALID_MESSAGE_TYPE_EXCEPTION int32 = types.INVALID_MESSAGE_TYPE_EXCEPTION

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

func PrependError(prepend string, err error) error {
	return types.PrependError(prepend, err)
}

func NewSerialChannel(protocol types.Protocol) *types.SerialChannel {
	return types.NewSerialChannel(protocol)
}

func Float32Ptr(v float32) *float32 { return &v }
func Float64Ptr(v float64) *float64 { return &v }
func IntPtr(v int) *int             { return &v }
func Int8Ptr(v int8) *int8          { return &v }
func Int16Ptr(v int16) *int16       { return &v }
func Int32Ptr(v int32) *int32       { return &v }
func Int64Ptr(v int64) *int64       { return &v }
func StringPtr(v string) *string    { return &v }
func Uint8Ptr(v uint8) *uint8       { return &v }
func Uint16Ptr(v uint16) *uint16    { return &v }
func Uint32Ptr(v uint32) *uint32    { return &v }
func Uint64Ptr(v uint64) *uint64    { return &v }
func BoolPtr(v bool) *bool          { return &v }
func BytePtr(v byte) *byte          { return &v }
func ByteSlicePtr(v []byte) *[]byte { return &v }

func NewTransportException(t int, msg string) types.TransportException {
	return types.NewTransportException(t, msg)
}

func NewFormatException(err error) FormatException {
	return types.NewProtocolException(err)
}

func NewTransportExceptionFromError(err error) types.TransportException {
	return types.NewTransportExceptionFromError(err)
}
