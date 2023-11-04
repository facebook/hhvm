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

#include <thrift/compiler/ast/t_struct.h>

namespace apache {
namespace thrift {
namespace compiler {

enum class t_error_kind {
  unspecified = 0, // The kind of error was not specified and the associated RPC
                   // might succeed if retried.
  transient, // The associated RPC might succeed if retried.
  stateful, // Server state must be change for the associated RPC to have
            // any chance of succeeding.
  permanent, // The associated RPC can never succeed, and should not be retried.
};

enum class t_error_blame {
  unspecified = 0, // The blame for the error was not specified.
  server, // The error was the fault of the server.
  client, // The error was the fault of the client's request.
};

enum class t_error_safety {
  unspecified = 0, // The safety for the error was not specified.
  safe, // It is guarneteed the associated RPC failed completely, and no
        // significant server state changed while trying to process the
        // RPC.
};

/**
 * Represents an exception definition.
 *
 * Exceptions are similar to structs but can only be used in throws clauses.
 */
// TODO(afuller): Inherit from t_structured instead.
class t_exception : public t_struct {
 public:
  using t_struct::t_struct;

  t_error_kind kind() const { return kind_; }
  void set_kind(t_error_kind kind) { kind_ = kind; }

  t_error_blame blame() const { return blame_; }
  void set_blame(t_error_blame blame) { blame_ = blame; }

  t_error_safety safety() const { return safety_; }
  void set_safety(t_error_safety safety) { safety_ = safety; }

  const t_field* get_message_field() const {
    for (const auto* field : get_members()) {
      if (field->find_structured_annotation_or_null(kExceptionMessageUri)) {
        return field;
      }
    }
    const auto* value = find_annotation_or_null("message");
    return value ? get_field_by_name(*value) : nullptr;
  }

 private:
  t_error_kind kind_{};
  t_error_blame blame_{};
  t_error_safety safety_{};

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  bool is_exception() const override { return true; }

 private:
  friend class t_struct;
  t_exception* clone_DO_NOT_USE() const override {
    auto clone = std::make_unique<t_exception>(program_, name_);
    clone_structured(clone.get());
    clone->kind_ = kind_;
    clone->blame_ = blame_;
    clone->safety_ = safety_;
    return clone.release();
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
