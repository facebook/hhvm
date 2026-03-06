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

namespace FBThrift
{
    /// <summary>
    /// Wire type identifiers used in Thrift binary and compact protocol serialization.
    /// These identify the encoding format of a value on the wire (e.g., "this field
    /// is encoded as a 32-bit integer"), NOT the Thrift IDL type system.
    /// See https://github.com/apache/thrift/blob/master/doc/specs/thrift-binary-protocol.md
    /// </summary>
    public enum ThriftWireType : byte
    {
        Stop = 0,
        Void = 1,
        Bool = 2,
        Byte = 3,
        Double = 4,
        I16 = 6,
        I32 = 8,
        I64 = 10,
        String = 11,
        Struct = 12,
        Map = 13,
        Set = 14,
        List = 15,
        Float = 19,
    }
}
