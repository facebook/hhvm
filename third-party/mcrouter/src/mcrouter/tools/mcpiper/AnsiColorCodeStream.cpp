/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "AnsiColorCodeStream.h"

namespace facebook {
namespace memcache {

namespace {

static const char* colorCode(Color color) {
  switch (color) {
    case Color::BLACK:
      return "\x1b[0;30m";
    case Color::DARKGRAY:
      return "\x1b[30;1m";
    case Color::GRAY:
      return "\x1b[0;37m";
    case Color::WHITE:
      return "\x1b[37;1m";

    case Color::RED:
      return "\x1b[31;1m";
    case Color::GREEN:
      return "\x1b[32;1m";
    case Color::YELLOW:
      return "\x1b[33;1m";
    case Color::BLUE:
      return "\x1b[34;1m";
    case Color::MAGENTA:
      return "\x1b[35;1m";
    case Color::CYAN:
      return "\x1b[36;1m";

    case Color::DARKRED:
      return "\x1b[0;31m";
    case Color::DARKGREEN:
      return "\x1b[0;32m";
    case Color::DARKYELLOW:
      return "\x1b[0;33m";
    case Color::DARKBLUE:
      return "\x1b[0;34m";
    case Color::DARKMAGENTA:
      return "\x1b[0;35m";
    case Color::DARKCYAN:
      return "\x1b[0;36m";

    case Color::DEFAULT:
    default:
      return "\x1b[0m";
  }
}

} // namespace

AnsiColorCodeEncoder::AnsiColorCodeEncoder(std::ostream& out)
    : out_(out), isReset_(true) {}

void AnsiColorCodeEncoder::write(const StyledString& str) {
  auto text = str.text();
  size_t b = 0;

  while (b < text.size()) {
    Color currentColor = str.fgColorAt(b);
    out_ << colorCode(currentColor);

    /* Output the maximal string of the same color
       that does not include a newline */
    size_t e = b;
    while (e < text.size() && text[e] != '\n' &&
           str.fgColorAt(e) == currentColor) {
      ++e;
    }
    out_.write(text.begin() + b, e - b);

    /* We reset color before every newline to play nicely with 'less'
       and similar */
    if (e < text.size() && text[e] == '\n') {
      ++e;
      reset();
      out_ << '\n';
    }

    b = e;
  }

  isReset_ = false;
}

void AnsiColorCodeEncoder::flush() const {
  out_.flush();
}

void AnsiColorCodeEncoder::reset() {
  out_ << colorCode(Color::DEFAULT);
  isReset_ = true;
}
} // namespace memcache
} // namespace facebook
