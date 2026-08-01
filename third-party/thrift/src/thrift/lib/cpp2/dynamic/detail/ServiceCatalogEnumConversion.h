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

#include <thrift/lib/thrift/gen-cpp2/schema_types.h>
#include <thrift/lib/thrift/gen-cpp2/service_catalog_types.h>

namespace apache::thrift::dynamic::detail {

inline type_system::ExceptionSafety toSerializableSafety(
    type::ErrorSafety value) {
  switch (value) {
    case type::ErrorSafety::Unspecified:
      return type_system::ExceptionSafety::Unspecified;
    case type::ErrorSafety::Safe:
      return type_system::ExceptionSafety::Safe;
  }
  return type_system::ExceptionSafety::Unspecified;
}

inline type::ErrorSafety fromSerializableSafety(
    type_system::ExceptionSafety value) {
  switch (value) {
    case type_system::ExceptionSafety::Unspecified:
      return type::ErrorSafety::Unspecified;
    case type_system::ExceptionSafety::Safe:
      return type::ErrorSafety::Safe;
  }
  return type::ErrorSafety::Unspecified;
}

inline type_system::ExceptionKind toSerializableKind(type::ErrorKind value) {
  switch (value) {
    case type::ErrorKind::Unspecified:
      return type_system::ExceptionKind::Unspecified;
    case type::ErrorKind::Transient:
      return type_system::ExceptionKind::Transient;
    case type::ErrorKind::Stateful:
      return type_system::ExceptionKind::Stateful;
    case type::ErrorKind::Permanent:
      return type_system::ExceptionKind::Permanent;
  }
  return type_system::ExceptionKind::Unspecified;
}

inline type::ErrorKind fromSerializableKind(type_system::ExceptionKind value) {
  switch (value) {
    case type_system::ExceptionKind::Unspecified:
      return type::ErrorKind::Unspecified;
    case type_system::ExceptionKind::Transient:
      return type::ErrorKind::Transient;
    case type_system::ExceptionKind::Stateful:
      return type::ErrorKind::Stateful;
    case type_system::ExceptionKind::Permanent:
      return type::ErrorKind::Permanent;
  }
  return type::ErrorKind::Unspecified;
}

inline type_system::ExceptionBlame toSerializableBlame(type::ErrorBlame value) {
  switch (value) {
    case type::ErrorBlame::Unspecified:
      return type_system::ExceptionBlame::Unspecified;
    case type::ErrorBlame::Server:
      return type_system::ExceptionBlame::Server;
    case type::ErrorBlame::Client:
      return type_system::ExceptionBlame::Client;
  }
  return type_system::ExceptionBlame::Unspecified;
}

inline type::ErrorBlame fromSerializableBlame(
    type_system::ExceptionBlame value) {
  switch (value) {
    case type_system::ExceptionBlame::Unspecified:
      return type::ErrorBlame::Unspecified;
    case type_system::ExceptionBlame::Server:
      return type::ErrorBlame::Server;
    case type_system::ExceptionBlame::Client:
      return type::ErrorBlame::Client;
  }
  return type::ErrorBlame::Unspecified;
}

} // namespace apache::thrift::dynamic::detail
