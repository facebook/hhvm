(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections
module Unix = Caml_unix

(** HACK: Raised when we encounter a construct in a type declaration which we
    have chosen not to handle (because it occurs in a type declaration which we
    do not need to convert at this time). *)
exception Skip_type_decl of string

let log_indent = ref 0

let log fmt =
  ksprintf (Printf.eprintf "%s%s\n%!" (String.make !log_indent ' ')) fmt

let with_log_indent f =
  let old_indent = !log_indent in
  try
    log_indent := old_indent + 2;
    let result = f () in
    log_indent := old_indent;
    result
  with exn ->
    log_indent := old_indent;
    raise exn

let with_tempfile f =
  Random.self_init ();
  let name = "hh_oxidize_" ^ string_of_int (Random.int Int.max_value) in
  let temp_file = Filename.concat "/tmp" name in
  try
    f temp_file;
    Unix.unlink temp_file
  with exn ->
    (try Unix.unlink temp_file with _ -> ());
    raise exn

let rust_keywords =
  SSet.of_list
    [
      "as";
      "break";
      "const";
      "continue";
      "crate";
      "else";
      "enum";
      "extern";
      "false";
      "fn";
      "for";
      "if";
      "impl";
      "in";
      "let";
      "loop";
      "match";
      "mod";
      "move";
      "mut";
      "pub";
      "ref";
      "return";
      "self";
      "Self";
      "static";
      "struct";
      "super";
      "trait";
      "true";
      "type";
      "unsafe";
      "use";
      "where";
      "while";
      "dyn";
      "abstract";
      "become";
      "box";
      "do";
      "final";
      "macro";
      "override";
      "priv";
      "typeof";
      "unsized";
      "virtual";
      "yield";
      "async";
      "await";
      "try";
      "union";
    ]

let map_and_concat ?sep l ~f = List.map l ~f |> String.concat ?sep

let common_prefix a b =
  let len = min (String.length a) (String.length b) in
  let rec aux i =
    if i = len || not (Char.equal a.[i] b.[i]) then
      String.sub a 0 i
    else
      aux (i + 1)
  in
  aux 0

let common_prefix_of_list strs =
  match strs with
  | []
  | [_] ->
    ""
  | _ -> List.reduce_exn strs ~f:common_prefix

let split_on_uppercase str =
  let len = String.length str in
  let rec loop acc last_pos pos =
    if pos = -1 then
      String.sub str ~pos:0 ~len:last_pos :: acc
    else if Char.is_uppercase str.[pos] then
      let sub_str = String.sub str ~pos ~len:(last_pos - pos) in
      loop (sub_str :: acc) pos (pos - 1)
    else
      loop acc last_pos (pos - 1)
  in
  loop [] len (len - 1)

let add_trailing_underscore original_name name =
  if
    Char.equal original_name.[String.length original_name - 1] '_'
    || SSet.mem rust_keywords name
  then
    name ^ "_"
  else
    name

let convert_type_name name =
  name
  |> String.split ~on:'_'
  |> map_and_concat ~f:String.capitalize
  |> add_trailing_underscore name

let convert_module_name name =
  name
  |> String.uncapitalize
  |> split_on_uppercase
  |> map_and_concat ~f:String.uncapitalize ~sep:"_"
  |> add_trailing_underscore name

let convert_field_name name = add_trailing_underscore name name
