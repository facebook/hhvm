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

#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/detail/mustache/visitor/has_token.h>

namespace apache {
namespace thrift {
namespace mstch {

class get_token {
 public:
  get_token(const std::string& token, const node& node)
      : m_token(token), m_node(node) {}

  template <class T>
  const node& operator()(const T&) const {
    return m_node;
  }

  const node& operator()(const map& map) const { return map.at(m_token); }

  const node& operator()(const std::shared_ptr<object>& object) const {
    return object->at(m_token);
  }

 private:
  const std::string& m_token;
  const node& m_node;
};

} // namespace mstch
} // namespace thrift
} // namespace apache
