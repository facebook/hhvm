(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let to_range (pos : Pos.t) : Lsp.range =
  let (first_line, first_col) = Pos.line_column pos in
  let (last_line, last_col) = Pos.end_line_column pos in
  {
    Lsp.start = { Lsp.line = first_line - 1; character = first_col };
    end_ = { Lsp.line = last_line - 1; character = last_col };
  }

let stub_method_action
    ~(is_static : bool)
    (class_name : string)
    (parent_name : string)
    ((meth_name, meth) : string * Typing_defs.class_elt) : Pos.t Quickfix.t =
  let new_text =
    Typing_skeleton.of_method ~is_static ~is_override:true meth_name meth ^ "\n"
  in
  let title =
    Printf.sprintf
      "Add override for %s::%s"
      (Utils.strip_ns parent_name)
      meth_name
  in
  Quickfix.make_classish ~title ~new_text ~classish_name:class_name

(* Return a list of quickfixes for [cls] which add a method that
   overrides one in [parent_name]. *)
let override_method_quickfixes
    (env : Tast_env.env) (cls : Tast.class_) (parent_name : string) :
    Pos.t Quickfix.t list =
  let (_, class_name) = cls.Aast.c_name in
  let existing_methods =
    SSet.of_list (List.map cls.Aast.c_methods ~f:(fun m -> snd m.Aast.m_name))
  in

  match Decl_provider.get_class (Tast_env.get_ctx env) parent_name with
  | Some decl ->
    (* Offer an override action for any inherited method which isn't
       final and that the current class hasn't already overridden. *)
    let actions_for_methods ~is_static methods =
      methods
      |> List.filter ~f:(fun (name, meth) ->
             (not (SSet.mem name existing_methods))
             && not (Typing_defs.get_ce_final meth))
      |> List.map ~f:(stub_method_action ~is_static class_name parent_name)
    in
    actions_for_methods ~is_static:false (Decl_provider.Class.methods decl)
    @ actions_for_methods ~is_static:true (Decl_provider.Class.smethods decl)
  | None -> []

(* Quickfixes available at cursor position [start_line] and
   [start_col]. These aren't associated with errors, rather they
   transform code from one valid state to another. *)
let override_method_refactorings_at ~start_line ~start_col =
  object
    inherit [_] Tast_visitor.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_class_ env c =
      let acc = super#on_class_ env c in
      let meth_actions =
        match c.Aast.c_kind with
        | Ast_defs.Cclass _ ->
          List.map c.Aast.c_extends ~f:(fun (parent_id_pos, parent_hint) ->
              if Pos.inside parent_id_pos start_line start_col then
                match parent_hint with
                | Aast.Happly ((_, parent_name), _) ->
                  override_method_quickfixes env c parent_name
                | _ -> []
              else
                [])
        | _ -> []
      in

      List.concat meth_actions @ acc
  end

let text_edits (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Lsp.TextEdit.t list =
  let edits = Quickfix.get_edits ~classish_starts quickfix in
  List.map edits ~f:(fun (new_text, pos) ->
      { Lsp.TextEdit.range = to_range pos; newText = new_text })

let refactor_action
    path (classish_starts : Pos.t SMap.t) (quickfix : Pos.t Quickfix.t) :
    Code_action_types.Refactor.t =
  let edit =
    lazy
      (let changes =
         Lsp.DocumentUri.Map.singleton
           (Lsp_helpers.path_to_lsp_uri path)
           (text_edits classish_starts quickfix)
       in
       Lsp.WorkspaceEdit.{ changes })
  in
  Code_action_types.Refactor.{ title = Quickfix.get_title quickfix; edit }

let find ~entry pos ctx =
  let (start_line, start_col) = Pos.line_column pos in
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in
  let path = entry.Provider_context.path in
  let source_text = Ast_provider.compute_source_text ~entry in

  let classish_starts = Quickfix_ffp.classish_starts tree source_text path in

  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in

  let override_method_refactorings =
    (override_method_refactorings_at ~start_line ~start_col)#go
      ctx
      tast.Tast_with_dynamic.under_normal_assumptions
  in
  List.map
    override_method_refactorings
    ~f:(refactor_action path classish_starts)
