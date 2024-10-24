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
[@@deriving show]

type t = cell list

let ipynb_of_json (ipynb_json : Hh_json.json) : (t, string) Result.t =
  let fold_cells_json_exn (acc : t) cell_json : t =
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
  in
  try
    let find_exn = List.Assoc.find_exn ~equal:String.equal in
    let obj = Hh_json.get_object_exn ipynb_json in
    let cells_json = find_exn obj "cells" in
    let cells_jsons = Hh_json.get_array_exn cells_json in
    Ok (cells_jsons |> List.fold ~init:[] ~f:fold_cells_json_exn)
  with
  | e -> Error (Exn.to_string e)

let make_ipynb_cell ~(cell_type : string) ~(source : string list) =
  Hh_json.(
    JSON_Object
      [
        ("cell_type", JSON_String cell_type);
        ("source", JSON_Array (List.map source ~f:Hh_json.string_));
        ("outputs", JSON_Array []);
        ("execution_count", JSON_Null);
        ("metadata", JSON_Object []);
      ])

let ipynb_to_json (cells : t) : Hh_json.json =
  let json_cells =
    List.map cells ~f:(function
        | Non_hack { cell_type; contents } ->
          make_ipynb_cell ~cell_type ~source:(String.split_lines contents)
        | Hack hack ->
          make_ipynb_cell ~cell_type:"code" ~source:(String.split_lines hack))
  in
  Hh_json.(
    JSON_Object
      [
        ("cells", JSON_Array json_cells);
        ( "metadata",
          JSON_Object
            [
              ( "kernelspec",
                JSON_Object
                  [
                    ("display_name", JSON_String "hack");
                    ("language", JSON_String "hack");
                    ("name", JSON_String "bento_kernel_hack");
                  ] );
              ("orig_nbformat", JSON_Number "4");
            ] );
      ])

let cell_of_chunk Notebook_chunk.{ id = _; chunk_kind; contents } : cell =
  match chunk_kind with
  | Notebook_chunk.(Top_level | Stmt) -> Hack contents
  | Notebook_chunk.(Non_hack { cell_type }) -> Non_hack { cell_type; contents }

(** Partial function: can only combine cells with the same cell type *)
let combine_cells_exn (cell1 : cell) (cell2 : cell) : cell =
  match (cell1, cell2) with
  | (Hack contents1, Hack contents2) -> Hack (contents1 ^ contents2)
  | ( Non_hack { cell_type = cell_type1; contents = contents1 },
      Non_hack { cell_type = cell_type2; contents = contents2 } )
    when String.equal cell_type1 cell_type2 ->
    Non_hack { cell_type = cell_type1; contents = contents1 ^ contents2 }
  | _ ->
    failwith
      (Printf.sprintf
         "Should be unreachable. attempted to combine cells with incompatible cell types. Got cell1: %s and cell2: %s"
         (show_cell cell1)
         (show_cell cell2))

let ipynb_of_chunks_exn (chunks : Notebook_chunk.t list) : t =
  let chunks_grouped_by_id =
    List.sort_and_group
      chunks
      ~compare:(fun
                 Notebook_chunk.{ id = id1; _ }
                 Notebook_chunk.{ id = id2; _ }
               -> Notebook_chunk.Id.compare id1 id2)
  in
  List.map chunks_grouped_by_id ~f:(fun chunks ->
      chunks
      |> List.map ~f:cell_of_chunk
      (* reduce_exn is safe because groups created by sort_and_group are non-empty *)
      |> List.reduce_exn ~f:combine_cells_exn)
