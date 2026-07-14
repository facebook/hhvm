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
	"encoding/base64"
)

const (
	UNKNOWN_PROTOCOL_EXCEPTION = 0
	INVALID_DATA               = 1
	NEGATIVE_SIZE              = 2
	SIZE_LIMIT                 = 3
	BAD_VERSION                = 4
	NOT_IMPLEMENTED            = 5
	DEPTH_LIMIT                = 6
)

type ProtocolException struct {
	typeID  int
	message string
}

func (p *ProtocolException) TypeID() int {
	return p.typeID
}

func (p *ProtocolException) String() string {
	return p.message
}

func (p *ProtocolException) Error() string {
	return p.message
}

// NewProtocolException creates a new ProtocolException
func NewProtocolException(err error) error {
	if err == nil {
		return nil
	}
	if protoEx, ok := err.(*ProtocolException); ok {
		return protoEx
	}
	if _, ok := err.(base64.CorruptInputError); ok {
		return &ProtocolException{INVALID_DATA, err.Error()}
	}
	return &ProtocolException{UNKNOWN_PROTOCOL_EXCEPTION, err.Error()}
}

// NewProtocolExceptionWithType create a new ProtocolException with an error type
func NewProtocolExceptionWithType(errType int, err error) error {
	if err == nil {
		return nil
	}
	return &ProtocolException{errType, err.Error()}
}
