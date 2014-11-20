(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let r_param = Str.regexp "@param \\([^ \n]+\\) \\(\\$[^ \n]+\\)"
let r_return = Str.regexp "@return \\([^ \n]+\\)"

let ret_key = ""

let parse s =
  (* Ugh, the regexp APIs are so obnoxiously imperative. *)
  let i = ref 0 in
  let m = ref Smap.empty in
  begin try while true do
    ignore (Str.search_forward r_param s !i);
    i := Str.match_end ();
    let ty_str = Str.matched_group 1 s in
    let var = Str.matched_group 2 s in
    let ty = At_parse.parse ty_str in
    m := Smap.add var ty !m
  done with Not_found -> () end;
  begin try
    ignore (Str.search_forward r_return s 0);
    let ty_str = Str.matched_group 1 s in
    let ty = At_parse.parse ty_str in
    m := Smap.add ret_key ty !m
  with Not_found -> () end;
  !m
