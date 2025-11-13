open Hh_prelude

let go_fast genv env file =
  let ctx = Provider_utils.ctx_from_server_env env in
  let path = Relative_path.create_detect_prefix file in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  let files =
    List.fold ast ~init:Relative_path.Set.empty ~f:(fun acc def ->
        let open Aast_defs in
        let deps =
          match def with
          | Class { c_name = (_, name); _ }
          | Typedef { t_name = (_, name); _ } ->
            FindRefsService.get_dependent_files
              ctx
              genv.ServerEnv.workers
              (SSet.singleton name)
            |> Relative_path.Set.union acc
          | Fun { fd_name = (_, name); _ } ->
            FindRefsService.get_dependent_files_function
              ctx
              genv.ServerEnv.workers
              name
            |> Relative_path.Set.union acc
          | Constant { cst_name = (_, name); _ } ->
            FindRefsService.get_dependent_files_gconst
              ctx
              genv.ServerEnv.workers
              name
            |> Relative_path.Set.union acc
          | Stmt _
          | Namespace _
          | FileAttributes _
          | Module _
          | NamespaceUse _
          | SetNamespaceEnv _
          | SetModule _ ->
            acc
        in
        deps)
  in
  (env, files)

let go genv env file =
  let ctx = Provider_utils.ctx_from_server_env env in
  let path = Relative_path.create_detect_prefix file in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in

  let target def =
    let open Aast_defs in
    match def with
    | Class { c_name = (_, name); _ }
    | Typedef { t_name = (_, name); _ } ->
      Some (ServerCommandTypes.Find_refs.Class name)
    | Fun { fd_name = (_, name); _ } ->
      Some (ServerCommandTypes.Find_refs.Function name)
    | Constant { cst_name = (_, name); _ } ->
      Some (ServerCommandTypes.Find_refs.GConst name)
    | Stmt _
    | Namespace _
    | FileAttributes _
    | Module _
    | NamespaceUse _
    | SetNamespaceEnv _
    | SetModule _ ->
      None
  in
  let refs_or_retry env (acc : ServerCommandTypes.Package_lint.result) def =
    let open ServerCommandTypes.Done_or_retry in
    match target def with
    | None -> (env, Done acc)
    | Some target ->
      ServerFindRefs.(
        go ctx target false ~hints:[] ~stream_file:None genv env
        |> map_env ~f:(fun x -> (target, to_absolute x) :: acc))
  in
  (* Small helper to short circuit if any of the Find_refs return Retry *)
  let rec fold env ~acc ~f l =
    let open ServerCommandTypes.Done_or_retry in
    match (acc, l) with
    | (Retry, _)
    | (_, []) ->
      (env, acc)
    | (Done refs_acc, x :: xs) ->
      let (env, acc) = f env refs_acc x in
      fold env ~acc ~f xs
  in
  fold env ~acc:(Done []) ~f:refs_or_retry ast
