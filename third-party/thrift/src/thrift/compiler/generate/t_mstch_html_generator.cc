/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache::thrift::compiler {

class t_mstch_html_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::optional<whisker_options> use_whisker() const override {
    whisker_options opts;
    opts.allowed_undefined_variables = {};
    return opts;
  }

  std::string template_prefix() const override { return "html"; }

  void generate_program() override {
    out_dir_base_ = "gen-mstch_html";
    // Generate index.html.
    render_to_file(*this->get_program(), "index.html", "index.html");
  }
};

THRIFT_REGISTER_GENERATOR(mstch_html, "HTML", "");

} // namespace apache::thrift::compiler
