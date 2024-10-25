(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module Syn = Full_fidelity_positioned_syntax

let split_by_chunk_comments
    (hack : string) ~(is_from_toplevel_statements : bool) :
    Notebook_chunk.t list =
  let append_to_previous (item : 'a) : 'a list list -> 'a list list = function
    | h :: t -> (item :: h) :: t
    | [] -> [[]]
    (* ignore anything before the first chunk start comment.
       In practice this is the <?hh header *)
  in
  hack
  |> String.split_lines
  |> List.fold ~init:[] ~f:(fun acc line ->
         if Notebook_chunk.is_chunk_start_comment line then
           [line] :: acc
         else
           append_to_previous line acc)
  |> List.rev
  |> List.map ~f:List.rev
  |> List.filter_map ~f:(function
         | [] -> None
         | chunk_start_comment :: hack_lines_rev ->
           hack_lines_rev
           |> String.concat ~sep:"\n"
           |> Notebook_chunk.of_hack_exn
                ~comment:chunk_start_comment
                ~is_from_toplevel_statements
           |> Option.some)

let format_chunk (chunk : Notebook_chunk.t) : Notebook_chunk.t =
  let contents =
    (* Hackfmt refuses to format code unless it starts with "<?hh" *)
    let prefix = "<?hh\n" in
    let unformatted =
      let raw_contents = chunk.Notebook_chunk.contents in
      if String.is_prefix ~prefix raw_contents then
        raw_contents
      else
        prefix ^ raw_contents
    in
    unformatted
    |> Notebook_convert_util.hackfmt
    (*
    * We strip the leading <?hh regardless of whether we added it
    * or the user wrote it.
    * While <?hh is technically allowed in the first notebook cell,
    * it's unidiomatic.
    *)
    |> String.chop_prefix_if_exists ~prefix
  in
  Notebook_chunk.{ chunk with contents }

let hack_to_notebook_exn (syntax : Full_fidelity_positioned_syntax.t) :
    Hh_json.json =
  let script_children =
    match syntax with
    | Syn.{ syntax = Script { script_declarations }; _ } ->
      Syn.children script_declarations
    | _ ->
      failwith
        "Internal error: expected Hack script. This should be unreachable since we validate inputs are well-formed Hack."
  in
  let decls =
    match script_children with
    | Syn.{ syntax = MarkupSection _; _ } :: decls -> decls
    | _ -> failwith "Expected <?hh at top of file"
  in
  let (main_function_bodies, other) =
    List.partition_map decls ~f:(function
        | Syn.
            {
              syntax =
                FunctionDeclaration
                  {
                    function_declaration_header =
                      {
                        syntax = FunctionDeclarationHeader { function_name; _ };
                        _;
                      };
                    function_body =
                      {
                        syntax = CompoundStatement { compound_statements; _ };
                        _;
                      };
                    _;
                  };
              _;
            }
          when String.is_prefix ~prefix:"notebook_main"
               @@ String.strip
               @@ Syn.text function_name ->
          Either.First compound_statements
        | other -> Either.Second other)
  in
  let top_level_statements_code =
    begin
      match main_function_bodies with
      | body_parts :: [] ->
        body_parts
        |> Syn.children
        |> List.map ~f:Syn.full_text
        |> String.concat ~sep:"\n"
      | _ -> failwith "Must be exactly one function with prefix notebook_main"
    end
  in
  let other_code =
    other |> List.map ~f:Syn.full_text |> String.concat ~sep:"\n"
  in
  let chunks : Notebook_chunk.t list =
    let top_level_statements_chunks =
      split_by_chunk_comments
        top_level_statements_code
        ~is_from_toplevel_statements:true
    in
    let other_chunks =
      split_by_chunk_comments other_code ~is_from_toplevel_statements:false
    in
    other_chunks @ top_level_statements_chunks
  in
  chunks
  |> List.map ~f:format_chunk
  |> Ipynb.ipynb_of_chunks_exn
  |> Ipynb.ipynb_to_json

let hack_to_notebook (syntax : Full_fidelity_positioned_syntax.t) :
    (Hh_json.json, string) result =
  try Ok (hack_to_notebook_exn syntax) with
  | e -> Error (Exn.to_string e)
