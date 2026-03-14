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

using System;
using System.IO;

namespace FBThrift
{
    /// <summary>
    /// Convenience methods for serializing/deserializing Thrift structs to/from byte arrays.
    /// Equivalent to C++ BinarySerializer::serialize, Go format.EncodeBinary,
    /// and Rust binary_protocol::serialize.
    /// </summary>
    public static class ThriftSerializer
    {
        public static byte[] EncodeBinary<T>(T obj) where T : IThriftSerializable
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            obj.__fbthrift_write(writer);
            return ms.ToArray();
        }

        public static T DecodeBinary<T>(byte[] data) where T : IThriftSerializable, new()
        {
            if (data == null || data.Length == 0)
            {
                throw new ArgumentException("Cannot deserialize empty data", nameof(data));
            }

            using var ms = new MemoryStream(data);
            var reader = new ThriftBinaryReader(ms);
            var result = new T();
            result.__fbthrift_read(reader);
            return result;
        }

        public static byte[] EncodeCompact<T>(T obj) where T : IThriftSerializable
        {
            if (obj == null)
            {
                throw new ArgumentNullException(nameof(obj));
            }

            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            obj.__fbthrift_write(writer);
            return ms.ToArray();
        }

        public static T DecodeCompact<T>(byte[] data) where T : IThriftSerializable, new()
        {
            if (data == null || data.Length == 0)
            {
                throw new ArgumentException("Cannot deserialize empty data", nameof(data));
            }

            using var ms = new MemoryStream(data);
            var reader = new ThriftCompactReader(ms);
            var result = new T();
            result.__fbthrift_read(reader);
            return result;
        }
    }
}
