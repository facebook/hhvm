(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SUtils = SearchUtils

let print_results (results : SearchUtils.result) : unit =
  List.iter results ~f:(fun res ->
      let pos_string = Pos.string res.SUtils.pos in
      let desc_string = SearchUtils.kind_to_string res.SUtils.result_type in
      print_endline
        (pos_string ^ " " ^ Utils.strip_ns res.SUtils.name ^ ", " ^ desc_string))

let print_results_json (results : SearchUtils.result) : unit =
  let results =
    Hh_json.JSON_Array (List.map results ~f:ServerSearch.result_to_json)
  in
  print_endline (Hh_json.json_to_string results)

let go (results : SearchUtils.result) (output_json : bool) : unit =
  if output_json then
    print_results_json results
  else
    print_results results
