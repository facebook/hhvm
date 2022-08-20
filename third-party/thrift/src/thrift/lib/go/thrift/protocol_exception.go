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
	"encoding/base64"
)

// ProtocolException is the thrift protocol exception
type ProtocolException interface {
	Exception
	TypeID() int
}

const (
	UNKNOWN_PROTOCOL_EXCEPTION = 0
	INVALID_DATA               = 1
	NEGATIVE_SIZE              = 2
	SIZE_LIMIT                 = 3
	BAD_VERSION                = 4
	NOT_IMPLEMENTED            = 5
	DEPTH_LIMIT                = 6
)

type protocolException struct {
	typeID  int
	message string
}

func (p *protocolException) TypeID() int {
	return p.typeID
}

func (p *protocolException) String() string {
	return p.message
}

func (p *protocolException) Error() string {
	return p.message
}

// NewProtocolException creates a new ProtocolException
func NewProtocolException(err error) ProtocolException {
	if err == nil {
		return nil
	}
	if e, ok := err.(ProtocolException); ok {
		return e
	}
	if _, ok := err.(base64.CorruptInputError); ok {
		return &protocolException{INVALID_DATA, err.Error()}
	}
	return &protocolException{UNKNOWN_PROTOCOL_EXCEPTION, err.Error()}
}

// NewProtocolExceptionWithType create a new ProtocolException with an error type
func NewProtocolExceptionWithType(errType int, err error) ProtocolException {
	if err == nil {
		return nil
	}
	return &protocolException{errType, err.Error()}
}
