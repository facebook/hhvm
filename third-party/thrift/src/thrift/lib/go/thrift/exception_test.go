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
	"testing"
)

func TestPrependError(t *testing.T) {
	err := NewApplicationException(INTERNAL_ERROR, "original error")
	err2, ok := PrependError("Prepend: ", err).(ApplicationException)
	if !ok {
		t.Fatal("Couldn't cast error ApplicationException")
	}
	if err2.Error() != "Prepend: original error" {
		t.Fatal("Unexpected error string")
	}
	if err2.TypeID() != INTERNAL_ERROR {
		t.Fatal("Unexpected type error")
	}

	err3 := NewProtocolExceptionWithType(INVALID_DATA, errors.New("original error"))
	err4, ok := PrependError("Prepend: ", err3).(ProtocolException)
	if !ok {
		t.Fatal("Couldn't cast error ProtocolException")
	}
	if err4.Error() != "Prepend: original error" {
		t.Fatal("Unexpected error string")
	}
	if err4.TypeID() != INVALID_DATA {
		t.Fatal("Unexpected type error")
	}

	err5 := NewTransportException(TIMED_OUT, "original error")
	err6, ok := PrependError("Prepend: ", err5).(TransportException)
	if !ok {
		t.Fatal("Couldn't cast error TransportException")
	}
	if err6.Error() != "Prepend: original error" {
		t.Fatal("Unexpected error string")
	}
	if err6.TypeID() != TIMED_OUT {
		t.Fatal("Unexpected type error")
	}

	err7 := errors.New("original error")
	err8 := PrependError("Prepend: ", err7)
	if err8.Error() != "Prepend: original error" {
		t.Fatal("Unexpected error string")
	}
}
