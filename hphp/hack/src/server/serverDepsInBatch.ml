open Hh_prelude
open ServerDepsUtil

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
    ServerFindRefs.(go ctx action false ~stream_file:None ~hints:[] genv env)
    |> ServerCommandTypes.Done_or_retry.map_env ~f:ServerFindRefs.to_absolute
    |> snd

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
    let (acc_ctx_out, entry, _, get_def) = get_def_setup acc_ctx_in file in
    (*Other files can only depend on things declared in this one*)
    let declarations =
      IdentifySymbolService.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:(fun s -> s.SymbolOccurrence.is_declaration)
    in
    let target_symbols = List.filter declarations ~f:(is_target line column) in
    let deps =
      List.concat_map
        target_symbols
        ~f:(body_references ~ctx ~entry ~genv ~env ~declarations ~get_def)
    in
    (acc_ctx_out, deps)
  in
  List.fold_map pos_list ~init:ctx ~f:deps_in_of_location |> snd |> List.concat
