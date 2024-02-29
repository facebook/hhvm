// Copyright (C) 2018 Bill Segall
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
    public unsafe class Issue560Tests
    {
        private byte[] _buffer;
        private DirectBuffer _directBuffer;
        private Issue560.MessageHeader _messageHeader;
        private Issue560.Issue560 _issue560;

        //public Issue560.Issue560 Issue560 { get => _issue560; set => _issue560 = value; }

        [TestInitialize]
        public void SetUp()
        {
            _buffer = new Byte[4096];
            _directBuffer = new DirectBuffer(_buffer);
            _issue560 = new Issue560.Issue560();
            _messageHeader = new Issue560.MessageHeader();

            _messageHeader.Wrap(_directBuffer, 0, Issue560.MessageHeader.SbeSchemaVersion);
            _messageHeader.BlockLength = Issue560.Issue560.BlockLength;
            _messageHeader.SchemaId = Issue560.Issue560.SchemaId;
            _messageHeader.TemplateId = Issue560.Issue560.TemplateId;
            _messageHeader.Version = Issue560.Issue560.SchemaVersion;

        }

        [TestMethod]
        public void Issue560Test()
        {
            Assert.AreEqual(_issue560.DiscountedModel, Issue560.Model.C, "Incorrect const enum valueref");
        }
    }
}
