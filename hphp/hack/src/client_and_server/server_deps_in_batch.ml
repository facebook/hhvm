open Hh_prelude
open Server_deps_util

let references
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(genv : Server_env.genv)
    ~(env : Server_env.env)
    (occ : Relative_path.t Symbol_occurrence.t) :
    Server_command_types.Find_refs.result_or_retry =
  let (line, column, _) = Pos.info_pos occ.Symbol_occurrence.pos in
  match
    Server_find_refs.go_from_file_ctx
      ~ctx
      ~entry
      (File_content.Position.from_one_based line column)
  with
  | None -> Server_command_types.Done_or_retry.Done []
  | Some (_, action) ->
    Server_find_refs.(go ctx action false ~stream_file:None ~hints:[] genv env)
    |> Server_command_types.Done_or_retry.map_env
         ~f:Server_find_refs.to_absolute
    |> snd

let body_references
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(genv : Server_env.genv)
    ~(env : Server_env.env)
    ~(declarations : Relative_path.t Symbol_occurrence.t list)
    ~(get_def :
       Relative_path.t Symbol_occurrence.t ->
       Relative_path.t Symbol_definition.t option)
    (occ : Relative_path.t Symbol_occurrence.t) :
    Server_command_types.Find_refs.result_or_retry list =
  match get_def occ with
  | None -> [Server_command_types.Done_or_retry.Done []]
  | Some def ->
    let symbols_to_find =
      occ :: body_symbols ~ctx ~entry declarations occ def
    in
    List.map symbols_to_find ~f:(references ~ctx ~entry ~genv ~env)

let go
    ~(ctx : Provider_context.t)
    ~(genv : Server_env.genv)
    ~(env : Server_env.env)
    (pos_list : (string * int * int) list) :
    Server_command_types.Find_refs.result_or_retry list =
  let deps_in_of_location acc_ctx_in (file, line, column) :
      Provider_context.t * Server_command_types.Find_refs.result_or_retry list =
    let (acc_ctx_out, entry, _, get_def) = get_def_setup acc_ctx_in file in
    (*Other files can only depend on things declared in this one*)
    let declarations =
      Identify_symbol_service.all_symbols_ctx ~ctx:acc_ctx_out ~entry
      |> List.filter ~f:(fun s ->
             Option.is_some s.Symbol_occurrence.is_declaration)
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
