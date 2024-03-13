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

#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <folly/container/F14Map.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/sbe/MessageWrapper.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/CustomerResponse.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/GroupSizeEncoding.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/facebook_sbe_test/MessageHeader.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/Customer.h>

namespace facebook::sbe::test {

class DataLoader {
 public:
  DataLoader() = default;

  void loadLines(const std::string& path, std::vector<std::string>& lines);

  void loadIntoMap(
      const std::string& path, folly::F14FastMap<std::string, Customer>& map);
};

} // namespace facebook::sbe::test
