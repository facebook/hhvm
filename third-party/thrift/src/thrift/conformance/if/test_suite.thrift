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

namespace cpp2 apache.thrift.conformance
namespace php apache_thrift
namespace py thrift.conformance.test_suite
namespace py.asyncio thrift_asyncio.conformance.test_suite
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance

include "thrift/conformance/if/serialization.thrift"
include "thrift/conformance/if/rpc.thrift"
include "thrift/conformance/if/patch_data.thrift"

// A Collection of tests.
struct TestSuite {
  // The name of the suite.
  1: string name;

  // A description of the test suite, if useful.
  2: optional string description;

  // The test cases included in the suite.
  3: list<Test> tests;

  // Tags for the whole suite.
  4: set<string> tags;
}

// A collection of test cases.
struct Test {
  // The name of the test.
  1: string name;

  // A description of the test, if useful.
  2: optional string description;

  // The test cases included in the test.
  3: list<TestCase> testCases;

  // Tags for all test cases in this test.
  4: set<string> tags;
}

// A single test case.
struct TestCase {
  // The name of the test.
  1: string name;

  // A description of the test, if useful.
  2: optional string description;

  // The test case to run.
  3: TestCaseUnion test (cpp.mixin);

  // Tags for this test case.
  4: set<string> tags;
}

// A union of all supported test case types.
union TestCaseUnion {
  1: serialization.RoundTripTestCase roundTrip;
  2: rpc.RpcTestCase rpc;
  3: patch_data.PatchOpTestCase objectPatch;
}
