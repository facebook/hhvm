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
	"errors"
	"testing"

	"github.com/stretchr/testify/require"
)

func TestPrependError(t *testing.T) {
	err := NewApplicationException(INTERNAL_ERROR, "original error")
	err2 := PrependError("Prepend: ", err)
	var appErr *ApplicationException
	require.ErrorAs(t, err2, &appErr)
	require.Equal(t, "Prepend: original error", appErr.Error())
	require.EqualValues(t, INTERNAL_ERROR, appErr.TypeID())

	err3 := NewProtocolExceptionWithType(INVALID_DATA, errors.New("original error"))
	err4 := PrependError("Prepend: ", err3)
	var protErr *protocolException
	require.ErrorAs(t, err4, &protErr)
	require.Equal(t, "Prepend: original error", protErr.Error())
	require.EqualValues(t, INVALID_DATA, protErr.TypeID())

	err5 := NewTransportException(TIMED_OUT, "original error")
	err6 := PrependError("Prepend: ", err5)
	var transErr *transportException
	require.ErrorAs(t, err6, &transErr)
	require.Equal(t, "Prepend: original error", transErr.Error())
	require.EqualValues(t, TIMED_OUT, transErr.TypeID())

	err7 := errors.New("original error")
	err8 := PrependError("Prepend: ", err7)
	require.Equal(t, "Prepend: original error", err8.Error())
}
