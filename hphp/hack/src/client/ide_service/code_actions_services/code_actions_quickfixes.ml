(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let text_edits
    (ctx : Provider_context.t)
    (entry : Provider_context.entry)
    (classish_positions : Pos.t Classish_positions.t)
    (quickfix : Pos.t Quickfix.t) : Code_action_types.edit list =
  match Quickfix.get_edits quickfix with
  | Quickfix.Eager eager_edits ->
    List.map eager_edits ~f:(fun (text, pos) -> Code_action_types.{ pos; text })
  | Quickfix.Classish_end { classish_end_new_text; classish_end_name } ->
    (match
       Classish_positions.find
         (Classish_positions_types.Classish_end_of_body classish_end_name)
         classish_positions
     with
    | Some classish_end ->
      [Code_action_types.{ pos = classish_end; text = classish_end_new_text }]
    | None ->
      let () =
        HackEventLogger.invariant_violation_bug
          ~data:classish_end_name
          "Could not find class position for quickfix"
      in
      [])
  | Quickfix.Add_function_attribute { function_pos; attribute_name } ->
    Code_actions_edits_add_attribute.create
      ctx
      entry
      ~function_pos
      ~attribute_name

let convert_quickfix
    (ctx : Provider_context.t)
    (entry : Provider_context.entry)
    path
    (classish_positions : Pos.t Classish_positions.t)
    (quickfix : Pos.t Quickfix.t) : Code_action_types.quickfix =
  let edits =
    lazy
      (Relative_path.Map.singleton
         path
         (text_edits ctx entry classish_positions quickfix))
  in

  Code_action_types.(
    Quickfix
      {
        title = Quickfix.get_title quickfix;
        edits;
        selection = None;
        trigger_inline_suggest = false;
      })

let quickfix_positions_for_error
    (classish_positions : Pos.t Classish_positions.t)
    (error : Diagnostics.diagnostic) : Pos.t list =
  let quickfixes = User_diagnostic.quickfixes error in
  let hint_styles = List.bind ~f:Quickfix.get_hint_styles quickfixes in
  let available_positions =
    List.filter_map
      hint_styles
      ~f:
        Quickfix.(
          function
          | HintStyleSilent p
          | HintStyleHint p ->
            Classish_positions.find p classish_positions)
  in
  User_diagnostic.get_pos error :: available_positions

let errors_to_quickfixes
    (ctx : Provider_context.t)
    (entry : Provider_context.entry)
    (errors : Diagnostics.t)
    (path : Relative_path.t)
    (classish_positions : Pos.t Classish_positions.t)
    (selection : Pos.t) : Code_action_types.quickfix list =
  let errors = Diagnostics.get_diagnostic_list ~drop_fixmed:false errors in
  let errors_here =
    List.filter errors ~f:(fun e ->
        let available_positions =
          quickfix_positions_for_error classish_positions e
        in
        List.exists available_positions ~f:(fun p -> Pos.contains p selection))
  in
  let quickfixes = List.bind ~f:User_diagnostic.quickfixes errors_here in
  let standard_quickfixes =
    List.map quickfixes ~f:(convert_quickfix ctx entry path classish_positions)
  in
  let quickfixes_from_refactors =
    errors_here |> List.bind ~f:(Quickfixes_from_refactors.find ctx entry)
  in
  standard_quickfixes @ quickfixes_from_refactors

let find ~entry pos ctx ~error_filter : Code_action_types.quickfix list =
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in

  let classish_positions =
    match entry.Provider_context.source_text with
    | Some source_text ->
      Classish_positions.extract tree source_text entry.Provider_context.path
    | None -> Classish_positions.empty
  in

  let { Tast_provider.Compute_tast_and_errors.diagnostics; _ } =
    Tast_provider.compute_tast_and_errors_quarantined ~ctx ~entry ~error_filter
  in
  let path = entry.Provider_context.path in
  errors_to_quickfixes ctx entry diagnostics path classish_positions pos
