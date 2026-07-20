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
using FBThrift;
using NUnit.Framework;
using ThriftType = facebook.thrift.type;
using ThriftVoid = facebook.thrift.type.Void;

namespace FBThrift.Tests
{
    [TestFixture]
    public class GeneratedAnyTests
    {
        [Test]
        public void AnyStructBinaryRoundTripPreservesNestedTypeAndPayload()
        {
            var original = new ThriftType.AnyStruct
            {
                type = new ThriftType.TypeStruct
                {
                    name = new ThriftType.TypeName { mapType = ThriftVoid.Unused },
                    @params = new List<ThriftType.TypeStruct>
                    {
                        PrimitiveType(new ThriftType.TypeName { stringType = ThriftVoid.Unused }),
                        new ThriftType.TypeStruct
                        {
                            name = new ThriftType.TypeName { listType = ThriftVoid.Unused },
                            @params = new List<ThriftType.TypeStruct>
                            {
                                PrimitiveType(new ThriftType.TypeName { i64Type = ThriftVoid.Unused }),
                            },
                        },
                    },
                },
                protocol = new ThriftType.ProtocolUnion { standard = ThriftType.StandardProtocol.Compact },
                data = new byte[] { 0x00, 0x01, 0x7f, 0x80, 0xff },
            };

            var decoded = RoundTripBinary(original);

            Assert.AreEqual(original, decoded);
            CollectionAssert.AreEqual(original.data, decoded.data);
        }

        [Test]
        public void AnyStructCompactRoundTripPreservesNamedTypeAndCustomProtocol()
        {
            var original = new ThriftType.AnyStruct
            {
                type = new ThriftType.TypeStruct
                {
                    name = new ThriftType.TypeName
                    {
                        structType = new ThriftType.TypeUri { uri = "example.com/types/Record" },
                    },
                },
                protocol = new ThriftType.ProtocolUnion { custom = "example.com/protocol/custom" },
                data = new byte[] { 0x10, 0x20, 0x30 },
            };

            var encoded = ThriftSerializer.EncodeCompact(original);
            var decoded = ThriftSerializer.DecodeCompact<ThriftType.AnyStruct>(encoded);

            Assert.AreEqual(original, decoded);
        }

        [Test]
        public void SemiAnyStructRoundTripPreservesOpaqueDataWithoutMetadata()
        {
            var original = new ThriftType.SemiAnyStruct
            {
                data = new byte[] { 0xde, 0xad, 0xbe, 0xef },
            };

            var decoded = RoundTripBinary(original);

            Assert.IsTrue(decoded.type.__fbthrift_is_empty());
            Assert.IsTrue(decoded.protocol.__fbthrift_is_empty());
            CollectionAssert.AreEqual(original.data, decoded.data);
        }

        [Test]
        public void TypeNameAllVariantsRoundTrip()
        {
            var variants = new[]
            {
                new ThriftType.TypeName { boolType = ThriftVoid.Unused },
                new ThriftType.TypeName { byteType = ThriftVoid.Unused },
                new ThriftType.TypeName { i16Type = ThriftVoid.Unused },
                new ThriftType.TypeName { i32Type = ThriftVoid.Unused },
                new ThriftType.TypeName { i64Type = ThriftVoid.Unused },
                new ThriftType.TypeName { floatType = ThriftVoid.Unused },
                new ThriftType.TypeName { doubleType = ThriftVoid.Unused },
                new ThriftType.TypeName { stringType = ThriftVoid.Unused },
                new ThriftType.TypeName { binaryType = ThriftVoid.Unused },
                new ThriftType.TypeName { enumType = TypeUri() },
                new ThriftType.TypeName { typedefType = TypeUri() },
                new ThriftType.TypeName { structType = TypeUri() },
                new ThriftType.TypeName { unionType = TypeUri() },
                new ThriftType.TypeName { exceptionType = TypeUri() },
                new ThriftType.TypeName { listType = ThriftVoid.Unused },
                new ThriftType.TypeName { setType = ThriftVoid.Unused },
                new ThriftType.TypeName { mapType = ThriftVoid.Unused },
            };

            foreach (var variant in variants)
            {
                Assert.AreEqual(variant, RoundTripBinary(variant));
                Assert.AreEqual(
                    variant,
                    ThriftSerializer.DecodeCompact<ThriftType.TypeName>(ThriftSerializer.EncodeCompact(variant)));
            }
        }

