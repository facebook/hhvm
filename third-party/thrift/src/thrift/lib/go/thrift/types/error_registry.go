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
	"reflect"
	"sync"
)

// ErrorBlame indicates which party is at fault for an error.
// Mirrors t_error_blame in the Thrift compiler.
type ErrorBlame int

const (
	ErrorBlameUnspecified ErrorBlame = iota
	ErrorBlameServer
	ErrorBlameClient
)

// ErrorKind indicates the retry semantics of an error.
// Mirrors t_error_kind in the Thrift compiler.
type ErrorKind int

const (
	ErrorKindUnspecified ErrorKind = iota
	ErrorKindTransient
	ErrorKindStateful
	ErrorKindPermanent
)

// ErrorSafety indicates whether retrying is safe (no side effects observed).
// Mirrors t_error_safety in the Thrift compiler.
type ErrorSafety int

const (
	ErrorSafetyUnspecified ErrorSafety = iota
	ErrorSafetySafe
)

// ErrorSpec describes the static metadata associated with a Thrift exception type.
type ErrorSpec struct {
	TypeName string
	Blame    ErrorBlame
	Kind     ErrorKind
	Safety   ErrorSafety
}

var (
	errorRegistryMu sync.RWMutex
	errorRegistry   = make(map[reflect.Type]*ErrorSpec)
)

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalRegisterError registers static metadata for a Thrift exception type.
// This is intended to be called from generated code's init() function.
// The err argument is used only to derive the reflect.Type; a typed-nil pointer is acceptable.
func InternalRegisterError(err error, typeName string, blame ErrorBlame, kind ErrorKind, safety ErrorSafety) {
	errorRegistryMu.Lock()
	defer errorRegistryMu.Unlock()

	errorRegistry[reflect.TypeOf(err)] = &ErrorSpec{
		TypeName: typeName,
		Blame:    blame,
		Kind:     kind,
		Safety:   safety,
	}
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalErrorRegistryGet returns the registered ErrorSpec for the given error's
// concrete type, or nil if no spec is registered. It does not walk the error chain.
func InternalErrorRegistryGet(err error) *ErrorSpec {
	errorRegistryMu.RLock()
	defer errorRegistryMu.RUnlock()

	return errorRegistry[reflect.TypeOf(err)]
}
