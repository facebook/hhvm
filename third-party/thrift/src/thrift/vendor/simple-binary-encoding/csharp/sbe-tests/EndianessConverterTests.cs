// Portions Copyright (C) 2017 MarketFactory, Inc
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

using System;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Org.SbeTool.Sbe.Dll;

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public class EndianessConverterTests
    {
        [TestMethod]
        public void ApplyShortWithLittleEndianShouldNoOp()
        {
            const short input = 12;

            var result = EndianessConverter.ApplyInt16(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyShortWithBigEndianShouldReverseBytes()
        {
            const short input = 12;

            var result = EndianessConverter.ApplyInt16(ByteOrder.BigEndian, input);

            short expected = BitConverter.ToInt16(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyUShortWithLittleEndianShouldNoOp()
        {
            const ushort input = 12;

            var result = EndianessConverter.ApplyUint16(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyUShortWithBigEndianShouldReverseBytes()
        {
            const ushort input = 12;

            var result = EndianessConverter.ApplyUint16(ByteOrder.BigEndian, input);

            ushort expected = BitConverter.ToUInt16(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyIntWithLittleEndianShouldNoOp()
        {
            const int input = 12;

            var result = EndianessConverter.ApplyInt32(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyIntWithBigEndianShouldReverseBytes()
        {
            const int input = 12;

            var result = EndianessConverter.ApplyInt32(ByteOrder.BigEndian, input);

            int expected = BitConverter.ToInt32(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyUIntWithLittleEndianShouldNoOp()
        {
            const uint input = 12;

            var result = EndianessConverter.ApplyUint32(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyUIntWithBigEndianShouldReverseBytes()
        {
            const uint input = 12;

            var result = EndianessConverter.ApplyUint32(ByteOrder.BigEndian, input);

            uint expected = BitConverter.ToUInt32(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyULongWithLittleEndianShouldNoOp()
        {
            const ulong input = 12;

            var result = EndianessConverter.ApplyUint64(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyULongWithBigEndianShouldReverseBytes()
        {
            const ulong input = 12;

            var result = EndianessConverter.ApplyUint64(ByteOrder.BigEndian, input);
            
            ulong expected = BitConverter.ToUInt64(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyLongWithLittleEndianShouldNoOp()
        {
            const long input = 12;

            var result = EndianessConverter.ApplyInt64(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyLongWithBigEndianShouldReverseBytes()
        {
            const long input = 12;

            var result = EndianessConverter.ApplyInt64(ByteOrder.BigEndian, input);

            long expected = BitConverter.ToInt64(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyDoubleWithLittleEndianShouldNoOp()
        {
            const double input = 12;

            var result = EndianessConverter.ApplyDouble(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyDoubleWithBigEndianShouldReverseBytes()
        {
            const double input = 12;

            var result = EndianessConverter.ApplyDouble(ByteOrder.BigEndian, input);

            double expected = BitConverter.ToDouble(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }

        [TestMethod]
        public void ApplyFloatWithLittleEndianShouldNoOp()
        {
            const float input = 12;

            var result = EndianessConverter.ApplyFloat(ByteOrder.LittleEndian, input);

            Assert.AreEqual(input, result);
        }

        [TestMethod]
        public void ApplyFloatWithBigEndianShouldReverseBytes()
        {
            const float input = 12;

            var result = EndianessConverter.ApplyFloat(ByteOrder.BigEndian, input);

            float expected = BitConverter.ToSingle(BitConverter.GetBytes(input).Reverse().ToArray(), 0);
            Assert.AreEqual(expected, result);
        }
    }
}
