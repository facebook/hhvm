(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val program :
  ?quick:bool ->
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ?keep_errors:bool ->
  ParserOptions.t ->
  Relative_path.t ->
  string -> Parser_return.t

val program_with_default_popt :
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ?keep_errors:bool ->
  Relative_path.t ->
  string -> Parser_return.t

(* Parses a file *)
val from_file_with_default_popt :
  ?quick:bool -> Relative_path.t -> Parser_return.t
val from_file :
  ?quick:bool -> ParserOptions.t -> Relative_path.t -> Parser_return.t
val get_file_mode :
  ParserOptions.t -> Relative_path.t -> string -> FileInfo.mode option

type saved_lb
type assoc
val save_lexbuf_state: Lexing.lexbuf -> saved_lb
val restore_lexbuf_state: Lexing.lexbuf -> saved_lb -> unit
val get_priority: Lexer_hack.token -> assoc * int
