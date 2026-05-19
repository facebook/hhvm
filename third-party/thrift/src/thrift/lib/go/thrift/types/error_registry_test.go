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

// Mock error type for testing.
type mockRegistryError struct {
	msg string
}

func (e *mockRegistryError) Error() string {
	return e.msg
}

// A second mock error type to verify per-type isolation.
type anotherMockRegistryError struct{}

func (e *anotherMockRegistryError) Error() string {
	return "another"
}

// An error type intentionally left unregistered.
type unregisteredMockError struct{}

func (e *unregisteredMockError) Error() string {
	return "unregistered"
}

func TestRegisterAndGetError(t *testing.T) {
	InternalRegisterError(
		(*mockRegistryError)(nil),
		"module.MockRegistryError",
		ErrorBlameServer,
		ErrorKindTransient,
		ErrorSafetySafe,
	)

	spec := InternalErrorRegistryGet(&mockRegistryError{msg: "boom"})
	require.NotNil(t, spec)
	require.Equal(t, "module.MockRegistryError", spec.TypeName)
	require.Equal(t, ErrorBlameServer, spec.Blame)
	require.Equal(t, ErrorKindTransient, spec.Kind)
	require.Equal(t, ErrorSafetySafe, spec.Safety)
}

func TestGetErrorNotRegistered(t *testing.T) {
	require.Nil(t, InternalErrorRegistryGet(&unregisteredMockError{}))
}

func TestGetErrorNilInput(t *testing.T) {
	// Nil error has no reflect.Type entry in the registry.
	require.Nil(t, InternalErrorRegistryGet(nil))
}

func TestRegisterMultipleErrors(t *testing.T) {
	InternalRegisterError(
		(*mockRegistryError)(nil),
		"module.MockRegistryError",
		ErrorBlameClient,
		ErrorKindPermanent,
		ErrorSafetyUnspecified,
	)
	InternalRegisterError(
		(*anotherMockRegistryError)(nil),
		"module.AnotherMockRegistryError",
		ErrorBlameServer,
		ErrorKindStateful,
		ErrorSafetySafe,
	)

	spec1 := InternalErrorRegistryGet(&mockRegistryError{})
	require.NotNil(t, spec1)
	require.Equal(t, "module.MockRegistryError", spec1.TypeName)
	require.Equal(t, ErrorBlameClient, spec1.Blame)
	require.Equal(t, ErrorKindPermanent, spec1.Kind)
	require.Equal(t, ErrorSafetyUnspecified, spec1.Safety)

	spec2 := InternalErrorRegistryGet(&anotherMockRegistryError{})
	require.NotNil(t, spec2)
	require.Equal(t, "module.AnotherMockRegistryError", spec2.TypeName)
	require.Equal(t, ErrorBlameServer, spec2.Blame)
	require.Equal(t, ErrorKindStateful, spec2.Kind)
	require.Equal(t, ErrorSafetySafe, spec2.Safety)
}

func TestRegisterOverwritesExistingError(t *testing.T) {
	InternalRegisterError(
		(*mockRegistryError)(nil),
		"old.Name",
		ErrorBlameServer,
		ErrorKindTransient,
		ErrorSafetySafe,
	)
	InternalRegisterError(
		(*mockRegistryError)(nil),
		"new.Name",
		ErrorBlameClient,
		ErrorKindPermanent,
		ErrorSafetyUnspecified,
	)

	spec := InternalErrorRegistryGet(&mockRegistryError{})
	require.NotNil(t, spec)
	require.Equal(t, "new.Name", spec.TypeName)
	require.Equal(t, ErrorBlameClient, spec.Blame)
	require.Equal(t, ErrorKindPermanent, spec.Kind)
	require.Equal(t, ErrorSafetyUnspecified, spec.Safety)
}

func TestGetErrorDoesNotUnwrap(t *testing.T) {
	InternalRegisterError(
		(*mockRegistryError)(nil),
		"module.MockRegistryError",
		ErrorBlameServer,
		ErrorKindTransient,
		ErrorSafetySafe,
	)

	// A wrapped error has a different concrete type and should not match.
	wrapped := errors.Join(&mockRegistryError{msg: "inner"})
	require.Nil(t, InternalErrorRegistryGet(wrapped))
}
