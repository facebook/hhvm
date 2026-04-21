open Hh_prelude

let go_fast _genv env file =
  let ctx = Provider_utils.ctx_from_server_env env in
  let path = Relative_path.create_detect_prefix file in
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  let ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  (* Phase 1: Collect all dep hashes from AST definitions *)
  let open Typing_deps in
  let dep_set =
    List.fold ast ~init:(DepSet.make ()) ~f:(fun deps def ->
        let open Aast_defs in
        match def with
        | Class { c_name = (_, name); _ }
        | Typedef { t_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.Type name))
        | Fun { fd_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.Fun name))
        | Constant { cst_name = (_, name); _ } ->
          DepSet.add deps (Dep.make (Dep.GConst name))
        | ClassAlias _
        | Stmt _
        | Namespace _
        | FileAttributes _
        | Module _
        | NamespaceUse _
        | SetNamespaceEnv _
        | SetModule _ ->
          deps)
  in
  (* Phase 2: Single batched dep graph lookup + single file resolution *)
  let deps_mode = Provider_context.get_deps_mode ctx in
  let all_ideps = add_typing_deps deps_mode dep_set in
  let files = Naming_provider.get_files ctx all_ideps in
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
    | ClassAlias _
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
