open Hh_prelude

type candidate =
  | Of_expr of {
      (* the function or class containing the shape expression *)
      expr_container_pos: Pos.t;
      shape_ty_string: Code_action_types.Type_string.t;
    }
  | Of_hint of {
      hint_container_pos: Pos.t;
      hint_pos: Pos.t;
    }

(** We use distinct titles so we can tell the refactors apart in analytics *)
let title_of_candidate = function
  | Of_expr _ -> "Extract shape type"
  | Of_hint _ -> "Extract shape type to alias"

type state =
  | Searching of (Pos.t * candidate) option
  | Selected_non_shape_type of Pos.t

(** When searching for an expression of shape type,
we don't want to provide the refactor for larger expressions
that happen to contain an expression of shape type.
For example, we don't want to provide a refactor for this selection range:
$x = /*range-start*/(() ==> shape('a' => 2, 'b' => $a)['a'])()/*range-end*/;
*)
let plus_state (a : state) (b : state) : state =
  match (a, b) with
  | (Searching (Some _), Searching (Some (pos_b, _))) ->
    HackEventLogger.invariant_violation_bug
      ~path:(Pos.filename pos_b)
      ~pos:(Pos.string @@ Pos.to_absolute pos_b)
      "expected only one candidate to be found, since we select the largest shape-typed expression containing the selection";
    (* Safe to continue in spite of being in an unexpected situation:
       We still provide *a* reasonable refactoring by picking arbitrarily
    *)
    a
  | (Searching c1, Searching c2) -> Searching (Option.first_some c1 c2)
  | (Searching (Some (pos1, _)), Selected_non_shape_type pos2)
    when Pos.contains pos1 pos2 ->
    (* A shape can contain a non-shape *)
    a
  | (Selected_non_shape_type pos1, Selected_non_shape_type pos2) ->
    Selected_non_shape_type (Pos.merge pos1 pos2)
  | (Selected_non_shape_type _, _) -> a
  | (_, Selected_non_shape_type _) -> b

let find_candidate ~(selection : Pos.t) ~entry ctx : candidate option =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let visitor =
    let container_pos = ref None in

    object
      inherit [state] Tast_visitor.reduce as super

      method zero = Searching None

      method plus = plus_state

      method! on_class_ env class_ =
        let pos = class_.Aast_defs.c_span in
        if Pos.contains pos selection then (
          container_pos := Some pos;
          super#on_class_ env class_
        ) else
          Searching None

      method! on_type_hint_ env hint_ =
        match Option.both hint_ !container_pos with
        | Some ((hint_pos, Aast_defs.Hshape _), hint_container_pos)
          when Pos.contains selection hint_pos ->
          Searching (Some (hint_pos, Of_hint { hint_container_pos; hint_pos }))
        | _ -> super#on_type_hint_ env hint_

      method! on_fun_def env fd =
        let pos = Aast_defs.(fd.fd_fun.f_span) in
        if Pos.contains pos selection then (
          container_pos := Some pos;
          super#on_fun_def env fd
        ) else
          Searching None

      method! on_stmt env stmt =
        let stmt_pos = fst stmt in
        if Pos.contains selection stmt_pos then
          Selected_non_shape_type stmt_pos
        else
          super#on_stmt env stmt

      method! on_expr env expr =
        let (ty, expr_pos, _) = expr in
        if Pos.contains selection expr_pos then
          let ty_ = Typing_defs_core.get_node ty in
          match (ty_, !container_pos) with
          | (Typing_defs_core.Tshape _, Some expr_container_pos) ->
            Searching
              (Some
                 ( expr_pos,
                   Of_expr
                     {
                       expr_container_pos;
                       shape_ty_string =
                         Code_action_types.Type_string.of_locl_ty env ty;
                     } ))
          | _ -> Selected_non_shape_type expr_pos
        else
          super#on_expr env expr
    end
  in
  match visitor#go ctx tast.Tast_with_dynamic.under_normal_assumptions with
  | Searching (Some (_, candidate)) -> Some candidate
  | Searching _
  | Selected_non_shape_type _ ->
    None

let snippet_for_decl_of : string -> string =
  Printf.sprintf "type T${0:placeholder_} = %s;\n\n"

let snippet_for_use = "T${0:placeholder_}"

let range_of_container_pos container_pos : Lsp.range =
  let pos = Pos.shrink_to_start container_pos in
  Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal pos

let edit_of_candidate source_text ~path candidate : Lsp.WorkspaceEdit.t =
  let sub_of_pos = Full_fidelity_source_text.sub_of_pos source_text in
  let edits =
    match candidate with
    | Of_expr { shape_ty_string; expr_container_pos } ->
      let range = range_of_container_pos expr_container_pos in
      let text =
        snippet_for_decl_of
          (Code_action_types.Type_string.to_string shape_ty_string)
      in
      [Lsp.TextEdit.{ range; newText = text }]
    | Of_hint { hint_container_pos; hint_pos } ->
      let decl_edit =
        let range = range_of_container_pos hint_container_pos in
        let text =
          let ty_text = sub_of_pos hint_pos in
          snippet_for_decl_of ty_text
        in
        Lsp.TextEdit.{ range; newText = text }
      in
      let use_edit =
        let range =
          Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal hint_pos
        in
        Lsp.TextEdit.{ range; newText = snippet_for_use }
      in
      [decl_edit; use_edit]
  in
  let changes =
    Lsp.DocumentUri.Map.singleton (Lsp_helpers.path_to_lsp_uri path) edits
  in
  Lsp.WorkspaceEdit.{ changes }

let to_refactor source_text ~path candidate =
  let edit = lazy (edit_of_candidate source_text ~path candidate) in
  Code_action_types.Refactor.{ title = title_of_candidate candidate; edit }

let find ~entry ~(range : Lsp.range) ctx =
  let source_text = Ast_provider.compute_source_text ~entry in
  let line_to_offset line =
    Full_fidelity_source_text.position_to_offset source_text (line, 0)
  in
  let path = entry.Provider_context.path in
  let selection = Lsp_helpers.lsp_range_to_pos ~line_to_offset path range in
  find_candidate ~selection ~entry ctx
  |> Option.map ~f:(to_refactor source_text ~path)
  |> Option.to_list
