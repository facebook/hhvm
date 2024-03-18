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

// ApplicationException is an application level Thrift exception
type ApplicationException interface {
	Exception
	TypeID() int32
	Read(iprot Format) (ApplicationException, error)
	Write(oprot Format) error
}

type applicationException struct {
	message       string
	exceptionType int32
	// The original error that caused the application exception
	cause error
}

func (e applicationException) Error() string {
	return e.message
}

// NewApplicationException creates a new ApplicationException
func NewApplicationException(exceptionType int32, message string) ApplicationException {
	return &applicationException{message: message, exceptionType: exceptionType}
}

// NewApplicationExceptionCause creates a new ApplicationException with a root cause error
func NewApplicationExceptionCause(exceptionType int32, message string, cause error) ApplicationException {
	return &applicationException{message, exceptionType, cause}
}

// TypeID returns the exception type
func (e *applicationException) TypeID() int32 {
	return e.exceptionType
}

// Unwrap returns the original error that cause the application error. Returns nil if there was no wrapped error.
func (e *applicationException) Unwrap() error {
	return e.cause
}

// Read reads an ApplicationException from the protocol
func (e *applicationException) Read(iprot Format) (ApplicationException, error) {
	_, err := iprot.ReadStructBegin()
	if err != nil {
		return nil, err
	}

	message := ""
	exceptionType := int32(UNKNOWN_APPLICATION_EXCEPTION)

	for {
		_, ttype, id, err := iprot.ReadFieldBegin()
		if err != nil {
			return nil, err
		}
		if ttype == STOP {
			break
		}
		switch id {
		case 1:
			if ttype == STRING {
				if message, err = iprot.ReadString(); err != nil {
					return nil, err
				}
			} else {
				if err = SkipDefaultDepth(iprot, ttype); err != nil {
					return nil, err
				}
			}
		case 2:
			if ttype == I32 {
				if exceptionType, err = iprot.ReadI32(); err != nil {
					return nil, err
				}
			} else {
				if err = SkipDefaultDepth(iprot, ttype); err != nil {
					return nil, err
				}
			}
		default:
			if err = SkipDefaultDepth(iprot, ttype); err != nil {
				return nil, err
			}
		}
		if err = iprot.ReadFieldEnd(); err != nil {
			return nil, err
		}
	}
	return NewApplicationException(exceptionType, message), iprot.ReadStructEnd()
}

// Write writes an exception to the protocol
func (e *applicationException) Write(oprot Format) (err error) {
	err = oprot.WriteStructBegin("TApplicationException")
	if len(e.Error()) > 0 {
		err = oprot.WriteFieldBegin("message", STRING, 1)
		if err != nil {
			return
		}
		err = oprot.WriteString(e.Error())
		if err != nil {
			return
		}
		err = oprot.WriteFieldEnd()
		if err != nil {
			return
		}
	}
	err = oprot.WriteFieldBegin("type", I32, 2)
	if err != nil {
		return
	}
	err = oprot.WriteI32(e.exceptionType)
	if err != nil {
		return
	}
	err = oprot.WriteFieldEnd()
	if err != nil {
		return
	}
	err = oprot.WriteFieldStop()
	if err != nil {
		return
	}
	err = oprot.WriteStructEnd()
	return
}

// sendException is a utility function to send the exception for the specified
// method.
func sendException(oprot Format, name string, seqID int32, err ApplicationException) error {
	if e2 := oprot.WriteMessageBegin(name, EXCEPTION, seqID); e2 != nil {
		return e2
	} else if e2 := err.Write(oprot); e2 != nil {
		return e2
	} else if e2 := oprot.WriteMessageEnd(); e2 != nil {
		return e2
	} else if e2 := oprot.Flush(); e2 != nil {
		return e2
	}
	return nil
}
