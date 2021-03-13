/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/

struct hackc_compile_native_environment {
  char const* filepath;
  char const * aliased_namespaces;
  char const * include_roots;
  int32_t emit_class_pointers;
  int32_t check_int_overflow;
  uint32_t hhbc_flags;
  uint32_t parser_flags;
  uint8_t flags;
};

struct hackc_compile_output_config {
  bool include_header;
  char const* output_file;
};

struct hackc_error_buf_t {
  char* buf;
  int buf_siz;
};
