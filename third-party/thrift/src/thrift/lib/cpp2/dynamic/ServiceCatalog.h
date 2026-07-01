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

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace apache::thrift::dynamic {

class ServiceDescriptor;

/**
 * A lookup of the services in a schema, keyed by service URI (universal name).
 *
 * The URI is the unique identity for a service, matching how user-defined types
 * are addressed in `TypeSystem`. A service without a URI (no package and no
 * `@thrift.Uri`) is not indexed: it has no stable name to look it up by.
 */
class ServiceCatalog {
 public:
  virtual ~ServiceCatalog() = default;

  /**
   * Returns the service with the given URI, or nullptr if no such service is in
   * the catalog.
   */
  virtual const ServiceDescriptor* getService(std::string_view uri) const = 0;

  /**
   * The URIs of every service in the catalog, in unspecified order.
   */
  virtual std::vector<std::string_view> serviceUris() const = 0;

  /**
   * Like `getService`, but throws `std::out_of_range` when the URI is absent.
   */
  const ServiceDescriptor& getServiceOrThrow(std::string_view uri) const {
    if (const ServiceDescriptor* service = getService(uri)) {
      return *service;
    }
    throw std::out_of_range("No service with URI: " + std::string(uri));
  }
};

} // namespace apache::thrift::dynamic
