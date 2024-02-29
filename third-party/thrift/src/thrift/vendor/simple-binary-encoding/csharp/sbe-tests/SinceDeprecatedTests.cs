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
    public unsafe class SinceDeprecatedTests
    {
        [TestMethod]
        public void SinceDeprecatedTest()
        {
            byte[] buffer = new Byte[4096];
            DirectBuffer directBuffer = new DirectBuffer(buffer);
            SinceDeprecated.SinceDeprecated sinceDeprecated = new SinceDeprecated.SinceDeprecated();


            // These versions are explicit in the test schema and the  reason for this test
            //    Version 1 was initial with just field v1
            //    Version 2 added field v2
            //    Version 3 added field v3
            //    Version 4 deprecated field v3
            Assert.AreEqual(0, SinceDeprecated.SinceDeprecated.V1SinceVersion);
            Assert.AreEqual(2, SinceDeprecated.SinceDeprecated.V2SinceVersion);
            Assert.AreEqual(3, SinceDeprecated.SinceDeprecated.V3SinceVersion);

            Assert.AreEqual(0, SinceDeprecated.SinceDeprecated.V1Deprecated);
            Assert.AreEqual(0, SinceDeprecated.SinceDeprecated.V2Deprecated);
            Assert.AreEqual(4, SinceDeprecated.SinceDeprecated.V3Deprecated);

            // Assign some values
            sinceDeprecated.WrapForEncode(directBuffer, SinceDeprecated.MessageHeader.Size);
            sinceDeprecated.V1 = 1;
            sinceDeprecated.V2 = 2;
            sinceDeprecated.V3 = 3;

            // SchemaVersion is 4
            sinceDeprecated.WrapForDecode(directBuffer, SinceDeprecated.MessageHeader.Size, SinceDeprecated.SinceDeprecated.BlockLength, SinceDeprecated.SinceDeprecated.SchemaVersion);
            Assert.AreEqual(sinceDeprecated.V1, 1UL, "Incorrect V1 schemaversion:4");
            Assert.AreEqual(sinceDeprecated.V2, 2UL, "Incorrect V2 schemaversion:4");
            Assert.AreEqual(sinceDeprecated.V3, 3UL, "Incorrect V3 schemaversion:4");

            // SchemaVersion is 3
            sinceDeprecated.WrapForDecode(directBuffer, SinceDeprecated.MessageHeader.Size, SinceDeprecated.SinceDeprecated.BlockLength, 3);
            Assert.AreEqual(sinceDeprecated.V1, 1UL, "Incorrect V1 schemaversion:3");
            Assert.AreEqual(sinceDeprecated.V2, 2UL, "Incorrect V2 schemaversion:3");
            Assert.AreEqual(sinceDeprecated.V3, 3UL, "Incorrect V3 schemaversion:3");
            
            // SchemaVersion is 2
            sinceDeprecated.WrapForDecode(directBuffer, SinceDeprecated.MessageHeader.Size, SinceDeprecated.SinceDeprecated.BlockLength, 2);
            Assert.AreEqual(sinceDeprecated.V1, 1UL, "Incorrect V1 schemaversion:2");
            Assert.AreEqual(sinceDeprecated.V2, 2UL, "Incorrect V2 schemaversion:2");
            Assert.AreEqual(sinceDeprecated.V3, SinceDeprecated.SinceDeprecated.V3NullValue, "Incorrect V3 schemaversion:2");
            
            // SchemaVersion is 1
            sinceDeprecated.WrapForDecode(directBuffer, SinceDeprecated.MessageHeader.Size, SinceDeprecated.SinceDeprecated.BlockLength, 1);
            Assert.AreEqual(sinceDeprecated.V1, 1UL, "Incorrect V1 schemaversion:1");
            Assert.AreEqual(sinceDeprecated.V2, SinceDeprecated.SinceDeprecated.V2NullValue, "Incorrect V2 schemaversion:1");
            Assert.AreEqual(sinceDeprecated.V3, SinceDeprecated.SinceDeprecated.V3NullValue, "Incorrect V3 schemaversion:1");
        }
    }
}
