(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_positioned_syntax
module Tree = Full_fidelity_syntax_tree.WithSyntax (Syn)

module Id : sig
  type t

  val zero : t

  val next : t -> t

  val to_int : t -> int
end = struct
  type t = int

  let zero = 0

  let next = ( + ) 1

  let to_int = Fn.id
end

let parse code : Syn.t =
  let env = Full_fidelity_parser_env.make () in
  let source_text = Full_fidelity_source_text.make Relative_path.default code in
  let positioned_syntax_tree = Tree.make ~env source_text in
  Tree.root positioned_syntax_tree

type chunk_kind =
  | Top_level
  | Stmt
  | Non_hack of { cell_type: string }
[@@deriving eq]

(** A chunk has the information we need to generate Hack
* and remember the cell ID.
*)
type chunk = {
  chunk_kind: chunk_kind;
  id: Id.t;
  contents: string;
}

(** We calculate the current chunk Id by
* looking at the head of the list of chunks:
* this works because we build up the list backwards
* and then reverse.
*)
let head_id (chunks : chunk list) : Id.t =
  List.hd chunks
  |> Option.map ~f:(fun { id; _ } -> id)
  |> Option.value ~default:Id.zero

(** A chunk has the information we need to generate Hack
* and remember the cell ID (cell ID will be stored in comments).
* We build the list up in reverse to stay O(n) and merge
* chunks where we can to avoid redundant comments.
*)
let chunks_of_hack ~(acc : chunk list) id (source : string) : chunk list =
  let syn = parse source in
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
              Stmt
            else
              Top_level
          in
          match acc with
          | { chunk_kind; contents = prev_contents; _ } :: tl
            when equal_chunk_kind chunk_kind kind ->
            (* We merge chunks to avoid redundant comments identifying chunk positions *)
            {
              chunk_kind;
              id;
              contents = Printf.sprintf "%s\n%s" prev_contents contents;
            }
            :: tl
          | _ :: _
          | [] ->
            (* create a new chunk *)
            { chunk_kind = kind; id; contents } :: acc)
  | _ -> failwith @@ Printf.sprintf "Could not parse Hack %s" source

(** Massage the cells from the ipynb into an internal format more amenable
* to generating Hack. *)
let chunks_of_cells (cells : Ipynb.cell list) : chunk list =
  cells
  |> List.fold ~init:[] ~f:(fun chunks cell ->
         let id = Id.next (head_id chunks) in
         match cell with
         | Ipynb.Hack source -> chunks_of_hack ~acc:chunks id source
         | Ipynb.(Non_hack { cell_type; contents : string }) ->
           let chunk = { contents; chunk_kind = Non_hack { cell_type }; id } in
           chunk :: chunks)
  |> List.rev

(** Add a magic comment that preserves information about cells. This will enable us to
reconstruct the notebook from the generated Hack (with some loss of position information).
*)
let string_of_hack_chunk id contents =
  Printf.sprintf
    "//@bento-cell:{\"id\": %d, \"cell_type\": \"code\"}\n%s\n"
    (Id.to_int id)
    contents

(** Concatenate cells. Some considerations:
* - We preserve metadata so we can reconstruct the notebook structure
* - We convert top-level statements into a single top-level function
*)
let hack_of_cells ~(notebook_name : string) (cells : Ipynb.cell list) : string =
  let (non_stmts, stmts) =
    cells
    |> chunks_of_cells
    |> List.partition_map ~f:(function
           | { chunk_kind = Non_hack { cell_type }; id; contents } ->
             Either.First
               (Printf.sprintf
                  "//@bento-cell:{\"id\": %d, \"cell_type\": \"%s\"}\n/*\n%s\n*/\n"
                  (Id.to_int id)
                  cell_type
                  contents)
           | { chunk_kind = Top_level; id; contents } ->
             Either.First (string_of_hack_chunk id contents)
           | { chunk_kind = Stmt; id; contents } ->
             Either.Second (string_of_hack_chunk id contents))
  in
  let non_stmts_code = String.concat non_stmts ~sep:"\n" in
  let main_fn_code =
    stmts
    |> List.bind ~f:(fun block ->
           block |> String.split_lines |> List.map ~f:(Printf.sprintf "    %s"))
    |> String.concat ~sep:"\n"
    |> Printf.sprintf "function notebook_main_%s(): void {\n%s\n}" notebook_name
  in
  Printf.sprintf "<?hh\n\n%s\n\n%s\n" non_stmts_code main_fn_code

let notebook_to_hack ~(notebook_name : string) (ipynb_json : Hh_json.json) :
    (string, string) Result.t =
  ipynb_json
  |> Ipynb.ipynb_of_json
  |> Result.map ~f:(hack_of_cells ~notebook_name)
