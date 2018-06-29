(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

type parsed_query = {
  function_params : string list;
  function_output : string;
}

let re_function_query = Str.regexp "[ \t]*function[ \t]*(\\([^)]*\\))[ \t]*:\\([^)]*\\)"

(* Match query to a format of function(type,type,_):type *)
let parse_query (query : string) : parsed_query option =
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
     in
     Some {function_params; function_output}
  with
  | Not_found -> None
