(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Nast

exception Eval_error of Pos.t

let unescape_string unescaper s =
  let buf = Buffer.create (String.length s) in
  let i = ref 0 in
  while !i < String.length s do
    begin match s.[!i] with
    | '\\' ->
        i := !i + 1;
        unescaper s i buf
    | c ->
        Buffer.add_char buf c
    end;
    i := !i + 1
  done;
  Buffer.contents buf

(* For single-quoted strings, slashes and single quotes need to be escaped, and
 * all other characters are treated literally -- i.e. '\n' is the literal
 * slash + 'n' *)
let single_quote_unescaper s i buf =
  match s.[!i] with
  | '\\'
  | '\'' as c -> Buffer.add_char buf c
  | c ->
      Buffer.add_char buf '\\';
      Buffer.add_char buf c

let double_quote_unescaper s i buf =
  match s.[!i] with
  | '\\'
  | '\'' as c -> Buffer.add_char buf c
  | 'n' -> Buffer.add_char buf '\n'
  | 'r' -> Buffer.add_char buf '\r'
  | 't' -> Buffer.add_char buf '\t'
  | 'v' -> Buffer.add_char buf '\x0b'
  | 'e' -> Buffer.add_char buf '\x1b'
  | 'f' -> Buffer.add_char buf '\x0c'
  | '$' -> Buffer.add_char buf '$'
  (* should also handle octal / hex escape sequences but I'm lazy *)
  | c ->
      Buffer.add_char buf '\\';
      Buffer.add_char buf c

let rec static_string = function
  | _, Binop (Ast.Dot, s1, s2) ->
      let p1, s1 = static_string s1 in
      let _p2, s2 = static_string s2 in
      p1, s1 ^ s2
  | _, String (p, s) -> p, unescape_string single_quote_unescaper s
  (* This matches double-quoted strings w/o interpolated variables *)
  | p, String2 ([], s) -> p, unescape_string double_quote_unescaper s
  | p, _ -> raise (Eval_error p)
