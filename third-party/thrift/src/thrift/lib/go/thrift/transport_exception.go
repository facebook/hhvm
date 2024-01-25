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
	"errors"
	"io"
)

type timeoutable interface {
	Timeout() bool
}

// TransportException is the interface for transport exceptions
type TransportException interface {
	Exception
	TypeID() int
	Err() error
}

const (
	UNKNOWN_TRANSPORT_EXCEPTION = 0
	NOT_OPEN                    = 1
	ALREADY_OPEN                = 2
	TIMED_OUT                   = 3
	END_OF_FILE                 = 4
	INTERRUPTED                 = 5
	BAD_ARGS                    = 6
	CORRUPTED_DATA              = 7
	NOT_SUPPORTED               = 9
	INVALID_STATE               = 10
	INVALID_FRAME_SIZE          = 11
	SSL_ERROR                   = 12
	COULD_NOT_BIND              = 13
	NETWORK_ERROR               = 15
	INVALID_HEADERS_TYPE        = 16
)

type transportException struct {
	typeID int
	err    error
}

func (p *transportException) TypeID() int {
	return p.typeID
}

func (p *transportException) Error() string {
	return p.err.Error()
}

// Unwrap is used for errors.Unwrap.
func (p *transportException) Unwrap() error {
	return p.err
}

func (p *transportException) Err() error {
	return p.err
}

// NewTransportException creates a new TransportException
func NewTransportException(t int, e string) TransportException {
	return &transportException{typeID: t, err: errors.New(e)}
}

// NewTransportExceptionFromError creates a TransportException from an error
func NewTransportExceptionFromError(e error) TransportException {
	if e == nil {
		return nil
	}

	if t, ok := e.(TransportException); ok {
		return t
	}

	switch v := e.(type) {
	case TransportException:
		return v
	case timeoutable:
		if v.Timeout() {
			return &transportException{typeID: TIMED_OUT, err: e}
		}
	}

	if e == io.EOF {
		return &transportException{typeID: END_OF_FILE, err: e}
	}

	return &transportException{typeID: UNKNOWN_TRANSPORT_EXCEPTION, err: e}
}
