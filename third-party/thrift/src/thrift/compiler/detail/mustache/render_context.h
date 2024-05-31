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

#include <deque>
#include <list>
#include <sstream>
#include <stack>
#include <string>

#include <thrift/compiler/detail/mustache/mstch.h>
#include <thrift/compiler/detail/mustache/state/render_state.h>
#include <thrift/compiler/detail/mustache/template_type.h>

namespace apache {
namespace thrift {
namespace mstch {

class render_context {
 public:
  class push {
   public:
    /* implicit */ push(render_context& context, const node& node = {});
    ~push();
    void render(const template_type& templt);

   private:
    render_context& m_context;
  };

  render_context(
      const node& node, const std::map<std::string, template_type>& partials);
  const node& get_node(const std::string& token);
  void render(const template_type& templt, const std::string& prefix = "");
  void render_partial(
      const std::string& partial_name, const std::string& prefix);
  template <class T, class... Args>
  void set_state(Args&&... args) {
    m_state.top() =
        std::unique_ptr<render_state>(new T(std::forward<Args>(args)...));
  }

 private:
  static const node null_node;
  const node& find_node(
      const std::string& token, std::list<const node*> current_nodes);
  std::map<std::string, template_type> m_partials;
  std::deque<node> m_nodes;
  std::list<const node*> m_node_ptrs;
  std::stack<std::unique_ptr<render_state>> m_state;

 public:
  std::string out;
};

} // namespace mstch
} // namespace thrift
} // namespace apache
