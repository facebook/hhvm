(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Lexing

type t = {
    pos_file: string ;
    pos_start: Lexing.position ;
    pos_end: Lexing.position ;
  }

let file = ref ""

let none = {
  pos_file = "" ;
  pos_start = dummy_pos ;
  pos_end = dummy_pos ;
}

let filename p = p.pos_file

let make (lb:Lexing.lexbuf) =
 let pos_start = lexeme_start_p lb in
 let pos_end = lexeme_end_p lb in
 { pos_file = !file; pos_start = pos_start ;
   pos_end = pos_end }

let make_from_file() =
  let pos = Lexing.dummy_pos in
  { pos_file = !file;
    pos_start = pos;
    pos_end = pos;
  }

let make_from file =
  let pos = Lexing.dummy_pos in
  { pos_file = file;
    pos_start = pos;
    pos_end = pos;
  }

let btw x1 x2 =
  if x1.pos_file <> x2.pos_file
  then failwith "Position in separate files" ;
  if x1.pos_end > x2.pos_end
  then failwith "Invalid positions Pos.btw" ;
  { x1 with pos_end = x2.pos_end }

let lhs p =
  { p with pos_end = p.pos_start }

let rhs p =
  { p with pos_start = p.pos_end }

let info_pos t =
  let line = t.pos_start.pos_lnum in
  let start = t.pos_start.pos_cnum - t.pos_start.pos_bol + 1 in
  let end_ = start + t.pos_end.pos_cnum - t.pos_start.pos_cnum - 1 in
  line, start, end_

let info_raw t = t.pos_start.pos_cnum, t.pos_end.pos_cnum
let length t = t.pos_end.pos_cnum - t.pos_start.pos_cnum

let string t =
  let line, start, end_ = info_pos t in
  Printf.sprintf "File %S, line %d, characters %d-%d:"
    t.pos_file line start end_

let compare x y =
  String.compare (string x) (string y)
