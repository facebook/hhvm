/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(COMPILE_FFI_H)
#  define COMPILE_FFI_H

#  include<stdint.h>
#  include<stddef.h>

enum env_flags {
    IS_SYSTEMLIB=1 << 0
  , IS_EVALED=1 << 1
  , FOR_DEBUGGER_EVAL=1 << 2
  , DUMP_SYMBOL_REFS=1 << 3
  , DISABLE_TOPLEVEL_ENUMERATION=1 << 4
};

struct environment {
  char const* filepath;
  char const* const* config_jsons;
  size_t num_config_jsons;
  char const* const* config_list;
  size_t num_config_list;
  uint8_t flags;
};

struct output_config {
  bool include_header;
  char const* output_file;
};

struct buf_t {
  char* buf;
  int buf_siz;
};

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/
char const* compile_from_text_cpp_ffi(
       environment const* env
     , char const* source_text
     , output_config const* config
     , buf_t* error_buf );

void compile_from_text_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}
#  endif /*defined(__cplusplus)*/

#endif/*!defined(COMPILE_FFI_H)*/
