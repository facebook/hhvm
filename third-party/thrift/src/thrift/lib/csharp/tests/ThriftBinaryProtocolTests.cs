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
using System.Linq;
using FBThrift;
using NUnit.Framework;

namespace FBThrift.Tests
{
    /// <summary>
    /// Round-trip tests for Binary protocol.
    /// </summary>
    [TestFixture]
    public class ThriftProtocolRoundTripTests
    {
        private IThriftProtocolWriter CreateWriter(MemoryStream ms)
        {
            return new ThriftBinaryWriter(ms);
        }

        private IThriftProtocolReader CreateReader(MemoryStream ms)
        {
            ms.Position = 0;
            return new ThriftBinaryReader(ms);
        }

        // === Primitive round-trip tests ===

        [Test]
        public void TestBoolTrue()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteBool(true);

            var reader = CreateReader(ms);
            Assert.AreEqual(true, reader.ReadBool());
        }

        [Test]
        public void TestBoolFalse()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteBool(false);

            var reader = CreateReader(ms);
            Assert.AreEqual(false, reader.ReadBool());
        }

        [Test]
        public void TestByte()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteByte(42);
            writer.WriteByte(-1);
            writer.WriteByte(sbyte.MaxValue);
            writer.WriteByte(sbyte.MinValue);

            var reader = CreateReader(ms);
            Assert.AreEqual((sbyte)42, reader.ReadByte());
            Assert.AreEqual((sbyte)-1, reader.ReadByte());
            Assert.AreEqual(sbyte.MaxValue, reader.ReadByte());
            Assert.AreEqual(sbyte.MinValue, reader.ReadByte());
        }

        [Test]
        public void TestI16()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteI16(12345);
            writer.WriteI16(-12345);
            writer.WriteI16(short.MaxValue);
            writer.WriteI16(short.MinValue);

            var reader = CreateReader(ms);
            Assert.AreEqual((short)12345, reader.ReadI16());
            Assert.AreEqual((short)-12345, reader.ReadI16());
            Assert.AreEqual(short.MaxValue, reader.ReadI16());
            Assert.AreEqual(short.MinValue, reader.ReadI16());
        }

        [Test]
        public void TestI32()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteI32(123456789);
            writer.WriteI32(-123456789);
            writer.WriteI32(int.MaxValue);
            writer.WriteI32(int.MinValue);

            var reader = CreateReader(ms);
            Assert.AreEqual(123456789, reader.ReadI32());
            Assert.AreEqual(-123456789, reader.ReadI32());
            Assert.AreEqual(int.MaxValue, reader.ReadI32());
            Assert.AreEqual(int.MinValue, reader.ReadI32());
        }

        [Test]
        public void TestI64()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteI64(1234567890123456789L);
            writer.WriteI64(-1234567890123456789L);
            writer.WriteI64(long.MaxValue);
            writer.WriteI64(long.MinValue);

            var reader = CreateReader(ms);
            Assert.AreEqual(1234567890123456789L, reader.ReadI64());
            Assert.AreEqual(-1234567890123456789L, reader.ReadI64());
            Assert.AreEqual(long.MaxValue, reader.ReadI64());
            Assert.AreEqual(long.MinValue, reader.ReadI64());
        }

        [Test]
        public void TestFloat()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteFloat(3.14159f);
            writer.WriteFloat(-2.71828f);
            writer.WriteFloat(float.MaxValue);
            writer.WriteFloat(float.MinValue);
            writer.WriteFloat(0.0f);

            var reader = CreateReader(ms);
            Assert.AreEqual(3.14159f, reader.ReadFloat());
            Assert.AreEqual(-2.71828f, reader.ReadFloat());
            Assert.AreEqual(float.MaxValue, reader.ReadFloat());
            Assert.AreEqual(float.MinValue, reader.ReadFloat());
            Assert.AreEqual(0.0f, reader.ReadFloat());
        }

        [Test]
        public void TestDouble()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteDouble(3.14159265358979);
            writer.WriteDouble(-2.71828182845904);
            writer.WriteDouble(double.MaxValue);
            writer.WriteDouble(double.MinValue);
            writer.WriteDouble(0.0);

            var reader = CreateReader(ms);
            Assert.AreEqual(3.14159265358979, reader.ReadDouble());
            Assert.AreEqual(-2.71828182845904, reader.ReadDouble());
            Assert.AreEqual(double.MaxValue, reader.ReadDouble());
            Assert.AreEqual(double.MinValue, reader.ReadDouble());
            Assert.AreEqual(0.0, reader.ReadDouble());
        }

        [Test]
        public void TestString()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString("Hello, World!");
            writer.WriteString("日本語テスト");
            writer.WriteString("🎉 Emoji test");

            var reader = CreateReader(ms);
            Assert.AreEqual("Hello, World!", reader.ReadString());
            Assert.AreEqual("日本語テスト", reader.ReadString());
            Assert.AreEqual("🎉 Emoji test", reader.ReadString());
        }

        [Test]
        public void TestEmptyString()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString("");

            var reader = CreateReader(ms);
            Assert.AreEqual("", reader.ReadString());
        }

        [Test]
        public void TestReadStringRejectsInvalidUtf8()
        {
            // Construct a stream with a valid i32 length prefix followed by invalid UTF-8 bytes.
            // ReadString() uses StrictUtf8 encoding which should throw on invalid sequences.
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteI32(3);
            ms.Write(new byte[] { 0x72, 0x01, 0xFF });

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<System.Text.DecoderFallbackException>(() => reader.ReadString());
        }

        [Test]
        public void TestBinaryWithHighBytes()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            var bytes = new byte[] { 0x01, 0x02, 0x03, 0xFF, 0xFE };
            writer.WriteBinary(bytes);

            var reader = CreateReader(ms);
            Assert.AreEqual(bytes, reader.ReadBinary());
        }

        [Test]
        public void TestBinaryFieldWithInvalidUtf8_RoundTrip()
        {
            // This test reproduces the "bad_utf8" conformance test scenario.
            // The value "\x72\x01\xff" contains 0xFF which is invalid UTF-8.
            // Binary fields should handle this without UTF-8 decoding errors.
            var badUtf8Bytes = new byte[] { 0x72, 0x01, 0xFF };

            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);

            // Write a struct with a binary field (field id 1, type STRING=11 for binary)
            writer.WriteFieldBegin(ThriftWireType.String, 1);
            writer.WriteBinary(badUtf8Bytes);
            writer.WriteFieldStop();

            // Read it back
            var reader = CreateReader(ms);
            var (fieldType, fieldId) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.String, fieldType);
            Assert.AreEqual(1, fieldId);

            var readBytes = reader.ReadBinary();
            Assert.AreEqual(badUtf8Bytes, readBytes, "Binary field with invalid UTF-8 should round-trip correctly");
        }

        [Test]
        public void TestSkipBinaryFieldWithInvalidUtf8()
        {
            // This test reproduces the "bad_utf8" conformance test failure scenario.
            // When skipping a field with ThriftWireType.String (used for both string
            // and binary), we must NOT try to UTF-8 decode it because binary data
            // may contain invalid UTF-8 sequences like 0xFF.
            var badUtf8Bytes = new byte[] { 0x72, 0x01, 0xFF };

            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);

            // Write a struct with a binary field (field id 1), then another field
            writer.WriteFieldBegin(ThriftWireType.String, 1);
            writer.WriteBinary(badUtf8Bytes);
            writer.WriteFieldBegin(ThriftWireType.I32, 2);
            writer.WriteI32(42);
            writer.WriteFieldStop();

            // Read back, skip field 1 (binary with bad UTF-8), read field 2
            var reader = CreateReader(ms);
            var (fieldType1, fieldId1) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.String, fieldType1);
            Assert.AreEqual(1, fieldId1);

            // This should NOT throw even though the binary contains invalid UTF-8
            Assert.DoesNotThrow(() => reader.Skip(ThriftWireType.String),
                "Skipping binary field with invalid UTF-8 should not throw");

            // Verify we can continue reading the next field
            var (fieldType2, fieldId2) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, fieldType2);
            Assert.AreEqual(2, fieldId2);
            Assert.AreEqual(42, reader.ReadI32());
        }

        [Test]
        public void TestEmptyBinary()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteBinary(Array.Empty<byte>());

            var reader = CreateReader(ms);
            Assert.AreEqual(0, reader.ReadBinary().Length);
        }

        // === Container header round-trip tests ===

        [Test]
        public void TestListBegin()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteListBegin(ThriftWireType.I32, 3);
            writer.WriteI32(1);
            writer.WriteI32(2);
            writer.WriteI32(3);

            var reader = CreateReader(ms);
            var (elemType, size) = reader.ReadListBegin();
            Assert.AreEqual(ThriftWireType.I32, elemType);
            Assert.AreEqual(3, size);
        }

        [Test]
        public void TestSetBegin()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteSetBegin(ThriftWireType.String, 2);
            writer.WriteString("a");
            writer.WriteString("b");

            var reader = CreateReader(ms);
            var (elemType, size) = reader.ReadSetBegin();
            Assert.AreEqual(ThriftWireType.String, elemType);
            Assert.AreEqual(2, size);
        }

        [Test]
        public void TestMapBegin()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteMapBegin(ThriftWireType.String, ThriftWireType.I64, 2);
            writer.WriteString("k1");
            writer.WriteI64(1L);
            writer.WriteString("k2");
            writer.WriteI64(2L);

            var reader = CreateReader(ms);
            var (keyType, valType, size) = reader.ReadMapBegin();
            Assert.AreEqual(ThriftWireType.String, keyType);
            Assert.AreEqual(ThriftWireType.I64, valType);
            Assert.AreEqual(2, size);
        }

        // === Skip tests ===

        [Test]
        public void TestSkipBool()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteBool(true);
            writer.WriteI32(42);

            var reader = CreateReader(ms);
            reader.Skip(ThriftWireType.Bool);
            Assert.AreEqual(42, reader.ReadI32());
        }

        [Test]
        public void TestSkipString()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString("Skip this string");
            writer.WriteI32(42);

            var reader = CreateReader(ms);
            reader.Skip(ThriftWireType.String);
            Assert.AreEqual(42, reader.ReadI32());
        }

        [Test]
        public void TestSkipList()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteListBegin(ThriftWireType.I32, 3);
            writer.WriteI32(1);
            writer.WriteI32(2);
            writer.WriteI32(3);
            writer.WriteI64(42L);

            var reader = CreateReader(ms);
            reader.Skip(ThriftWireType.List);
            Assert.AreEqual(42L, reader.ReadI64());
        }

        [Test]
        public void TestSkipMap()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteMapBegin(ThriftWireType.String, ThriftWireType.I32, 2);
            writer.WriteString("key1");
            writer.WriteI32(1);
            writer.WriteString("key2");
            writer.WriteI32(2);
            writer.WriteI64(42L);

            var reader = CreateReader(ms);
            reader.Skip(ThriftWireType.Map);
            Assert.AreEqual(42L, reader.ReadI64());
        }

        [Test]
        public void TestSkipStruct()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteFieldBegin(ThriftWireType.I32, 1);
            writer.WriteI32(100);
            writer.WriteFieldBegin(ThriftWireType.String, 2);
            writer.WriteString("test");
            writer.WriteFieldStop();
            writer.WriteI64(42L);

            var reader = CreateReader(ms);
            reader.Skip(ThriftWireType.Struct);
            Assert.AreEqual(42L, reader.ReadI64());
        }

        [Test]
        public void TestSkipModerateNestingSucceeds()
        {
            const int depth = 10;
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);

            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldBegin(ThriftWireType.Struct, 1);
            }
            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldStop();
            }
            writer.WriteFieldStop();
            writer.WriteI64(42L);

            var reader = CreateReader(ms);
            Assert.DoesNotThrow(() => reader.Skip(ThriftWireType.Struct));
            Assert.AreEqual(42L, reader.ReadI64());
        }

        // === String/Binary size limit tests ===

        [Test]
        public void TestReadStringRejectsExceedingStringSizeLimit()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString(new string('x', 100));

            var reader = CreateReader(ms);
            reader.StringSizeLimit = 50;
            Assert.Throws<ThriftProtocolException>(() => reader.ReadString());
        }

        [Test]
        public void TestReadBinaryRejectsExceedingStringSizeLimit()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteBinary(new byte[100]);

            var reader = CreateReader(ms);
            reader.StringSizeLimit = 50;
            Assert.Throws<ThriftProtocolException>(() => reader.ReadBinary());
        }

        [Test]
        public void TestReadStringSucceedsWithinStringSizeLimit()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString("hello");

            var reader = CreateReader(ms);
            reader.StringSizeLimit = 1024;
            Assert.AreEqual("hello", reader.ReadString());
        }

        [Test]
        public void TestReadStringSucceedsWithDefaultNoLimit()
        {
            using var ms = new MemoryStream();
            var writer = CreateWriter(ms);
            writer.WriteString("hello");

            var reader = CreateReader(ms);
            Assert.AreEqual("hello", reader.ReadString());
        }

        [Test]
        public void TestStringSizeLimitDefaultsToZero()
        {
            using var ms = new MemoryStream();
            var reader = CreateReader(ms);
            Assert.AreEqual(0, reader.StringSizeLimit);
        }

        // === ThriftWireType enum tests (protocol-independent) ===

        [Test]
        public void TestThriftWireTypeToStringKnownTypes()
        {
            Assert.AreEqual("Bool", ThriftWireType.Bool.ToString());
            Assert.AreEqual("Struct", ThriftWireType.Struct.ToString());
            Assert.AreEqual("Map", ThriftWireType.Map.ToString());
            Assert.AreEqual("List", ThriftWireType.List.ToString());
            Assert.AreEqual("I32", ThriftWireType.I32.ToString());
            Assert.AreEqual("String", ThriftWireType.String.ToString());
            Assert.AreEqual("Float", ThriftWireType.Float.ToString());
        }

        [Test]
        public void TestThriftWireTypeIsDefinedRejectsUnknownValues()
        {
            var validSet = new System.Collections.Generic.HashSet<byte>(
                Enum.GetValues(typeof(ThriftWireType)).Cast<byte>());
            for (int i = 0; i <= 255; i++)
            {
                var b = (byte)i;
                var expected = validSet.Contains(b);
                Assert.AreEqual(expected, Enum.IsDefined(typeof(ThriftWireType), b),
                    $"Wire type {b}: expected IsDefined={expected}");
            }
        }
    }

    /// <summary>
    /// Tests specific to the binary protocol wire format.
    /// </summary>
    [TestFixture]
    public class ThriftBinarySpecificTests
    {
        // === Big-endian byte order tests ===

        [Test]
        public void TestI16BigEndian()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteI16(0x1234);

            var bytes = ms.ToArray();
            Assert.AreEqual((byte)0x12, bytes[0]);
            Assert.AreEqual((byte)0x34, bytes[1]);
        }

        [Test]
        public void TestI32BigEndian()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteI32(0x12345678);

            var bytes = ms.ToArray();
            Assert.AreEqual((byte)0x12, bytes[0]);
            Assert.AreEqual((byte)0x34, bytes[1]);
            Assert.AreEqual((byte)0x56, bytes[2]);
            Assert.AreEqual((byte)0x78, bytes[3]);
        }

        [Test]
        public void TestI64BigEndian()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteI64(0x123456789ABCDEF0L);

            var bytes = ms.ToArray();
            Assert.AreEqual((byte)0x12, bytes[0]);
            Assert.AreEqual((byte)0x34, bytes[1]);
            Assert.AreEqual((byte)0x56, bytes[2]);
            Assert.AreEqual((byte)0x78, bytes[3]);
            Assert.AreEqual((byte)0x9A, bytes[4]);
            Assert.AreEqual((byte)0xBC, bytes[5]);
            Assert.AreEqual((byte)0xDE, bytes[6]);
            Assert.AreEqual((byte)0xF0, bytes[7]);
        }

        [Test]
        public void TestFloatBigEndian()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteFloat(1.0f);

            var bytes = ms.ToArray();
            // IEEE 754: 1.0f = 0x3F800000 in big-endian
            Assert.AreEqual((byte)0x3F, bytes[0]);
            Assert.AreEqual((byte)0x80, bytes[1]);
            Assert.AreEqual((byte)0x00, bytes[2]);
            Assert.AreEqual((byte)0x00, bytes[3]);
        }

        [Test]
        public void TestDoubleBigEndian()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteDouble(1.0);

            var bytes = ms.ToArray();
            // IEEE 754: 1.0d = 0x3FF0000000000000 in big-endian
            Assert.AreEqual((byte)0x3F, bytes[0]);
            Assert.AreEqual((byte)0xF0, bytes[1]);
            Assert.AreEqual((byte)0x00, bytes[2]);
            Assert.AreEqual((byte)0x00, bytes[3]);
            Assert.AreEqual((byte)0x00, bytes[4]);
            Assert.AreEqual((byte)0x00, bytes[5]);
            Assert.AreEqual((byte)0x00, bytes[6]);
            Assert.AreEqual((byte)0x00, bytes[7]);
        }

        // === Null rejection tests ===

        [Test]
        public void TestWriteBinaryNullThrows()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            Assert.Throws<ArgumentNullException>(() => writer.WriteBinary(null));
        }

        [Test]
        public void TestWriteStringNullThrows()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            Assert.Throws<ArgumentNullException>(() => writer.WriteString(null));
        }

        // === Bool validation ===

        [Test]
        public void TestBoolRejectsInvalidValue()
        {
            using var ms = new MemoryStream(new byte[] { 0x02 });
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.ReadBool());
        }

        // === Malformed payload tests (binary-specific wire format) ===

        private static MemoryStream MakeStringPayload(int declaredLength, int actualBodyBytes)
        {
            var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            writer.WriteI32(declaredLength);
            if (actualBodyBytes > 0)
            {
                ms.Write(new byte[actualBodyBytes], 0, actualBodyBytes);
            }
            ms.Position = 0;
            return ms;
        }

        [Test]
        public void TestReadStringRejectsNegativeLength()
        {
            using var ms = MakeStringPayload(-1, 0);
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.ReadString());
        }

        [Test]
        public void TestReadBinaryRejectsNegativeLength()
        {
            using var ms = MakeStringPayload(-1, 0);
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.ReadBinary());
        }

        [Test]
        public void TestReadStringRejectsLengthExceedingRemainingBytes()
        {
            using var ms = MakeStringPayload(1000, 10);
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.ReadString());
        }

        [Test]
        public void TestReadBinaryRejectsLengthExceedingRemainingBytes()
        {
            using var ms = MakeStringPayload(1000, 10);
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.ReadBinary());
        }

        // === Nesting depth limit tests (binary uses ThriftProtocolHelper with depth tracking) ===

        [Test]
        public void TestSkipDeeplyNestedStructThrowsDepthExceeded()
        {
            const int depth = 200;
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);

            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldBegin(ThriftWireType.Struct, 1);
            }
            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldStop();
            }

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            Assert.Throws<ThriftProtocolException>(() => reader.Skip(ThriftWireType.Struct));
        }

        // === Skip error message tests ===

        [Test]
        public void TestSkipDepthExceededIncludesFieldIdFromNestedStruct()
        {
            const int depth = 200;
            const short fieldId = 7;
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);

            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldBegin(ThriftWireType.Struct, fieldId);
            }
            for (int i = 0; i < depth; i++)
            {
                writer.WriteFieldStop();
            }

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.Skip(ThriftWireType.Struct));
            StringAssert.Contains($"field id={fieldId}", ex.Message);
            StringAssert.Contains("Struct", ex.Message);
        }

        [Test]
        public void TestSkipDepthExceededWithoutFieldIdShowsType()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);

            for (int i = 0; i < 200; i++)
            {
                writer.WriteFieldBegin(ThriftWireType.Struct, 1);
            }
            for (int i = 0; i < 200; i++)
            {
                writer.WriteFieldStop();
            }

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.Skip(ThriftWireType.Struct));
            StringAssert.Contains("exceeded while skipping", ex.Message);
            StringAssert.Contains("Struct", ex.Message);
        }

        [Test]
        public void TestSkipWithExplicitFieldIdIncludesItInError()
        {
            using var ms = new MemoryStream();
            var reader = new ThriftBinaryReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(
                () => reader.Skip((ThriftWireType)99, fieldId: 42));
            StringAssert.Contains("field id=42", ex.Message);
        }

        // === ThriftProtocolHelper.ReadValue/WriteValue generic tests ===

        [Test]
        public void TestReadWriteValueListRoundTrip()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            var original = new System.Collections.Generic.List<int> { 1, 2, 3, 4, 5 };
            ThriftProtocolHelper.WriteValue(writer, original);

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.List<int>>(reader);
            Assert.AreEqual(original.Count, result.Count);
            for (int i = 0; i < original.Count; i++)
            {
                Assert.AreEqual(original[i], result[i]);
            }
        }

        [Test]
        public void TestReadWriteValueDictionaryRoundTrip()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            var original = new System.Collections.Generic.Dictionary<string, int>
            {
                { "alpha", 1 },
                { "beta", 2 },
                { "gamma", 3 }
            };
            ThriftProtocolHelper.WriteValue(writer, original);

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.Dictionary<string, int>>(reader);
            Assert.AreEqual(original.Count, result.Count);
            foreach (var kvp in original)
            {
                Assert.IsTrue(result.ContainsKey(kvp.Key));
                Assert.AreEqual(kvp.Value, result[kvp.Key]);
            }
        }

        [Test]
        public void TestReadWriteValueHashSetRoundTrip()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            var original = new System.Collections.Generic.HashSet<string> { "x", "y", "z" };
            ThriftProtocolHelper.WriteValue(writer, original);

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.HashSet<string>>(reader);
            Assert.AreEqual(original.Count, result.Count);
            foreach (var item in original)
            {
                Assert.IsTrue(result.Contains(item));
            }
        }

        [Test]
        public void TestReadWriteValueNestedListRoundTrip()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            var original = new System.Collections.Generic.List<System.Collections.Generic.List<int>>
            {
                new System.Collections.Generic.List<int> { 1, 2 },
                new System.Collections.Generic.List<int> { 3, 4, 5 }
            };
            ThriftProtocolHelper.WriteValue(writer, original);

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.List<System.Collections.Generic.List<int>>>(reader);
            Assert.AreEqual(original.Count, result.Count);
            for (int i = 0; i < original.Count; i++)
            {
                Assert.AreEqual(original[i].Count, result[i].Count);
                for (int j = 0; j < original[i].Count; j++)
                {
                    Assert.AreEqual(original[i][j], result[i][j]);
                }
            }
        }

        [Test]
        public void TestReadWriteValueRepeatedCallsExerciseCache()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            for (int i = 0; i < 100; i++)
            {
                ThriftProtocolHelper.WriteValue(writer, new System.Collections.Generic.List<int> { i, i * 2 });
            }

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            for (int i = 0; i < 100; i++)
            {
                var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.List<int>>(reader);
                Assert.AreEqual(2, result.Count);
                Assert.AreEqual(i, result[0]);
                Assert.AreEqual(i * 2, result[1]);
            }
        }

        [Test]
        public void TestReadWriteValueMapRepeatedCallsExerciseCache()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftBinaryWriter(ms);
            for (int i = 0; i < 50; i++)
            {
                var dict = new System.Collections.Generic.Dictionary<string, long>
                {
                    { $"key{i}", i * 100L }
                };
                ThriftProtocolHelper.WriteValue(writer, dict);
            }

            ms.Position = 0;
            var reader = new ThriftBinaryReader(ms);
            for (int i = 0; i < 50; i++)
            {
                var result = ThriftProtocolHelper.ReadValue<System.Collections.Generic.Dictionary<string, long>>(reader);
                Assert.AreEqual(1, result.Count);
                Assert.AreEqual(i * 100L, result[$"key{i}"]);
            }
        }

        // === Union bool round-trip test (reproducing conformance failure) ===

        [Test]
        public void TestUnionBoolTrueRoundTripBinary()
        {
            // Simulate a union with a single bool field (field_1, id=1) set to true.
            // This matches the conformance test: testset.union.bool/true.Binary
            //
            // Expected Binary wire format:
            //   field header: type=BOOL(2), field_id=1 → bytes: 02, 00, 01
            //   bool value: true → byte: 01
            //   field stop → byte: 00
            var expected = new byte[] { 0x02, 0x00, 0x01, 0x01, 0x00 };

            // Write: manually construct the same wire bytes
            using var writeMs = new MemoryStream();
            var writer = new ThriftBinaryWriter(writeMs);
            writer.WriteFieldBegin(ThriftWireType.Bool, 1);
            writer.WriteBool(true);
            writer.WriteFieldStop();
            var written = writeMs.ToArray();

            Assert.AreEqual(expected, written,
                $"Written bytes mismatch.");

            // Read back and verify
            using var readMs = new MemoryStream(written);
            var reader = new ThriftBinaryReader(readMs);
            var (fieldType, fieldId) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Bool, fieldType);
            Assert.AreEqual((short)1, fieldId);
            Assert.AreEqual(true, reader.ReadBool());
            var (stopType, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stopType);

            // Full round-trip: read then write again, compare bytes
            using var readMs2 = new MemoryStream(expected);
            var reader2 = new ThriftBinaryReader(readMs2);
            var (ft2, fid2) = reader2.ReadFieldBegin();
            var boolVal = reader2.ReadBool();
            var (stop2, _) = reader2.ReadFieldBegin();

            using var writeMs2 = new MemoryStream();
            var writer2 = new ThriftBinaryWriter(writeMs2);
            writer2.WriteFieldBegin(ft2, fid2);
            writer2.WriteBool(boolVal);
            writer2.WriteFieldStop();
            var roundTripped = writeMs2.ToArray();

            Assert.AreEqual(expected, roundTripped,
                $"Round-trip bytes mismatch.");
        }
    }
}
