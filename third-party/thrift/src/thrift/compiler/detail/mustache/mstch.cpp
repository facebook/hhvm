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

#include <thrift/compiler/detail/mustache/mstch.h>

#include <limits>
#include <stdexcept>
#include <fmt/core.h>
#include <thrift/compiler/detail/mustache/render_context.h>

namespace apache::thrift::mstch {

node::node(std::size_t i) : base(static_cast<int>(i)) {
  if (i > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
    throw std::overflow_error(
        fmt::format("size_t greater than int max: {}", i));
  }
}

std::string render(
    const std::string& tmplt,
    const node& root,
    const std::map<std::string, std::string>& partials) {
  std::map<std::string, template_type> partial_templates;
  for (auto& partial : partials) {
    partial_templates.insert({partial.first, {partial.second}});
  }

  render_context ctx{root, partial_templates};
  ctx.render(tmplt);
  return std::move(ctx.out);
}

} // namespace apache::thrift::mstch
