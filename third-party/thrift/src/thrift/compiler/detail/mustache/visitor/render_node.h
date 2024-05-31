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

#include <sstream>

#include <fmt/core.h>
#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/detail/mustache/render_context.h>
#include <thrift/compiler/detail/mustache/utils.h>

namespace apache {
namespace thrift {
namespace mstch {

class render_node {
 public:
  render_node(render_context& ctx) : m_ctx(ctx) {}

  template <class T>
  void operator()(const T&) const {}

  void operator()(const int& value) const {
    fmt::format_to(std::back_inserter(m_ctx.out), "{}", value);
  }

  void operator()(const double& value) const {
    fmt::format_to(std::back_inserter(m_ctx.out), "{}", value);
  }

  void operator()(const bool& value) const {
    m_ctx.out += value ? "true" : "false";
  }

  void operator()(const lambda& value) const {
    // Reset m_ctx to an empty output.
    std::string prior_output;
    std::swap(prior_output, m_ctx.out);

    // Render the lamdba's lazy value into m_ctx.
    value([this](const node& n) { n.visit(render_node(m_ctx)); });

    // Parse a template out of the lambda's output.
    template_type interpreted{m_ctx.out};

    // Restore the original value of m_ctx, and continue rendering with the
    // obtained template.
    m_ctx.out = std::move(prior_output);
    render_context::push(m_ctx).render(interpreted);
  }

  void operator()(const std::string& value) const { m_ctx.out += value; }

 private:
  render_context& m_ctx;
};

} // namespace mstch
} // namespace thrift
} // namespace apache
