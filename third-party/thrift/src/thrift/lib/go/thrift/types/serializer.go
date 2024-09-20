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

// WritableStruct is an interface used to encapsulate a message that can be written to a protocol
type WritableStruct interface {
	Write(p Encoder) error
}

// WritableException is an interface used to encapsulate an exception that can be written to a protocol
type WritableException interface {
	WritableStruct
	Exception
}

// WritableResult is an interface used to encapsulate a result struct that can be written to a protocol
type WritableResult interface {
	WritableStruct
	Exception() WritableException
}

// Struct is the interface used to encapsulate a message that can be read and written to a protocol
type Struct interface {
	Write(p Encoder) error
	Read(p Decoder) error
}
