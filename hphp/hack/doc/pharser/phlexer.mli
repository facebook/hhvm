(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type state

exception Error of string

val new_state :
  strict_lexer:bool ->
  verbose_lexer:bool ->
  case_sensitive:bool ->
  xhp_builtin:bool ->
  facebook_lang_extensions:bool ->
  unit -> state

val token : state -> Lexing.lexbuf -> Phparser.token

val dump_modes : state -> string
