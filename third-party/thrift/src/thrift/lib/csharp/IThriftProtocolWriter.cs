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
using System.Collections.Generic;

namespace FBThrift
{
    /// <summary>
    /// Protocol-agnostic interface for writing Thrift data.
    /// Implemented by ThriftBinaryWriter and ThriftCompactWriter.
    /// </summary>
    public interface IThriftProtocolWriter
    {
        void WriteFieldBegin(ThriftWireType fieldType, short fieldId);
        void WriteFieldStop();
        void WriteBool(bool value);
        void WriteByte(sbyte value);
        void WriteI16(short value);
        void WriteI32(int value);
        void WriteI64(long value);
        void WriteFloat(float value);
        void WriteDouble(double value);
        void WriteString(string value);
        void WriteBinary(byte[] value);
        void WriteListBegin(ThriftWireType elemType, int size);
        void WriteSetBegin(ThriftWireType elemType, int size);
        void WriteMapBegin(ThriftWireType keyType, ThriftWireType valType, int size);
        void WriteStruct(IThriftSerializable value);
        void WriteSet<T>(IEnumerable<T> value);
        void WriteList<T>(IEnumerable<T> value);
        void WriteMap<K, V>(IDictionary<K, V> value);
        void WriteValue<T>(T value);
    }
}
