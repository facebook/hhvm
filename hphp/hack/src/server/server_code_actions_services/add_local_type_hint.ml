open Hh_prelude

type candidate = {
  lhs_var: string;
  lhs_type: string;
  lhs_pos: Pos.t;
}

let find_candidate ~(selection : Pos.t) ~entry ctx : candidate option =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  let visitor =
    object
      inherit [candidate option] Tast_visitor.reduce as super

      method zero = None

      method plus = Option.first_some

      method! on_class_ env class_ =
        let pos = class_.Aast_defs.c_span in
        if Pos.contains pos selection then
          super#on_class_ env class_
        else
          None

      method! on_method_ env meth =
        let pos = Aast_defs.(meth.m_span) in
        if Pos.contains pos selection then
          super#on_method_ env meth
        else
          None

      method! on_fun_def env fd =
        let pos = Aast_defs.(fd.fd_fun.f_span) in
        if Pos.contains pos selection then
          super#on_fun_def env fd
        else
          None

      method! on_stmt env stmt =
        let (pos, stmt_) = stmt in
        if Pos.contains pos selection then
          let open Aast in
          match stmt_ with
          | Expr
              ( _,
                _,
                Binop
                  {
                    bop = Ast_defs.Eq None;
                    lhs = (lvar_ty, _, Lvar (lid_pos, lid));
                    _;
                  } ) ->
            let tenv = Tast_env.tast_env_as_typing_env env in
            Some
              {
                lhs_var = Local_id.get_name lid;
                lhs_type = Typing_print.full_strip_ns tenv lvar_ty;
                lhs_pos = lid_pos;
              }
          | _ -> super#on_stmt env stmt
        else
          None
    end
  in
  visitor#go ctx tast

let edit_of_candidate ~path { lhs_var; lhs_type; lhs_pos } : Lsp.WorkspaceEdit.t
    =
  let edit =
    let range =
      Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal lhs_pos
    in
    let text = Printf.sprintf "let %s : %s " lhs_var lhs_type in
    Lsp.TextEdit.{ range; newText = text }
  in
  let changes = SMap.singleton (Relative_path.to_absolute path) [edit] in
  Lsp.WorkspaceEdit.{ changes }

let to_refactor ~path candidate =
  let edit = lazy (edit_of_candidate ~path candidate) in
  Code_action_types.Refactor.{ title = "Add local type hint"; edit }

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
