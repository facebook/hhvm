(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type parser_return = {
    file_mode  : FileInfo.mode option; (* None if PHP *)
    comments   : (Pos.t * string) list;
    ast        : Ast.program;
    content   : string;
  }

val program :
  ?quick:bool ->
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ?keep_errors:bool ->
  ParserOptions.t ->
  Relative_path.t ->
  string -> parser_return

val program_with_default_popt :
  ?elaborate_namespaces:bool ->
  ?include_line_comments:bool ->
  ?keep_errors:bool ->
  Relative_path.t ->
  string -> parser_return

(* Parses a file *)
val from_file_with_default_popt :
  ?quick:bool -> Relative_path.t -> parser_return
val from_file :
  ?quick:bool -> ParserOptions.t -> Relative_path.t -> parser_return
val get_file_mode :
  ParserOptions.t -> Relative_path.t -> string -> FileInfo.mode option

type saved_lb
type assoc
val save_lexbuf_state: Lexing.lexbuf -> saved_lb
val restore_lexbuf_state: Lexing.lexbuf -> saved_lb -> unit
val get_priority: Lexer_hack.token -> assoc * int
