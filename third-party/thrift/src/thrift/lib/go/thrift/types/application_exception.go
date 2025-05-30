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

const (
	UNKNOWN_APPLICATION_EXCEPTION  = 0
	UNKNOWN_METHOD                 = 1
	INVALID_MESSAGE_TYPE_EXCEPTION = 2
	WRONG_METHOD_NAME              = 3
	BAD_SEQUENCE_ID                = 4
	MISSING_RESULT                 = 5
	INTERNAL_ERROR                 = 6
	PROTOCOL_ERROR                 = 7
)

// ApplicationExceptionIf is an application level Thrift exception
// TODO: to be deprecated and removed.
type ApplicationExceptionIf interface {
	error
	Struct
	TypeID() int32
}

// ApplicationException is an application level Thrift exception
type ApplicationException struct {
	message       string
	exceptionType int32
	// The original error that caused the application exception
	cause error
}

var _ error = (*ApplicationException)(nil)
var _ Struct = (*ApplicationException)(nil)

// NewApplicationException creates a new ApplicationException
func NewApplicationException(exceptionType int32, message string) *ApplicationException {
	return &ApplicationException{message: message, exceptionType: exceptionType}
}

// NewApplicationExceptionCause creates a new ApplicationException with a root cause error
func NewApplicationExceptionCause(exceptionType int32, message string, cause error) *ApplicationException {
	return &ApplicationException{message, exceptionType, cause}
}

// Error returns the error message
func (e *ApplicationException) Error() string {
	return e.message
}

// TypeID returns the exception type
func (e *ApplicationException) TypeID() int32 {
	return e.exceptionType
}

// Unwrap returns the original error that cause the application error. Returns nil if there was no wrapped error.
func (e *ApplicationException) Unwrap() error {
	return e.cause
}

// Read reads an ApplicationException from the protocol
func (e *ApplicationException) Read(prot Decoder) error {
	_, err := prot.ReadStructBegin()
	if err != nil {
		return err
	}

	message := ""
	exceptionType := int32(UNKNOWN_APPLICATION_EXCEPTION)

	for {
		_, ttype, id, err := prot.ReadFieldBegin()
		if err != nil {
			return err
		}
		if ttype == STOP {
			break
		}
		switch id {
		case 1:
			if ttype == STRING {
				if message, err = prot.ReadString(); err != nil {
					return err
				}
			} else {
				if err = SkipDefaultDepth(prot, ttype); err != nil {
					return err
				}
			}
		case 2:
			if ttype == I32 {
				if exceptionType, err = prot.ReadI32(); err != nil {
					return err
				}
			} else {
				if err = SkipDefaultDepth(prot, ttype); err != nil {
					return err
				}
			}
		default:
			if err = SkipDefaultDepth(prot, ttype); err != nil {
				return err
			}
		}
		if err = prot.ReadFieldEnd(); err != nil {
			return err
		}
	}

	if err := prot.ReadStructEnd(); err != nil {
		return err
	}

	e.message = message
	e.exceptionType = exceptionType
	return nil
}

// Write writes an exception to the protocol
func (e *ApplicationException) Write(prot Encoder) error {
	err := prot.WriteStructBegin("TApplicationException")
	if err != nil {
		return err
	}
	if len(e.Error()) > 0 {
		err = prot.WriteFieldBegin("message", STRING, 1)
		if err != nil {
			return err
		}
		err = prot.WriteString(e.Error())
		if err != nil {
			return err
		}
		err = prot.WriteFieldEnd()
		if err != nil {
			return err
		}
	}
	err = prot.WriteFieldBegin("type", I32, 2)
	if err != nil {
		return err
	}
	err = prot.WriteI32(e.exceptionType)
	if err != nil {
		return err
	}
	err = prot.WriteFieldEnd()
	if err != nil {
		return err
	}
	err = prot.WriteFieldStop()
	if err != nil {
		return err
	}
	return prot.WriteStructEnd()
}
