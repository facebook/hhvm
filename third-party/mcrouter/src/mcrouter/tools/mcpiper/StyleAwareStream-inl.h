/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace facebook {
namespace memcache {

template <class Encoder>
StyleAwareStream<Encoder>& endl(StyleAwareStream<Encoder>& stream) {
  stream << '\n';
  stream.flush();
  return stream;
}

template <class Encoder>
StyleAwareStream<Encoder>::StyleAwareStream(std::ostream& out)
    : encoder_(out), useColor_(true) {}

template <class Encoder>
void StyleAwareStream<Encoder>::setColorOutput(bool useColor) {
  useColor_ = useColor;
}

template <class Encoder>
void StyleAwareStream<Encoder>::writePlain(folly::StringPiece sp) {
  encoder_.writePlain(sp);
}

template <class Encoder>
template <class T>
StyleAwareStream<Encoder>& StyleAwareStream<Encoder>::operator<<(const T& t) {
  encoder_.writePlain(t);
  return *this;
}

template <class Encoder>
StyleAwareStream<Encoder>& StyleAwareStream<Encoder>::operator<<(
    const StyledString& s) {
  if (useColor_) {
    encoder_.write(s);
  } else {
    encoder_.writePlain(s.text());
  }
  return *this;
}

template <class Encoder>
template <bool containerMode, class... Args>
StyleAwareStream<Encoder>& StyleAwareStream<Encoder>::operator<<(
    const folly::Formatter<containerMode, Args...>& formatter) {
  auto writer = [this](folly::StringPiece sp) { this->writePlain(sp); };
  formatter(writer);
  return *this;
}

template <class Encoder>
StyleAwareStream<Encoder>& StyleAwareStream<Encoder>::operator<<(
    StyleAwareStream<Encoder>& (*f)(StyleAwareStream<Encoder>&)) {
  return f(*this);
}

template <class Encoder>
void StyleAwareStream<Encoder>::flush() const {
  encoder_.flush();
}
} // namespace memcache
} // namespace facebook
