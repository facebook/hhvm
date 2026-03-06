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

using System.Collections.Generic;

namespace FBThrift
{
    /// <summary>
    /// Protocol-agnostic interface for reading Thrift data.
    /// Implemented by ThriftBinaryReader and ThriftCompactReader.
    /// </summary>
    public interface IThriftProtocolReader
    {
        /// <summary>
        /// Gets or sets the maximum allowed size for string and binary reads.
        /// When set to 0 (default), no configurable limit is enforced, but reads
        /// are still validated against remaining stream bytes.
        /// </summary>
        int StringSizeLimit { get; set; }

        (ThriftWireType fieldType, short fieldId) ReadFieldBegin();
        bool ReadBool();
        sbyte ReadByte();
        short ReadI16();
        int ReadI32();
        long ReadI64();
        float ReadFloat();
        double ReadDouble();
        string ReadString();
        byte[] ReadBinary();
        (ThriftWireType elemType, int size) ReadListBegin();
        (ThriftWireType elemType, int size) ReadSetBegin();
        (ThriftWireType keyType, ThriftWireType valType, int size) ReadMapBegin();
        T ReadStruct<T>() where T : IThriftSerializable, new();
        HashSet<T> ReadSet<T>();
        List<T> ReadList<T>();
        Dictionary<K, V> ReadMap<K, V>();
        void Skip(ThriftWireType fieldType, short? fieldId = null);
        T ReadValue<T>();
    }
}
