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
using System.IO;
using System.Text;

namespace FBThrift
{
    /// <summary>
    /// Writes Thrift data types using the binary protocol format.
    /// All multi-byte values are written in big-endian (network) byte order.
    /// </summary>
    public class ThriftBinaryWriter : IThriftProtocolWriter
    {
        private readonly Stream _stream;

        public ThriftBinaryWriter(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        /// <summary>
        /// Gets the underlying stream.
        /// </summary>
        public Stream BaseStream => _stream;

        /// <summary>
        /// Writes a field header with the given type and field ID.
        /// </summary>
        public void WriteFieldBegin(ThriftWireType fieldType, short fieldId)
        {
            WriteByte((sbyte)fieldType);
            WriteI16(fieldId);
        }

        /// <summary>
        /// Writes a field stop marker (type = 0) to indicate the end of a struct.
        /// </summary>
        public void WriteFieldStop()
        {
            _stream.WriteByte((byte)ThriftWireType.Stop);
        }

        /// <summary>
        /// Writes a boolean value (1 byte: 0x01 for true, 0x00 for false).
        /// </summary>
        public void WriteBool(bool value)
        {
            _stream.WriteByte(value ? (byte)1 : (byte)0);
        }

        /// <summary>
        /// Writes a signed byte value.
        /// </summary>
        public void WriteByte(sbyte value)
        {
            _stream.WriteByte((byte)value);
        }

        /// <summary>
        /// Writes a 16-bit signed integer in big-endian format.
        /// </summary>
        public void WriteI16(short value)
        {
            Span<byte> buf = stackalloc byte[2];
            BinaryPrimitives.WriteInt16BigEndian(buf, value);
            _stream.Write(buf);
        }

        /// <summary>
        /// Writes a 32-bit signed integer in big-endian format.
        /// </summary>
        public void WriteI32(int value)
        {
            Span<byte> buf = stackalloc byte[4];
            BinaryPrimitives.WriteInt32BigEndian(buf, value);
            _stream.Write(buf);
        }

        /// <summary>
        /// Writes a 64-bit signed integer in big-endian format.
        /// </summary>
        public void WriteI64(long value)
        {
            Span<byte> buf = stackalloc byte[8];
            BinaryPrimitives.WriteInt64BigEndian(buf, value);
            _stream.Write(buf);
        }

        /// <summary>
        /// Writes a 32-bit floating point value in big-endian format.
        /// </summary>
        public void WriteFloat(float value)
        {
            var intBits = BitConverter.SingleToInt32Bits(value);
            WriteI32(intBits);
        }

        /// <summary>
        /// Writes a 64-bit floating point value in big-endian format.
        /// </summary>
        public void WriteDouble(double value)
        {
            var longBits = BitConverter.DoubleToInt64Bits(value);
            WriteI64(longBits);
        }

        /// <summary>
        /// Writes a string as a length-prefixed UTF-8 byte sequence.
        /// </summary>
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
                WriteI32(byteCount);
                _stream.Write(rented, 0, byteCount);
            }
            finally
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
        }

        /// <summary>
        /// Writes binary data as a length-prefixed byte sequence.
        /// </summary>
        public void WriteBinary(byte[] value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value),
                    "Cannot write null binary — use field-level optionality to omit unset fields");
            }

            WriteI32(value.Length);
            _stream.Write(value, 0, value.Length);
        }

        /// <summary>
        /// Writes a list header with element type and size.
        /// </summary>
        public void WriteListBegin(ThriftWireType elemType, int size)
        {
            _stream.WriteByte((byte)elemType);
            WriteI32(size);
        }

        /// <summary>
        /// Writes a set header with element type and size.
        /// </summary>
        public void WriteSetBegin(ThriftWireType elemType, int size)
        {
            WriteListBegin(elemType, size);
        }

        /// <summary>
        /// Writes a map header with key type, value type, and size.
        /// </summary>
        public void WriteMapBegin(ThriftWireType keyType, ThriftWireType valType, int size)
        {
            _stream.WriteByte((byte)keyType);
            _stream.WriteByte((byte)valType);
            WriteI32(size);
        }

        /// <summary>
        /// Writes a struct value by calling its Write method.
        /// </summary>
        public void WriteStruct(IThriftSerializable value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value),
                    "Cannot write null struct — this would produce invalid wire data (missing field stop marker)");
            }
            value.Write(this);
        }

        /// <summary>
        /// Writes a set of values.
        /// </summary>
        public void WriteSet<T>(System.Collections.Generic.IEnumerable<T> value)
        {
            var list = value as System.Collections.Generic.ICollection<T> ?? new System.Collections.Generic.List<T>(value);
            WriteSetBegin(ThriftProtocolHelper.GetWireType<T>(), list.Count);
            foreach (var item in list)
            {
                WriteValue(item);
            }
        }

        /// <summary>
        /// Writes a list of values.
        /// </summary>
        public void WriteList<T>(System.Collections.Generic.IEnumerable<T> value)
        {
            var list = value as System.Collections.Generic.ICollection<T> ?? new System.Collections.Generic.List<T>(value);
            WriteListBegin(ThriftProtocolHelper.GetWireType<T>(), list.Count);
            foreach (var item in list)
            {
                WriteValue(item);
            }
        }

        /// <summary>
        /// Writes a map of key-value pairs.
        /// </summary>
        public void WriteMap<K, V>(System.Collections.Generic.IDictionary<K, V> value)
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

    }
}
