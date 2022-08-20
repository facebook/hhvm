/*

The source code contained in this file is based on the original code by
Daniel Sipka (https://github.com/no1msd/mstch). The original license by Daniel
Sipka can be read below:

The MIT License (MIT)

Copyright (c) 2015 Daniel Sipka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <thrift/compiler/detail/mustache/token.h>
#include <thrift/compiler/detail/mustache/utils.h>

namespace apache {
namespace thrift {
namespace mstch {

token::type token::token_info(char c) {
  switch (c) {
    case '>':
      return type::partial;
    case '^':
      return type::inverted_section_open;
    case '/':
      return type::section_close;
    case '#':
      return type::section_open;
    case '!':
      return type::comment;
    default:
      return type::variable;
  }
}

token::token(const std::string& str, std::size_t left, std::size_t right)
    : m_raw(str), m_eol(false), m_ws_only(false) {
  if (left != 0 && right != 0) {
    if (str[left] == '=' && str[str.size() - right - 1] == '=') {
      m_type = type::delimiter_change;
    } else {
      auto c = first_not_ws(str.begin() + left, str.end() - right);
      m_type = token_info(*c);
      if (m_type != type::variable) {
        c = first_not_ws(c + 1, str.end() - right);
      }
      m_name = {c, first_not_ws(str.rbegin() + right, str.rend() - left) + 1};
      m_delims = {
          {str.begin(), str.begin() + left}, {str.end() - right, str.end()}};
    }
  } else {
    m_type = type::text;
    m_eol = (str.size() > 0 && str[str.size() - 1] == '\n');
    m_ws_only = (str.find_first_not_of(" \r\n\t") == std::string::npos);
  }
}

} // namespace mstch
} // namespace thrift
} // namespace apache
