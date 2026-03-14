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
    /// Compact Protocol type constants.
    /// These differ from binary protocol type IDs — compact protocol packs
    /// types into 4 bits (0–13) for use in field and collection headers.
    /// </summary>
    public enum CompactType : byte
    {
        Stop = 0,
        BoolTrue = 1,
        Bool = BoolTrue,
        BoolFalse = 2,
        Byte = 3,
        I16 = 4,
        I32 = 5,
        I64 = 6,
        Double = 7,
        Binary = 8,
        List = 9,
        Set = 10,
        Map = 11,
        Struct = 12,
        Float = 13,
    }

    /// <summary>
    /// Extension methods for converting between CompactType and ThriftWireType.
    /// </summary>
    public static class CompactTypeExtensions
    {
        /// <summary>
        /// Converts a compact protocol type ID to the standard ThriftWireType.
        /// </summary>
        public static ThriftWireType ToThriftWireType(this CompactType compactType)
        {
            switch (compactType)
            {
                case CompactType.Stop: return ThriftWireType.Stop;
                case CompactType.BoolTrue:
                case CompactType.BoolFalse: return ThriftWireType.Bool;
                case CompactType.Byte: return ThriftWireType.Byte;
                case CompactType.I16: return ThriftWireType.I16;
                case CompactType.I32: return ThriftWireType.I32;
                case CompactType.I64: return ThriftWireType.I64;
                case CompactType.Double: return ThriftWireType.Double;
                case CompactType.Binary: return ThriftWireType.String;
                case CompactType.List: return ThriftWireType.List;
                case CompactType.Set: return ThriftWireType.Set;
                case CompactType.Map: return ThriftWireType.Map;
                case CompactType.Struct: return ThriftWireType.Struct;
                case CompactType.Float: return ThriftWireType.Float;
                default:
                    throw new ThriftProtocolException($"Unknown compact type: {compactType}");
            }
        }

        /// <summary>
        /// Converts a standard ThriftWireType to the compact protocol type ID.
        /// </summary>
        public static CompactType ToCompactType(this ThriftWireType thriftType)
        {
            switch (thriftType)
            {
                case ThriftWireType.Stop: return CompactType.Stop;
                case ThriftWireType.Bool: return CompactType.Bool;
                case ThriftWireType.Byte: return CompactType.Byte;
                case ThriftWireType.I16: return CompactType.I16;
                case ThriftWireType.I32: return CompactType.I32;
                case ThriftWireType.I64: return CompactType.I64;
                case ThriftWireType.Double: return CompactType.Double;
                case ThriftWireType.String: return CompactType.Binary;
                case ThriftWireType.List: return CompactType.List;
                case ThriftWireType.Set: return CompactType.Set;
                case ThriftWireType.Map: return CompactType.Map;
                case ThriftWireType.Struct: return CompactType.Struct;
                case ThriftWireType.Float: return CompactType.Float;
                default:
                    throw new ThriftProtocolException($"Unknown thrift type: {thriftType}");
            }
        }
    }

    /// <summary>
    /// Reads Thrift data types using the compact protocol format.
    /// Uses varint encoding for integers, zigzag encoding for signed types,
    /// delta-encoded field IDs, and compressed collection headers.
    /// </summary>
    public class ThriftCompactReader : IThriftProtocolReader
    {
        private readonly Stream _stream;
        private readonly byte[] _buffer = new byte[8];
        private short _lastFieldId;
        private bool _pendingBoolValue;
        private bool _hasPendingBool;

        /// <summary>
        /// Strict UTF-8 encoding that throws on invalid byte sequences instead of
        /// silently replacing them with U+FFFD.
        /// </summary>
        private static readonly Encoding StrictUtf8 = new UTF8Encoding(
            encoderShouldEmitUTF8Identifier: false,
            throwOnInvalidBytes: true);

        public ThriftCompactReader(Stream stream)
        {
            _stream = stream ?? throw new ArgumentNullException(nameof(stream));
        }

        /// <summary>
        /// Gets or sets the maximum allowed size for string and binary reads.
        /// When set to 0 (default), no configurable limit is enforced.
        /// </summary>
        public int StringSizeLimit { get; set; }

        public Stream BaseStream => _stream;

        /// <summary>
        /// Gets the number of bytes remaining in the stream.
        /// Returns long.MaxValue for non-seekable streams.
        /// </summary>
        public long RemainingBytes => _stream.CanSeek
            ? _stream.Length - _stream.Position
            : long.MaxValue;

        public (ThriftWireType fieldType, short fieldId) ReadFieldBegin()
        {
            var header = ReadRawByte();
            if (header == (byte)CompactType.Stop)
            {
                return (ThriftWireType.Stop, 0);
            }

            var compactType = (CompactType)(header & 0x0F);
            var delta = (short)((header >> 4) & 0x0F);

            short fieldId;
            if (delta != 0)
            {
                fieldId = (short)(_lastFieldId + delta);
            }
            else
            {
                fieldId = ReadI16();
            }
            _lastFieldId = fieldId;

            if (compactType == CompactType.BoolTrue)
            {
                _pendingBoolValue = true;
                _hasPendingBool = true;
                return (ThriftWireType.Bool, fieldId);
            }
            if (compactType == CompactType.BoolFalse)
            {
                _pendingBoolValue = false;
                _hasPendingBool = true;
                return (ThriftWireType.Bool, fieldId);
            }

            return (compactType.ToThriftWireType(), fieldId);
        }

        public bool ReadBool()
        {
            if (_hasPendingBool)
            {
                _hasPendingBool = false;
                return _pendingBoolValue;
            }
            return ReadRawByte() == (byte)CompactType.BoolTrue;
        }

        public sbyte ReadByte()
        {
            return (sbyte)ReadRawByte();
        }

        public short ReadI16()
        {
            return (short)ZigzagToInt(ReadVarint32());
        }

        public int ReadI32()
        {
            return ZigzagToInt(ReadVarint32());
        }

        public long ReadI64()
        {
            return ZigzagToLong(ReadVarint64());
        }

        public float ReadFloat()
        {
            ReadExact(_buffer, 4);
            int intBits = BinaryPrimitives.ReadInt32BigEndian(_buffer);
            return BitConverter.Int32BitsToSingle(intBits);
        }

        public double ReadDouble()
        {
            ReadExact(_buffer, 8);
            long longBits = BinaryPrimitives.ReadInt64BigEndian(_buffer);
            return BitConverter.Int64BitsToDouble(longBits);
        }

        public string ReadString()
        {
            var length = (int)ReadVarint32();
            if (length < 0)
            {
                throw new ThriftProtocolException($"Negative string length: {length}");
            }
            if (StringSizeLimit > 0 && length > StringSizeLimit)
            {
                throw new ThriftProtocolException(
                    $"String/binary length {length} exceeds size limit {StringSizeLimit}");
            }
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

        public byte[] ReadBinary()
        {
            var length = (int)ReadVarint32();
            if (length < 0)
            {
                throw new ThriftProtocolException($"Negative binary length: {length}");
            }
            if (StringSizeLimit > 0 && length > StringSizeLimit)
            {
                throw new ThriftProtocolException(
                    $"String/binary length {length} exceeds size limit {StringSizeLimit}");
            }
            if (length == 0)
            {
                return Array.Empty<byte>();
            }
            var bytes = new byte[length];
            ReadExact(bytes, length);
            return bytes;
        }

        public (ThriftWireType elemType, int size) ReadListBegin()
        {
            var header = ReadRawByte();
            var size = (header >> 4) & 0x0F;
            var compactElemType = (CompactType)(header & 0x0F);

            if (size == 15)
            {
                size = (int)ReadVarint32();
            }

            ValidateCollectionSize(size);
            return (compactElemType.ToThriftWireType(), size);
        }

        public (ThriftWireType elemType, int size) ReadSetBegin()
        {
            return ReadListBegin();
        }

        public (ThriftWireType keyType, ThriftWireType valType, int size) ReadMapBegin()
        {
            var size = (int)ReadVarint32();
            if (size == 0)
            {
                return (ThriftWireType.Stop, ThriftWireType.Stop, 0);
            }

            ValidateCollectionSize(size, minBytesPerElement: 2);

            var types = ReadRawByte();
            var keyCompactType = (CompactType)((types >> 4) & 0x0F);
            var valCompactType = (CompactType)(types & 0x0F);

            return (keyCompactType.ToThriftWireType(), valCompactType.ToThriftWireType(), size);
        }

        public T ReadStruct<T>() where T : IThriftSerializable, new()
        {
            var savedFieldId = _lastFieldId;
            _lastFieldId = 0;

            var result = new T();
            result.__fbthrift_read(this);

            _lastFieldId = savedFieldId;
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

        public void Skip(ThriftWireType fieldType, short? fieldId = null)
        {
            switch (fieldType)
            {
                case ThriftWireType.Struct:
                    var savedFieldId = _lastFieldId;
                    _lastFieldId = 0;
                    while (true)
                    {
                        var (ft, _) = ReadFieldBegin();
                        if (ft == ThriftWireType.Stop)
                        {
                            break;
                        }
                        Skip(ft);
                    }
                    _lastFieldId = savedFieldId;
                    break;
                default:
                    ThriftProtocolHelper.Skip(this, fieldType);
                    break;
            }
        }

        /// <summary>
        /// Generic method to read any supported value type.
        /// </summary>
        public T ReadValue<T>() => ThriftProtocolHelper.ReadValue<T>(this);

        // --- Varint / Zigzag helpers ---

        private uint ReadVarint32()
        {
            uint result = 0;
            int shift = 0;
            while (true)
            {
                byte b = ReadRawByte();
                result |= (uint)(b & 0x7F) << shift;
                if ((b & 0x80) == 0)
                {
                    break;
                }
                shift += 7;
                if (shift > 28)
                {
                    throw new ThriftProtocolException("Varint32 too long");
                }
            }
            return result;
        }

        private ulong ReadVarint64()
        {
            ulong result = 0;
            int shift = 0;
            while (true)
            {
                byte b = ReadRawByte();
                result |= (ulong)(b & 0x7F) << shift;
                if ((b & 0x80) == 0)
                {
                    break;
                }
                shift += 7;
                if (shift > 63)
                {
                    throw new ThriftProtocolException("Varint64 too long");
                }
            }
            return result;
        }

        private static int ZigzagToInt(uint n)
        {
            return (int)(n >> 1) ^ -(int)(n & 1);
        }

        private static long ZigzagToLong(ulong n)
        {
            return (long)(n >> 1) ^ -(long)(n & 1);
        }

        private byte ReadRawByte()
        {
            var b = _stream.ReadByte();
            if (b < 0)
            {
                throw new EndOfStreamException("Unexpected end of stream");
            }
            return (byte)b;
        }

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

    }
}
