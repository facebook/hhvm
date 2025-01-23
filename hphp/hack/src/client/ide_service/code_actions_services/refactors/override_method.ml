(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type method_action_location =
  | OnParentName of string
  | InBetweenClassElements of Pos.t

type t = {
  title: string;
  skeleton: Typing_skeleton.t;
  location: method_action_location;
}

let stub_method_action
    ~(location : method_action_location)
    ~(is_static : bool)
    ~(parent_name : string)
    ((meth_name, meth) : string * Typing_defs.class_elt) : t =
  let skeleton =
    Typing_skeleton.of_method
      ~is_static
      ~is_override:true
      ~open_braces:true
      meth_name
      meth
  in
  let skeleton =
    match location with
    | OnParentName _ -> Typing_skeleton.add_suffix "\n" skeleton
    | InBetweenClassElements _ ->
      skeleton
      |> Typing_skeleton.strip_suffix "\n"
      |> Typing_skeleton.add_prefix "\n"
  in
  let title =
    Printf.sprintf
      "Add override for %s::%s"
      (Utils.strip_ns parent_name)
      meth_name
  in
  { title; skeleton; location }

(* Return a list of quickfixes for [cls] which add a method that
   overrides one in [parent_name]. *)
let override_method_quickfixes
    (location : method_action_location)
    (env : Tast_env.env)
    (cls : Tast.class_)
    (parent_name : string) : t list =
  let existing_methods =
    SSet.of_list (List.map cls.Aast.c_methods ~f:(fun m -> snd m.Aast.m_name))
  in

  match Decl_provider.get_class (Tast_env.get_ctx env) parent_name with
  | Decl_entry.Found decl ->
    (* Offer an override action for any inherited method which isn't
       final and that the current class hasn't already overridden. *)
    let actions_for_methods ~is_static methods =
      methods
      |> List.filter ~f:(fun (name, meth) ->
             (not (SSet.mem name existing_methods))
             && not (Typing_defs.get_ce_final meth))
      |> List.map ~f:(stub_method_action ~location ~is_static ~parent_name)
    in
    actions_for_methods ~is_static:false (Folded_class.methods decl)
    @ actions_for_methods ~is_static:true (Folded_class.smethods decl)
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []

(* Quickfixes available at cursor position [cursor_line] and
   [cursor_col]. These aren't associated with errors, rather they
   transform code from one valid state to another. *)
let override_method_refactorings_at classish_positions ~cursor_line ~cursor_col
    =
  (* Is the current cursor in between methods? *)
  let inbetween_positions =
    Classish_positions.inbetween_body_element_positions classish_positions
  in
  let inbetween_class =
    List.hd
    @@ SMap.elements
    @@ SMap.filter_map
         (fun _cls posl ->
           List.find posl ~f:(fun whitespace_range ->
               Pos.inside whitespace_range cursor_line cursor_col))
         inbetween_positions
  in

  let parent_name_of_class_hint (h : Aast.class_hint) : string option =
    match snd h with
    | Aast.Happly ((_, parent_name), _) -> Some parent_name
    | _ -> None
  in

  object
    inherit [_] Tast_visitor.reduce as super

    method zero = []

    method plus = ( @ )

    method! on_class_ env c =
      let class_name = snd c.Aast.c_name in
      let acc = super#on_class_ env c in

      (* When cursor is on parent *)
      let on_parent_qfs =
        match c.Aast.c_kind with
        | Ast_defs.Cclass _ ->
          List.map c.Aast.c_extends ~f:(fun h ->
              if Pos.inside (fst h) cursor_line cursor_col then
                match parent_name_of_class_hint h with
                | Some parent_name ->
                  override_method_quickfixes
                    (OnParentName class_name)
                    env
                    c
                    parent_name
                | None -> []
              else
                [])
        | _ -> []
      in

      (* When cursor is on whitespace in between class body elements *)
      let inbetween_qfs =
        match inbetween_class with
        | Some (inbetween_class_name, inbetween_class_pos)
          when String.equal class_name inbetween_class_name ->
          List.map c.Aast.c_extends ~f:(fun h ->
              match parent_name_of_class_hint h with
              | Some parent_name ->
                override_method_quickfixes
                  (InBetweenClassElements inbetween_class_pos)
                  env
                  c
                  parent_name
              | None -> [])
        | _ -> []
      in

      List.concat inbetween_qfs @ List.concat on_parent_qfs @ acc
  end

let to_edits_and_selection
    (classish_positions : Pos.t Classish_positions.t) (quickfix : t) :
    Code_action_types.edit list * Pos.t option =
  let pos =
    match quickfix.location with
    | OnParentName class_name -> begin
      match
        Classish_positions.find
          (Classish_positions_types.Classish_start_of_body class_name)
          classish_positions
      with
      | Some classish_start -> Some classish_start
      | None ->
        let () =
          HackEventLogger.invariant_violation_bug
            ~data:class_name
            "Could not find class position for quickfix"
        in
        None
    end
    | InBetweenClassElements pos -> Some (Pos.shrink_to_start pos)
  in
  match pos with
  | Some pos ->
    ( [
        Code_action_types.
          { pos; text = Typing_skeleton.to_string quickfix.skeleton };
      ],
      Some (Typing_skeleton.cursor_after_insert pos quickfix.skeleton) )
  | None -> ([], None)

let refactor_action
    path (classish_positions : Pos.t Classish_positions.t) (quickfix : t) :
    Code_action_types.refactor =
  let (edits, selection) = to_edits_and_selection classish_positions quickfix in
  let edits = lazy (Relative_path.Map.singleton path edits) in
  Code_action_types.(
    Refactor
      {
        title = quickfix.title;
        edits;
        selection;
        trigger_inline_suggest = Option.is_some selection;
      })

let find ~entry pos ctx =
  let (cursor_line, cursor_col) = Pos.line_column pos in
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  let tree = Provider_context.PositionedSyntaxTree.root cst in
  let path = entry.Provider_context.path in
  let source_text = Ast_provider.compute_source_text ~entry in

  let classish_positions = Classish_positions.extract tree source_text path in

  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in

  let override_method_refactorings =
    (override_method_refactorings_at
       classish_positions
       ~cursor_line
       ~cursor_col)
      #go
      ctx
      tast.Tast_with_dynamic.under_normal_assumptions
  in
  List.map
    override_method_refactorings
    ~f:(refactor_action path classish_positions)
