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
	"fmt"
	"reflect"
	"sync"
)

type channelClientConstructor func(channel RequestChannel) any

var (
	registryMu sync.RWMutex
	registry   = make(map[reflect.Type]channelClientConstructor)
)

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalRegisterClientConstructor registers a constructor function for a client type.
// This is intended to be called from generated code's init() function.
func InternalRegisterClientConstructor[T any](constructor func(channel RequestChannel) T) {
	registryMu.Lock()
	defer registryMu.Unlock()

	clientType := reflect.TypeFor[T]()
	registry[clientType] = func(ch RequestChannel) any {
		return constructor(ch)
	}
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalConstructClientFromRegistry creates a client of type T from a channel using the registered constructor.
// Returns the client or an error if no constructor was found for the type.
func InternalConstructClientFromRegistry[T any](channel RequestChannel) (T, error) {
	registryMu.RLock()
	defer registryMu.RUnlock()

	clientType := reflect.TypeFor[T]()
	if constructor, ok := registry[clientType]; ok {
		return constructor(channel).(T), nil
	}
	var zero T
	return zero, fmt.Errorf("no registered client constructor for type %v", clientType)
}
