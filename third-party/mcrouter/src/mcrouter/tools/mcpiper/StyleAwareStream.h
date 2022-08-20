/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Format.h>

#include "mcrouter/tools/mcpiper/StyledString.h"

namespace facebook {
namespace memcache {

/**
 * A StyleAwareStream uses an Encoder to print out StyledStrings.
 * The idea is to have a drop-in replacement for std::ostream that
 * can also issue special formatting instructions.
 *
 * Example: an HtmlEncoder might be written that prints out
 * CSS instructions to render StyledStrings with the correct colors.
 */
template <class Encoder>
class StyleAwareStream {
 public:
  /**
   * @param out The underlying output stream
   */
  explicit StyleAwareStream(std::ostream& out);

  /**
   * Toggle color-coding output instructions on or off.
   */
  void setColorOutput(bool useColor);

  /**
   * Write a regular string in some default color.
   */
  void writePlain(folly::StringPiece sp);

  template <class T>
  StyleAwareStream& operator<<(const T& t);

  template <bool containerMode, class... Args>
  StyleAwareStream& operator<<(
      const folly::Formatter<containerMode, Args...>& formatter);
  StyleAwareStream& operator<<(
      StyleAwareStream<Encoder>& (*f)(StyleAwareStream<Encoder>&));

  /**
   * Write a StyledString possibly in color (if enabled).
   */
  StyleAwareStream& operator<<(const StyledString& s);

  /**
   * Flush the underlying output stream.
   */
  void flush() const;

 private:
  Encoder encoder_;
  bool useColor_;
};

template <class Encoder>
StyleAwareStream<Encoder>& endl(StyleAwareStream<Encoder>&);
} // namespace memcache
} // namespace facebook

#include "StyleAwareStream-inl.h"
