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
#pragma once

#include <string>

namespace apache {
namespace thrift {
namespace mstch {

using delim_type = std::pair<std::string, std::string>;

class token {
 public:
  enum class type {
    text,
    variable,
    section_open,
    section_close,
    inverted_section_open,
    comment,
    partial,
    delimiter_change
  };
  /* implicit */ token(
      const std::string& str, std::size_t left = 0, std::size_t right = 0);
  type token_type() const { return m_type; }
  const std::string& raw() const { return m_raw; }
  const std::string& name() const { return m_name; }
  const std::string& partial_prefix() const { return m_partial_prefix; }
  const delim_type& delims() const { return m_delims; }
  void partial_prefix(const std::string& p_partial_prefix) {
    m_partial_prefix = p_partial_prefix;
  }
  bool eol() const { return m_eol; }
  void eol(bool eol) { m_eol = eol; }
  bool ws_only() const { return m_ws_only; }

 private:
  type m_type;
  std::string m_name;
  std::string m_raw;
  std::string m_partial_prefix;
  delim_type m_delims;
  bool m_eol;
  bool m_ws_only;
  type token_info(char c);
};

} // namespace mstch
} // namespace thrift
} // namespace apache
