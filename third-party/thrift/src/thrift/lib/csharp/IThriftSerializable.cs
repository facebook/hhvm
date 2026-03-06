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
    /// Interface for types that can be serialized and deserialized
    /// using any Thrift protocol (Binary, Compact, etc.).
    /// </summary>
    public interface IThriftSerializable
    {
        /// <summary>
        /// Writes this object to the specified Thrift protocol writer.
        /// </summary>
        /// <param name="writer">The protocol writer to write to.</param>
        void Write(IThriftProtocolWriter writer);

        /// <summary>
        /// Reads this object's fields from the specified Thrift protocol reader.
        /// </summary>
        /// <param name="reader">The protocol reader to read from.</param>
        void Read(IThriftProtocolReader reader);
    }
}
