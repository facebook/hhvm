// Copyright (C) 2017 MarketFactory, Inc
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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Org.SbeTool.Sbe.Dll;

namespace Org.SbeTool.Sbe.Tests
{
    [TestClass]
    public unsafe class Issue435Tests
    {
        private byte[] _buffer;
        private DirectBuffer _directBuffer;
        private Issue435.MessageHeader _messageHeader;
        private Issue435.Issue435 _issue435;

        [TestInitialize]
        public void SetUp()
        {
            _buffer = new Byte[4096];
            _directBuffer = new DirectBuffer(_buffer);
            _issue435 = new Issue435.Issue435();
            _messageHeader = new Issue435.MessageHeader();

            _messageHeader.Wrap(_directBuffer, 0, Issue435.MessageHeader.SbeSchemaVersion);
            _messageHeader.BlockLength = Issue435.Issue435.BlockLength;
            _messageHeader.SchemaId = Issue435.Issue435.SchemaId;
            _messageHeader.TemplateId = Issue435.Issue435.TemplateId;
            _messageHeader.Version = Issue435.Issue435.SchemaVersion;

            // <ref> element in non-standard message header
            _messageHeader.S = Issue435.SetRef.One;
        }

        [TestMethod]
        public void MessageHeaderNonStandardSizeTest()
        {
            // Check size is normal eight bytes plus one extra for S
            Assert.IsTrue(Issue435.MessageHeader.Size == 9, "MessageHeader size has unexpected length");
        }

        [TestMethod]
        public void Issue435RefTest()
        {
            _issue435.WrapForEncode(_directBuffer, Issue435.MessageHeader.Size);

            // <ref> tag in body
            _issue435.Example.E = Issue435.EnumRef.Two;

            // Check size is normal eight bytes plus one extra for S
            Assert.IsTrue(Issue435.Issue435.BlockLength == 1, "Issue435 size has unexpected BlockLength");

            // Now let's decode
            _messageHeader.Wrap(_directBuffer, 0, Issue435.Issue435.SchemaVersion);

            Assert.AreEqual(Issue435.Issue435.BlockLength, _messageHeader.BlockLength, "Incorrect BlockLength");
            Assert.AreEqual(Issue435.Issue435.SchemaId, _messageHeader.SchemaId, "Incorrect SchemaId");
            Assert.AreEqual(Issue435.Issue435.TemplateId, _messageHeader.TemplateId, "Incorrect TemplateId");
            Assert.AreEqual(Issue435.Issue435.SchemaVersion, _messageHeader.Version, "Incorrect SchemaVersion");
            Assert.AreEqual(Issue435.SetRef.One, _messageHeader.S, "Incorrect SetRef.One");

            _issue435.WrapForDecode(_directBuffer, Issue435.MessageHeader.Size, _messageHeader.BlockLength, _messageHeader.Version);

            Assert.AreEqual(Issue435.EnumRef.Two, _issue435.Example.E, "Incorrect EnuRef");
        }
    }
}
