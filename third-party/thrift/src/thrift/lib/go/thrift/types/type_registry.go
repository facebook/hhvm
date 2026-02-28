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
	"crypto/sha256"
	"strings"
	"sync"
)

var (
	typeRegistryMu     sync.RWMutex
	typeRegistry       = make(map[string]*TypeSpec)
	typeHash32Registry = make(map[[32]byte]string)
	typeHash16Registry = make(map[[16]byte]string)
	typeHash8Registry  = make(map[[8]byte]string)
)

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalRegisterType registers a URI + type spec pair for Thrit Any support.
// This is intended to be called from generated code's init() function.
func InternalRegisterType(uri string, spec *TypeSpec) {
	const thriftURIPrefix = "fbthrift://"

	uri, _ = strings.CutPrefix(uri, thriftURIPrefix)
	h := sha256.New()
	h.Write([]byte(thriftURIPrefix + uri))
	hash := h.Sum(nil)
	hash32 := *(*[32]byte)(hash[0:32])
	hash16 := *(*[16]byte)(hash[0:16])
	hash8 := *(*[8]byte)(hash[0:8])

	typeRegistryMu.Lock()
	defer typeRegistryMu.Unlock()

	typeRegistry[uri] = spec
	typeHash32Registry[hash32] = uri
	typeHash16Registry[hash16] = uri
	typeHash8Registry[hash8] = uri
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalTypeRegistryGetFromURI returns the type spec for a given URI, if it exists.
func InternalTypeRegistryGetFromURI(uri string) (*TypeSpec, bool) {
	typeRegistryMu.RLock()
	defer typeRegistryMu.RUnlock()

	res, ok := typeRegistry[uri]
	return res, ok
}

// THIS FUNCTION IS FOR INTERNAL USE ONLY.
// InternalTypeRegistryGetFromHash returns the type spec for a given hash, if it exists.
func InternalTypeRegistryGetFromHash(hash []byte) (*TypeSpec, bool) {
	typeRegistryMu.RLock()
	defer typeRegistryMu.RUnlock()

	var uri string
	var ok bool
	switch len(hash) {
	case 32:
		hash32 := *(*[32]byte)(hash[0:32])
		uri, ok = typeHash32Registry[hash32]
	case 16:
		hash16 := *(*[16]byte)(hash[0:16])
		uri, ok = typeHash16Registry[hash16]
	case 8:
		hash8 := *(*[8]byte)(hash[0:8])
		uri, ok = typeHash8Registry[hash8]
	}

	if !ok {
		return nil, false
	}

	res, ok := typeRegistry[uri]
	return res, ok
}
