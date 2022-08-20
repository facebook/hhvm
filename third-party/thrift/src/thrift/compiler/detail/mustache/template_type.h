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
#include <vector>

#include <thrift/compiler/detail/mustache/token.h>
#include <thrift/compiler/detail/mustache/utils.h>

namespace apache {
namespace thrift {
namespace mstch {

class template_type {
 public:
  template_type() = default;
  /* implicit */ template_type(const std::string& str);
  template_type(const std::string& str, const delim_type& delims);
  std::vector<token>::const_iterator begin() const { return m_tokens.begin(); }
  std::vector<token>::const_iterator end() const { return m_tokens.end(); }
  void operator<<(const token& token) { m_tokens.push_back(token); }

 private:
  std::vector<token> m_tokens;
  std::string m_open;
  std::string m_close;
  void strip_whitespace();
  void process_text(citer beg, citer end);
  void tokenize(const std::string& tmp);
  void store_prefixes(std::vector<token>::iterator beg);
};

} // namespace mstch
} // namespace thrift
} // namespace apache
