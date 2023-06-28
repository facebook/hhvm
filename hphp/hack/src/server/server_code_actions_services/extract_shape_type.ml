open Hh_prelude

type candidate = {
  tast_env: Tast_env.t;
  (* the function or class containing the shape expression *)
  container_pos: Pos.t;
  shape_ty: Typing_defs.locl_ty;
}

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
  | (Searching (Some _), Searching (Some _)) ->
    failwith
      "expected only one candidate to be found, since we select the largest shape-typed expression containing the selection"
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

      method! on_fun_def env fd =
        let pos = Aast_defs.(fd.fd_fun.f_span) in
        if Pos.contains pos selection then (
          container_pos := Some pos;
          super#on_fun_def env fd
        ) else
          Searching None

      method! on_expr env expr =
        let (ty, expr_pos, _) = expr in
        if Pos.contains selection expr_pos then
          let ty_ = Typing_defs_core.get_node ty in
          match (ty_, !container_pos) with
          | (Typing_defs_core.Tshape _, Some container_pos) ->
            Searching
              (Some (expr_pos, { tast_env = env; container_pos; shape_ty = ty }))
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

let edit_of_candidate ~path { shape_ty; container_pos; tast_env } :
    Lsp.WorkspaceEdit.t =
  let edit =
    let pos = Pos.shrink_to_start container_pos in
    let range =
      Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal pos
    in
    let tenv = Tast_env.tast_env_as_typing_env tast_env in
    let ty_text = Typing_print.full_strip_ns tenv shape_ty in
    let text = Printf.sprintf "type placeholder_ = %s;\n\n" ty_text in
    Lsp.TextEdit.{ range; newText = text }
  in
  let changes = SMap.singleton (Relative_path.to_absolute path) [edit] in
  Lsp.WorkspaceEdit.{ changes }

let to_refactor ~path candidate =
  let edit = lazy (edit_of_candidate ~path candidate) in
  Code_action_types.Refactor.{ title = "Extract shape type"; edit }

let find ~entry ~(range : Lsp.range) ctx =
  let source_text = Ast_provider.compute_source_text ~entry in
  let line_to_offset line =
    Full_fidelity_source_text.position_to_offset source_text (line, 0)
  in
  let path = entry.Provider_context.path in
  let selection = Lsp_helpers.lsp_range_to_pos ~line_to_offset path range in
  find_candidate ~selection ~entry ctx
  |> Option.map ~f:(to_refactor ~path)
  |> Option.to_list
