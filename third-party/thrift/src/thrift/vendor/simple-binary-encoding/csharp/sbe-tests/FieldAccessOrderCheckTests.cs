using System;
using System.Diagnostics;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Order.Check;
using Org.SbeTool.Sbe.Dll;

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public class FieldAccessOrderCheckTests
    {
        private const int Offset = 0;
        private readonly DirectBuffer _buffer = new DirectBuffer();
        private readonly MessageHeader _messageHeader = new MessageHeader();

        [TestInitialize]
        public void SetUp()
        {
            var byteArray = new byte[128];
            new Random().NextBytes(byteArray);
            _buffer.Wrap(byteArray);
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingVariableLengthFieldsInSchemaDefinedOrder()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.SetC("def");
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
            Assert.AreEqual("def", decoder.GetC());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B='abc'|C='def'"));
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelPrimitiveFields()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.SetC("def");
            encoder.A = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(43, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
            Assert.AreEqual("def", decoder.GetC());
        }

        [TestMethod]
        public void DisallowsSkippingEncodingOfVariableLengthField1()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetC("def"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsSkippingEncodingOfVariableLengthField2()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var def = Encoding.ASCII.GetBytes("def");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetC(def));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsReEncodingEarlierVariableLengthFields()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.SetC("def");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetB("ghi"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b\" in state: V0_C_DONE"));
        }

        [TestMethod]
        public void DisallowsReEncodingLatestVariableLengthField()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.SetC("def");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetC("ghi"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_C_DONE"));
        }

        [TestMethod]
        public void DisallowsSkippingDecodingOfVariableLengthField1()
        {
            var decoder = DecodeUntilVarLengthFields();

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetC());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsSkippingDecodingOfVariableLengthField2()
        {
            var decoder = DecodeUntilVarLengthFields();

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetCBytes());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsSkippingDecodingOfVariableLengthField3()
        {
            var decoder = DecodeUntilVarLengthFields();

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetC(new byte[8]));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void AllowsRepeatedDecodingOfVariableLengthDataLength()
        {
            var decoder = DecodeUntilVarLengthFields();
            Assert.AreEqual(3, decoder.BLength());
            Assert.AreEqual(3, decoder.BLength());
            Assert.AreEqual(3, decoder.BLength());
            Assert.AreEqual("abc", decoder.GetB());
            Assert.AreEqual(3, decoder.CLength());
            Assert.AreEqual(3, decoder.CLength());
            Assert.AreEqual(3, decoder.CLength());
        }

        [TestMethod]
        public void DisallowsReDecodingEarlierVariableLengthField()
        {
            var decoder = DecodeUntilVarLengthFields();
            Assert.AreEqual("abc", decoder.GetB());
            Assert.AreEqual("def", decoder.GetC());

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetB());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b\" in state: V0_C_DONE"));
        }

        [TestMethod]
        public void DisallowsReDecodingLatestVariableLengthField()
        {
            var decoder = DecodeUntilVarLengthFields();
            Assert.AreEqual("abc", decoder.GetB());
            Assert.AreEqual("def", decoder.GetC());

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetC());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"c\" in state: V0_C_DONE"));
        }

        private MultipleVarLength DecodeUntilVarLengthFields()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.SetC("def");
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);

            return decoder;
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingGroupAndVariableLengthFieldsInSchemaDefinedOrder()
        {
            var encoder = new GroupAndVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder.Next().C = 1;
            bEncoder.Next().C = 2;
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(1, bDecoder.Next().C);
            Assert.AreEqual(2, bDecoder.Next().C);
            Assert.AreEqual("abc", decoder.GetD());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[(C=1),(C=2)]|D='abc'"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingEmptyGroupAndVariableLengthFieldsInSchemaDefinedOrder()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(0);
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bs = decoder.B;
            Assert.AreEqual(0, bs.Count);
            Assert.AreEqual("abc", decoder.GetD());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[]|D='abc'"));
        }

        [TestMethod]
        public void AllowsEncoderToResetZeroGroupLengthToZero()
        {
            var encoder = new GroupAndVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(0).ResetCountToIndex();
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(0, bDecoder.Count);
            Assert.AreEqual("abc", decoder.GetD());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[]|D='abc'"));
        }

        [TestMethod]
        public void AllowsEncoderToResetNonZeroGroupLengthToZeroBeforeCallingNext()
        {
            var encoder = new GroupAndVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(2).ResetCountToIndex();
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(0, bDecoder.Count);
            Assert.AreEqual("abc", decoder.GetD());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[]|D='abc'"));
        }

        [TestMethod]
        public void AllowsEncoderToResetNonZeroGroupLengthToNonZero()
        {
            var encoder = new GroupAndVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder.Next().C = 43;
            bEncoder.ResetCountToIndex();
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(43, bDecoder.Next().C);
            Assert.AreEqual("abc", decoder.GetD());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[(C=43)]|D='abc'"));
        }

        [TestMethod]
        public void DisallowsEncoderToResetGroupLengthMidGroupElement()
        {
            var encoder = new NestedGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            bEncoder.C = 43;
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.ResetCountToIndex());
            Assert.IsTrue(exception.Message.Contains(
                "Cannot reset count of repeating group \"b\" in state: V0_B_N_BLOCK"));
        }

        [TestMethod]
        public void DisallowsEncodingGroupElementBeforeCallingNext()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C = 1);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingGroupElementBeforeCallingNext()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder.Next()
                .C = 2;
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bs = decoder.B;
            Assert.AreEqual(2, bs.Count);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => _ = bs.C);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsSkippingEncodingOfGroup()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetD("abc"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"d\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsReEncodingVariableLengthFieldAfterGroup()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder.Next()
                .C = 2;
            encoder.SetD("abc");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetD("def"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"d\" in state: V0_D_DONE"));
        }

        [TestMethod]
        public void DisallowsReEncodingGroupCount()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder.Next()
                .C = 2;
            encoder.SetD("abc");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.BCount(1));
            Assert.IsTrue(
                exception.Message.Contains("Cannot encode count of repeating group \"b\" in state: V0_D_DONE"));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfGroupBeforeVariableLengthField()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder
                .Next()
                .C = 2;
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetD());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"d\" in state: V0_BLOCK"));
        }

        [TestMethod]
        public void DisallowsReDecodingVariableLengthFieldAfterGroup()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder
                .Next()
                .C = 2;
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bs = decoder.B;
            Assert.AreEqual(2, bs.Count);
            Assert.AreEqual(1, bs.Next().C);
            Assert.AreEqual(2, bs.Next().C);
            Assert.AreEqual("abc", decoder.GetD());
            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetD());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"d\" in state: V0_D_DONE"));
        }

        [TestMethod]
        public void DisallowsReDecodingGroupAfterVariableLengthField()
        {
            var encoder = new GroupAndVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder
                .Next()
                .C = 2;
            encoder.SetD("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupAndVarLength()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bs = decoder.B;
            Assert.AreEqual(2, bs.Count);
            Assert.AreEqual(1, bs.Next().C);
            Assert.AreEqual(2, bs.Next().C);
            Assert.AreEqual("abc", decoder.GetD());
            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.B);
            Assert.IsTrue(
                exception.Message.Contains("Cannot decode count of repeating group \"b\" in state: V0_D_DONE"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingVariableLengthFieldInsideGroupInSchemaDefinedOrder()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder
                .Next()
                .C = 1;
            bEncoder
                .SetD("abc");
            bEncoder
                .Next()
                .C = 2;
            bEncoder
                .SetD("def");
            encoder.SetE("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new VarLengthInsideGroup()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bs = decoder.B;
            Assert.AreEqual(2, bs.Count);
            Assert.AreEqual(1, bs.Next().C);
            Assert.AreEqual("abc", bs.GetD());
            Assert.AreEqual(2, bs.Next().C);
            Assert.AreEqual("def", bs.GetD());
            Assert.AreEqual("ghi", decoder.GetE());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[(C=1|D='abc'),(C=2|D='def')]|E='ghi'"));
        }

        [TestMethod]
        public void DisallowsMissedGroupElementVariableLengthFieldToEncodeAtTopLevel()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(1).Next().C = 1;
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetE("abc"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"e\" in state: V0_B_1_BLOCK"));
        }

        [TestMethod]
        public void DisallowsMissedGroupElementVariableLengthFieldToEncodeNextElement()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.Next());
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK"));
        }

        [TestMethod]
        public void DisallowsMissedGroupElementEncoding()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2);
            bEncoder.Next().C = 1;
            bEncoder.SetD("abc");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.SetE("abc"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"e\" in state: V0_B_N_D_DONE"));
        }

        [TestMethod]
        public void DisallowsReEncodingGroupElementVariableLengthField()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 1;
            bEncoder.SetD("abc");
            encoder.SetE("def");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.SetD("ghi"));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.d\" in state: V0_E_DONE"));
        }

        [TestMethod]
        public void DisallowsReDecodingGroupElementVariableLengthField()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            bEncoder.C = 1;
            bEncoder.SetD("abc");
            bEncoder.Next().C = 2;
            bEncoder.SetD("def");
            encoder.SetE("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new VarLengthInsideGroup()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(1, bDecoder.Next().C);
            Assert.AreEqual("abc", bDecoder.GetD());
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bDecoder.GetD());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.d\" in state: V0_B_N_D_DONE"));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfGroupElementVariableLengthFieldToNextElement()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            bEncoder.C = 1;
            bEncoder.SetD("abc");
            bEncoder.Next().C = 2;
            bEncoder.SetD("def");
            encoder.SetE("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new VarLengthInsideGroup()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(1, bDecoder.Next().C);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bDecoder.Next());
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b\" in state: V0_B_N_BLOCK"));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfGroupElementVariableLengthFieldToTopLevel()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 1;
            bEncoder.SetD("abc");
            encoder.SetE("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new VarLengthInsideGroup()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(1, bDecoder.Next().C);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetE());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"e\" in state: V0_B_1_BLOCK"));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfGroupElement()
        {
            var encoder = new VarLengthInsideGroup()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            bEncoder.C = 1;
            bEncoder.SetD("abc");
            bEncoder.Next().C = 2;
            bEncoder.SetD("def");
            encoder.SetE("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new VarLengthInsideGroup()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(1, bDecoder.Next().C);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetE());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"e\" in state: V0_B_N_BLOCK"));
        }

        [TestMethod]
        public void AllowsEncodingNestedGroupsInSchemaDefinedOrder()
        {
            var encoder = new NestedGroups().WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(2).Next();
            bEncoder.C = 1;
            var dEncoder = bEncoder.DCount(2).Next();
            dEncoder.E = 2;
            dEncoder = dEncoder.Next();
            dEncoder.E = 3;
            var fEncoder = bEncoder.FCount(1).Next();
            fEncoder.G = 4;
            bEncoder = bEncoder.Next();
            bEncoder.C = 5;
            dEncoder = bEncoder.DCount(1).Next();
            dEncoder.E = 6;
            fEncoder = bEncoder.FCount(1).Next();
            fEncoder.G = 7;
            var hEncoder = encoder.HCount(1).Next();
            hEncoder.I = 8;
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroups().WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            var b0 = bDecoder.Next();
            Assert.AreEqual(1, b0.C);
            var b0ds = b0.D;
            Assert.AreEqual(2, b0ds.Count);
            Assert.AreEqual(2, b0ds.Next().E);
            Assert.AreEqual(3, b0ds.Next().E);
            var b0Fs = b0.F;
            Assert.AreEqual(1, b0Fs.Count);
            Assert.AreEqual(4, b0Fs.Next().G);
            var b1 = bDecoder.Next();
            Assert.AreEqual(5, b1.C);
            var b1ds = b1.D;
            Assert.AreEqual(1, b1ds.Count);
            Assert.AreEqual(6, b1ds.Next().E);
            var b1Fs = b1.F;
            Assert.AreEqual(1, b1Fs.Count);
            Assert.AreEqual(7, b1Fs.Next().G);
            var hs = decoder.H;
            Assert.AreEqual(1, hs.Count);
            Assert.AreEqual(8, hs.Next().I);
            Assert.IsTrue(decoder.ToString()
                .Contains("A=42|B=[(C=1|D=[(E=2),(E=3)]|F=[(G=4)]),(C=5|D=[(E=6)]|F=[(G=7)])]|H=[(I=8)]"));
        }

        [TestMethod]
        public void AllowsEncodingEmptyNestedGroupsInSchemaDefinedOrder()
        {
            var encoder = new NestedGroups().WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(0);
            encoder.HCount(0);
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroups().WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(0, bDecoder.Count);
            var hDecoder = decoder.H;
            Assert.AreEqual(0, hDecoder.Count);
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[]|H=[]"));
        }

        [TestMethod]
        public void DisallowsMissedEncodingOfNestedGroup()
        {
            var encoder = new NestedGroups().WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 1;
            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.FCount(1));
            Assert.IsTrue(
                exception.Message.Contains("Cannot encode count of repeating group \"b.f\" in state: V0_B_1_BLOCK"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingCompositeInsideGroupInSchemaDefinedOrder()
        {
            var encoder = new CompositeInsideGroup().WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup().WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aDecoder = decoder.A;
            Assert.AreEqual(1, aDecoder.X);
            Assert.AreEqual(2, aDecoder.Y);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            var cDecoder = bDecoder.Next().C;
            Assert.AreEqual(3, cDecoder.X);
            Assert.AreEqual(4, cDecoder.Y);
            Assert.IsTrue(decoder.ToString().Contains("A=(X=1|Y=2)|B=[(C=(X=3|Y=4))]"));
        }

        [TestMethod]
        public void DisallowsEncodingCompositeInsideGroupBeforeCallingNext()
        {
            var encoder = new CompositeInsideGroup().WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1);
            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelCompositeViaReWrap()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C.X = 3;
            bEncoder.C.Y = 4;
            aEncoder = encoder.A;
            aEncoder.X = 5;
            aEncoder.Y = 6;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(5, a.X);
            Assert.AreEqual(6, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelCompositeViaEncoderReference()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C.X = 3;
            bEncoder.C.Y = 4;
            aEncoder.X = 5;
            aEncoder.Y = 6;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(5, a.X);
            Assert.AreEqual(6, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
        }

        [TestMethod]
        public void AllowsReEncodingGroupElementCompositeViaReWrap()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            cEncoder = bEncoder.C;
            cEncoder.X = 5;
            cEncoder.Y = 6;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(1, a.X);
            Assert.AreEqual(2, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(5, c.X);
            Assert.AreEqual(6, c.Y);
        }

        [TestMethod]
        public void AllowsReEncodingGroupElementCompositeViaEncoderReference()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            cEncoder.X = 5;
            cEncoder.Y = 6;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(1, a.X);
            Assert.AreEqual(2, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(5, c.X);
            Assert.AreEqual(6, c.Y);
        }

        [TestMethod]
        public void AllowsReDecodingTopLevelCompositeViaReWrap()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a1 = decoder.A;
            Assert.AreEqual(1, a1.X);
            Assert.AreEqual(2, a1.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
            var a2 = decoder.A;
            Assert.AreEqual(1, a2.X);
            Assert.AreEqual(2, a2.Y);
        }

        [TestMethod]
        public void AllowsReDecodingTopLevelCompositeViaEncoderReference()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(1, a.X);
            Assert.AreEqual(2, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
            Assert.AreEqual(1, a.X);
            Assert.AreEqual(2, a.Y);
        }

        [TestMethod]
        public void AllowsReDecodingGroupElementCompositeViaEncoderReference()
        {
            var encoder = new CompositeInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.A;
            aEncoder.X = 1;
            aEncoder.Y = 2;
            var bEncoder = encoder.BCount(1).Next();
            var cEncoder = bEncoder.C;
            cEncoder.X = 3;
            cEncoder.Y = 4;
            encoder.CheckEncodingIsComplete();

            var decoder = new CompositeInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var a = decoder.A;
            Assert.AreEqual(1, a.X);
            Assert.AreEqual(2, a.Y);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            var c = b.Next().C;
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
            Assert.AreEqual(3, c.X);
            Assert.AreEqual(4, c.Y);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedPrimitiveField()
        {
            var encoder = new AddPrimitiveV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.B = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(2, decoder.B);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingPrimitiveFieldAsNullValue()
        {
            var encoder = new AddPrimitiveV0()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddPrimitiveV1()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(AddPrimitiveV1.BNullValue, decoder.B);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedPrimitiveFieldBeforeGroup()
        {
            var encoder = new AddPrimitiveBeforeGroupV1()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = 3;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveBeforeGroupV1()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(3, decoder.D);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingPrimitiveFieldBeforeGroupAsNullValue()
        {
            var encoder = new AddPrimitiveBeforeGroupV0()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddPrimitiveBeforeGroupV1()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(AddPrimitiveBeforeGroupV1.DNullValue, decoder.D);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeGroup()
        {
            var encoder = new AddPrimitiveBeforeGroupV1()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = 3;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveBeforeGroupV1()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedPrimitiveFieldBeforeGroup()
        {
            var encoder = new AddPrimitiveBeforeGroupV1()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = 3;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddPrimitiveBeforeGroupV0()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedPrimitiveFieldBeforeVarData()
        {
            var encoder = new AddPrimitiveBeforeVarDataV1()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.C = 3;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveBeforeVarDataV1()
                .WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(3, decoder.C);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingPrimitiveFieldBeforeVarDataAsNullValue()
        {
            var encoder = new AddPrimitiveBeforeVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddPrimitiveBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(AddPrimitiveBeforeVarDataV1.CNullValue, decoder.C);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedPrimitiveFieldBeforeVarData()
        {
            var encoder = new AddPrimitiveBeforeVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.C = 3;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedPrimitiveFieldBeforeVarData()
        {
            var encoder = new AddPrimitiveBeforeVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.C = 3;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddPrimitiveBeforeVarDataV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedPrimitiveFieldInsideGroup()
        {
            var encoder = new AddPrimitiveInsideGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var group1 = encoder.BCount(1);
            group1.Next();
            group1.C = 2;
            group1.D = 3;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveInsideGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var group2 = decoder.B;
            Assert.AreEqual(1, group2.Count);
            Assert.AreEqual(2, group2.Next().C);
            Assert.AreEqual(3, group2.D);
        }


        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingPrimitiveFieldInsideGroupAsNullValue()
        {
            var encoder = new AddPrimitiveInsideGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddPrimitiveInsideGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
            Assert.AreEqual(AddPrimitiveInsideGroupV1.BGroup.DNullValue, b.D);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedPrimitiveFieldInsideGroup()
        {
            var encoder = new AddPrimitiveInsideGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var group = encoder.BCount(2);
            group.Next().C = 2;
            group.D = 3;
            group.Next().C = 4;
            group.D = 5;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddPrimitiveInsideGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(2, b.Count);
            Assert.AreEqual(2, b.Next().C);
            Assert.AreEqual(4, b.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedPrimitiveFieldInsideGroup()
        {
            var encoder = new AddPrimitiveInsideGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var group = encoder.BCount(2);
            group.Next().C = 2;
            group.D = 3;
            group.Next().C = 4;
            group.D = 5;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddPrimitiveInsideGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(2, b.Count);
            Assert.AreEqual(2, b.Next().C);
            Assert.AreEqual(4, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedGroupBeforeVarData()
        {
            var encoder = new AddGroupBeforeVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var groupEncoder = encoder.CCount(1);
            groupEncoder.Next().D = 2;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new AddGroupBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var groupDecoder = decoder.C;
            Assert.AreEqual(1, groupDecoder.Count);
            Assert.AreEqual(2, groupDecoder.Next().D);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingGroupBeforeVarDataAsNullValue()
        {
            var encoder = new AddGroupBeforeVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddGroupBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var groupDecoder = decoder.C;
            Assert.AreEqual(0, groupDecoder.Count);
            Assert.AreEqual("abc", decoder.GetB());
            Assert.IsTrue(decoder.ToString().Contains("A=1|C=[]|B='abc'"));
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipMissingGroupBeforeVarData()
        {
            var encoder = new AddGroupBeforeVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddGroupBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void DisallowsNewDecoderToSkipPresentButAddedGroupBeforeVarData()
        {
            var encoder = new AddGroupBeforeVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var groupEncoder = encoder.CCount(1);
            groupEncoder.Next().D = 2;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new AddGroupBeforeVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);

            var exception = Assert.ThrowsException<InvalidOperationException>(() => decoder.GetB());
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b\" in state: V1_BLOCK"));
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedGroupBeforeVarData()
        {
            var encoder = new AddGroupBeforeVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            _messageHeader.NumGroups = 1;
            encoder.A = 1;
            var groupEncoder = encoder.CCount(1);
            groupEncoder.Next().D = 2;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddGroupBeforeVarDataV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);

            for (int i = 0; i < _messageHeader.NumGroups; i++)
            {
                SkipGroup(decoder);
            }

            Assert.AreEqual("abc", decoder.GetB());
        }

        private void SkipGroup(AddGroupBeforeVarDataV0 decoder)
        {
            var groupSizeEncoding = new GroupSizeEncoding();
            groupSizeEncoding.Wrap(_buffer, decoder.Limit, MessageHeader.SbeSchemaVersion);
            var bytesToSkip = GroupSizeEncoding.Size +
                              groupSizeEncoding.BlockLength * groupSizeEncoding.NumInGroup;
            decoder.Limit += bytesToSkip;
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedEnumFieldBeforeGroup()
        {
            var encoder = new AddEnumBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Direction.BUY;
            var groupEncoder = encoder.BCount(1);
            groupEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddEnumBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(Direction.BUY, decoder.D);
            var groupDecoder = decoder.B;
            Assert.AreEqual(1, groupDecoder.Count);
            Assert.AreEqual(2, groupDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingEnumFieldBeforeGroupAsNullValue()
        {
            var encoder = new AddEnumBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var myGroup = encoder.BCount(1);
            myGroup.Next();
            myGroup.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddEnumBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(Direction.NULL_VALUE, decoder.D);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedEnumFieldBeforeGroup()
        {
            var encoder = new AddEnumBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Direction.SELL;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddEnumBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedEnumFieldBeforeGroup()
        {
            var encoder = new AddEnumBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Direction.BUY;
            encoder.BCount(1).Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddEnumBeforeGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var b = decoder.B;
            Assert.AreEqual(1, b.Count);
            Assert.AreEqual(2, b.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedCompositeFieldBeforeGroup()
        {
            var encoder = new AddCompositeBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var dEncoder = encoder.D;
            dEncoder.X = -1;
            dEncoder.Y = -2;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddCompositeBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var d = decoder.D;
            Assert.IsNotNull(d);
            Assert.AreEqual(-1, d.X);
            Assert.AreEqual(-2, d.Y);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingCompositeFieldBeforeGroupAsNullValue()
        {
            var encoder = new AddCompositeBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddCompositeBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.IsNull(decoder.D);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedCompositeFieldBeforeGroup()
        {
            var encoder = new AddCompositeBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var dEncoder = encoder.D;
            dEncoder.X = -1;
            dEncoder.Y = -2;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddCompositeBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedCompositeFieldBeforeGroup()
        {
            var encoder = new AddCompositeBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var dEncoder = encoder.D;
            dEncoder.X = -1;
            dEncoder.Y = -2;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddCompositeBeforeGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedArrayFieldBeforeGroup1()
        {
            var encoder = new AddArrayBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var dEncoder = encoder.DAsSpan();
            dEncoder[0] = 1;
            dEncoder[1] = 2;
            dEncoder[2] = 3;
            dEncoder[3] = 4;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddArrayBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual((short)1, decoder.GetD(0));
            Assert.AreEqual((short)2, decoder.GetD(1));
            Assert.AreEqual((short)3, decoder.GetD(2));
            Assert.AreEqual((short)4, decoder.GetD(3));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedArrayFieldBeforeGroup2()
        {
            var encoder = new AddArrayBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD(0, 1);
            encoder.SetD(1, 2);
            encoder.SetD(2, 3);
            encoder.SetD(3, 4);
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddArrayBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual((short)1, decoder.GetD(0));
            Assert.AreEqual((short)2, decoder.GetD(1));
            Assert.AreEqual((short)3, decoder.GetD(2));
            Assert.AreEqual((short)4, decoder.GetD(3));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue1()
        {
            var encoder = new AddArrayBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddArrayBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(AddArrayBeforeGroupV1.DNullValue, decoder.GetD(0));
            Assert.AreEqual(AddArrayBeforeGroupV1.DNullValue, decoder.GetD(1));
            Assert.AreEqual(AddArrayBeforeGroupV1.DNullValue, decoder.GetD(2));
            Assert.AreEqual(AddArrayBeforeGroupV1.DNullValue, decoder.GetD(3));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingArrayFieldBeforeGroupAsNullValue2()
        {
            var encoder = new AddArrayBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddArrayBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(0, decoder.D.Length);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedArrayFieldBeforeGroup()
        {
            var encoder = new AddArrayBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD(0, 1);
            encoder.SetD(1, 2);
            encoder.SetD(2, 3);
            encoder.SetD(3, 4);
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddArrayBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedArrayFieldBeforeGroup()
        {
            var encoder = new AddArrayBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD(0, 1);
            encoder.SetD(1, 2);
            encoder.SetD(2, 3);
            encoder.SetD(3, 4);
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddArrayBeforeGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedBitSetFieldBeforeGroup()
        {
            var encoder = new AddBitSetBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Flags.Guacamole | Flags.Cheese;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddBitSetBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.IsNotNull(decoder.D);
            Assert.AreEqual(true, decoder.D.HasFlag(Flags.Guacamole));
            Assert.AreEqual(true, decoder.D.HasFlag(Flags.Cheese));
            Assert.AreEqual(false, decoder.D.HasFlag(Flags.SourCream));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingBitSetFieldBeforeGroupAsNullValue()
        {
            var encoder = new AddBitSetBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddBitSetBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual((byte)decoder.D, 0);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedBitSetFieldBeforeGroup()
        {
            var encoder = new AddBitSetBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Flags.Guacamole | Flags.Cheese;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddBitSetBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedBitSetFieldBeforeGroup()
        {
            var encoder = new AddBitSetBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.D = Flags.Guacamole | Flags.Cheese;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddBitSetBeforeGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingEnumInsideGroupInSchemaDefinedOrder()
        {
            var encoder = new EnumInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Direction.BUY;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Direction.SELL;
            encoder.CheckEncodingIsComplete();

            var decoder = new EnumInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(Direction.BUY, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(Direction.SELL, bDecoder.Next().C);
            Assert.IsTrue(decoder.ToString().Contains("A=BUY|B=[(C=SELL)]"));
        }

        [TestMethod]
        public void DisallowsEncodingEnumInsideGroupBeforeCallingNext()
        {
            var encoder = new EnumInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Direction.BUY;
            var bEncoder = encoder.BCount(1);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C = Direction.SELL);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingEnumInsideGroupBeforeCallingNext()
        {
            var encoder = new EnumInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Direction.BUY;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Direction.SELL;
            encoder.CheckEncodingIsComplete();

            var decoder = new EnumInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(Direction.BUY, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            var exception = Assert.ThrowsException<InvalidOperationException>(() =>
            {
                var _ = bDecoder.C;
            });
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelEnum()
        {
            var encoder = new EnumInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Direction.BUY;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Direction.SELL;

            encoder.A = Direction.SELL;
            encoder.CheckEncodingIsComplete();

            var decoder = new EnumInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(Direction.SELL, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(Direction.SELL, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingBitSetInsideGroupInSchemaDefinedOrder()
        {
            var encoder = new BitSetInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Flags.Cheese | Flags.Guacamole;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Flags.SourCream;
            encoder.CheckEncodingIsComplete();

            var decoder = new BitSetInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(true, decoder.A.HasFlag((Flags.Guacamole)));
            Assert.AreEqual(true, decoder.A.HasFlag(Flags.Cheese));
            Assert.AreEqual(false, decoder.A.HasFlag(Flags.SourCream));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            var cDecoder = bDecoder.Next().C;
            Assert.AreEqual(false, cDecoder.HasFlag((Flags.Guacamole)));
            Assert.AreEqual(false, cDecoder.HasFlag(Flags.Cheese));
            Assert.AreEqual(true, cDecoder.HasFlag(Flags.SourCream));
            Assert.IsTrue(decoder.ToString().Contains("A={Guacamole, Cheese}|B=[(C={SourCream})]"));
        }

        [TestMethod]
        public void DisallowsEncodingBitSetInsideGroupBeforeCallingNext()
        {
            var encoder = new BitSetInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Flags.Cheese | Flags.Guacamole;
            var bEncoder = encoder.BCount(1);
            var exception = Assert.ThrowsException<InvalidOperationException>(() =>
            {
                var _ = bEncoder.C;
            });
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingBitSetInsideGroupBeforeCallingNext()
        {
            var encoder = new BitSetInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Flags.Cheese | Flags.Guacamole;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Flags.SourCream;
            encoder.CheckEncodingIsComplete();

            var decoder = new BitSetInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(true, decoder.A.HasFlag((Flags.Guacamole)));
            Assert.AreEqual(true, decoder.A.HasFlag(Flags.Cheese));
            Assert.AreEqual(false, decoder.A.HasFlag(Flags.SourCream));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            var exception = Assert.ThrowsException<InvalidOperationException>(() =>
            {
                var _ = bDecoder.C;
            });
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelBitSetViaReWrap()
        {
            var encoder = new BitSetInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = Flags.Cheese | Flags.Guacamole;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next().C = Flags.SourCream;

            encoder.A |= Flags.SourCream;
            encoder.CheckEncodingIsComplete();

            var decoder = new BitSetInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(true, decoder.A.HasFlag((Flags.Guacamole)));
            Assert.AreEqual(true, decoder.A.HasFlag(Flags.Cheese));
            Assert.AreEqual(true, decoder.A.HasFlag(Flags.SourCream));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            var cDecoder = bDecoder.Next().C;
            Assert.AreEqual(false, cDecoder.HasFlag((Flags.Guacamole)));
            Assert.AreEqual(false, cDecoder.HasFlag(Flags.Cheese));
            Assert.AreEqual(true, cDecoder.HasFlag(Flags.SourCream));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingArrayInsideGroupInSchemaDefinedOrder()
        {
            var encoder = new ArrayInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA(0, 1);
            encoder.SetA(1, 2);
            encoder.SetA(2, 3);
            encoder.SetA(3, 4);
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.SetC(0, 5);
            bEncoder.SetC(1, 6);
            bEncoder.SetC(2, 7);
            bEncoder.SetC(3, 8);
            encoder.CheckEncodingIsComplete();

            var decoder = new ArrayInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A[0]);
            Assert.AreEqual(2, decoder.A[1]);
            Assert.AreEqual(3, decoder.A[2]);
            Assert.AreEqual(4, decoder.A[3]);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            bDecoder.Next();
            Assert.AreEqual(5, bDecoder.C[0]);
            Assert.AreEqual(6, bDecoder.C[1]);
            Assert.AreEqual(7, bDecoder.C[2]);
            Assert.AreEqual(8, bDecoder.C[3]);
            StringAssert.Contains(decoder.ToString(), "A=[1,2,3,4]|B=[(C=[5,6,7,8])]");
        }

        [TestMethod]
        public void DisallowsEncodingArrayInsideGroupBeforeCallingNext1()
        {
            var bEncoder = EncodeUntilGroupWithArrayInside();
            var exception = Assert.ThrowsException<InvalidOperationException>(() => { bEncoder.SetC(0, 5); });
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingArrayInsideGroupBeforeCallingNext2()
        {
            var bEncoder = EncodeUntilGroupWithArrayInside();
            var exception = Assert.ThrowsException<InvalidOperationException>(() => { bEncoder.CAsSpan()[0] = 1; });
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        private ArrayInsideGroup.BGroup EncodeUntilGroupWithArrayInside()
        {
            var encoder = new ArrayInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA(0, 1);
            encoder.SetA(1, 2);
            encoder.SetA(2, 3);
            encoder.SetA(3, 4);
            return encoder.BCount(1);
        }

        [TestMethod]
        public void DisallowsDecodingArrayInsideGroupBeforeCallingNext1()
        {
            var bDecoder = DecodeUntilGroupWithArrayInside();
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bDecoder.C[0]);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingArrayInsideGroupBeforeCallingNext2()
        {
            var bDecoder = DecodeUntilGroupWithArrayInside();
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bDecoder.GetC(0));
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        private ArrayInsideGroup.BGroup DecodeUntilGroupWithArrayInside()
        {
            var encoder = new ArrayInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA(0, 1);
            encoder.SetA(1, 2);
            encoder.SetA(2, 3);
            encoder.SetA(3, 4);
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.SetC(0, 5);
            bEncoder.SetC(1, 6);
            bEncoder.SetC(2, 7);
            bEncoder.SetC(3, 8);

            encoder.CheckEncodingIsComplete();

            var decoder = new ArrayInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual((short)1, decoder.A[0]);
            Assert.AreEqual((short)2, decoder.A[1]);
            Assert.AreEqual((short)3, decoder.A[2]);
            Assert.AreEqual((short)4, decoder.A[3]);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            return bDecoder;
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelArrayViaReWrap()
        {
            var encoder = new ArrayInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA(0, 1);
            encoder.SetA(1, 2);
            encoder.SetA(2, 3);
            encoder.SetA(3, 4);
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.SetC(0, 5);
            bEncoder.SetC(1, 6);
            bEncoder.SetC(2, 7);
            bEncoder.SetC(3, 8);

            encoder.SetA(0, 9);
            encoder.SetA(1, 10);
            encoder.SetA(2, 11);
            encoder.SetA(3, 12);
            encoder.CheckEncodingIsComplete();

            var decoder = new ArrayInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(9, decoder.A[0]);
            Assert.AreEqual(10, decoder.A[1]);
            Assert.AreEqual(11, decoder.A[2]);
            Assert.AreEqual(12, decoder.A[3]);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            bDecoder.Next();
            Assert.AreEqual(5, bDecoder.C[0]);
            Assert.AreEqual(6, bDecoder.C[1]);
            Assert.AreEqual(7, bDecoder.C[2]);
            Assert.AreEqual(8, bDecoder.C[3]);
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder1()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.BCount(0);
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual(0, decoder.B.Count);
            var dDecoder = decoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(dDecoder.Next().E, 43);
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=[]|D=[(E=43)]"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingGroupFieldsInSchemaDefinedOrder2()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(41, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(42, bDecoder.Next().C);
            var dDecoder = decoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(43, dDecoder.Next().E);
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelPrimitiveFieldsAfterGroups()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.A = 44;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(44, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(42, bDecoder.Next().C);
            var dDecoder = decoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(43, dDecoder.Next().E);
        }

        [TestMethod]
        public void DisallowsMissedEncodingOfGroupField()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            Assert.ThrowsException<InvalidOperationException>(() => encoder.DCount(0));
        }

        [TestMethod]
        public void DisallowsReEncodingEarlierGroupFields()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            Assert.ThrowsException<InvalidOperationException>(() => encoder.BCount(1));
        }

        [TestMethod]
        public void DisallowsReEncodingLatestGroupField()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            Assert.ThrowsException<InvalidOperationException>(() => encoder.DCount(1));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfGroupField()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(41, decoder.A);
            Assert.ThrowsException<InvalidOperationException>(() =>
            {
                // ReSharper disable once UnusedVariable
                var ignored = decoder.D;
            });
        }

        [TestMethod]
        public void DisallowsReDecodingEarlierGroupField()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(41, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(42, bDecoder.Next().C);
            var dDecoder = decoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(43, dDecoder.Next().E);
            Assert.ThrowsException<InvalidOperationException>(() =>
            {
                // ReSharper disable once UnusedVariable
                var ignored = decoder.B;
            });
        }

        [TestMethod]
        public void DisallowsReDecodingLatestGroupField()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 41;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 42;
            var dEncoder = encoder.DCount(1).Next();
            dEncoder.E = 43;
            encoder.CheckEncodingIsComplete();

            var decoder = new MultipleGroups();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(41, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(42, bDecoder.Next().C);
            var dDecoder = decoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(43, dDecoder.Next().E);
            Assert.ThrowsException<InvalidOperationException>(() =>
            {
                // ReSharper disable once UnusedVariable
                var ignored = decoder.D;
            });
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedVarData()
        {
            var encoder = new AddVarDataV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new AddVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual("abc", decoder.GetB());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAddedVarDataAsNullValue1()
        {
            var encoder = new AddVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual("", decoder.GetB());
            Assert.IsTrue(decoder.ToString().Contains("A=42|B=''"));
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAddedVarDataAsNullValue2()
        {
            var encoder = new AddVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual(0, decoder.BLength());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAddedVarDataAsNullValue3()
        {
            var encoder = new AddVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual(0, decoder.GetB(new byte[16]));
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAddedVarDataAsNullValue4()
        {
            var encoder = new AddVarDataV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddVarDataV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(42, decoder.A);
            Assert.AreEqual(0, decoder.GetB(new byte[16], 0, 16));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder1()
        {
            var encoder = new AsciiInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA("GBPUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.SetC("EURUSD");
            encoder.CheckEncodingIsComplete();

            var decoder = new AsciiInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual("GBPUSD", decoder.GetA());
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            bDecoder.Next();
            Assert.AreEqual("EURUSD", bDecoder.GetC());
            Assert.IsTrue(decoder.ToString().Contains("A='GBPUSD'|B=[(C='EURUSD')]"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingAsciiInsideGroupInSchemaDefinedOrder2()
        {
            var encoder = new AsciiInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var gbpUsdBytes = Encoding.ASCII.GetBytes("GBPUSD");
            encoder.SetA(gbpUsdBytes, 0);
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.SetC(Encoding.ASCII.GetBytes("EURUSD"), 0);
            encoder.CheckEncodingIsComplete();

            var decoder = new AsciiInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aBytes = new byte[6];
            decoder.GetA(aBytes, 0);
            Assert.IsTrue(gbpUsdBytes.SequenceEqual(aBytes));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            bDecoder.Next();
            Assert.AreEqual((byte)'E', bDecoder.GetC(0));
            Assert.AreEqual((byte)'U', bDecoder.GetC(1));
            Assert.AreEqual((byte)'R', bDecoder.GetC(2));
            Assert.AreEqual((byte)'U', bDecoder.GetC(3));
            Assert.AreEqual((byte)'S', bDecoder.GetC(4));
            Assert.AreEqual((byte)'D', bDecoder.GetC(5));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext1()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.SetC("EURUSD"));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext2()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.SetC(0, (byte)'E'));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext3()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception =
                Assert.ThrowsException<InvalidOperationException>(() => bEncoder.CAsSpan()[0] = (byte)'E');

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext4()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() =>
            {
                bEncoder.SetC(Encoding.ASCII.GetBytes("EURUSD"), 0);
            });

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext5()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() =>
                bEncoder.SetC(Encoding.ASCII.GetBytes("EURUSD"), 0));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsEncodingAsciiInsideGroupBeforeCallingNext6()
        {
            var bEncoder = EncodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() =>
                bEncoder.SetC(Encoding.ASCII.GetBytes("EURUSD")));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        private AsciiInsideGroup.BGroup EncodeUntilGroupWithAsciiInside()
        {
            var encoder = new AsciiInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA("GBPUSD");
            return encoder.BCount(1);
        }

        [TestMethod]
        public void DisallowsDecodingAsciiInsideGroupBeforeCallingNext1()
        {
            var b = DecodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => b.GetC(0));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingAsciiInsideGroupBeforeCallingNext2()
        {
            var b = DecodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => b.GetC());

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingAsciiInsideGroupBeforeCallingNext3()
        {
            var b = DecodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => b.C[0]);

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingAsciiInsideGroupBeforeCallingNext4()
        {
            var b = DecodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => b.GetC(new byte[16], 0));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        [TestMethod]
        public void DisallowsDecodingAsciiInsideGroupBeforeCallingNext5()
        {
            var b = DecodeUntilGroupWithAsciiInside();

            Exception exception = Assert.ThrowsException<InvalidOperationException>(() => b.GetC(new byte[16]));

            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_B_N"));
        }

        private AsciiInsideGroup.BGroup DecodeUntilGroupWithAsciiInside()
        {
            var encoder = new AsciiInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA("GBPUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.SetC("EURUSD");
            encoder.CheckEncodingIsComplete();

            var decoder = new AsciiInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual("GBPUSD", decoder.GetA());
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            return bDecoder;
        }

        [TestMethod]
        public void AllowsReEncodingTopLevelAsciiViaReWrap()
        {
            var encoder = new AsciiInsideGroup();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA("GBPUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.SetC("EURUSD");

            encoder.SetA("CADUSD");
            encoder.CheckEncodingIsComplete();

            var decoder = new AsciiInsideGroup();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual("CADUSD", decoder.GetA());
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            bDecoder.Next();
            Assert.AreEqual("EURUSD", bDecoder.GetC());
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup1()
        {
            var encoder = new AddAsciiBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD("EURUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual("EURUSD", decoder.GetD());
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup2()
        {
            var encoder = new AddAsciiBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD("EURUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual((byte)'E', decoder.GetD(0));
            Assert.AreEqual((byte)'U', decoder.GetD(1));
            Assert.AreEqual((byte)'R', decoder.GetD(2));
            Assert.AreEqual((byte)'U', decoder.GetD(3));
            Assert.AreEqual((byte)'S', decoder.GetD(4));
            Assert.AreEqual((byte)'D', decoder.GetD(5));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeAddedAsciiFieldBeforeGroup3()
        {
            var encoder = new AddAsciiBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD("EURUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual((byte)'E', decoder.D[0]);
            Assert.AreEqual((byte)'U', decoder.D[1]);
            Assert.AreEqual((byte)'R', decoder.D[2]);
            Assert.AreEqual((byte)'U', decoder.D[3]);
            Assert.AreEqual((byte)'S', decoder.D[4]);
            Assert.AreEqual((byte)'D', decoder.D[5]);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue1()
        {
            var encoder = new AddAsciiBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(0));
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(1));
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(2));
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(3));
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(4));
            Assert.AreEqual(AddAsciiBeforeGroupV1.DNullValue, decoder.GetD(5));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue2()
        {
            var encoder = new AddAsciiBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(0, decoder.D.Length);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue3()
        {
            var encoder = new AddAsciiBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(0, decoder.GetD(new byte[16]));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToDecodeMissingAsciiFieldBeforeGroupAsNullValue4()
        {
            var encoder = new AddAsciiBeforeGroupV0();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion0();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            Assert.AreEqual(0, decoder.GetD(new byte[16], 0));
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsNewDecoderToSkipPresentButAddedAsciiFieldBeforeGroup()
        {
            var encoder = new AddAsciiBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD("EURUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            var decoder = new AddAsciiBeforeGroupV1();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsOldDecoderToSkipAddedAsciiFieldBeforeGroup()
        {
            var encoder = new AddAsciiBeforeGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetD("EURUSD");
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 2;
            encoder.CheckEncodingIsComplete();

            ModifyHeaderToLookLikeVersion1();

            var decoder = new AddAsciiBeforeGroupV0();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
        }

        [TestMethod]
        public void AllowsEncodeAndDecodeOfMessagesWithNoABlock()
        {
            var encoder = new NoBlock();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.SetA("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new NoBlock();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual("abc", decoder.GetA());
        }

        [TestMethod]
        public void AllowsEncodeAndDecodeOfGroupsWithNoBlock()
        {
            var encoder = new GroupWithNoBlock();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aEncoder = encoder.ACount(1);
            aEncoder.Next();
            aEncoder.SetB("abc");
            encoder.CheckEncodingIsComplete();

            var decoder = new GroupWithNoBlock();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var aDecoder = decoder.A;
            Assert.AreEqual(1, aDecoder.Count);
            Assert.AreEqual("abc", aDecoder.Next().GetB());
        }

        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup1()
        {
            var encoder = new MultipleGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(0);
            var dEncoder = encoder.DCount(1);
            dEncoder.Next();
            dEncoder.E = 43;

            var ex = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C = 44);
            Assert.IsTrue(ex.Message.Contains("Cannot access field \"b.c\" in state: V0_D_1_BLOCK"));
        }

        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup2()
        {
            var encoder = new NestedGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 43;
            var dEncoder = bEncoder.DCount(0);
            bEncoder.FCount(0);
            encoder.HCount(0);

            var ex = Assert.ThrowsException<InvalidOperationException>(() => dEncoder.E = 44);
            Assert.IsTrue(ex.Message.Contains("Cannot access field \"b.d.e\" in state: V0_H_DONE"));
        }

        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup3()
        {
            var encoder = new NestedGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 43;
            var dEncoder = bEncoder.DCount(0);
            bEncoder.FCount(0);
            encoder.HCount(0);

            var ex = Assert.ThrowsException<InvalidOperationException>(() => dEncoder.E = 44);
            Assert.IsTrue(ex.Message.Contains("Cannot access field \"b.d.e\" in state: V0_H_DONE"));
        }

        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup4()
        {
            var encoder = new NestedGroups();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(1);
            bEncoder.Next();
            bEncoder.C = 43;
            var dEncoder = bEncoder.DCount(0);
            bEncoder.FCount(0);

            var ex = Assert.ThrowsException<InvalidOperationException>(() => dEncoder.E = 44);
            Assert.IsTrue(ex.Message.Contains("Cannot access field \"b.d.e\" in state: V0_B_1_F_DONE"));
        }


        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup5()
        {
            var encoder = new AddPrimitiveInsideGroupV1();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(0);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C = 43);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V1_B_DONE"));
        }

        [TestMethod]
        public void DisallowsEncodingElementOfEmptyGroup6()
        {
            var encoder = new GroupAndVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 42;
            var bEncoder = encoder.BCount(0);
            encoder.SetD("abc");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bEncoder.C = 43);
            Assert.IsTrue(exception.Message.Contains("Cannot access field \"b.c\" in state: V0_D_DONE"));
        }

        [TestMethod]
        public void AllowsEncodingAndDecodingNestedGroupWithVarDataInSchemaDefinedOrder()
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(3);
            var bNext = bEncoder.Next();
            bNext.C = 2;
            bNext.DCount(0);
            bNext = bEncoder.Next();
            bNext.C = 3;
            var dEncoder = bNext.DCount(1);
            var dNext = dEncoder.Next();
            dNext.E = 4;
            dEncoder.SetF("abc");
            bNext = bEncoder.Next();
            bNext.C = 5;
            dEncoder = bNext.DCount(2);
            dNext = dEncoder.Next();
            dNext.E = 6;
            dEncoder.SetF("def");
            dNext = dEncoder.Next();
            dNext.E = 7;
            dEncoder.SetF("ghi");
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroupWithVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(3, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
            Assert.AreEqual(0, bDecoder.D.Count);
            Assert.AreEqual(3, bDecoder.Next().C);
            var dDecoder = bDecoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(4, dDecoder.Next().E);
            Assert.AreEqual("abc", dDecoder.GetF());
            Assert.AreEqual(5, bDecoder.Next().C);
            Assert.AreEqual(dDecoder, bDecoder.D);
            Assert.AreEqual(2, dDecoder.Count);
            Assert.AreEqual(6, dDecoder.Next().E);
            Assert.AreEqual("def", dDecoder.GetF());
            Assert.AreEqual(7, dDecoder.Next().E);
            Assert.AreEqual("ghi", dDecoder.GetF());
        }

        [DataTestMethod]
        [DataRow(1, "V0_B_N_D_1_BLOCK")]
        [DataRow(2, "V0_B_N_D_N_BLOCK")]
        public void DisallowsMissedEncodingOfVarLengthFieldInNestedGroupToNextOuterElement(int dCount,
            string expectedState)
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(2);
            var bNext = bEncoder.Next();
            bNext.C = 3;
            var dEncoder = bNext.DCount(dCount);
            dEncoder.Next().E = 4;
            var exception = Assert.ThrowsException<InvalidOperationException>(bEncoder.Next);
            Assert.IsTrue(exception.Message.Contains("Cannot access next element in repeating group \"b\" in state: " +
                                                     expectedState));
            Assert.IsTrue(
                exception.Message.Contains(
                    "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement1()
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1);
            var bNext = bEncoder.Next();
            bNext.C = 2;
            var dEncoder = bNext.DCount(2);
            var dNext = dEncoder.Next();
            dNext.E = 3;
            dEncoder.SetF("abc");
            dNext = dEncoder.Next();
            dNext.E = 4;
            dEncoder.SetF("def");
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroupWithVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(1, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
            var dDecoder = bDecoder.D;
            Assert.AreEqual(2, dDecoder.Count);
            Assert.AreEqual(3, dDecoder.Next().E);
            var exception = Assert.ThrowsException<InvalidOperationException>(dDecoder.Next);
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b.d\" in state: V0_B_1_D_N_BLOCK."));
            Assert.IsTrue(
                exception.Message.Contains(
                    "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextInnerElement2()
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(2);
            var bNext = bEncoder.Next();
            bNext.C = 2;
            var dEncoder = bNext.DCount(2);
            var dNext = dEncoder.Next();
            dNext.E = 3;
            dEncoder.SetF("abc");
            dNext = dEncoder.Next();
            dNext.E = 4;
            dEncoder.SetF("def");
            bEncoder.Next().C = 5;
            bEncoder.DCount(0);
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroupWithVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
            var dDecoder = bDecoder.D;
            Assert.AreEqual(2, dDecoder.Count);
            Assert.AreEqual(3, dDecoder.Next().E);
            var exception = Assert.ThrowsException<InvalidOperationException>(dDecoder.Next);
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b.d\" in state: V0_B_N_D_N_BLOCK."));
            Assert.IsTrue(
                exception.Message.Contains(
                    "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement1()
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(2);
            var bNext = bEncoder.Next();
            bNext.C = 2;
            var dEncoder = bNext.DCount(2);
            var dNext = dEncoder.Next();
            dNext.E = 3;
            dEncoder.SetF("abc");
            dNext = dEncoder.Next();
            dNext.E = 4;
            dEncoder.SetF("def");
            bEncoder.Next().C = 5;
            bEncoder.DCount(0);
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroupWithVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
            var dDecoder = bDecoder.D;
            Assert.AreEqual(2, dDecoder.Count);
            Assert.AreEqual(3, dDecoder.Next().E);
            var exception = Assert.ThrowsException<InvalidOperationException>(bDecoder.Next);
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b\" in state: V0_B_N_D_N_BLOCK."));
            Assert.IsTrue(
                exception.Message.Contains(
                    "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
        }

        [TestMethod]
        public void DisallowsMissedDecodingOfVarLengthFieldInNestedGroupToNextOuterElement2()
        {
            var encoder = new NestedGroupWithVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(2);
            var bNext = bEncoder.Next();
            bNext.C = 2;
            var dEncoder = bNext.DCount(1);
            var dNext = dEncoder.Next();
            dNext.E = 3;
            dEncoder.SetF("abc");
            bEncoder.Next().C = 5;
            bEncoder.DCount(0);
            encoder.CheckEncodingIsComplete();

            var decoder = new NestedGroupWithVarLength();
            decoder.WrapForDecodeAndApplyHeader(_buffer, Offset, _messageHeader);
            Assert.AreEqual(1, decoder.A);
            var bDecoder = decoder.B;
            Assert.AreEqual(2, bDecoder.Count);
            Assert.AreEqual(2, bDecoder.Next().C);
            var dDecoder = bDecoder.D;
            Assert.AreEqual(1, dDecoder.Count);
            Assert.AreEqual(3, dDecoder.Next().E);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => bDecoder.Next());
            Assert.IsTrue(
                exception.Message.Contains(
                    "Cannot access next element in repeating group \"b\" in state: V0_B_N_D_1_BLOCK."));
            Assert.IsTrue(
                exception.Message.Contains(
                    "Expected one of these transitions: [\"b.d.e(?)\", \"b.d.fLength()\", \"b.d.f(?)\"]."));
        }

        [TestMethod]
        public void DisallowsIncompleteMessagesDueToMissingVarLengthField1()
        {
            var encoder = new MultipleVarLength();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.SetB("abc");
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.CheckEncodingIsComplete());
            Assert.IsTrue(
                exception.Message.Contains(
                    "Not fully encoded, current state: V0_B_DONE, allowed transitions: \"cLength()\", \"c(?)\""));
        }

        [TestMethod]
        public void DisallowsIncompleteMessagesDueToMissingVarLengthField2()
        {
            var encoder = new NoBlock();
            encoder.WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            var exception = Assert.ThrowsException<InvalidOperationException>(() => encoder.CheckEncodingIsComplete());
            Assert.IsTrue(
                exception.Message.Contains(
                    "Not fully encoded, current state: V0_BLOCK, allowed transitions: \"aLength()\", \"a(?)\""));
        }

        [TestMethod]
        public void DisallowsIncompleteMessagesDueToMissingTopLevelGroup1()
        {
            var encoder = new MultipleGroups()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            encoder.BCount(0);
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message,
                "Not fully encoded, current state: V0_B_DONE, allowed transitions: " +
                "\"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\"");
        }

        [TestMethod]
        public void DisallowsIncompleteMessagesDueToMissingTopLevelGroup2()
        {
            var encoder = new MultipleGroups()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(1).Next();
            bEncoder.C = 2;
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message,
                "Not fully encoded, current state: V0_B_1_BLOCK, allowed transitions:" +
                " \"b.c(?)\", \"b.resetCountToIndex()\", \"dCount(0)\", \"dCount(>0)\"");
        }

        [TestMethod]
        public void DisallowsIncompleteMessagesDueToMissingTopLevelGroup3()
        {
            var encoder = new MultipleGroups()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message,
                "Not fully encoded, current state: V0_BLOCK, allowed transitions: \"a(?)\", \"bCount(0)\", \"bCount(>0)\"");
        }

        [DataTestMethod]
        [DataRow(1, "V0_B_1_BLOCK")]
        [DataRow(2, "V0_B_N_BLOCK")]
        public void DisallowsIncompleteMessagesDueToMissingNestedGroup1(int bCount, string expectedState)
        {
            var encoder = new NestedGroupWithVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(bCount).Next();
            bEncoder.C = 2;
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message, $"Not fully encoded, current state: {expectedState}");
        }

        [DataTestMethod]
        [DataRow(1, 1, "V0_B_1_D_N")]
        [DataRow(1, 2, "V0_B_1_D_N")]
        [DataRow(2, 0, "V0_B_N_D_DONE")]
        [DataRow(2, 1, "V0_B_N_D_N")]
        [DataRow(2, 2, "V0_B_N_D_N")]
        public void DisallowsIncompleteMessagesDueToMissingNestedGroup2(
            int bCount,
            int dCount,
            string expectedState)
        {
            var encoder = new NestedGroupWithVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(bCount).Next();
            bEncoder.C = 2;
            bEncoder.DCount(dCount);
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message, $"Not fully encoded, current state: {expectedState}");
        }

        [DataTestMethod]
        [DataRow(1, 1, "V0_B_1_D_1_BLOCK")]
        [DataRow(1, 2, "V0_B_1_D_N_BLOCK")]
        [DataRow(2, 1, "V0_B_N_D_1_BLOCK")]
        [DataRow(2, 2, "V0_B_N_D_N_BLOCK")]
        public void DisallowsIncompleteMessagesDueToMissingVarDataInNestedGroup(
            int bCount,
            int dCount,
            string expectedState)
        {
            var encoder = new NestedGroupWithVarLength()
                .WrapForEncodeAndApplyHeader(_buffer, Offset, _messageHeader);
            encoder.A = 1;
            var bEncoder = encoder.BCount(bCount).Next();
            bEncoder.C = 2;
            bEncoder.DCount(dCount).Next().E = 10;
            var exception = Assert.ThrowsException<InvalidOperationException>(encoder.CheckEncodingIsComplete);
            StringAssert.Contains(exception.Message, $"Not fully encoded, current state: {expectedState}");
        }

        private void ModifyHeaderToLookLikeVersion0()
        {
            var messageHeaderDecoder = new MessageHeader();
            messageHeaderDecoder.Wrap(_buffer, Offset, MessageHeader.SbeSchemaVersion);
            int v1TemplateId = messageHeaderDecoder.TemplateId + 1_000;
            var messageHeaderEncoder = new MessageHeader();
            messageHeaderEncoder.Wrap(_buffer, Offset, MessageHeader.SbeSchemaVersion);
            messageHeaderEncoder.TemplateId = (ushort)v1TemplateId;
            messageHeaderEncoder.Version = 0;
        }

        private void ModifyHeaderToLookLikeVersion1()
        {
            var messageHeaderDecoder = new MessageHeader();
            messageHeaderDecoder.Wrap(_buffer, Offset, MessageHeader.SbeSchemaVersion);
            Debug.Assert(messageHeaderDecoder.Version == 1);
            int v0TemplateId = messageHeaderDecoder.TemplateId - 1_000;
            var messageHeaderEncoder = new MessageHeader();
            messageHeaderEncoder.Wrap(_buffer, Offset, MessageHeader.SbeSchemaVersion);
            messageHeaderEncoder.TemplateId = (ushort)v0TemplateId;
        }
    }
}