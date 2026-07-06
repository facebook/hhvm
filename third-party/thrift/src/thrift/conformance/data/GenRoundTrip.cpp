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

#include <folly/init/Init.h>
#include <thrift/conformance/cpp2/internal/AnyRegistry.h>
#include <thrift/conformance/data/Json5Registration.h>
#include <thrift/conformance/data/TestGenerator.h>

using apache::thrift::conformance::StandardProtocol;
using apache::thrift::conformance::data::createRoundTripSuite;
using apache::thrift::conformance::data::serializeToFile;

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  // The generated registry has no Json5 serializer; register it by hand.
  apache::thrift::conformance::data::registerRoundTripJson5Serializers(
      apache::thrift::conformance::detail::getGeneratedAnyRegistry());

  // Json5 is not in kDefaultProtocols; request it explicitly.
  serializeToFile<apache::thrift::BinaryProtocolWriter>(
      createRoundTripSuite(
          {StandardProtocol::Binary,
           StandardProtocol::Compact,
           StandardProtocol::Json5}),
      stdout);
}
