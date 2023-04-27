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

#include <string>

namespace wangle {

/**
 * An enum representing different kinds
 * of secure transports we can negotiate.
 */
enum class SecureTransportType {
  NONE, // Transport is not secure.
  TLS, // Transport is based on TLS
};

std::string getSecureTransportName(const SecureTransportType& type);

} // namespace wangle

// This enum used to be un-namespaced. This is here for temporary backwards
// compatibility with old uses of this enum.
using wangle::SecureTransportType;
