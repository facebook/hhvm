/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StyledString.h"

namespace facebook {
namespace memcache {

StyledString::StyledString() {}

StyledString::StyledString(std::string s, Color color) : text_(std::move(s)) {
  fg_.resize(text_.size(), color);
}

folly::StringPiece StyledString::text() const {
  return text_;
}

StyledString StyledString::operator+(const StyledString& b) const {
  StyledString c(*this);
  c += b;
  return c;
}

void StyledString::pushAppendColor(Color color) {
  stack_.push_back(color);
}

void StyledString::popAppendColor() {
  stack_.pop_back();
}

void StyledString::append(const std::string& s) {
  text_.append(s);
  fg_.resize(text_.size(), stack_.empty() ? Color::DEFAULT : stack_.back());
}

void StyledString::append(const std::string& s, Color color) {
  text_.append(s);
  fg_.resize(text_.size(), color);
}

void StyledString::append(const StyledString& s) {
  text_.append(s.text_);
  fg_.insert(fg_.end(), s.fg_.begin(), s.fg_.end());
}

void StyledString::pushBack(char c) {
  text_.push_back(c);
  fg_.push_back(stack_.empty() ? Color::DEFAULT : stack_.back());
}

void StyledString::pushBack(char c, Color color) {
  text_.push_back(c);
  fg_.push_back(color);
}

void StyledString::setFg(size_t begin, size_t size, Color color) {
  for (size_t i = begin; i < begin + size; ++i) {
    fg_[i] = color;
  }
}

Color StyledString::fgColorAt(size_t i) const {
  return fg_[i];
}

size_t StyledString::size() const {
  return text_.size();
}
} // namespace memcache
} // namespace facebook
