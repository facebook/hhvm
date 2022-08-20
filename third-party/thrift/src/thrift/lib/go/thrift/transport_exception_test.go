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
	"fmt"
	"io"

	"testing"
)

type timeout struct{ timedout bool }

func (t *timeout) Timeout() bool {
	return t.timedout
}

func (t *timeout) Error() string {
	return fmt.Sprintf("Timeout: %v", t.timedout)
}

func TestExceptionTimeout(t *testing.T) {
	timeout := &timeout{true}
	exception := NewTransportExceptionFromError(timeout)
	if timeout.Error() != exception.Error() {
		t.Fatalf("Error did not match: expected %q, got %q", timeout.Error(), exception.Error())
	}

	if exception.TypeID() != TIMED_OUT {
		t.Fatalf("TypeID was not TIMED_OUT: expected %v, got %v", TIMED_OUT, exception.TypeID())
	}
}

func TestExceptionEOF(t *testing.T) {
	exception := NewTransportExceptionFromError(io.EOF)
	if io.EOF.Error() != exception.Error() {
		t.Fatalf("Error did not match: expected %q, got %q", io.EOF.Error(), exception.Error())
	}

	if exception.TypeID() != END_OF_FILE {
		t.Fatalf("TypeID was not END_OF_FILE: expected %v, got %v", END_OF_FILE, exception.TypeID())
	}
}

func TestTransportExceptionUnwrap(t *testing.T) {
	origErr := errors.New("unit-test")
	exception := &transportException{
		err: origErr,
	}
	if !errors.Is(exception, origErr) {
		t.Fatalf("exception %T:%v wasn't unwrapped to %T:%v",
			exception, exception, origErr, origErr)
	}
}
