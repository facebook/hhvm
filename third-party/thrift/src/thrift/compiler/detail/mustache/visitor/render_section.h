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
      flag p_flag = flag::none)
      : m_ctx(ctx), m_section(section), m_flag(p_flag) {}

  template <class T>
  void operator()(const T& t) const {
    render_context::push(m_ctx, t).render(m_section);
  }

  void operator()(const lambda& fun) const {
    // Concatenate the unrendered contents of this section.
    std::string section_str;
    for (auto& token : m_section)
      section_str += token.raw();

    // Reset m_ctx to an empty output.
    std::string prior_output;
    std::swap(prior_output, m_ctx.out);

    // Pass unrendered section content to lambda, and render lambda's lazy value
    // into m_ctx.
    fun([this](const node& n) { n.visit(render_node(m_ctx)); }, section_str);

    // Parse a template out of the lamdba's output.
    template_type interpreted{m_ctx.out};

    // Restore the original value of m_ctx, and continue rendering with the
    // obtained template.
    m_ctx.out = std::move(prior_output);
    render_context::push(m_ctx).render(interpreted);
  }

  void operator()(const array& array) const {
    if (m_flag == flag::keep_array)
      render_context::push(m_ctx, array).render(m_section);
    else
      for (auto& item : array)
        item.visit(render_section(m_ctx, m_section, flag::keep_array));
  }

 private:
  render_context& m_ctx;
  const template_type& m_section;
  flag m_flag;
};

} // namespace mstch
} // namespace thrift
} // namespace apache
