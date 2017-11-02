(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module B = Buffer
open Hh_core

(* Inspired by the instruction sequence, string sequence contains tree like
 * strings of logical order *)

type t =
| String_list of string list
| String_concat of t list

(* Some helper constructors *)
let str x = String_list [x]
let strs x = String_list x
let gather x = String_concat x
let empty = String_list []

let rec string_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    | String_list strl ->
      string_seq_to_list_aux sl (List.rev_append strl result)
    | String_concat sl' -> string_seq_to_list_aux (List.append sl' sl) result

let seq_to_string t = String.concat "" @@ string_seq_to_list_aux [t] []

let add_string_from_seq buf t =
  List.iter ~f:(B.add_string buf) @@ string_seq_to_list_aux [t] []
