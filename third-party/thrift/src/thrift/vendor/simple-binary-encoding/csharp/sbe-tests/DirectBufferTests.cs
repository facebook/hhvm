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
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Org.SbeTool.Sbe.Dll;

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public unsafe class DirectBufferTests
    {
        private byte[] _buffer;
        private DirectBuffer _directBuffer;
        private byte* _pBuffer;

        [TestInitialize]
        public void SetUp()
        {
            _buffer = new Byte[16];
            _directBuffer = new DirectBuffer(_buffer);
            var handle = GCHandle.Alloc(_buffer, GCHandleType.Pinned);

            _pBuffer = (byte*)handle.AddrOfPinnedObject().ToPointer();
        }

        [TestMethod]
        public void CheckPositionShouldNotThrowWhenPositionIsInRange()
        {
            _directBuffer.CheckLimit(_buffer.Length);
        }

        [TestMethod]
        public void CheckPositionShouldThrowWhenPositionIsNotInRange()
        {
            Assert.ThrowsException<IndexOutOfRangeException>(() => _directBuffer.CheckLimit(_buffer.Length + 1));
        }

        [TestMethod]
        public void ConstructFromNativeBuffer()
        {
            var managedBuffer = new Byte[16];
            var handle = GCHandle.Alloc(managedBuffer, GCHandleType.Pinned);
            var unmanagedBuffer = (byte*) handle.AddrOfPinnedObject().ToPointer();

            const int value = 5;
            const int index = 0;
            
            using (var directBufferFromUnmanagedBuffer = new DirectBuffer(unmanagedBuffer, managedBuffer.Length))
            {
                directBufferFromUnmanagedBuffer.Int64PutLittleEndian(index, value);
                Assert.AreEqual(value, *(long*) (unmanagedBuffer + index));
            }
        }

        [TestMethod]
        public void ConstructFromByteArray()
        {
            var buffer = new byte[16];

            const int value = 5;
            const int index = 0;
            
            using (var directBuffer = new DirectBuffer(buffer))
            {
                directBuffer.Int64PutLittleEndian(index, value);
                Assert.AreEqual(value, buffer[index]);
            }
        }

        [TestMethod]
        public void ConstructFromArraySegment()
        {
            const int value = 5;
            const int index = 0;
            const int offset = 512;
            const int size = 1024;

            var bigBuffer = new byte[size];
            var buffer = new ArraySegment<byte>(bigBuffer, offset, size - offset);
            
            using (var directBuffer = new DirectBuffer(buffer))
            {
                directBuffer.Int64PutLittleEndian(index, value);
                Assert.AreEqual(value, bigBuffer[offset + index]);
                Assert.AreEqual(value, buffer.AsSpan<byte>()[index]);
            }
        }

        [TestMethod]
        public void Recycle()
        {
            using (var directBuffer = new DirectBuffer())
            {
                var firstBuffer = new Byte[16];
                directBuffer.Wrap(firstBuffer);

                directBuffer.Int64PutLittleEndian(0, 1);
                Assert.AreEqual(1, BitConverter.ToInt64(firstBuffer, 0));

                var secondBuffer = new byte[16];
                var secondBufferHandle = GCHandle.Alloc(secondBuffer, GCHandleType.Pinned);
                var secondUnmanagedBuffer = (byte*)secondBufferHandle.AddrOfPinnedObject().ToPointer();
                directBuffer.Wrap(secondUnmanagedBuffer, 16);
                directBuffer.Int64PutLittleEndian(0, 2);
                Assert.AreEqual(2, BitConverter.ToInt64(secondBuffer, 0));
            }
        }

        [TestMethod]
        public void Reallocate()
        {
            const int initialBufferSize = 8;
            var initialBuffer = new Byte[initialBufferSize];
            
            const int biggerBufferSize = 100;
            var biggerBuffer = new byte[biggerBufferSize];
            
            var reallocableBuffer = new DirectBuffer(initialBuffer, 
                (existingSize, requestedSize) =>
                {
                    Assert.AreEqual(initialBufferSize, existingSize);
                    Assert.AreEqual(16, requestedSize);
                    return biggerBuffer;
                });
            
            reallocableBuffer.CheckLimit(8);
            reallocableBuffer.Int64PutLittleEndian(0, 1);
            Assert.AreEqual(initialBufferSize, reallocableBuffer.Capacity);

            reallocableBuffer.CheckLimit(16);
            reallocableBuffer.Int64PutLittleEndian(8, 2);
            Assert.AreEqual(biggerBufferSize, reallocableBuffer.Capacity);

            Assert.AreEqual(1, BitConverter.ToInt64(biggerBuffer, 0));
            Assert.AreEqual(2, BitConverter.ToInt64(biggerBuffer, 8));
        }

        [TestMethod]
        public void ReallocateFailure()
        {
            const int initialBufferSize = 8;
            var initialBuffer = new Byte[initialBufferSize];
            var reallocableBuffer = new DirectBuffer(initialBuffer, (existingSize, requestedSize) =>
            {
                Assert.AreEqual(initialBufferSize, existingSize);
                Assert.AreEqual(16, requestedSize);
                return null;
            });

            reallocableBuffer.CheckLimit(8);
            reallocableBuffer.Int64PutLittleEndian(0, 1);
            Assert.AreEqual(initialBufferSize, reallocableBuffer.Capacity);
            Assert.ThrowsException<IndexOutOfRangeException>(() => reallocableBuffer.CheckLimit(16));
        }

        #region Byte

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        public void ShouldPutByte(byte value, int index)
        {
            _directBuffer.CharPut(index, value);

            Assert.AreEqual(value, _buffer[index]);
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        public void ShouldGetByte(byte value, int index)
        {
            _buffer[index] = value;

            var result = _directBuffer.CharGet(index);

            Assert.AreEqual(value, result);
        }

        #endregion

        #region Int8

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt8(sbyte value, int index)
        {
            _directBuffer.Int8Put(index, value);

            Assert.AreEqual(value, *(sbyte*) (_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt8(sbyte value, int index)
        {
            _buffer[index] = *(byte*) &value;

            var result = _directBuffer.Int8Get(index);

            Assert.AreEqual(value, result);
        }

        #endregion

        #region Int16

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt16LittleEndian(short value, int index)
        {
            _directBuffer.Int16PutLittleEndian(index, value);

            Assert.AreEqual(value, *(short*) (_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt16BigEndian(short value, int index)
        {
            _directBuffer.Int16PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyInt16(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(short*) (_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt16LittleEndian(short value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 2);

            var result = _directBuffer.Int16GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt16BigEndian(short value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 2);

            var result = _directBuffer.Int16GetBigEndian(index);

            var expected = EndianessConverter.ApplyInt16(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region Int32

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt32LittleEndian(int value, int index)
        {
            _directBuffer.Int32PutLittleEndian(index, value);

            Assert.AreEqual(value, *(int*)(_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt32BigEndian(int value, int index)
        {
            _directBuffer.Int32PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyInt32(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(int*)(_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt32LittleEndian(int value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.Int32GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt32BigEndian(int value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.Int32GetBigEndian(index);

            var expected = EndianessConverter.ApplyInt32(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region Int64

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt64LittleEndian(int value, int index)
        {
            _directBuffer.Int64PutLittleEndian(index, value);

            Assert.AreEqual(value, *(long*)(_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldPutInt64BigEndian(long value, int index)
        {
            _directBuffer.Int64PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyInt64(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(long*)(_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt64LittleEndian(long value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.Int64GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        [DataRow(-5, 8)]
        public void ShouldGetInt64BigEndian(long value, int index)
        {
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.Int64GetBigEndian(index);
            var expected = EndianessConverter.ApplyInt64(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region UInt8

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        public void ShouldPutUInt8(byte value, int index)
        {
            _directBuffer.Uint8Put(index, value);

            Assert.AreEqual(value, *(_pBuffer + index));
        }

        [DataRow(5, 0)]
        [DataRow(5, 8)]
        public void ShouldGetUInt8(byte value, int index)
        {
            _buffer[index] = *&value;

            var result = _directBuffer.Uint8Get(index);

            Assert.AreEqual(value, result);
        }

        #endregion

        #region UInt16

        [TestMethod]
        public void ShouldPutUInt16LittleEndian()
        {
            const ushort value = 5;
            const int index = 0;
            _directBuffer.Uint16PutLittleEndian(index, value);

            Assert.AreEqual(value, *(ushort*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldPutUInt16BigEndian()
        {
            const ushort value = 5;
            const int index = 0;
            _directBuffer.Uint16PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyUint16(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(ushort*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldGetUInt16LittleEndian()
        {
            const ushort value = 5;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 2);

            var result = _directBuffer.Uint16GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

       [TestMethod]
        public void ShouldGetUInt16BigEndian()
        {
            const ushort value = 5;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 2);

            var result = _directBuffer.Uint16GetBigEndian(index);

            var expected = EndianessConverter.ApplyUint16(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region UInt32

        [TestMethod]
        public void ShouldPutUInt32LittleEndian()
        {
            const uint value = 5;
            const int index = 0;
            _directBuffer.Uint32PutLittleEndian(index, value);

            Assert.AreEqual(value, *(uint*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldPutUInt32BigEndian()
        {
            const uint value = 5;
            const int index = 0;
            _directBuffer.Uint32PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyUint32(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(uint*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldGetUInt32LittleEndian()
        {
            const uint value = 5;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.Uint32GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldGetUInt32BigEndian()
        {
            const uint value = 5;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.Uint32GetBigEndian(index);

            var expected = EndianessConverter.ApplyUint32(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region UInt64

        [TestMethod]
        public void ShouldPutUInt64LittleEndian()
        {
            const ulong value = ulong.MaxValue - 1;
            const int index = 0;
            _directBuffer.Uint64PutLittleEndian(index, value);

            Assert.AreEqual(value, *(ulong*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldPutUInt64BigEndian()
        {
            const ulong value = ulong.MaxValue - 1;
            const int index = 0;
            _directBuffer.Uint64PutBigEndian(index, value);

            var expected = EndianessConverter.ApplyUint64(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(ulong*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldGetUInt64LittleEndian()
        {
            const ulong value = ulong.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.Uint64GetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldGetUInt64BigEndian()
        {
            const ulong value = ulong.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.Uint64GetBigEndian(index);

            var expected = EndianessConverter.ApplyUint64(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region Float

        [TestMethod]
        public void ShouldPutFloatLittleEndian()
        {
            const float value = float.MaxValue - 1;
            const int index = 0;
            _directBuffer.FloatPutLittleEndian(index, value);

            Assert.AreEqual(value, *(float*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldPutFloatBigEndian()
        {
            const float value = float.MaxValue - 1;
            const int index = 0;
            _directBuffer.FloatPutBigEndian(index, value);

            var expected = EndianessConverter.ApplyFloat(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(float*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldGetFloatLittleEndian()
        {
            const float value = float.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.FloatGetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldGetFloatBigEndian()
        {
            const float value = float.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 4);

            var result = _directBuffer.FloatGetBigEndian(index);

            var expected = EndianessConverter.ApplyFloat(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region Double

        [TestMethod]
        public void ShouldPutDoubleLittleEndian()
        {
            const double value = double.MaxValue - 1;
            const int index = 0;
            _directBuffer.DoublePutLittleEndian(index, value);

            Assert.AreEqual(value, *(double*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldPutDoubleBigEndian()
        {
            const double value = double.MaxValue - 1;
            const int index = 0;
            _directBuffer.DoublePutBigEndian(index, value);

            var expected = EndianessConverter.ApplyDouble(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, *(double*)(_pBuffer + index));
        }

        [TestMethod]
        public void ShouldGetDoubleLittleEndian()
        {
            const double value = double.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.DoubleGetLittleEndian(index);

            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldGetDoubleBigEndian()
        {
            const double value = double.MaxValue - 1;
            const int index = 0;
            var bytes = BitConverter.GetBytes(value);
            Array.Copy(bytes, 0, _buffer, index, 8);

            var result = _directBuffer.DoubleGetBigEndian(index);

            var expected = EndianessConverter.ApplyDouble(ByteOrder.BigEndian, value);
            Assert.AreEqual(expected, result);
        }

        #endregion

        #region NullTerminatedBytesFromString
        const byte Terminator = (byte)0;
        readonly static Encoding AsciiEncoding = Encoding.UTF8;

        [TestMethod]
        public void ShouldPutStringWithNullTermination()
        {
            // the string length is less than the max and less than the capacity - should add null terminator
            const string value = "abc123";
            const int index = 0;
            var written = _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, 100, Terminator);
            var read = new byte[written];
            var numBytesWritten = _directBuffer.GetBytes(index, read, 0, written);
            Assert.AreEqual(7, numBytesWritten);
            Assert.AreEqual(Terminator, read[read.Length - 1]);  // should be terminated with the null value
            string result = AsciiEncoding.GetString(read);
            Assert.AreEqual(value, result.Substring(0, read.Length - 1));
        }

        [TestMethod]
        public void ShouldPutStringWithoutNullTerminator()
        {
            // the string length is equal to the max length parameter
            const string value = "abc123";
            const int index = 0;
            var written = _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, value.Length, Terminator);
            var read = new byte[written];
            var numBytesWritten = _directBuffer.GetBytes(index, read, 0, written);
            Assert.AreEqual(6, numBytesWritten);
            Assert.AreNotEqual(Terminator, read[read.Length - 1]);  // should NOT be terminated with the null value
            string result = AsciiEncoding.GetString(read);
            Assert.AreEqual(value, result.Substring(0, read.Length));
        }

        [TestMethod]
        public void ShouldPutStringWithoutNullTerminatorIfBytesToWriteEqualsRemainingCapacity()
        {
            // the string length is equal to the remaining space in the buffer
            const string value = "abc123";
            int index = _directBuffer.Capacity - value.Length;
            var written = _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, 100, Terminator);
            var read = new byte[written];
            var numBytesWritten = _directBuffer.GetBytes(index, read, 0, written);
            Assert.AreEqual(6, numBytesWritten);
            Assert.AreNotEqual(Terminator, read[read.Length - 1]);  // should NOT be terminated with the null value
            string result = AsciiEncoding.GetString(read);
            Assert.AreEqual(value, result.Substring(0, read.Length));
        }

        [TestMethod]
        public void ShouldThrowIndexOutOfRangeExceptionWhenStringLengthGreaterThanMax()
        {
            const string value = "abc123";
            const int index = 0;
            int maxLength = value.Length - 1;  // max length is less than the value length
            Assert.ThrowsException<IndexOutOfRangeException>(() =>  _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, maxLength, Terminator));
        }

        [TestMethod]
        public void ShouldThrowIndexOutOfRangeExceptionWhenStringLengthGreaterThanCapacity()
        {
            const string value = "abc123";
            int index = _directBuffer.Capacity - value.Length + 1; // remaining capacity will be too short to write the string bytes
            Assert.ThrowsException<IndexOutOfRangeException>(() =>  _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, 100, Terminator));
        }

        [TestMethod]
        public void ShouldPutNullStringNullTerminated()
        {
            const string value = null;

            const int index = 0;
            var written = _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, 100, Terminator);
            var read = new byte[written];
            var len = _directBuffer.GetBytes(index, read, 0, written);
            Assert.AreEqual(1, len);
            Assert.AreEqual(Terminator, read[read.Length - 1]);
        }

        [TestMethod]
        public void ShouldPutEmptyStringNullTerminated()
        {
            const string value = "";

            const int index = 0;
            var written = _directBuffer.SetNullTerminatedBytesFromString(AsciiEncoding, value, index, 100, Terminator);
            var read = new byte[written];
            var len = _directBuffer.GetBytes(index, read, 0, written);
            Assert.AreEqual(1, len);
            Assert.AreEqual(Terminator, read[read.Length - 1]);
        }


        [TestMethod]
        public void ShouldGetStringNullTerminated()
        {
            var encoding = System.Text.Encoding.ASCII;
            const string value = "abc123";
            var bytes = encoding.GetBytes(value);
            const int index = 0;
            var written = _directBuffer.SetBytes(index, bytes, 0, bytes.Length);
            Assert.AreEqual(bytes.Length, written);
            var written2 = _directBuffer.SetBytes(index + bytes.Length, new byte[]{ Terminator }, 0, 1);
            Assert.AreEqual(1, written2);
            string result = _directBuffer.GetStringFromNullTerminatedBytes(encoding, index, _directBuffer.Capacity - index, Terminator);
            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldGetStringWithoutNullTermination()
        {
            var encoding = System.Text.Encoding.UTF8;
            const string value = "abc123";
            var bytes = encoding.GetBytes(value);
            const int index = 10;  // pushes the write to the end of the buffer so no terminator will be added
            var written = _directBuffer.SetBytes(index, bytes, 0, bytes.Length);
            Assert.AreEqual(bytes.Length, written);
            string result = _directBuffer.GetStringFromNullTerminatedBytes(encoding, index, 100, Terminator);
            Assert.AreEqual(value, result);
        }

        #endregion

        #region SetBytesFromString

        [TestMethod]
        public void ShouldSetBytesFromString()
        {
            const string value = "abc123";
            var written = _directBuffer.SetBytesFromString(AsciiEncoding, value, 0);
            Assert.AreEqual(6, written);
            string result = _directBuffer.GetStringFromBytes(AsciiEncoding, 0, 6);
            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldSetZeroBytesFromEmptyString()
        {
            const string value = "";
            var written = _directBuffer.SetBytesFromString(AsciiEncoding, value, 0);
            Assert.AreEqual(0, written);
            string result = _directBuffer.GetStringFromBytes(AsciiEncoding, 0, 0);
            Assert.AreEqual(value, result);
        }


        [TestMethod]
        public void ShouldThrowExceptionIfNotEnoughRoomInBufferForBytes()
        {
            const string value = "a";
            Assert.ThrowsException<IndexOutOfRangeException>(() => _directBuffer.SetBytesFromString(AsciiEncoding, value, _directBuffer.Capacity - value.Length + 1));
        }

        #endregion

        #region GetStringFromBytes

        [TestMethod]
        public void ShouldGetStringFromBytes()
        {
            const string value = "abc123";
            _directBuffer.SetBytesFromString(AsciiEncoding, value, 0);
            string result = _directBuffer.GetStringFromBytes(AsciiEncoding, 0, AsciiEncoding.GetByteCount(value));
            Assert.AreEqual(value, result);
        }

        [TestMethod]
        public void ShouldThrowExceptionIfNotEnoughBytesToGetStringFromBytes()
        {
            Assert.ThrowsException<IndexOutOfRangeException>(() =>  _directBuffer.GetStringFromBytes(AsciiEncoding, _directBuffer.Capacity, 1));
        }

        #endregion
    }
}