        [Test]
        public void TypeUriAllVariantsRoundTrip()
        {
            var variants = new[]
            {
                new ThriftType.TypeUri { uri = "example.com/types/Record" },
                new ThriftType.TypeUri { typeHashPrefixSha2_256 = new byte[] { 0x01, 0x02, 0x03 } },
                new ThriftType.TypeUri { scopedName = "types.Record" },
                new ThriftType.TypeUri { definitionKey = new byte[] { 0xaa, 0xbb } },
            };

            foreach (var variant in variants)
            {
                Assert.AreEqual(variant, RoundTripBinary(variant));
            }
        }

        [Test]
        public void ProtocolUnionAllVariantsRoundTrip()
        {
            var variants = new[]
            {
                new ThriftType.ProtocolUnion { standard = ThriftType.StandardProtocol.Binary },
                new ThriftType.ProtocolUnion { custom = "example.com/protocol/custom" },
                new ThriftType.ProtocolUnion { id = 123456789L },
                new ThriftType.ProtocolUnion
                {
                    compressed = new ThriftType.CompressedProtocolStruct
                    {
                        protocol = new ThriftType.ProtocolUnion { standard = ThriftType.StandardProtocol.Compact },
                        compression = new ThriftType.CompressionSpecStruct
                        {
                            id = 7,
                            uncompressed_data_size_bytes = 4096,
                            compression_parameters = new byte[] { 0x01, 0x02 },
                        },
                    },
                },
            };

            foreach (var variant in variants)
            {
                Assert.AreEqual(variant, RoundTripBinary(variant));
            }
        }

        [Test]
        public void AnyStructRejectsNullRequiredFields()
        {
            var value = new ThriftType.AnyStruct();

            Assert.Throws<ArgumentNullException>(() => value.type = null!);
            Assert.Throws<ArgumentNullException>(() => value.protocol = null!);
            Assert.Throws<ArgumentNullException>(() => value.data = null!);
        }

        [Test]
        public void AnyStructBinaryGoldenValueIsStable()
        {
            var golden = new byte[]
            {
                0x0c, 0x00, 0x01,
                0x0c, 0x00, 0x01,
                0x08, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
                0x00,
                0x0f, 0x00, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00,
                0x00,
                0x0c, 0x00, 0x02,
                0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02,
                0x00,
                0x0b, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04,
                0x00, 0x00, 0x00, 0x2a,
                0x00,
            };

            var decoded = ThriftSerializer.DecodeBinary<ThriftType.AnyStruct>(golden);

            Assert.AreEqual(ThriftType.TypeName.Type.i32Type, decoded.type.name.GetUnionType());
            Assert.AreEqual(ThriftType.StandardProtocol.Compact, decoded.protocol.standard);
            CollectionAssert.AreEqual(new byte[] { 0x00, 0x00, 0x00, 0x2a }, decoded.data);
            CollectionAssert.AreEqual(golden, ThriftSerializer.EncodeBinary(decoded));
        }

        private static ThriftType.TypeStruct PrimitiveType(ThriftType.TypeName name)
        {
            return new ThriftType.TypeStruct { name = name };
        }

        private static ThriftType.TypeUri TypeUri()
        {
            return new ThriftType.TypeUri { uri = "example.com/types/Record" };
        }

        private static T RoundTripBinary<T>(T value) where T : IThriftSerializable, new()
        {
            return ThriftSerializer.DecodeBinary<T>(ThriftSerializer.EncodeBinary(value));
        }
    }
}
