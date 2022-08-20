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
	"testing"
)

func TestApplicationException(t *testing.T) {
	exc := NewApplicationException(UNKNOWN_APPLICATION_EXCEPTION, "")
	if exc.Error() != "" {
		t.Fatalf("Expected empty string for exception but found '%s'", exc.Error())
	}
	if exc.TypeID() != UNKNOWN_APPLICATION_EXCEPTION {
		t.Fatalf("Expected type UNKNOWN for exception but found '%d'", exc.TypeID())
	}
	exc = NewApplicationException(WRONG_METHOD_NAME, "junk_method")
	if exc.Error() != "junk_method" {
		t.Fatalf("Expected 'junk_method' for exception but found '%s'", exc.Error())
	}
	if exc.TypeID() != WRONG_METHOD_NAME {
		t.Fatalf("Expected type WRONG_METHOD_NAME for exception but found '%d'", exc.TypeID())
	}
}

func TestApplicationExceptionCause(t *testing.T) {
	err := fmt.Errorf("A specific error")
	exc := NewApplicationExceptionCause(INTERNAL_ERROR, err.Error(), err)

	if exc.Error() != "A specific error" {
		t.Fatalf("Expected 'A specific error' for exception but found '%s'", exc.Error())
	}

	if errors.Unwrap(exc) != err {
		t.Fatalf("Expected inner error from exception but found '%s'", errors.Unwrap(exc))
	}
}
