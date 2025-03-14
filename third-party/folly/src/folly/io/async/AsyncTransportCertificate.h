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

#include <optional>
#include <string>

namespace folly {

/**
 * Generic interface applications may implement to convey self or peer
 * certificate related information.
 */
class AsyncTransportCertificate {
 public:
  virtual ~AsyncTransportCertificate() = default;

  /**
   * Returns the identity this certificate conveys.
   *
   * An identity is an opaque string that may be used by the application for
   * authentication or authorization purposes. The exact structure and
   * semantics of the identity string are determined by concrete
   * implementations of AsyncTransport.
   */
  virtual std::string getIdentity() const = 0;

  /**
   * Returns the DER representation of this certificate, if available.
   *
   * NOTE: Not every AsyncTransportCertificate implementation will
   * have a DER representation. Whenever possible, applications should
   * prefer to structure logic around the _identity_ that the
   * certificate conveys (with `getIdentity()`), rather than
   * certificate itself.
   */
  virtual std::optional<std::string> getDER() const = 0;
};
} // namespace folly
