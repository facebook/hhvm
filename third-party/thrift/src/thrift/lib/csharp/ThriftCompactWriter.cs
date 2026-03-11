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
using System.Buffers;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace FBThrift
{
    /// <summary>
    /// Writes Thrift data types using the compact protocol format.
    /// Uses varint encoding for integers, zigzag encoding for signed types,
    /// delta-encoded field IDs, and compressed collection headers.
    /// </summary>
    public class ThriftCompactWriter : IThriftProtocolWriter
    {
        private readonly Stream _stream;
        private short _lastFieldId;
        private bool _pendingBool;
        private short _pendingFieldId;

        public ThriftCompactWriter(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        public Stream BaseStream => _stream;

        public void WriteFieldBegin(ThriftWireType fieldType, short fieldId)
        {
            if (fieldType == ThriftWireType.Bool)
            {
                _pendingBool = true;
                _pendingFieldId = fieldId;
                return;
            }

            WriteFieldHeader(fieldType.ToCompactType(), fieldId);
        }

        private void WriteFieldHeader(CompactType compactType, short fieldId)
        {
            var delta = fieldId - _lastFieldId;
            if (delta > 0 && delta <= 15)
            {
                _stream.WriteByte((byte)((delta << 4) | (byte)compactType));
            }
            else
            {
                _stream.WriteByte((byte)compactType);
                WriteZigzagVarint16(fieldId);
            }
            _lastFieldId = fieldId;
        }

        public void WriteFieldStop()
        {
            _stream.WriteByte((byte)CompactType.Stop);
        }

        public void WriteBool(bool value)
        {
            if (_pendingBool)
            {
                var compactType = value ? CompactType.BoolTrue : CompactType.BoolFalse;
                WriteFieldHeader(compactType, _pendingFieldId);
                _pendingBool = false;
            }
            else
            {
                _stream.WriteByte((byte)(value ? CompactType.BoolTrue : CompactType.BoolFalse));
            }
        }

        public void WriteByte(sbyte value)
        {
            _stream.WriteByte((byte)value);
        }

        public void WriteI16(short value)
        {
            WriteVarint32(IntToZigzag(value));
        }

        public void WriteI32(int value)
        {
            WriteVarint32(IntToZigzag(value));
        }

        public void WriteI64(long value)
        {
            WriteVarint64(LongToZigzag(value));
        }

        public void WriteFloat(float value)
        {
            Span<byte> buf = stackalloc byte[4];
            BinaryPrimitives.WriteInt32BigEndian(buf, BitConverter.SingleToInt32Bits(value));
            _stream.Write(buf);
        }

        public void WriteDouble(double value)
        {
            Span<byte> buf = stackalloc byte[8];
            BinaryPrimitives.WriteInt64BigEndian(buf, BitConverter.DoubleToInt64Bits(value));
            _stream.Write(buf);
        }

        public void WriteString(string value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value),
                    "Cannot write null string — use field-level optionality to omit unset fields");
            }

            var maxByteCount = Encoding.UTF8.GetMaxByteCount(value.Length);
            var rented = ArrayPool<byte>.Shared.Rent(maxByteCount);
            try
            {
                var byteCount = Encoding.UTF8.GetBytes(value, 0, value.Length, rented, 0);
                WriteVarint32((uint)byteCount);
                _stream.Write(rented, 0, byteCount);
            }
            finally
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
        }

        public void WriteBinary(byte[] value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value),
                    "Cannot write null binary — use field-level optionality to omit unset fields");
            }
            WriteVarint32((uint)value.Length);
            _stream.Write(value, 0, value.Length);
        }

        public void WriteListBegin(ThriftWireType elemType, int size)
        {
            var compactElemType = elemType.ToCompactType();
            if (size <= 14)
            {
                _stream.WriteByte((byte)((size << 4) | (byte)compactElemType));
            }
            else
            {
                _stream.WriteByte((byte)(0xF0 | (byte)compactElemType));
                WriteVarint32((uint)size);
            }
        }

        public void WriteSetBegin(ThriftWireType elemType, int size)
        {
            WriteListBegin(elemType, size);
        }

        public void WriteMapBegin(ThriftWireType keyType, ThriftWireType valType, int size)
        {
            WriteVarint32((uint)size);
            if (size == 0)
            {
                return;
            }
            var compactKeyType = keyType.ToCompactType();
            var compactValType = valType.ToCompactType();
            _stream.WriteByte((byte)(((byte)compactKeyType << 4) | (byte)compactValType));
        }

        public void WriteStruct(IThriftSerializable value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value),
                    "Cannot write null struct — this would produce invalid wire data (missing field stop marker)");
            }
            var savedFieldId = _lastFieldId;
            _lastFieldId = 0;
            value.Write(this);
            _lastFieldId = savedFieldId;
        }

        /// <summary>
        /// Writes a set of values.
        /// </summary>
        public void WriteSet<T>(IEnumerable<T> value)
        {
            var list = value as ICollection<T> ?? new List<T>(value);
            WriteSetBegin(ThriftProtocolHelper.GetWireType<T>(), list.Count);
            foreach (var item in list)
            {
                WriteValue(item);
            }
        }

        /// <summary>
        /// Writes a list of values.
        /// </summary>
        public void WriteList<T>(IEnumerable<T> value)
        {
            var list = value as ICollection<T> ?? new List<T>(value);
            WriteListBegin(ThriftProtocolHelper.GetWireType<T>(), list.Count);
            foreach (var item in list)
            {
                WriteValue(item);
            }
        }

        /// <summary>
        /// Writes a map of key-value pairs.
        /// </summary>
        public void WriteMap<K, V>(IDictionary<K, V> value)
        {
            WriteMapBegin(ThriftProtocolHelper.GetWireType<K>(), ThriftProtocolHelper.GetWireType<V>(), value.Count);
            foreach (var kvp in value)
            {
                WriteValue(kvp.Key);
                WriteValue(kvp.Value);
            }
        }

        /// <summary>
        /// Generic method to write any supported value type.
        /// </summary>
        public void WriteValue<T>(T value) => ThriftProtocolHelper.WriteValue(this, value);

        // --- Varint / Zigzag helpers ---

        private void WriteVarint32(uint n)
        {
            Span<byte> buf = stackalloc byte[5];
            var pos = 0;
            while ((n & ~0x7Fu) != 0)
            {
                buf[pos++] = (byte)((n & 0x7F) | 0x80);
                n >>= 7;
            }
            buf[pos++] = (byte)n;
            _stream.Write(buf.Slice(0, pos));
        }

        private void WriteVarint64(ulong n)
        {
            Span<byte> buf = stackalloc byte[10];
            var pos = 0;
            while ((n & ~0x7FuL) != 0)
            {
                buf[pos++] = (byte)((n & 0x7F) | 0x80);
                n >>= 7;
            }
            buf[pos++] = (byte)n;
            _stream.Write(buf.Slice(0, pos));
        }

        private void WriteZigzagVarint16(short n)
        {
            WriteVarint32(IntToZigzag(n));
        }

        private static uint IntToZigzag(int n)
        {
            return (uint)((n << 1) ^ (n >> 31));
        }

        private static ulong LongToZigzag(long n)
        {
            return (ulong)((n << 1) ^ (n >> 63));
        }

    }
}
