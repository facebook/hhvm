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
using Test.Fixtures.Basic;

namespace FBThrift.Tests
{
    /// <summary>
    /// Tests for generated Thrift structs using the main basic fixture.
    /// </summary>
    [TestFixture]
    public class GeneratedStructsTests
    {
        [Test]
        public void TestBasicStructInstantiation()
        {
            var myStruct = new MyStruct();
            Assert.IsNotNull(myStruct);
        }

        [Test]
        public void TestBasicStructFieldAccess()
        {
            var myStruct = new MyStruct();
            myStruct.@MyIntField = 42;
            myStruct.@MyStringField = "hello";
            Assert.AreEqual(42, myStruct.@MyIntField);
            Assert.AreEqual("hello", myStruct.@MyStringField);
        }

        [Test]
        public void TestStructWithEnumField()
        {
            var myStruct = new MyStruct();
            myStruct.@myEnum = MyEnum.MyValue1;
            Assert.AreEqual(MyEnum.MyValue1, myStruct.@myEnum);

            myStruct.@myEnum = MyEnum.MyValue2;
            Assert.AreEqual(MyEnum.MyValue2, myStruct.@myEnum);
        }

        [Test]
        public void TestEnumValues()
        {
            Assert.AreEqual(0, (int)MyEnum.MyValue1);
            Assert.AreEqual(1, (int)MyEnum.MyValue2);
        }

        [Test]
        public void TestContainersStruct()
        {
            var containers = new Containers();
            containers.@I32List = new List<int> { 1, 2, 3, 4, 5 };
            containers.@StringSet = new HashSet<string> { "alpha", "beta", "gamma" };
            containers.@StringToI64Map = new Dictionary<string, long>
            {
                { "one", 1L },
                { "two", 2L }
            };

            Assert.AreEqual(5, containers.@I32List.Count);
            Assert.AreEqual(1, containers.@I32List[0]);
            Assert.AreEqual(3, containers.@StringSet.Count);
            Assert.That(containers.@StringSet, Does.Contain("alpha"));
            Assert.AreEqual(2, containers.@StringToI64Map.Count);
            Assert.AreEqual(1L, containers.@StringToI64Map["one"]);
        }

        [Test]
        public void TestNestedStruct()
        {
            var inner = new MyDataItem();
            var outer = new MyStruct();
            outer.@MyDataField = inner;
            Assert.IsNotNull(outer.@MyDataField);
        }

        [Test]
        public void TestEmptyStruct()
        {
            var item = new MyDataItem();
            Assert.IsNotNull(item);
            Assert.IsTrue(item.__fbthrift_is_empty());
        }

        [Test]
        public void TestStructEquality()
        {
            var s1 = new MyStruct();
            s1.@MyIntField = 10;
            s1.@MyStringField = "test";

            var s2 = new MyStruct();
            s2.@MyIntField = 10;
            s2.@MyStringField = "test";

            Assert.AreEqual(s1, s2);
        }

        [Test]
        public void TestStructInequality()
        {
            var s1 = new MyStruct();
            s1.@MyIntField = 10;
            s1.@MyStringField = "test";

            var s2 = new MyStruct();
            s2.@MyIntField = 20;
            s2.@MyStringField = "different";

            Assert.AreNotEqual(s1, s2);
        }

        [Test]
        public void TestStructToString()
        {
            var myStruct = new MyStruct();
            myStruct.@MyIntField = 42;
            myStruct.@MyStringField = "hello";

            var str = myStruct.ToString();
            Assert.That(str, Does.Contain("42"));
            Assert.That(str, Does.Contain("hello"));
        }

        [Test]
        public void TestStructHashCode()
        {
            var s1 = new MyStruct();
            s1.@MyIntField = 10;
            s1.@MyStringField = "test";

            var s2 = new MyStruct();
            s2.@MyIntField = 10;
            s2.@MyStringField = "test";

            Assert.AreEqual(s1.GetHashCode(), s2.GetHashCode());
        }

        [Test]
        public void TestStructClear()
        {
            var myStruct = new MyStruct();
            myStruct.@MyIntField = 42;
            myStruct.@MyStringField = "hello";
            myStruct.__fbthrift_clear();

            Assert.AreEqual(0, myStruct.@MyIntField);
        }

        [Test]
        public void TestExceptionStruct()
        {
            var ex = new MyException();
            ex.@MyIntField = 404;
            ex.@MyStringField = "Not Found";
            Assert.AreEqual(404, ex.@MyIntField);
            Assert.AreEqual("Not Found", ex.@MyStringField);
            Assert.IsInstanceOf<Exception>(ex);
        }
    }
}
