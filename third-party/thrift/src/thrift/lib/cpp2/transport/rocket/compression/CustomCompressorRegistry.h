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

#include <memory>
#include <string>

#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorFactory.h>

namespace apache::thrift::rocket {

/**
 * This class provides a global registry of custom compressors for use in
 * Rocket. This complexity is required because open source thrift can't take a
 * direct dependency or have direct knowledge of application-specific
 * compressors. So we instead need a runtime registration interface and
 * mechanism.
 *
 * Currently, for the sake of simplicity, we only support registering a single
 * type of compressor per process. In future iterations, we may expand the
 * interface to support different compressors, and priority support.
 *
 * The expectation is that custom compressors register themselves once at static
 * init time via a pattern like:
 *
 * ```
 * struct Registerer {
 *   Registerer() {
 *     CustomCompressorRegistry::add("foo", foo_factory);
 *   }
 * }
 *
 * static Registerer registerer;
 * ```
 *
 * The methods on this class are all thread-safe.
 */
class CustomCompressorRegistry {
 public:
  /**
   * Register a factory for a given name.
   *
   * Intended to be called at static-init time. Pick a short `name`, as it
   * will be sent over the wire during connection setup.
   *
   * @return true if registration is successful. false otherwise (i.e. if a
   *         factory is already registered or we have a concurrent
   *         registration and this factory didn't make the cut).
   */
  static bool registerFactory(
      std::shared_ptr<const CustomCompressorFactory> factory);

  /**
   * Removes a factory from the registry.
   *
   * @return true if unregistration is successful. false otherwise (i.e.
   *         the compression is not registered or we have a race
   *         with other concurrent operations).
   */
  static bool unregister(const std::string& name);

  /**
   * Retrieve a compressor factory for a given name.
   *
   * Returns nullptr if no factory by the given name is registered.
   */
  static std::shared_ptr<const CustomCompressorFactory> get(
      const std::string& name);
};

} // namespace apache::thrift::rocket
