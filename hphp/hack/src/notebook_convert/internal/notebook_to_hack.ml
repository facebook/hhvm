(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_positioned_syntax

(** We calculate the current chunk Id by
* looking at the head of the list of chunks:
* this works because we build up the list backwards
* and then reverse.
*)
let head_id (chunks : Notebook_chunk.t list) : Notebook_chunk.Id.t =
  List.hd chunks
  |> Option.map ~f:(fun Notebook_chunk.{ id; _ } -> id)
  |> Option.value ~default:Notebook_chunk.Id.zero

(** A chunk has the information we need to generate Hack
* and remember the cell ID (cell ID will be stored in comments).
* We build the list up in reverse to stay O(n) and merge
* chunks where we can to avoid redundant comments.
*)
let chunks_of_hack_exn ~(acc : Notebook_chunk.t list) id (source : string) :
    Notebook_chunk.t list =
  let (syn, errors) = Notebook_convert_util.parse source in
  let () =
    if not @@ List.is_empty errors then failwith "Hack input has syntax errors"
  in
  match Syn.syntax syn with
  | Syn.(Script { script_declarations }) ->
    List.fold ~init:acc (Syn.children script_declarations) ~f:(fun acc syn ->
        let contents = Syn.text syn in
        if String.(is_empty @@ strip contents) then
          acc
        else
          (* We distinguish statements from things that can go at the top level (decls, comments)
             because we will need to generate Hack with top-level statements concatenated into a function body
             Here's why: top-level statements are not allowed in "real" WWW Hack but are allowed in notebooks
             for user convenience.
          *)
          let kind =
            if Full_fidelity_statement.is_statement (Syn.syntax syn) then
              Notebook_chunk.Stmt
            else
              Notebook_chunk.Top_level
          in
          match acc with
          | Notebook_chunk.{ chunk_kind; contents = prev_contents; _ } :: tl
            when Notebook_chunk.equal_chunk_kind chunk_kind kind ->
            (* We merge chunks to avoid redundant comments identifying chunk positions *)
            Notebook_chunk.
              {
                chunk_kind;
                id;
                contents = Printf.sprintf "%s\n%s" prev_contents contents;
              }
            :: tl
          | _ :: _
          | [] ->
            (* create a new chunk *)
            Notebook_chunk.{ chunk_kind = kind; id; contents } :: acc)
  | _ -> failwith @@ Printf.sprintf "Could not parse Hack %s" source

(** Massage the cells from the ipynb into an internal format more amenable
* to generating Hack. *)
let chunks_of_cells_exn (cells : Ipynb.cell list) : Notebook_chunk.t list =
  cells
  |> List.fold ~init:[] ~f:(fun chunks cell ->
         let id = Notebook_chunk.Id.next (head_id chunks) in
         match cell with
         | Ipynb.Hack source -> chunks_of_hack_exn ~acc:chunks id source
         | Ipynb.(Non_hack { cell_type; contents : string }) ->
           let chunk =
             Notebook_chunk.
               { contents; chunk_kind = Non_hack { cell_type }; id }
           in
           chunk :: chunks)
  |> List.rev

(** Concatenate cells. Some considerations:
* - We preserve metadata so we can reconstruct the notebook structure
* - We convert top-level statements into a single top-level function
*)
let hack_of_cells_exn ~(notebook_name : string) (cells : Ipynb.cell list) :
    string =
  let (non_stmts, stmts) =
    cells
    |> chunks_of_cells_exn
    |> List.partition_map ~f:(fun chunk ->
           let hack = Notebook_chunk.to_hack chunk in
           match chunk.Notebook_chunk.chunk_kind with
           | Notebook_chunk.Non_hack _
           | Notebook_chunk.Top_level ->
             Either.First hack
           | Notebook_chunk.Stmt -> Either.Second hack)
  in
  let non_stmts_code = String.concat non_stmts ~sep:"\n" in
  let main_fn_code =
    stmts
    |> List.bind ~f:(fun block ->
           block |> String.split_lines |> List.map ~f:(Printf.sprintf "    %s"))
    |> String.concat ~sep:"\n"
    |> Printf.sprintf "function notebook_main_%s(): void {\n%s\n}" notebook_name
  in
  let unformatted =
    Printf.sprintf "<?hh\n\n%s\n\n%s\n" non_stmts_code main_fn_code
  in
  Notebook_convert_util.hackfmt unformatted

let notebook_to_hack ~(notebook_name : string) (ipynb_json : Hh_json.json) :
    (string, string) Result.t =
  try
    ipynb_json
    |> Ipynb.ipynb_of_json
    |> Result.map ~f:(hack_of_cells_exn ~notebook_name)
  with
  | e -> Error (Exn.to_string e)
