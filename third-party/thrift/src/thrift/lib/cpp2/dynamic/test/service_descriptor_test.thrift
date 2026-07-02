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

package "facebook.com/thrift/service_descriptor_test"

exception TestException {
  1: string message;
}

struct AnnotationWithFields {
  1: string label;
  2: i32 number;
  3: list<string> tags;
}

@AnnotationWithFields{label = "service", number = 100, tags = ["svc"]}
service ServiceDescriptorTestService {
  i32 add(
    @AnnotationWithFields{label = "param", number = 1, tags = ["p"]}
    1: i32 a,
    2: i32 b,
  );
  void ping();
  string greet(1: string name) throws (
    @AnnotationWithFields{label = "exception", number = 2, tags = ["ex"]}
    1: TestException e,
  );
  i32, stream<string> streamNames(1: i32 count);
  sink<string, i32> collectStrings();
  sink<string>, stream<i32> bidiEcho();
  # @lint-ignore THRIFTCHECKS avoid-oneway-method (needed to test oneway RpcKind)
  oneway void fireAndForget();
  /// Annotated function docblock.
  @AnnotationWithFields{label = "hello", number = 7, tags = ["a", "b"]}
  i32 annotated(1: i32 x);
}
