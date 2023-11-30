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
namespace go thrift.conformance.test_suite
namespace php apache_thrift
namespace py thrift.conformance.test_suite
namespace py.asyncio thrift_asyncio.conformance.test_suite
namespace py3 thrift.conformance
namespace java.swift org.apache.thrift.conformance

include "thrift/conformance/if/serialization.thrift"
include "thrift/conformance/if/rpc.thrift"
include "thrift/conformance/if/patch_data.thrift"
include "thrift/annotation/thrift.thrift"

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
  @thrift.Mixin
  3: TestCaseUnion test;

  // Tags for this test case.
  4: set<string> tags;
}

// A union of all supported test case types.
union TestCaseUnion {
  1: serialization.RoundTripTestCase roundTrip;
  2: rpc.RpcTestCase rpc;
  3: patch_data.PatchOpTestCase objectPatch;
}

/**
 * A feature being tracked for a Thrift release.
 *
 * For example:
 *
 *   name: Any
 *   description: >
 *     A type that can contain any Thrift value (including void), even when the
 *     type is not known at gen-, compile- or even run-time.
 *   tags: [features/#Any]
 *   release: 1
 *
 */
struct Feature {
  /** The name of the feature. */
  1: string name;

  /** A short description of the feature. */
  2: string description;

  /**
   * The tags that roll up into this feature.
   *
   * For example: feature/MyFeature
   */
  3: set<string> tags;

  /**
   * The (target) Thrift release for this feature.
   *
   * Thrift release numbers are strictly monotonic integers.
   */
  4: i32 release;
}
