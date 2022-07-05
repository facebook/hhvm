open Hh_prelude

let is_target target_line target_char (occ : Relative_path.t SymbolOccurrence.t)
    =
  let open SymbolOccurrence in
  let pos = occ.pos in
  let (l, start, end_) = Pos.info_pos pos in
  l = target_line && start <= target_char && target_char - 1 <= end_

let references
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    (occ : Relative_path.t SymbolOccurrence.t) :
    ServerCommandTypes.Find_refs.result_or_retry =
  let (line, column, _) = Pos.info_pos occ.SymbolOccurrence.pos in
  match ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column with
  | None -> ServerCommandTypes.Done_or_retry.Done []
  | Some (_, action) ->
    ServerFindRefs.(go ctx action false genv env)
    |> ServerCommandTypes.Done_or_retry.map_env ~f:ServerFindRefs.to_absolute
    |> snd

let body_symbols
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    (declarations : Relative_path.t SymbolOccurrence.t list)
    (occ : Relative_path.t SymbolOccurrence.t)
    (def : Relative_path.t SymbolDefinition.t) :
    Relative_path.t SymbolOccurrence.t list =
  let open SymbolOccurrence in
  let open SymbolDefinition in
  let node_opt =
    ServerSymbolDefinition.get_definition_cst_node_ctx
      ~ctx
      ~entry
      ~kind:def.kind
      ~pos:def.pos
  in
  match node_opt with
  | None -> []
  | Some node ->
    let span_pos_opt =
      Full_fidelity_positioned_syntax.position (Pos.filename def.pos) node
    in
    (match span_pos_opt with
    | None -> []
    | Some span_pos ->
      let pos_filter (o : Relative_path.t SymbolOccurrence.t) =
        (not (phys_equal o occ)) && Pos.contains span_pos o.pos
      in
      List.filter declarations ~f:pos_filter)

let body_references
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~(declarations : Relative_path.t SymbolOccurrence.t list)
    ~(get_def :
       Relative_path.t SymbolOccurrence.t ->
       Relative_path.t SymbolDefinition.t option)
    (occ : Relative_path.t SymbolOccurrence.t) :
    ServerCommandTypes.Find_refs.result_or_retry list =
  match get_def occ with
  | None -> [ServerCommandTypes.Done_or_retry.Done []]
  | Some def ->
    let symbols_to_find =
      occ :: body_symbols ~ctx ~entry declarations occ def
    in
    List.map symbols_to_find ~f:(references ~ctx ~entry ~genv ~env)

let go
    ~(ctx : Provider_context.t)
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    (pos_list : (string * int * int) list) :
    ServerCommandTypes.Find_refs.result_or_retry list =
  let deps_in_of_location acc_ctx_in (file, line, column) :
      Provider_context.t * ServerCommandTypes.Find_refs.result_or_retry list =
    let path = Relative_path.create_detect_prefix file in
    let (acc_ctx_out, entry) =
      Provider_context.add_entry_if_missing ~ctx:acc_ctx_in ~path
    in
    let ast =
      Ast_provider.compute_ast
        ~popt:(Provider_context.get_popt acc_ctx_out)
        ~entry
    in
    (*Other files can only depend on things declared in this one*)
    let declarations =
      IdentifySymbolService.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:(fun s -> s.SymbolOccurrence.is_declaration)
    in
    let target_symbols = List.filter declarations ~f:(is_target line column) in
    let get_def = ServerSymbolDefinition.go acc_ctx_out (Some ast) in
    let deps =
      List.concat_map
        target_symbols
        ~f:(body_references ~ctx ~entry ~genv ~env ~declarations ~get_def)
    in
    (acc_ctx_out, deps)
  in
  List.fold_map pos_list ~init:ctx ~f:deps_in_of_location |> snd |> List.concat
