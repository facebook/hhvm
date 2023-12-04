(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sexplib.Std

(* TODO: Integrate these with the rest of the Hack error messages. *)

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

let make
    ?(child = None)
    ?(error_type = ParseError)
    ?(quickfixes = [])
    start_offset
    end_offset
    message =
  { child; error_type; start_offset; end_offset; message; quickfixes }

let rec to_positioned_string error offset_to_position =
  let child =
    match error.child with
    | Some child ->
      Printf.sprintf "\n  %s" (to_positioned_string child offset_to_position)
    | _ -> ""
  in
  let (sl, sc) = offset_to_position error.start_offset in
  let (el, ec) = offset_to_position error.end_offset in
  Printf.sprintf "(%d,%d)-(%d,%d) %s%s" sl sc el ec error.message child

let compare err1 err2 =
  if err1.start_offset < err2.start_offset then
    -1
  else if err1.start_offset > err2.start_offset then
    1
  else if err1.end_offset < err2.end_offset then
    -1
  else if err1.end_offset > err2.end_offset then
    1
  else
    0

let exactly_equal err1 err2 =
  err1.start_offset = err2.start_offset
  && err1.end_offset = err2.end_offset
  && err1.message = err2.message

let error_type err = err.error_type

let message err = err.message

let start_offset err = err.start_offset

let end_offset err = err.end_offset

let this_in_static = "Don't use $this in a static method, use static:: instead"
