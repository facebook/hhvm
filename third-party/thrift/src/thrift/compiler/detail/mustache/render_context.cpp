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
#include <thrift/compiler/detail/mustache/state/outside_section.h>
#include <thrift/compiler/detail/mustache/visitor/get_token.h>

namespace apache {
namespace thrift {
namespace mstch {

const node render_context::null_node;

render_context::push::push(render_context& context, const node& node)
    : m_context(context) {
  context.m_nodes.emplace_front(node);
  context.m_node_ptrs.emplace_front(&node);
  context.m_state.push(std::unique_ptr<render_state>(new outside_section));
}

render_context::push::~push() {
  m_context.m_nodes.pop_front();
  m_context.m_node_ptrs.pop_front();
  m_context.m_state.pop();
}

std::string render_context::push::render(const template_type& templt) {
  return m_context.render(templt);
}

render_context::render_context(
    const node& node, const std::map<std::string, template_type>& partials)
    : m_partials(partials), m_nodes(1, node), m_node_ptrs(1, &node) {
  m_state.push(std::unique_ptr<render_state>(new outside_section));
}

const node& render_context::find_node(
    const std::string& token, std::list<const node*> current_nodes) {
  if (token != "." && token.find('.') != std::string::npos) {
    return find_node(
        token.substr(token.rfind('.') + 1),
        {&find_node(token.substr(0, token.rfind('.')), current_nodes)});
  } else {
    for (auto& node : current_nodes) {
      if (node->visit(has_token(token))) {
        return node->visit(get_token(token, *node));
      }
    }
  }
  return null_node;
}

const node& render_context::get_node(const std::string& token) {
  return find_node(token, m_node_ptrs);
}

std::string render_context::render(
    const template_type& templt, const std::string& prefix) {
  std::string output;
  bool prev_eol = true;
  for (auto& token : templt) {
    if (prev_eol && prefix.length() != 0) {
      output += m_state.top()->render(*this, {prefix});
    }
    output += m_state.top()->render(*this, token);
    prev_eol = token.eol();
  }
  return output;
}

std::string render_context::render_partial(
    const std::string& partial_name, const std::string& prefix) {
  return m_partials.count(partial_name)
      ? render(m_partials.at(partial_name), prefix)
      : "";
}

} // namespace mstch
} // namespace thrift
} // namespace apache
