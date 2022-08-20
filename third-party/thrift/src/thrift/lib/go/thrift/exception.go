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
	"errors"
)

// Exception is a generic thrift exception
type Exception interface {
	error
}

// PrependError prepends additional information to an error without losing the thrift exception interface
func PrependError(prepend string, err error) error {
	if t, ok := err.(TransportException); ok {
		return NewTransportException(t.TypeID(), prepend+t.Error())
	}
	if t, ok := err.(ProtocolException); ok {
		return NewProtocolExceptionWithType(t.TypeID(), errors.New(prepend+err.Error()))
	}
	if t, ok := err.(ApplicationException); ok {
		return NewApplicationException(t.TypeID(), prepend+t.Error())
	}

	return errors.New(prepend + err.Error())
}
