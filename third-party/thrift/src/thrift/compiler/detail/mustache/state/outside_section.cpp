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
#include <thrift/compiler/detail/mustache/render_context.h>
#include <thrift/compiler/detail/mustache/state/in_section.h>
#include <thrift/compiler/detail/mustache/state/outside_section.h>
#include <thrift/compiler/detail/mustache/visitor/render_node.h>

namespace apache {
namespace thrift {
namespace mstch {

std::string outside_section::render(render_context& ctx, const token& token) {
  switch (token.token_type()) {
    case token::type::section_open:
      ctx.set_state<in_section>(in_section::type::normal, token);
      break;
    case token::type::inverted_section_open:
      ctx.set_state<in_section>(in_section::type::inverted, token);
      break;
    case token::type::variable:
      return ctx.get_node(token.name()).visit(render_node(ctx));
    case token::type::text:
      return token.raw();
    case token::type::partial:
      return ctx.render_partial(token.name(), token.partial_prefix());
    default:
      break;
  }
  return "";
}

} // namespace mstch
} // namespace thrift
} // namespace apache
