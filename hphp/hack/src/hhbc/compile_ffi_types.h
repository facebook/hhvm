/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/

#pragma once

#include <cstdint>

namespace HPHP { namespace hackc { namespace compile {

struct native_environment {
  char const* filepath;
  char const * aliased_namespaces;
  char const * include_roots;
  std::int32_t emit_class_pointers;
  std::int32_t check_int_overflow;
  std::uint32_t hhbc_flags;
  std::uint32_t parser_flags;
  std::uint8_t flags;
};

struct output_config {
  bool include_header;
  char const* output_file;
};

struct error_buf_t {
  char* buf;
  int buf_siz;
};

}}} //namespace HPHP::hackc::compile
