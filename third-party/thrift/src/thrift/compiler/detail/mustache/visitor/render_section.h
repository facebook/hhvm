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
#include <thrift/compiler/detail/mustache/render_context.h>
#include <thrift/compiler/detail/mustache/utils.h>
#include <thrift/compiler/detail/mustache/visitor/render_node.h>

namespace apache {
namespace thrift {
namespace mstch {

class render_section {
 public:
  enum class flag { none, keep_array };
  render_section(
      render_context& ctx,
      const template_type& section,
      const delim_type& delims,
      flag p_flag = flag::none)
      : m_ctx(ctx), m_section(section), m_delims(delims), m_flag(p_flag) {}

  template <class T>
  std::string operator()(const T& t) const {
    return render_context::push(m_ctx, t).render(m_section);
  }

  std::string operator()(const lambda& fun) const {
    std::string section_str;
    for (auto& token : m_section)
      section_str += token.raw();
    template_type interpreted{
        fun([this](const node& n) { return n.visit(render_node(m_ctx)); },
            section_str),
        m_delims};
    return render_context::push(m_ctx).render(interpreted);
  }

  std::string operator()(const array& array) const {
    std::string out;
    if (m_flag == flag::keep_array)
      return render_context::push(m_ctx, array).render(m_section);
    else
      for (auto& item : array)
        out += item.visit(
            render_section(m_ctx, m_section, m_delims, flag::keep_array));
    return out;
  }

 private:
  render_context& m_ctx;
  const template_type& m_section;
  const delim_type& m_delims;
  flag m_flag;
};

} // namespace mstch
} // namespace thrift
} // namespace apache
