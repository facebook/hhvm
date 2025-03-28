(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_positioned_syntax

(** Within Hack chunks, we distinguish statements from things allowed at the top
 * level (classes, functions, etc.) because we will wrap the former in a top-level
 * statement.
 *)
type annotation =
  | Anno_hack_stmt
  | Anno_hack_top_level
  | Anno_non_hack
[@@deriving eq]

type annotated_chunk = annotation * Notebook_chunk.t

(** We calculate the current chunk Id by
* looking at the head of the list of chunks:
* this works because we build up the list backwards
* and then reverse.
*)
let head_id (chunks : annotated_chunk list) : Notebook_chunk.Id.t =
  List.hd chunks
  |> Option.map ~f:(fun (_, Notebook_chunk.{ id; _ }) -> id)
  |> Option.value ~default:Notebook_chunk.Id.zero

(** A chunk has the information we need to generate Hack
* and remember the cell ID (cell ID will be stored in comments).
* We build the list up in reverse to stay O(n) and merge
* chunks where we can to avoid redundant comments.
*)
let chunks_of_hack_exn
    ~(acc : annotated_chunk list) ~cell_bento_metadata id (source : string) :
    annotated_chunk list =
  (* We tolerate some syntax errors in the Hack in notebooks,
     the intent being that users will fix such errors in their editor
     or in the notebook, using standard tooling for feedback on syntax errors.
  *)
  let (syn, _errors) = Notebook_convert_util.parse source in
  match Syn.syntax syn with
  | Syn.(Script { script_declarations }) ->
    List.fold ~init:acc (Syn.children script_declarations) ~f:(fun acc syn ->
        (* We strip to avoid extra newlines when we concatenate chunks of code to create Hack file contents *)
        let contents = String.strip @@ Syn.full_text syn in
        if String.is_empty contents then
          acc
        else
          (* We distinguish statements from things that can go at the top level (decls, comments)
             because we will need to generate Hack with top-level statements concatenated into a function body
             Here's why: top-level statements are not allowed in "real" WWW Hack but are allowed in notebooks
             for user convenience.
          *)
          let anno =
            if Full_fidelity_statement.is_statement (Syn.syntax syn) then
              Anno_hack_stmt
            else
              Anno_hack_top_level
          in
          match acc with
          | ( prev_anno,
              Notebook_chunk.
                {
                  chunk_kind;
                  contents = prev_contents;
                  id = prev_id;
                  cell_bento_metadata = _;
                } )
            :: tl
            when equal_annotation prev_anno anno
                 && Notebook_chunk.Id.compare id prev_id = 0 ->
            (* We merge chunks to avoid redundant comments identifying chunk positions *)
            ( prev_anno,
              Notebook_chunk.
                {
                  chunk_kind;
                  id;
                  contents = Printf.sprintf "%s\n%s" prev_contents contents;
                  cell_bento_metadata;
                } )
            :: tl
          | _ :: _
          | [] ->
            (* create a new chunk *)
            ( anno,
              Notebook_chunk.
                { chunk_kind = Hack; id; contents; cell_bento_metadata } )
            :: acc)
  | _ -> failwith @@ Printf.sprintf "Could not parse Hack %s" source

(** Massage the cells from the ipynb into an internal format more amenable
* to generating Hack. *)
let chunks_of_cells_exn (cells : Ipynb.cell list) : annotated_chunk list =
  cells
  |> List.fold ~init:[] ~f:(fun (chunks : annotated_chunk list) cell ->
         let id = Notebook_chunk.Id.next (head_id chunks) in
         match cell with
         | Ipynb.Hack { contents; cell_bento_metadata } ->
           chunks_of_hack_exn ~acc:chunks ~cell_bento_metadata id contents
         | Ipynb.(Non_hack { cell_type; contents; cell_bento_metadata }) ->
           let chunk =
             ( Anno_non_hack,
               Notebook_chunk.
                 {
                   contents;
                   chunk_kind = Non_hack { cell_type };
                   id;
                   cell_bento_metadata;
                 } )
           in
           chunk :: chunks)
  |> List.rev

(** Concatenate cells. Some considerations:
* - We preserve metadata so we can reconstruct the notebook structure
* - We convert top-level statements into a single top-level function
*)
let hack_of_ipynb_exn
    ~(notebook_number : string) ~(header : string) Ipynb.{ cells; kernelspec } :
    string =
  let (non_stmts, stmts) =
    cells
    |> chunks_of_cells_exn
    |> List.partition_map ~f:(fun (anno, chunk) ->
           let hack = Notebook_chunk.to_hack chunk in
           match anno with
           | Anno_non_hack
           | Anno_hack_top_level ->
             Either.First hack
           | Anno_hack_stmt -> Either.Second hack)
  in
  let non_stmts_code = String.concat non_stmts ~sep:"\n" in
  let main_fn_code =
    stmts
    |> List.bind ~f:(fun block ->
           block |> String.split_lines |> List.map ~f:(Printf.sprintf "    %s"))
    |> String.concat ~sep:"\n"
    |> Printf.sprintf
         "async function %s%s(): Awaitable<void> {\n%s\n}"
         Notebook_convert_constants.main_function_prefix
         notebook_number
  in
  let unformatted =
    let notebook_metadata_comment =
      Notebook_level_metadata.(to_comment { notebook_number; kernelspec })
    in
    Printf.sprintf
      "<?hh\n%s\n%s\n%s\n\n%s\n"
      header
      notebook_metadata_comment
      non_stmts_code
      main_fn_code
  in
  Notebook_convert_util.hackfmt unformatted

let notebook_to_hack
    ~(notebook_number : string) ~(header : string) (ipynb_json : Hh_json.json) :
    (string, string) Result.t =
  try
    ipynb_json
    |> Ipynb.ipynb_of_json
    |> Result.map ~f:(hack_of_ipynb_exn ~notebook_number ~header)
  with
  | e -> Error (Exn.to_string e)
