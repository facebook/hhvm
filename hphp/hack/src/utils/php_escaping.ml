(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Implementation of string escaping stuff. Ugggggggh.
 * See http://php.net/manual/en/language.types.string.php *)

open Hh_prelude

let is_printable c =
  let open Char in
  c >= ' ' && c <= '~'

let is_lit_printable c =
  let open Char in
  is_printable c && c <> '\\' && c <> '\"'

let escape_char = function
  | '\n' -> "\\n"
  | '\r' -> "\\r"
  | '\t' -> "\\t"
  | '\\' -> "\\\\"
  | '"' -> "\\\""
  | '$' -> "$"
  | c when is_lit_printable c -> String.make 1 c
  | c -> Printf.sprintf "\\%03o" (Char.to_int c)

let escape ?(f = escape_char) s =
  let buf = Buffer.create (String.length s) in
  Stdlib.String.iter (fun c -> Buffer.add_string buf @@ f c) s;
  Buffer.contents buf
