(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module T = Inline_method_types

(** We convert a `return` in the inlined method to a variable assignment.
 *  This is the name for the variable, modulo renaming for hygiene *)
let return_var_raw_name = "$res"

let pos_of_block block =
  let open Option.Let_syntax in
  let* (hd_pos, _) = List.hd block in
  let* (last_pos, _) = List.last block in
  let pos = Pos.merge hd_pos last_pos in
  Option.some_if (Pos.length pos > 0) pos

let to_lsp_range pos =
  Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal pos

(** Pair arguments and parameters *)
let calc_pre_assignments_text ~source_text ~call_arg_positions ~param_names :
    string =
  List.Monad_infix.(
    call_arg_positions
    >>| Full_fidelity_source_text.sub_of_pos source_text
    |> List.zip_exn param_names
    >>| Tuple2.uncurry @@ Format.sprintf "%s = %s;"
    |> String.concat ~sep:" ")

let calc_body_text r path ~source_text T.{ callee; _ } :
    Inline_method_rename.t * string =
  let block_source_text =
    Option.(
      pos_of_block callee.T.block
      >>| Full_fidelity_source_text.sub_of_pos source_text
      |> value ~default:"")
  in
  Inline_method_rewrite_block.rewrite_block
    r
    path
    block_source_text
    ~return_var_raw_name

let strip_leading_spaces ~source_text pos : Pos.t =
  let strip_length =
    pos
    |> Pos.set_col_start 0
    |> Full_fidelity_source_text.sub_of_pos source_text
    |> String.take_while ~f:(Char.equal ' ')
    |> String.length
  in
  let col_start = snd @@ Pos.line_column pos in
  pos |> Pos.set_col_start (col_start - strip_length)

(** Add the inlined method contents before the call site and
 * calculate a variable name for the `return` of the inlined method.
 * We use the return variable when replacing the call site.
 * *)
let edit_inline_and_return_var_of_candidate
    path ~source_text (T.{ caller; callee; call } as candidate) :
    Lsp.TextEdit.t * string =
  let used_vars = String.Set.of_list caller.T.var_names in
  let r = Inline_method_rename.create ~used_vars in
  let (r, param_names) =
    Inline_method_rename.rename_all r callee.T.param_names
  in
  let assignments_before_body =
    calc_pre_assignments_text
      ~source_text
      ~param_names
      ~call_arg_positions:call.T.call_arg_positions
  in
  let (r, body) = calc_body_text r path ~source_text candidate in
  (* Gives the correct `return_var` because `Inline_method_rename.rename` is idempotent *)
  let (_r, return_var) = Inline_method_rename.rename r return_var_raw_name in
  let range =
    let pre_call_pos = Pos.shrink_to_start call.T.call_stmt_pos in
    to_lsp_range pre_call_pos
  in
  let start_indent_amount = snd @@ Pos.line_column call.T.call_stmt_pos in
  let text =
    assignments_before_body ^ body
    |> Inline_method_rewrite_block.format_block ~start_indent_amount path
    |> String.lstrip
  in
  ({ Lsp.TextEdit.range; newText = text }, return_var)

(** Replace the call with a variable (`return_var`) and adjust *)
let edit_replace_call_of_candidate
    ~source_text ~(return_var : string) T.{ call; callee; _ } =
  if callee.T.has_void_return then
    let strip_trailing_semicolon p =
      let semicolon_pos = Pos.shrink_to_end p in
      if
        String.equal
          ";"
          (Full_fidelity_source_text.sub_of_pos
             source_text
             ~length:1
             semicolon_pos)
      then
        Pos.advance_one p
      else
        p
    in
    let range =
      call.T.call_pos
      |> strip_trailing_semicolon
      |> strip_leading_spaces ~source_text
      |> to_lsp_range
    in
    Lsp.{ TextEdit.range; newText = "" }
  else
    let range = to_lsp_range call.T.call_pos in
    { Lsp.TextEdit.range; newText = return_var }

(** Remove the inlined method *)
let edit_remove_method_of_candidate ~source_text candidate : Lsp.TextEdit.t =
  let text = "" in
  let range =
    T.(candidate.callee.method_pos)
    |> strip_leading_spaces ~source_text
    |> to_lsp_range
  in
  { Lsp.TextEdit.range; newText = text }

let edit_of_candidate ~source_text ~path candidate : Lsp.WorkspaceEdit.t =
  let edit_remove_method =
    edit_remove_method_of_candidate ~source_text candidate
  in
  let (edit_inline, return_var) =
    edit_inline_and_return_var_of_candidate path ~source_text candidate
  in
  let edit_replace_call =
    edit_replace_call_of_candidate ~source_text ~return_var candidate
  in
  let changes =
    SMap.singleton
      (Relative_path.to_absolute path)
      [edit_remove_method; edit_inline; edit_replace_call]
  in
  Lsp.WorkspaceEdit.{ changes }

let to_refactor ~source_text ~path candidate =
  let edit = lazy (edit_of_candidate ~source_text ~path candidate) in
  Code_action_types.Refactor.{ title = "Inline method"; edit }
