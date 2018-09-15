(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type type_specifier =
  | TSsimple of string
  | TSoption of type_specifier

type query_type =
  | QTtype of type_specifier
  | QTwildcard

type signature_query = {
  function_params : query_type list;
  function_output : query_type;
}

let re_function_query = Str.regexp "[ \t]*function[ \t]*(\\([^)]*\\))[ \t]*:\\([^)]*\\)"

let re_indexable_type = Str.regexp {|\??\\?[ \t]*[_a-zA-z][\_a-zA-Z0-9]*|}

let parse_query_type type_str =
  if not (Str.string_match re_indexable_type type_str 0) then None
  else if type_str = "_" then Some QTwildcard
  else if type_str.[0] = '?' then
    let type_str = Str.string_after type_str 1 in
    let type_str = String.trim type_str in
    Some (QTtype (TSoption (TSsimple type_str)))
  else Some (QTtype (TSsimple type_str))

(* Match query to a format of function(type,type,_):type *)
let parse_query (query : string) : signature_query option =
  try
    let _ : int = Str.search_forward re_function_query query 0 in
    let function_output = String.trim (Str.matched_group 2 query) in
    let function_params =
      Str.matched_group 1 query
      |> Str.split_delim (Str.regexp ",")
      |> List.filter_map ~f:(fun input ->
        match String.trim input with
        | "" -> None
        | trimmed_input -> Some trimmed_input
      )
      |> List.map ~f:parse_query_type
      |> Option.all
    in
    let function_output = parse_query_type function_output in
    match function_params, function_output with
    | Some function_params, Some function_output ->
      Some {function_params; function_output}
    | _ -> None
  with
  | Not_found -> None
