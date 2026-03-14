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
    /// Reads Thrift data types using the binary protocol format.
    /// All multi-byte values are read in big-endian (network) byte order.
    /// </summary>
    public class ThriftBinaryReader : IThriftProtocolReader
    {
        private readonly Stream _stream;
        private readonly byte[] _buffer = new byte[8];

        /// <summary>
        /// Strict UTF-8 encoding that throws on invalid byte sequences instead of
        /// silently replacing them with U+FFFD.
        /// </summary>
        private static readonly Encoding StrictUtf8 = new UTF8Encoding(
            encoderShouldEmitUTF8Identifier: false,
            throwOnInvalidBytes: true);


        public ThriftBinaryReader(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        /// <summary>
        /// Gets or sets the maximum allowed size for string and binary reads.
        /// When set to 0 (default), no configurable limit is enforced, but reads
        /// are still validated against remaining stream bytes.
        /// Mirrors the C++ thrift_cpp2_protocol_reader_string_limit gflag behavior.
        /// </summary>
        public int StringSizeLimit { get; set; }

        /// <summary>
        /// Gets the underlying stream.
        /// </summary>
        public Stream BaseStream => _stream;

        /// <summary>
        /// Gets the number of bytes remaining in the stream.
        /// Returns long.MaxValue for non-seekable streams.
        /// </summary>
        public long RemainingBytes => _stream.CanSeek
            ? _stream.Length - _stream.Position
            : long.MaxValue;

        /// <summary>
        /// Reads a field header and returns the field type and field ID.
        /// </summary>
        /// <returns>A tuple of (fieldType, fieldId). fieldType will be ThriftWireType.Stop (0) at end of struct.</returns>
        public (ThriftWireType fieldType, short fieldId) ReadFieldBegin()
        {
            var rawType = (byte)ReadByte();
            if (rawType == (byte)ThriftWireType.Stop)
            {
                return (ThriftWireType.Stop, 0);
            }
            if (!IsValidWireType(rawType))
            {
                throw new ThriftProtocolException(
                    $"Unknown field type {rawType} at stream position {(_stream.CanSeek ? _stream.Position.ToString() : "unknown")}");
            }
            var fieldType = (ThriftWireType)rawType;
            var fieldId = ReadI16();
            return (fieldType, fieldId);
        }

        /// <summary>
        /// Reads a boolean value (1 byte: 0x01 for true, 0x00 for false).
        /// </summary>
        public bool ReadBool()
        {
            var b = _stream.ReadByte();
            if (b < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream while reading bool");
            }
            if (b != 0 && b != 1)
            {
                throw new ThriftProtocolException(
                    $"Invalid bool value: 0x{b:X2} (expected 0x00 or 0x01)");
            }
            return b == 1;
        }

        /// <summary>
        /// Reads a signed byte value.
        /// </summary>
        public sbyte ReadByte()
        {
            var b = _stream.ReadByte();
            if (b < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream while reading byte");
            }
            return (sbyte)b;
        }

        /// <summary>
        /// Reads a 16-bit signed integer in big-endian format.
        /// </summary>
        public short ReadI16()
        {
            ReadExact(_buffer, 2);
            return BinaryPrimitives.ReadInt16BigEndian(_buffer);
        }

        /// <summary>
        /// Reads a 32-bit signed integer in big-endian format.
        /// </summary>
        public int ReadI32()
        {
            ReadExact(_buffer, 4);
            return BinaryPrimitives.ReadInt32BigEndian(_buffer);
        }

        /// <summary>
        /// Reads a 64-bit signed integer in big-endian format.
        /// </summary>
        public long ReadI64()
        {
            ReadExact(_buffer, 8);
            return BinaryPrimitives.ReadInt64BigEndian(_buffer);
        }

        /// <summary>
        /// Reads a 32-bit floating point value in big-endian format.
        /// </summary>
        public float ReadFloat()
        {
            var intBits = ReadI32();
            return BitConverter.Int32BitsToSingle(intBits);
        }

        /// <summary>
        /// Reads a 64-bit floating point value in big-endian format.
        /// </summary>
        public double ReadDouble()
        {
            var longBits = ReadI64();
            return BitConverter.Int64BitsToDouble(longBits);
        }

        /// <summary>
        /// Reads a string as a length-prefixed UTF-8 byte sequence.
        /// </summary>
        public string ReadString()
        {
            var length = ReadI32();
            CheckStringSize(length);
            if (length == 0)
            {
                return string.Empty;
            }

            var rented = ArrayPool<byte>.Shared.Rent(length);
            try
            {
                ReadExact(rented, length);
                return StrictUtf8.GetString(rented, 0, length);
            }
            finally
            {
                ArrayPool<byte>.Shared.Return(rented);
            }
        }

        /// <summary>
        /// Reads binary data as a length-prefixed byte sequence.
        /// </summary>
        public byte[] ReadBinary()
        {
            var length = ReadI32();
            CheckStringSize(length);
            if (length == 0)
            {
                return Array.Empty<byte>();
            }

            var bytes = new byte[length];
            ReadExact(bytes, length);
            return bytes;
        }

        /// <summary>
        /// Reads a list header and returns the element type and size.
        /// </summary>
        public (ThriftWireType elemType, int size) ReadListBegin()
        {
            var b = _stream.ReadByte();
            if (b < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream while reading list element type");
            }
            var elemType = (ThriftWireType)b;
            var size = ReadI32();
            ValidateCollectionSize(size);
            return (elemType, size);
        }

        /// <summary>
        /// Reads a set header and returns the element type and size.
        /// </summary>
        public (ThriftWireType elemType, int size) ReadSetBegin()
        {
            return ReadListBegin();
        }

        /// <summary>
        /// Reads a map header and returns the key type, value type, and size.
        /// </summary>
        public (ThriftWireType keyType, ThriftWireType valType, int size) ReadMapBegin()
        {
            var b1 = _stream.ReadByte();
            if (b1 < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream while reading map key type");
            }
            var keyType = (ThriftWireType)b1;
            var b2 = _stream.ReadByte();
            if (b2 < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream while reading map value type");
            }
            var valType = (ThriftWireType)b2;
            var size = ReadI32();
            ValidateCollectionSize(size, minBytesPerElement: 2);
            return (keyType, valType, size);
        }

        /// <summary>
        /// Reads a struct value by creating a new instance and calling its Read method.
        /// </summary>
        public T ReadStruct<T>() where T : IThriftSerializable, new()
        {
            var result = new T();
            result.__fbthrift_read(this);
            return result;
        }

        /// <summary>
        /// Reads a set of values.
        /// </summary>
        public HashSet<T> ReadSet<T>()
        {
            var (_, size) = ReadSetBegin();
            var result = new HashSet<T>(size);
            for (var i = 0; i < size; i++)
            {
                result.Add(ReadValue<T>());
            }
            return result;
        }

        /// <summary>
        /// Reads a list of values.
        /// </summary>
        public List<T> ReadList<T>()
        {
            var (_, size) = ReadListBegin();
            var result = new List<T>(size);
            for (var i = 0; i < size; i++)
            {
                result.Add(ReadValue<T>());
            }
            return result;
        }

        /// <summary>
        /// Reads a map of key-value pairs.
        /// </summary>
        public Dictionary<K, V> ReadMap<K, V>()
        {
            var (_, _, size) = ReadMapBegin();
            var result = new Dictionary<K, V>(size);
            for (var i = 0; i < size; i++)
            {
                var key = ReadValue<K>();
                var val = ReadValue<V>();
                result[key] = val;
            }
            return result;
        }

        /// <summary>
        /// Skips a value of the given type.
        /// </summary>
        public void Skip(ThriftWireType fieldType, short? fieldId = null) => ThriftProtocolHelper.Skip(this, fieldType, fieldId);

        /// <summary>
        /// Generic method to read any supported value type.
        /// </summary>
        public T ReadValue<T>() => ThriftProtocolHelper.ReadValue<T>(this);

        private void ReadExact(byte[] buffer, int count)
        {
            var offset = 0;
            while (offset < count)
            {
                var bytesRead = _stream.Read(buffer, offset, count - offset);
                if (bytesRead == 0)
                {
                    throw new EndOfStreamException($"Unexpected end of stream, expected {count} bytes but got {offset}");
                }
                offset += bytesRead;
            }
        }

        private void CheckStringSize(int size)
        {
            if (size < 0)
            {
                throw new ThriftProtocolException($"Negative string/binary length: {size}");
            }
            if (StringSizeLimit > 0 && size > StringSizeLimit)
            {
                throw new ThriftProtocolException(
                    $"String/binary length {size} exceeds size limit {StringSizeLimit}");
            }
            if (size > RemainingBytes)
            {
                throw new ThriftProtocolException(
                    $"String/binary length {size} exceeds remaining bytes {RemainingBytes}");
            }
        }

        private void ValidateCollectionSize(int size, int minBytesPerElement = 1)
        {
            if (size < 0)
            {
                throw new ThriftProtocolException($"Negative collection size: {size}");
            }
            var minRequired = (long)size * minBytesPerElement;
            if (minRequired > RemainingBytes)
            {
                throw new ThriftProtocolException(
                    $"Collection size {size} exceeds remaining bytes {RemainingBytes}");
            }
        }

        private static bool IsValidWireType(byte value) => Enum.IsDefined(typeof(ThriftWireType), value);
    }
}
