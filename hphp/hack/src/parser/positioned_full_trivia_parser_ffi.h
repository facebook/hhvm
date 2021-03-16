/* Copyright (c) 2021, Facebook, Inc.
 All rights reserved.

 This source code is licensed under the MIT license found in the
 LICENSE file in the "hack" directory of this source tree.
*/
#if !defined(POSITIONED_FULL_TRIVIA_PARSER_FFI_H)
#  define POSITIONED_FULL_TRIVIA_PARSER_FFI_H

#  include "hphp/hack/src/parser/positioned_full_trivia_parser_ffi_types_fwd.h"

#  if defined(__cplusplus)
extern "C" {
#  endif /*defined(__cplusplus)*/
char const* hackc_parse_positioned_full_trivia_cpp_ffi(
    char const* filename
  , char const* source_text
  , hackc_parse_positioned_full_trivia_environment const* env);

void hackc_parse_positioned_full_trivia_free_string_cpp_ffi(char const*);
#  if defined(__cplusplus)
}

#  include <memory>

namespace HPHP {

using hackc_parse_positioned_full_trivia_ptr =
  std::unique_ptr<char const, void(*)(char const*)>;

inline hackc_parse_positioned_full_trivia_ptr
  hackc_parse_positioned_full_trivia(
      char const* filename
    , char const* source_text
    , hackc_parse_positioned_full_trivia_environment const* env
  ) {
  return hackc_parse_positioned_full_trivia_ptr {
      hackc_parse_positioned_full_trivia_cpp_ffi(filename, source_text, env)
    , hackc_parse_positioned_full_trivia_free_string_cpp_ffi
  };
}

}//namepsace HPHP
#  endif /*defined(__cplusplus)*/

#endif/*!defined(POSITIONED_FULL_TRIVIA_PARSER_FFI_H)*/
