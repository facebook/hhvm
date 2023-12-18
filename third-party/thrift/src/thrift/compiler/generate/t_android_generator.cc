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

#include <thrift/compiler/generate/t_java_deprecated_generator.h>

#include <string>
#include <vector>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

/**
 * Java code generator for Android.
 */
class t_android_generator : public t_java_deprecated_generator {
 public:
  using t_java_deprecated_generator::t_java_deprecated_generator;

  void init_generator() override;

  bool has_bit_vector(const t_structured*) override { return false; }

  bool is_comparable(const t_type*, std::vector<const t_type*>*) override {
    return false;
  }
};

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 */
void t_android_generator::init_generator() {
  generate_field_metadata_ = false;
  generate_immutable_structs_ = true;
  generate_boxed_primitive = true;
  generate_builder = false;

  out_dir_base_ = "gen-android";

  // Make output directory.
  boost::filesystem::create_directory(get_out_dir());
  namespace_key_ = "android";
  package_name_ = program_->get_namespace(namespace_key_);

  std::string dir = package_name_;
  std::string subdir = get_out_dir();
  std::string::size_type loc;
  while ((loc = dir.find('.')) != std::string::npos) {
    subdir = subdir + "/" + dir.substr(0, loc);
    boost::filesystem::create_directory(subdir);
    dir = dir.substr(loc + 1);
  }
  if (dir.size() > 0) {
    subdir = subdir + "/" + dir;
    boost::filesystem::create_directory(subdir);
  }

  package_dir_ = subdir;
}

THRIFT_REGISTER_GENERATOR(android, "Android Java", "");

} // namespace
} // namespace compiler
} // namespace thrift
} // namespace apache
