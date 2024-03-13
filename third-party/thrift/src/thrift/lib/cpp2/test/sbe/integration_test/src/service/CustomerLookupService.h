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

#include <glog/logging.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/test/sbe/integration_test/src/service/CustomerLookupHandler.h>

namespace facebook::sbe::test {

std::shared_ptr<CustomerLookupHandler> createCustomerLookupHandler(
    const std::string&);

std::shared_ptr<apache::thrift::ThriftServer> createCustomerLookupService(
    std::shared_ptr<apache::thrift::ServiceHandler<CustomerLookupHandler>>
        handler = nullptr);

} // namespace facebook::sbe::test
