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
using FBThrift.Tests.Unions;

namespace FBThrift.Tests
{
    /// <summary>
    /// Tests for generated Thrift unions.
    /// </summary>
    [TestFixture]
    public class GeneratedUnionsTests
    {
        [Test]
        public void TestUnionInstantiation()
        {
            var u = new MyUnion();
            Assert.IsNotNull(u);
        }

        [Test]
        public void TestUnionSetEnumField()
        {
            var u = new MyUnion();
            u.myEnum = MyEnum.MyValue2;
            Assert.AreEqual(MyEnum.MyValue2, u.myEnum);
        }

        [Test]
        public void TestUnionSetStructField()
        {
            var u = new MyUnion();
            var data = new MyDataItem();
            u.myDataItem = data;

            Assert.IsNotNull(u.myDataItem);
        }

        [Test]
        public void TestUnionEquality()
        {
            var u1 = new MyUnion();
            u1.myEnum = MyEnum.MyValue1;

            var u2 = new MyUnion();
            u2.myEnum = MyEnum.MyValue1;

            Assert.AreEqual(u1, u2);
        }

        [Test]
        public void TestUnionInequality()
        {
            var u1 = new MyUnion();
            u1.myEnum = MyEnum.MyValue1;

            var u2 = new MyUnion();
            u2.myEnum = MyEnum.MyValue2;

            Assert.AreNotEqual(u1, u2);
        }

        [Test]
        public void TestUnionToString()
        {
            var u = new MyUnion();
            u.myEnum = MyEnum.MyValue1;

            var str = u.ToString();
            Assert.IsNotNull(str);
        }

        [Test]
        public void TestUnionHashCode()
        {
            var u1 = new MyUnion();
            u1.myEnum = MyEnum.MyValue1;

            var u2 = new MyUnion();
            u2.myEnum = MyEnum.MyValue1;

            Assert.AreEqual(u1.GetHashCode(), u2.GetHashCode());
        }

        [Test]
        public void TestBinaryUnionEqualityUsesContents()
        {
            var u1 = new BinaryUnion { data = new byte[] { 1, 2, 3 } };
            var u2 = new BinaryUnion { data = new byte[] { 1, 2, 3 } };

            Assert.AreEqual(u1, u2);
            Assert.AreEqual(u1.GetHashCode(), u2.GetHashCode());

            u2.data = new byte[] { 1, 2, 4 };
            Assert.AreNotEqual(u1, u2);
        }

        [Test]
        public void TestUnionToBeRenamed()
        {
            // Test another union type in the basic fixture
            var u = new UnionToBeRenamed();
            Assert.IsNotNull(u);
        }

        [Test]
        public void TestUnionSetNullEnumFieldThrows()
        {
            var u = new MyUnion();
            Assert.Throws<ArgumentNullException>(() => u.myEnum = null);
        }

        [Test]
        public void TestUnionSetNullStructFieldThrows()
        {
            var u = new MyUnion();
            Assert.Throws<ArgumentNullException>(() => u.myDataItem = null);
        }

        [Test]
        public void TestUnionSetNullPrimitiveFieldThrows()
        {
            var u = new UnionToBeRenamed();
            Assert.Throws<ArgumentNullException>(() => u.reserved_field = null);
        }

        [Test]
        public void TestUnionSetNullContainerFieldThrows()
        {
            var u = new TypedefContainerUnion();
            Assert.Throws<ArgumentNullException>(() => u.int_list_field = null);
        }
    }
}
