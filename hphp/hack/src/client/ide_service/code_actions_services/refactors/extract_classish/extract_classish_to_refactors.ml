(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module T = Extract_classish_types

let placeholder_name = "Placeholder_"

let interface_body_of_methods source_text T.{ selected_methods; _ } : string =
  let open Aast_defs in
  let abstractify_one meth =
    let stmts = meth.m_body.fb_ast in
    let remove_async_modifier : string -> string =
      match meth.m_fun_kind with
      | Ast_defs.FSync
      | Ast_defs.FGenerator ->
        Fn.id
      | Ast_defs.FAsync
      | Ast_defs.FAsyncGenerator ->
        String.substr_replace_first ~pos:0 ~pattern:"async " ~with_:""
    in
    match List.hd stmts with
    | Some (stmt_pos, _) when not (Pos.equal stmt_pos Pos.none) ->
      let body_until_first_statement_length =
        Pos.start_offset stmt_pos - Pos.start_offset meth.m_span
      in
      Full_fidelity_source_text.sub_of_pos
        source_text
        ~length:body_until_first_statement_length
        meth.m_span
      |> String.rstrip ~drop:(fun ch ->
             Char.is_whitespace ch || Char.equal ch '{')
      |> fun x -> x ^ ";" |> remove_async_modifier
    | Some _
    | None ->
      Full_fidelity_source_text.sub_of_pos source_text meth.m_span
      |> String.rstrip ~drop:(fun ch ->
             Char.is_whitespace ch || Char.equal ch '}')
      |> String.rstrip ~drop:(fun ch ->
             Char.is_whitespace ch || Char.equal ch '{')
      |> fun x -> x ^ ";" |> remove_async_modifier
  in
  selected_methods |> List.map ~f:abstractify_one |> String.concat ~sep:"\n"

let format_classish path ~(body : string) : string =
  let classish = Printf.sprintf "interface %s {\n%s\n}" placeholder_name body in
  let prefixed = "<?hh\n" ^ classish in
  let strip_prefix s =
    s
    |> String.split_lines
    |> (fun lines -> List.drop lines 1)
    |> String.concat ~sep:"\n"
  in
  prefixed
  |> Full_fidelity_source_text.make path
  |> PositionedTree.make
  |> Libhackfmt.format_tree
  |> strip_prefix
  |> fun x -> x ^ "\n\n"

(** Create text edit for "interface Placeholder_ { .... }" *)
let extracted_classish_text_edit source_text path candidate :
    Code_action_types.edit =
  let pos_of_extracted =
    Pos.shrink_to_start candidate.T.class_.Aast_defs.c_span
  in

  let body = interface_body_of_methods source_text candidate in
  let text = format_classish path ~body in
  Code_action_types.{ pos = pos_of_extracted; text }

(** Generate text edit like: "extends Placeholder_" *)
let update_implements_text_edit class_ : Code_action_types.edit =
  match List.last class_.Aast.c_implements with
  | Some (pos, _) ->
    let pos = Pos.shrink_to_end pos in
    let text = Printf.sprintf ", %s" placeholder_name in
    Code_action_types.{ pos; text }
  | None ->
    let pos =
      let extends_pos_opt =
        class_.Aast.c_extends |> List.hd |> Option.map ~f:fst
      in
      let c_name_pos = class_.Aast.c_name |> fst in
      extends_pos_opt |> Option.value ~default:c_name_pos |> Pos.shrink_to_end
    in
    let text = Printf.sprintf "\n  implements %s" placeholder_name in
    Code_action_types.{ pos; text }

let edits_of_candidate source_text path candidate : Code_action_types.edits =
  let edits =
    let extracted_edit =
      extracted_classish_text_edit source_text path candidate
    in
    let reference_edit = update_implements_text_edit candidate.T.class_ in
    [reference_edit; extracted_edit]
  in
  Relative_path.Map.singleton path edits

let to_refactor source_text path candidate : Code_action_types.refactor =
  let edits = lazy (edits_of_candidate source_text path candidate) in
  Code_action_types.{ title = "Extract interface"; edits; kind = `Refactor }

let to_refactors (source_text : Full_fidelity_source_text.t) path candidate :
    Code_action_types.refactor list =
  [to_refactor source_text path candidate]
