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

#ifndef THRIFT_FATAL_INDENTER_H_
#define THRIFT_FATAL_INDENTER_H_ 1

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace apache {
namespace thrift {

/**
 * Wraps a stream and provides facilities for automatically indenting output.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename OutputStream>
class indenter {
  struct scope {
    explicit scope(indenter* out) : out_(out) {}
    scope(const scope&) = delete;
    scope(scope&& rhs) noexcept : out_(rhs.out_) { rhs.out_ = nullptr; }

    ~scope() {
      if (out_) {
        out_->pop();
      }
    }

    scope start_scope() { return out_->start_scope(); }

    scope& skip() {
      out_->skip();
      return *this;
    }

    template <typename U>
    indenter& operator<<(U&& value) {
      return (*out_) << std::forward<U>(value);
    }

    scope& newline() {
      out_->newline();
      return *this;
    }

   private:
    indenter* out_;
  };

 public:
  template <typename UOutputStream, typename Indentation>
  indenter(UOutputStream&& out, Indentation&& indentation, std::string margin)
      : out_(out), margin_(std::move(margin)) {
    indentation_.reserve(4);
    indentation_.emplace_back();
    indentation_.emplace_back(std::forward<Indentation>(indentation));
  }

  void push(std::size_t levels = 1) {
    if (levels == 0) {
      return;
    }

    level_ += levels;

    assert(indentation_.size() > 1);
    for (auto i = indentation_.size(); i <= level_; ++i) {
      indentation_.emplace_back(indentation_.back() + indentation_[1]);
    }
    assert(level_ < indentation_.size());
  }

  scope start_scope() {
    push();
    return scope(this);
  }

  void pop(std::size_t levels = 1) {
    assert(levels >= 0);
    assert(level_ >= levels);
    level_ -= levels;
  }

  indenter& skip() {
    indent_ = false;
    return *this;
  }

  template <typename U>
  indenter& operator<<(U&& value) {
    if (indent_) {
      assert(level_ < indentation_.size());
      put_margin();
      out_ << indentation_[level_];
      indent_ = false;
    }

    put_margin();
    out_ << std::forward<U>(value);
    return *this;
  }

  indenter& newline() {
    out_ << '\n';
    indent_ = true;
    at_margin_ = true;
    return *this;
  }

  indenter& set_margin(std::string margin) {
    margin_ = std::move(margin);
    return *this;
  }

 private:
  OutputStream& out_;
  std::vector<std::string> indentation_;
  std::size_t level_ = 0;
  bool indent_ = true;
  bool at_margin_ = true;
  std::string margin_;

  void put_margin() {
    if (at_margin_) {
      out_ << margin_;
      at_margin_ = false;
    }
  }
};

template <typename OutputStream, typename Indentation>
indenter<std::remove_reference_t<OutputStream>> make_indenter(
    OutputStream&& out,
    Indentation&& indentation,
    std::string margin = std::string()) {
  return {out, std::forward<Indentation>(indentation), std::move(margin)};
}

} // namespace thrift
} // namespace apache

#endif // THRIFT_FATAL_INDENTER_H_
