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
using NUnit.Framework;
using Test.Fixtures.CsharpConstants;

namespace FBThrift.Tests
{
    /// <summary>
    /// Tests for generated Thrift constants.
    /// Uses the minimal csharp-constants fixture (no struct/union constants).
    /// TODO: Delete this test and use the full constants fixture once struct codegen lands (D94439431).
    /// </summary>
    [TestFixture]
    public class GeneratedConstantsTests
    {
        [Test]
        public void TestIntConstant()
        {
            Assert.AreEqual(1337, Constants.myInt);
        }

        [Test]
        public void TestLongConstant()
        {
            Assert.AreEqual(9223372036854775807L, Constants.myLong);
        }

        [Test]
        public void TestStringConstant()
        {
            Assert.AreEqual("Mark Zuckerberg", Constants.name);
        }

        [Test]
        public void TestDoubleConstant()
        {
            Assert.AreEqual(3.14159, Constants.pi, 0.00001);
        }

        [Test]
        public void TestBoolConstants()
        {
            Assert.AreEqual(true, Constants.enabled);
            Assert.AreEqual(false, Constants.disabled);
        }

        [Test]
        public void TestEnumConstant()
        {
            Assert.AreEqual(Color.GREEN, Constants.favoriteColor);
        }

        [Test]
        public void TestEnumValues()
        {
            Assert.AreEqual(0, (int)Color.RED);
            Assert.AreEqual(1, (int)Color.GREEN);
            Assert.AreEqual(2, (int)Color.BLUE);
        }

        [Test]
        public void TestListOfInts()
        {
            Assert.AreEqual(5, Constants.numbers.Count);
            Assert.AreEqual(1, Constants.numbers[0]);
            Assert.AreEqual(5, Constants.numbers[4]);
        }

        [Test]
        public void TestListOfStrings()
        {
            Assert.AreEqual(3, Constants.names.Count);
            Assert.AreEqual("Alice", Constants.names[0]);
            Assert.AreEqual("Bob", Constants.names[1]);
            Assert.AreEqual("Charlie", Constants.names[2]);
        }

        [Test]
        public void TestListOfEnums()
        {
            Assert.AreEqual(3, Constants.palette.Count);
            Assert.AreEqual(Color.RED, Constants.palette[0]);
            Assert.AreEqual(Color.GREEN, Constants.palette[1]);
            Assert.AreEqual(Color.BLUE, Constants.palette[2]);
        }

        [Test]
        public void TestSetOfInts()
        {
            Assert.AreEqual(3, Constants.uniqueNumbers.Count);
            Assert.That(Constants.uniqueNumbers, Does.Contain(1));
            Assert.That(Constants.uniqueNumbers, Does.Contain(2));
            Assert.That(Constants.uniqueNumbers, Does.Contain(3));
        }

        [Test]
        public void TestSetOfStrings()
        {
            Assert.AreEqual(2, Constants.tags.Count);
            Assert.That(Constants.tags, Does.Contain("alpha"));
            Assert.That(Constants.tags, Does.Contain("beta"));
        }

        [Test]
        public void TestMapStringToInt()
        {
            Assert.AreEqual(2, Constants.ages.Count);
            Assert.AreEqual(30, Constants.ages["Alice"]);
            Assert.AreEqual(25, Constants.ages["Bob"]);
        }

        [Test]
        public void TestMapIntToString()
        {
            Assert.AreEqual(2, Constants.idToName.Count);
            Assert.AreEqual("One", Constants.idToName[1]);
            Assert.AreEqual("Two", Constants.idToName[2]);
        }

        [Test]
        public void TestMapStringToEnum()
        {
            Assert.AreEqual(2, Constants.colorLookup.Count);
            Assert.AreEqual(Color.RED, Constants.colorLookup["red"]);
            Assert.AreEqual(Color.GREEN, Constants.colorLookup["green"]);
        }

        [Test]
        public void TestEmptyCollections()
        {
            Assert.AreEqual(0, Constants.emptyList.Count);
            Assert.AreEqual(0, Constants.emptySet.Count);
            Assert.AreEqual(0, Constants.emptyMap.Count);
        }

        [Test]
        public void TestStringEscapeSequences()
        {
            Assert.AreEqual("'", Constants.apostrophe);
            Assert.AreEqual("\"", Constants.quote);
            Assert.AreEqual("\\", Constants.backslash);
            Assert.AreEqual("\n", Constants.newline);
            Assert.AreEqual("\t", Constants.tab);
        }

        [Test]
        public void TestBinaryConstant()
        {
            Assert.AreEqual(new byte[] { 97, 0, 122 }, Constants.binData);
        }

        [Test]
        public void TestBoundaryValues()
        {
            Assert.AreEqual(long.MaxValue, Constants.maxLong);
            Assert.AreEqual(long.MinValue, Constants.minLong);
            Assert.AreEqual(0.0, Constants.zero);
        }
    }
}
