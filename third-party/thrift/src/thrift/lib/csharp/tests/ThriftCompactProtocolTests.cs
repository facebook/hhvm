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
using FBThrift;
using NUnit.Framework;

namespace FBThrift.Tests
{
    /// <summary>
    /// Tests specific to the compact protocol wire format:
    /// delta field IDs, inline booleans, compressed collection headers,
    /// varint/zigzag encoding, and CompactType conversions.
    /// </summary>
    [TestFixture]
    public class ThriftCompactSpecificTests
    {
        // === Delta field ID encoding ===

        [Test]
        public void TestDeltaFieldIds()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteFieldBegin(ThriftWireType.I32, 1);
            writer.WriteI32(10);
            writer.WriteFieldBegin(ThriftWireType.I32, 2);
            writer.WriteI32(20);
            writer.WriteFieldBegin(ThriftWireType.I32, 3);
            writer.WriteI32(30);
            writer.WriteFieldStop();

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);

            var (t1, id1) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t1);
            Assert.AreEqual((short)1, id1);
            Assert.AreEqual(10, reader.ReadI32());

            var (t2, id2) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t2);
            Assert.AreEqual((short)2, id2);
            Assert.AreEqual(20, reader.ReadI32());

            var (t3, id3) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t3);
            Assert.AreEqual((short)3, id3);
            Assert.AreEqual(30, reader.ReadI32());

            var (stop, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stop);
        }

        [Test]
        public void TestLargeFieldIdDelta()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteFieldBegin(ThriftWireType.I32, 1);
            writer.WriteI32(10);
            writer.WriteFieldBegin(ThriftWireType.I32, 100);
            writer.WriteI32(20);
            writer.WriteFieldStop();

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);

            var (t1, id1) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t1);
            Assert.AreEqual((short)1, id1);
            reader.ReadI32();

            var (t2, id2) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t2);
            Assert.AreEqual((short)100, id2);
            reader.ReadI32();

            var (stop, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stop);
        }

        // === Inline boolean encoding ===

        [Test]
        public void TestInlineBoolTrue()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteFieldBegin(ThriftWireType.Bool, 1);
            writer.WriteBool(true);
            writer.WriteFieldStop();

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);

            var (type, id) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Bool, type);
            Assert.AreEqual((short)1, id);
            Assert.AreEqual(true, reader.ReadBool());

            var (stop, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stop);
        }

        [Test]
        public void TestInlineBoolFalse()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteFieldBegin(ThriftWireType.Bool, 1);
            writer.WriteBool(false);
            writer.WriteFieldStop();

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);

            var (type, id) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Bool, type);
            Assert.AreEqual((short)1, id);
            Assert.AreEqual(false, reader.ReadBool());

            var (stop, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stop);
        }

        [Test]
        public void TestNonFieldBoolInCollection()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteListBegin(ThriftWireType.Bool, 3);
            writer.WriteBool(true);
            writer.WriteBool(false);
            writer.WriteBool(true);

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            var (elemType, size) = reader.ReadListBegin();
            Assert.AreEqual(ThriftWireType.Bool, elemType);
            Assert.AreEqual(3, size);
            Assert.AreEqual(true, reader.ReadBool());
            Assert.AreEqual(false, reader.ReadBool());
            Assert.AreEqual(true, reader.ReadBool());
        }

        // === Compressed collection headers ===

        [Test]
        public void TestSmallCollectionHeader()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteListBegin(ThriftWireType.I32, 3);

            // Small list (≤14 elements): single byte header (size << 4) | compactType
            // CompactType.I32 = 5, so byte = (3 << 4) | 5 = 0x35
            var bytes = ms.ToArray();
            Assert.AreEqual(1, bytes.Length);
            Assert.AreEqual(0x35, bytes[0]);
        }

        [Test]
        public void TestLargeCollectionHeader()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteListBegin(ThriftWireType.I32, 20);

            // Large list (>14 elements): 0xF0 | compactType, then varint(size)
            // CompactType.I32 = 5, so first byte = 0xF5, varint(20) = 0x14
            var bytes = ms.ToArray();
            Assert.AreEqual(2, bytes.Length);
            Assert.AreEqual(0xF5, bytes[0]);
            Assert.AreEqual(0x14, bytes[1]);
        }

        [Test]
        public void TestEmptyMap()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteMapBegin(ThriftWireType.String, ThriftWireType.I32, 0);

            // Empty map: just varint(0) = 0x00
            var bytes = ms.ToArray();
            Assert.AreEqual(1, bytes.Length);
            Assert.AreEqual(0x00, bytes[0]);

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            var (_, _, size) = reader.ReadMapBegin();
            Assert.AreEqual(0, size);
        }

        // === Zigzag encoding verification ===

        [Test]
        public void TestZigzagEncodingKnownValues()
        {
            // zigzag(0)=0, zigzag(-1)=1, zigzag(1)=2, zigzag(-2)=3
            AssertI32Bytes(0, new byte[] { 0x00 });
            AssertI32Bytes(-1, new byte[] { 0x01 });
            AssertI32Bytes(1, new byte[] { 0x02 });
            AssertI32Bytes(-2, new byte[] { 0x03 });
            AssertI32Bytes(int.MaxValue, new byte[] { 0xFE, 0xFF, 0xFF, 0xFF, 0x0F });
            AssertI32Bytes(int.MinValue, new byte[] { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F });
        }

        private static void AssertI32Bytes(int value, byte[] expected)
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteI32(value);
            Assert.AreEqual(expected, ms.ToArray(),
                $"I32 encoding mismatch for value {value}");
        }

        // === Varint overflow protection ===

        [Test]
        public void TestVarint32TooLongThrows()
        {
            // 5 continuation bytes triggers "Varint32 too long"
            var bytes = new byte[] { 0x80, 0x80, 0x80, 0x80, 0x80 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadI32());
            StringAssert.Contains("Varint32 too long", ex.Message);
        }

        [Test]
        public void TestVarint64TooLongThrows()
        {
            // 10 continuation bytes triggers "Varint64 too long"
            var bytes = new byte[] { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadI64());
            StringAssert.Contains("Varint64 too long", ex.Message);
        }

        // === CompactType conversion tests ===

        [Test]
        public void TestCompactTypeToThriftWireTypeKnownValues()
        {
            Assert.AreEqual(ThriftWireType.Stop, CompactType.Stop.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Bool, CompactType.BoolTrue.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Bool, CompactType.BoolFalse.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Byte, CompactType.Byte.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.I16, CompactType.I16.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.I32, CompactType.I32.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.I64, CompactType.I64.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Double, CompactType.Double.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.String, CompactType.Binary.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.List, CompactType.List.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Set, CompactType.Set.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Map, CompactType.Map.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Struct, CompactType.Struct.ToThriftWireType());
            Assert.AreEqual(ThriftWireType.Float, CompactType.Float.ToThriftWireType());
        }

        [Test]
        public void TestCompactTypeToThriftWireTypeUnknownThrows()
        {
            Assert.Throws<ThriftProtocolException>(() => ((CompactType)99).ToThriftWireType());
        }

        [Test]
        public void TestCompactTypeFromThriftWireTypeRoundTrip()
        {
            ThriftWireType[] types = {
                ThriftWireType.Stop, ThriftWireType.Bool, ThriftWireType.Byte,
                ThriftWireType.I16, ThriftWireType.I32, ThriftWireType.I64,
                ThriftWireType.Double, ThriftWireType.String, ThriftWireType.List,
                ThriftWireType.Set, ThriftWireType.Map, ThriftWireType.Struct,
                ThriftWireType.Float
            };

            foreach (var wireType in types)
            {
                var compact = wireType.ToCompactType();
                var roundTripped = compact.ToThriftWireType();
                Assert.AreEqual(wireType, roundTripped,
                    $"Round-trip failed for {wireType}");
            }
        }

        // === Nested struct field ID stack ===

        [Test]
        public void TestNestedStructFieldIdStack()
        {
            // Build compact-encoded bytes for:
            //   field 1 (I32, value=100)
            //   field 2 (Struct, containing field 1 (I32, value=200) + stop)
            //   field 3 (I32, value=300)
            //   stop
            //
            // This tests that Skip(Struct) correctly pushes/pops the field ID state.
            // After skipping the inner struct, field 3 should read as ID=3 (delta from 2),
            // not as ID=2 (which would happen if _lastFieldId wasn't restored).
            var bytes = new byte[] {
                0x15,               // field 1: delta=1, type=I32 (compact 5)
                0xC8, 0x01,         // zigzag(100) = 200
                0x1C,               // field 2: delta=1, type=Struct (compact 12)
                0x15,               // inner field 1: delta=1 from 0 (reset), type=I32
                0x90, 0x03,         // zigzag(200) = 400
                0x00,               // inner stop
                0x15,               // field 3: delta=1 from 2 (restored), type=I32
                0xD8, 0x04,         // zigzag(300) = 600
                0x00                // outer stop
            };

            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);

            // Read field 1
            var (t1, id1) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t1);
            Assert.AreEqual((short)1, id1);
            Assert.AreEqual(100, reader.ReadI32());

            // Read field 2 (struct)
            var (t2, id2) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Struct, t2);
            Assert.AreEqual((short)2, id2);

            // Skip the inner struct — this must push/pop field ID state
            reader.Skip(ThriftWireType.Struct);

            // Field 3 must be ID=3 (delta=1 from outer field 2), not ID=2
            var (t3, id3) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.I32, t3);
            Assert.AreEqual((short)3, id3);
            Assert.AreEqual(300, reader.ReadI32());

            var (stop, _) = reader.ReadFieldBegin();
            Assert.AreEqual(ThriftWireType.Stop, stop);
        }

        // === Collection size validation tests ===

        [Test]
        public void TestListSizeExceedsRemainingBytesThrows()
        {
            // Compact list header: high nibble = size (15 means read varint), low nibble = element type
            // CompactType.I32 = 5, so 0xF5 means size=15 (read varint), element type = I32
            // Then varint 0x80 0x80 0x01 = 16384 (way more than remaining bytes)
            var bytes = new byte[] { 0xF5, 0x80, 0x80, 0x01 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadListBegin());
            StringAssert.Contains("exceeds remaining bytes", ex.Message);
        }

        [Test]
        public void TestSetSizeExceedsRemainingBytesThrows()
        {
            // Same as list - set delegates to ReadListBegin
            var bytes = new byte[] { 0xF5, 0x80, 0x80, 0x01 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadSetBegin());
            StringAssert.Contains("exceeds remaining bytes", ex.Message);
        }

        [Test]
        public void TestMapSizeExceedsRemainingBytesThrows()
        {
            // Compact map: first is varint size, then types byte (if size > 0)
            // Varint 0x80 0x80 0x01 = 16384, then types byte 0x55 (I32 key, I32 value)
            var bytes = new byte[] { 0x80, 0x80, 0x01, 0x55 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadMapBegin());
            StringAssert.Contains("exceeds remaining bytes", ex.Message);
        }

        [Test]
        public void TestNegativeListSizeThrows()
        {
            // Compact uses unsigned varint for collection sizes, but when cast to int
            // a very large value becomes negative.
            // 0xFF 0xFF 0xFF 0xFF 0x0F = 4294967295 (uint max), cast to int = -1
            var bytes = new byte[] { 0xF5, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadListBegin());
            StringAssert.Contains("Negative collection size", ex.Message);
        }

        [Test]
        public void TestNegativeMapSizeThrows()
        {
            // Same pattern for map
            var bytes = new byte[] { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x55 };
            using var ms = new MemoryStream(bytes);
            var reader = new ThriftCompactReader(ms);
            var ex = Assert.Throws<ThriftProtocolException>(() => reader.ReadMapBegin());
            StringAssert.Contains("Negative collection size", ex.Message);
        }

        // === String round-trip tests ===

        [Test]
        public void TestStringRoundTripAscii()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteString("hello");

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            Assert.AreEqual("hello", reader.ReadString());
        }

        [Test]
        public void TestStringRoundTripEmpty()
        {
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteString("");

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            Assert.AreEqual("", reader.ReadString());
        }

        [Test]
        public void TestStringRoundTripUtf8()
        {
            var value = "こんにちは世界 🌍";
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteString(value);

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            Assert.AreEqual(value, reader.ReadString());
        }

        [Test]
        public void TestStringRoundTripLarge()
        {
            var value = new string('x', 4096);
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteString(value);

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            Assert.AreEqual(value, reader.ReadString());
        }

        [Test]
        public void TestValidCollectionSizePasses()
        {
            // A valid small list: 3 I32 elements (header 0x35 = size 3, type I32)
            // followed by actual data (3 zigzag-encoded varints)
            using var ms = new MemoryStream();
            var writer = new ThriftCompactWriter(ms);
            writer.WriteListBegin(ThriftWireType.I32, 3);
            writer.WriteI32(1);
            writer.WriteI32(2);
            writer.WriteI32(3);

            ms.Position = 0;
            var reader = new ThriftCompactReader(ms);
            var (elemType, size) = reader.ReadListBegin();
            Assert.AreEqual(ThriftWireType.I32, elemType);
            Assert.AreEqual(3, size);
            Assert.AreEqual(1, reader.ReadI32());
            Assert.AreEqual(2, reader.ReadI32());
            Assert.AreEqual(3, reader.ReadI32());
        }
    }
}
