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

///////////////////////////////////////////////////////////////////////////////
// This file is home to a helper that converts from various base types to
// respective pointer types. This is necessary because Go does not permit
// references to constants, nor can a pointer type to base type be allocated
// and initialized in a single expression.
//
// E.g., this is not allowed:
//
//    var ip *int = &5
//
// But this *is* allowed:
//
//    var ip *int = Pointerize(5)
//
// Since pointers to base types are commonplace as [optional] fields in
// exported thrift structs, we factor such helpers here.
///////////////////////////////////////////////////////////////////////////////

// ThriftPointerizable is a type constraint meant to prevent Pointerize() API abuse.
// e.g. using outside of Thrift context or with non-Thrift types
type ThriftPointerizable interface {
	bool | int8 | int16 | ~int32 | int64 | float32 | float64 | string
}

// Pointerize returns a pointer to the given value.
func Pointerize[T ThriftPointerizable](val T) *T {
	return &val
}
