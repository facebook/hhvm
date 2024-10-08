(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type cell =
  | Non_hack of {
      cell_type: string;
      contents: string;
    }
  | Hack of string

type t = cell list

let fold_cells_exn (acc : t) cell_json : t =
  let find_exn = List.Assoc.find_exn ~equal:String.equal in
  let obj = Hh_json.get_object_exn cell_json in
  let source_json = find_exn obj "source" in
  let source_lines_json = Hh_json.get_array_exn source_json in
  let source_lines = List.map source_lines_json ~f:Hh_json.get_string_exn in
  let type_json = find_exn obj "cell_type" in
  let contents = String.concat ~sep:"" source_lines in
  match Hh_json.get_string_exn type_json with
  | "code" when String.is_prefix contents ~prefix:"%%" ->
    Non_hack { cell_type = "unknown"; contents } :: acc
  | "code" -> Hack contents :: acc
  | cell_type -> Non_hack { cell_type; contents } :: acc

let ipynb_of_json (ipynb_json : Hh_json.json) : (t, string) Result.t =
  try
    let find_exn = List.Assoc.find_exn ~equal:String.equal in
    let obj = Hh_json.get_object_exn ipynb_json in
    let cells_json = find_exn obj "cells" in
    let cells_jsons = Hh_json.get_array_exn cells_json in
    Ok (cells_jsons |> List.fold ~init:[] ~f:fold_cells_exn)
  with
  | e -> Error (Exn.to_string e)
