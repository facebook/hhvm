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
      cell_bento_metadata: Hh_json.json option;
    }
  | Hack of {
      contents: string;
      cell_bento_metadata: Hh_json.json option;
    }
[@@deriving show]

type t = {
  cells: cell list;
  kernelspec: Hh_json.json;
}

let ipynb_of_json (ipynb_json : Hh_json.json) : (t, string) Result.t =
  let cell_of_json_exn (cell_json : Hh_json.json) : cell =
    let find_exn = List.Assoc.find_exn ~equal:String.equal in
    let obj = Hh_json.get_object_exn cell_json in
    let source_json = find_exn obj "source" in
    let source_lines_json = Hh_json.get_array_exn source_json in
    let source_lines = List.map source_lines_json ~f:Hh_json.get_string_exn in
    let type_json = find_exn obj "cell_type" in
    let contents = String.concat ~sep:"\n" source_lines in
    let cell_bento_metadata =
      List.Assoc.find obj "metadata" ~equal:String.equal
    in
    match Hh_json.get_string_exn type_json with
    | "code" when String.is_prefix contents ~prefix:"%%" ->
      Non_hack { cell_type = "unknown"; contents; cell_bento_metadata }
    | "code" -> Hack { contents; cell_bento_metadata }
    | cell_type -> Non_hack { cell_type; contents; cell_bento_metadata }
  in
  try
    let find_exn = List.Assoc.find_exn ~equal:String.equal in
    let obj = Hh_json.get_object_exn ipynb_json in
    let cells_json = find_exn obj "cells" in
    let metadata = Hh_json.get_object_exn @@ find_exn obj "metadata" in
    let kernelspec = find_exn metadata "kernelspec" in
    let cells_jsons = Hh_json.get_array_exn cells_json in
    let cells = cells_jsons |> List.map ~f:cell_of_json_exn in
    Ok { cells; kernelspec }
  with
  | e -> Error (Exn.to_string e)

let make_ipynb_cell
    ~(cell_type : string)
    ~(source : string list)
    ~(cell_bento_metadata : Hh_json.json option) =
  let source =
    source
    |> List.map ~f:(fun s ->
           if String.is_empty s then
             (* We mimic VSCode's behavior of normalizing "" to "\n".
              * Why: A notebook validator rejects notebooks that have empty lines
              * in the `source` array.
              *)
             "\n"
           else
             s)
    |> List.map ~f:Hh_json.string_
  in
  Hh_json.(
    JSON_Object
      [
        ("cell_type", JSON_String cell_type);
        ("source", JSON_Array source);
        ("outputs", JSON_Array []);
        ("execution_count", JSON_Null);
        ( "metadata",
          match cell_bento_metadata with
          | Some cell_bento_metadata -> cell_bento_metadata
          | None -> JSON_Object [] );
      ])

let ipynb_to_json ({ cells; kernelspec } : t) : Hh_json.json =
  let json_cells =
    List.map cells ~f:(function
        | Non_hack { cell_type; contents; cell_bento_metadata } ->
          make_ipynb_cell
            ~cell_type
            ~source:(String.split_lines contents)
            ~cell_bento_metadata
        | Hack { contents; cell_bento_metadata } ->
          make_ipynb_cell
            ~cell_type:"code"
            ~source:(String.split_lines contents)
            ~cell_bento_metadata)
  in
  Hh_json.(
    let nb_format = JSON_Number "4" in
    JSON_Object
      [
        ("cells", JSON_Array json_cells);
        ( "metadata",
          JSON_Object [("kernelspec", kernelspec); ("orig_nbformat", nb_format)]
        );
        ("nbformat", nb_format);
      ])

let cell_of_chunk
    Notebook_chunk.{ id = _; chunk_kind; contents; cell_bento_metadata } : cell
    =
  match chunk_kind with
  | Notebook_chunk.(Top_level | Stmt) -> Hack { contents; cell_bento_metadata }
  | Notebook_chunk.(Non_hack { cell_type }) ->
    Non_hack { cell_type; contents; cell_bento_metadata }

let combine_bento_cell_metadata_exn
    (md1 : Hh_json.json option) (md2 : Hh_json.json option) :
    Hh_json.json option =
  let json_equal a b =
    String.equal
      (Hh_json.json_to_string ~sort_keys:true a)
      (Hh_json.json_to_string ~sort_keys:true b)
  in
  match (md1, md2) with
  | (Some md1, Some md2) when not @@ json_equal md1 md2 ->
    failwith
      "Inconsistent cell metadata: two cells with the same id had different metadata"
  | _ -> Option.first_some md1 md2

(** Partial function: can only combine cells with the same cell type *)
let combine_cells_exn (cell1 : cell) (cell2 : cell) : cell =
  match (cell1, cell2) with
  | ( Hack { contents = contents1; cell_bento_metadata = cell_bento_metadata1 },
      Hack { contents = contents2; cell_bento_metadata = cell_bento_metadata2 }
    ) ->
    (* A Hack cell could be split between declarations and top-level statements.
       The metadata will likely be equal anyway.
    *)
    let cell_bento_metadata =
      combine_bento_cell_metadata_exn cell_bento_metadata1 cell_bento_metadata2
    in
    Hack { contents = contents1 ^ contents2; cell_bento_metadata }
  | ( Non_hack
        {
          cell_type = cell_type1;
          contents = contents1;
          cell_bento_metadata = cell_bento_metadata1;
        },
      Non_hack
        {
          cell_type = cell_type2;
          contents = contents2;
          cell_bento_metadata = cell_bento_metadata2;
        } )
    when String.equal cell_type1 cell_type2 ->
    let cell_bento_metadata =
      combine_bento_cell_metadata_exn cell_bento_metadata1 cell_bento_metadata2
    in
    Non_hack
      {
        cell_type = cell_type1;
        contents = contents1 ^ contents2;
        cell_bento_metadata;
      }
  | _ ->
    failwith
      (Printf.sprintf
         "Should be unreachable. attempted to combine cells with incompatible cell types. Got cell1: %s and cell2: %s"
         (show_cell cell1)
         (show_cell cell2))

let ipynb_of_chunks
    (chunks : Notebook_chunk.t list) ~(kernelspec : Hh_json.json) :
    (t, Notebook_convert_error.t) result =
  try
    let chunks_grouped_by_id =
      List.sort_and_group
        chunks
        ~compare:(fun
                   Notebook_chunk.{ id = id1; _ }
                   Notebook_chunk.{ id = id2; _ }
                 -> Notebook_chunk.Id.compare id1 id2)
    in
    let cells =
      List.map chunks_grouped_by_id ~f:(fun chunks ->
          chunks
          |> List.map ~f:cell_of_chunk
          (* reduce_exn is safe because groups created by sort_and_group are non-empty *)
          |> List.reduce_exn ~f:combine_cells_exn)
    in
    Ok { cells; kernelspec }
  with
  | e ->
    (* blame the user: failure during parsing is almost certainly their fault,
       such as corrupted //@bento-cell:$JSON_HERE
    *)
    Error (Notebook_convert_error.Invalid_input (Exn.to_string e))
