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
#include <thrift/compiler/detail/mustache/state/in_section.h>
#include <thrift/compiler/detail/mustache/state/outside_section.h>
#include <thrift/compiler/detail/mustache/visitor/is_node_empty.h>
#include <thrift/compiler/detail/mustache/visitor/render_section.h>

namespace apache {
namespace thrift {
namespace mstch {

in_section::in_section(type type, const token& start_token)
    : m_type(type), m_start_token(start_token), m_skipped_openings(0) {}

std::string in_section::render(render_context& ctx, const token& token) {
  if (token.token_type() == token::type::section_close) {
    if (token.name() == m_start_token.name() && m_skipped_openings == 0) {
      auto& node = ctx.get_node(m_start_token.name());
      std::string out;

      if (m_type == type::normal && !node.visit(is_node_empty())) {
        out =
            node.visit(render_section(ctx, m_section, m_start_token.delims()));
      } else if (m_type == type::inverted && node.visit(is_node_empty())) {
        out = render_context::push(ctx).render(m_section);
      }

      ctx.set_state<outside_section>();
      return out;
    } else {
      m_skipped_openings--;
    }
  } else if (
      token.token_type() == token::type::inverted_section_open ||
      token.token_type() == token::type::section_open) {
    m_skipped_openings++;
  }

  m_section << token;
  return "";
}

} // namespace mstch
} // namespace thrift
} // namespace apache
