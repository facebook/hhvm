(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type error_type =
  | ParseError
  | RuntimeError
[@@deriving show, sexp_of]

type syntax_quickfix = {
  title: string;
  edits: (int * int * string) list;
}
[@@deriving show, sexp_of]

type t = {
  child: t option;
  start_offset: int;
  end_offset: int;
  error_type: error_type;
  message: string;
  quickfixes: syntax_quickfix list;
}
[@@deriving show, sexp_of]

exception ParserFatal of t * Pos.t

val make :
  ?child:t option ->
  ?error_type:error_type ->
  ?quickfixes:syntax_quickfix list ->
  int ->
  int ->
  string ->
  t

val to_positioned_string : t -> (int -> int * int) -> string

val compare : t -> t -> int

val exactly_equal : t -> t -> bool

val error_type : t -> error_type

val message : t -> string

val start_offset : t -> int

val end_offset : t -> int

val this_in_static : string
