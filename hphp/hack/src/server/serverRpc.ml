(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix

let remove_dead_warning name =
  "hh_server was started without '--no-load', which is required when removing dead "
  ^ name
  ^ "s.\n"
  ^ "Please run 'hh_client restart --no-load' to restart it."

(** [take_max_errors n errors] truncate [errors] so its length is
  at most [n].

  Returns the truncated errors and the dropped error count. *)
let take_max_errors
    (max_errors : int option) (error_list : (_, _) User_error.t list) :
    (_, _) User_error.t list * int =
  match max_errors with
  | Some max_errors ->
    let (error_list, dropped_errors) = List.split_n error_list max_errors in
    (error_list, List.length dropped_errors)
  | None -> (error_list, 0)

let single_ctx env path file_input =
  let contents =
    match file_input with
    | ServerCommandTypes.FileName path -> Sys_utils.cat path
    | ServerCommandTypes.FileContent contents -> contents
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents

let log_check_response env =
  HackEventLogger.check_response
    (Errors.get_error_list env.ServerEnv.errorl
    |> List.map ~f:(fun { User_error.code; _ } -> code))

(** Might raise {!Naming_table.File_info_not_found} *)
let handle :
    type a.
    ServerEnv.genv ->
    ServerEnv.env ->
    is_stale:bool ->
    a ServerCommandTypes.t ->
    ServerEnv.env * a =
 fun genv env ~is_stale -> function
  | ServerCommandTypes.STATUS { max_errors; error_filter } ->
    log_check_response env;
    let (error_list, dropped_count) =
      env.ServerEnv.errorl
      |> Errors.sort_and_finalize
      |> Filter_errors.filter error_filter
      |> take_max_errors max_errors
    in
    let liveness =
      if is_stale then
        ServerCommandTypes.Stale_status
      else
        ServerCommandTypes.Live_status
    in
    let last_recheck_stats =
      match env.ServerEnv.last_recheck_loop_stats_for_actual_work with
      | None -> None
      | Some recheck_stats ->
        Some
          (ServerEnv.RecheckLoopStats.to_user_telemetry recheck_stats
          |> Telemetry.string_
               ~key:"init_id"
               ~value:ServerEnv.(env.init_env.init_id))
    in
    ( env,
      {
        ServerCommandTypes.Server_status.liveness;
        error_list;
        dropped_count;
        last_recheck_stats;
      } )
  | ServerCommandTypes.STATUS_SINGLE
      {
        file_names;
        max_errors;
        error_filter;
        preexisting_warnings;
        return_expanded_tast;
      } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let (errors, tasts) =
      ServerStatusSingle.go
        file_names
        ctx
        ~error_filter:
          {
            Tast_provider.ErrorFilter.error_filter;
            warnings_saved_state =
              ServerEnv.(env.init_env.mergebase_warning_hashes)
              >>= Option.some_if (not preexisting_warnings);
          }
    in
    let errors =
      errors |> Errors.sort_and_finalize |> take_max_errors max_errors
    in
    (* Unforced lazy values are closures which make serialization over RPC fail. *)
    let tasts =
      if return_expanded_tast then
        Some
          (Relative_path.Map.map
             tasts
             ~f:
               (Tast_with_dynamic.map ~f:(fun tast ->
                    tast
                    |> Tast_expand.expand_program ctx
                    |> Tast.force_lazy_values)))
      else
        None
    in
    (env, (errors, tasts))
  | ServerCommandTypes.INFER_TYPE (file_input, line, column) ->
    let path =
      match file_input with
      | ServerCommandTypes.FileName fn -> Relative_path.create_detect_prefix fn
      | ServerCommandTypes.FileContent _ ->
        Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerInferType.go_ctx ~ctx ~entry ~line ~column)
    in
    (env, result)
  | ServerCommandTypes.INFER_TYPE_BATCH positions ->
    (env, ServerInferTypeBatch.go genv.ServerEnv.workers positions env)
  | ServerCommandTypes.IS_SUBTYPE stdin ->
    (env, ServerIsSubtype.check genv.ServerEnv.workers stdin env)
  | ServerCommandTypes.TAST_HOLES (file_input, hole_filter) ->
    let path =
      match file_input with
      | ServerCommandTypes.FileName fn -> Relative_path.create_detect_prefix fn
      | ServerCommandTypes.FileContent _ ->
        Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerCollectTastHoles.go_ctx ~ctx ~entry ~hole_filter)
    in
    (env, result)
  | ServerCommandTypes.TAST_HOLES_BATCH files ->
    (env, ServerTastHolesBatch.go genv.ServerEnv.workers files env)
  | ServerCommandTypes.INFER_TYPE_ERROR (file_input, line, column) ->
    let path =
      match file_input with
      | ServerCommandTypes.FileName fn -> Relative_path.create_detect_prefix fn
      | ServerCommandTypes.FileContent _ ->
        Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerInferTypeError.go_ctx ~ctx ~entry ~line ~column)
    in
    (env, result)
  (* TODO: edit this to look for classname *)
  | ServerCommandTypes.IDENTIFY_SYMBOL arg ->
    let module SO = SymbolOccurrence in
    let ctx = Provider_utils.ctx_from_server_env env in
    let get_def_opt type_ name =
      ServerSymbolDefinition.go
        ctx
        None
        SO.{ type_; name; is_declaration = false; pos = Pos.none }
      |> Option.to_list
      |> List.map ~f:SymbolDefinition.to_absolute
    in
    let arg = Str.split (Str.regexp "::") arg in
    (* The following are all the different named entities I could think of in Hack. *)
    let results =
      match arg with
      | [c_name; member] ->
        let c_name = Utils.add_ns c_name in
        List.concat
          [
            get_def_opt (SO.Method (SO.ClassName c_name, member)) "";
            get_def_opt (SO.Property (SO.ClassName c_name, member)) "";
            get_def_opt (SO.XhpLiteralAttr (c_name, member)) "";
            get_def_opt (SO.ClassConst (SO.ClassName c_name, member)) "";
            get_def_opt (SO.Typeconst (c_name, member)) "";
          ]
      | [name] ->
        let name = Utils.add_ns name in
        List.concat
          [
            get_def_opt (SO.Class SO.ClassId) name;
            (* SO.Record and Class find the same things *)
            get_def_opt SO.Function name;
            get_def_opt SO.GConst name;
          ]
      | _ -> []
    in
    (env, results)
  | ServerCommandTypes.IDENTIFY_FUNCTION (filename, file_input, line, column) ->
    let (ctx, entry) =
      single_ctx env (Relative_path.create_detect_prefix filename) file_input
    in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerIdentifyFunction.go_quarantined_absolute
            ~ctx
            ~entry
            ~line
            ~column)
    in
    (env, result)
  | ServerCommandTypes.METHOD_JUMP (class_, filter, find_children) ->
    Printf.printf "%s" class_;
    let ctx = Provider_utils.ctx_from_server_env env in
    ( env,
      MethodJumps.get_inheritance
        ctx
        class_
        ~filter
        ~find_children
        env.ServerEnv.naming_table
        genv.ServerEnv.workers )
  | ServerCommandTypes.METHOD_JUMP_BATCH (classes, filter) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerMethodJumpsBatch.go ctx genv.ServerEnv.workers classes filter)
  | ServerCommandTypes.FIND_REFS find_refs_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open ServerCommandTypes.Done_or_retry in
        let include_defs = false in
        ServerFindRefs.(
          go
            ctx
            find_refs_action
            include_defs
            ~stream_file:None
            ~hints:[]
            genv
            env
          |> map_env ~f:to_absolute))
  | ServerCommandTypes.GO_TO_IMPL go_to_impl_action ->
    ServerCommandTypes.Done_or_retry.(
      ServerGoToImpl.go ~action:go_to_impl_action ~genv ~env
      |> map_env ~f:ServerFindRefs.to_absolute)
  | ServerCommandTypes.IDE_FIND_REFS_BY_SYMBOL
      {
        FindRefsWireFormat.CliArgs.symbol_name = _;
        action;
        stream_file;
        hint_suffixes;
      } ->
    let hints =
      List.map hint_suffixes ~f:(fun suffix -> Relative_path.from_root ~suffix)
    in
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open ServerCommandTypes.Done_or_retry in
        let include_defs = false in
        map_env
          ~f:ServerFindRefs.to_absolute
          (ServerFindRefs.go
             ctx
             action
             include_defs
             ~stream_file
             ~hints
             genv
             env))
  | ServerCommandTypes.IDE_GO_TO_IMPL_BY_SYMBOL
      { FindRefsWireFormat.CliArgs.symbol_name = _; action; _ } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open ServerCommandTypes.Done_or_retry in
        map_env
          ~f:ServerFindRefs.to_absolute
          (ServerGoToImpl.go ~action ~genv ~env))
  | ServerCommandTypes.RENAME rename_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let definition_for_wrapper =
          match rename_action with
          | ServerRenameTypes.ClassRename _
          | ServerRenameTypes.ClassConstRename _
          | ServerRenameTypes.LocalVarRename _ ->
            None
          | ServerRenameTypes.MethodRename { class_name; old_name; _ } ->
            ServerSymbolDefinition.go
              ctx
              None
              {
                SymbolOccurrence.name = "unused for lookup";
                type_ =
                  SymbolOccurrence.Method
                    ( SymbolOccurrence.ClassName (Utils.add_ns class_name),
                      old_name );
                is_declaration = false;
                pos = Pos.none;
              }
          | ServerRenameTypes.FunctionRename { old_name; _ } ->
            ServerSymbolDefinition.go
              ctx
              None
              {
                SymbolOccurrence.name = Utils.add_ns old_name;
                type_ = SymbolOccurrence.Function;
                is_declaration = false;
                pos = Pos.none;
              }
        in
        ServerRename.go ctx rename_action genv env ~definition_for_wrapper)
  | ServerCommandTypes.IDE_RENAME_BY_SYMBOL (action, new_name, symbol_definition)
    ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open ServerCommandTypes.Done_or_retry in
        match
          ServerRename.go_ide_with_find_refs_action
            ctx
            ~find_refs_action:action
            ~new_name
            ~symbol_definition
            genv
            env
        with
        | Error e -> (env, Done (Error e))
        | Ok r -> map_env r ~f:(fun x -> Ok x))
  | ServerCommandTypes.REMOVE_DEAD_FIXMES codes ->
    if genv.ServerEnv.options |> ServerArgs.no_load then (
      log_check_response env;
      (env, `Ok (ServerRename.get_fixme_patches codes env))
    ) else
      (env, `Error (remove_dead_warning "fixme"))
  | ServerCommandTypes.REMOVE_DEAD_UNSAFE_CASTS ->
    if genv.ServerEnv.options |> ServerArgs.no_load then (
      log_check_response env;
      (env, `Ok (ServerRename.get_dead_unsafe_cast_patches env))
    ) else
      (env, `Error (remove_dead_warning "unsafe cast"))
  | ServerCommandTypes.REWRITE_LAMBDA_PARAMETERS files ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerRename.get_lambda_parameter_rewrite_patches ctx files)
  | ServerCommandTypes.DUMP_SYMBOL_INFO file_list ->
    (env, SymbolInfoService.go genv.ServerEnv.workers file_list env)
  | ServerCommandTypes.IN_MEMORY_DEP_TABLE_SIZE ->
    (* TODO(hverr): Clean up 32-bit/migrate *)
    (env, Ok 0)
  | ServerCommandTypes.SAVE_NAMING filename ->
    (env, SaveStateService.go_naming env.ServerEnv.naming_table filename)
  | ServerCommandTypes.SAVE_STATE (filename, gen_saved_ignore_type_errors) ->
    if Errors.is_empty env.ServerEnv.errorl || gen_saved_ignore_type_errors then
      (env, SaveStateService.go env filename)
    else
      (env, Error "There are typecheck errors; cannot generate saved state.")
  | ServerCommandTypes.CHECK_LIVENESS ->
    (* This is for the client to know "is the server available to process requests?" *)
    (env, ())
  | ServerCommandTypes.LINT fnl ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go genv ctx fnl)
  | ServerCommandTypes.LINT_STDIN ServerCommandTypes.{ filename; contents } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go_stdin ctx ~filename ~contents)
  | ServerCommandTypes.LINT_ALL code ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.lint_all genv ctx code)
  | ServerCommandTypes.STATS -> (env, Stats.get_stats ())
  | ServerCommandTypes.DUMP_FULL_FIDELITY_PARSE file ->
    (env, FullFidelityParseService.go file)
  | ServerCommandTypes.RAGE -> (env, ServerRage.go genv env)
  | ServerCommandTypes.CST_SEARCH
      ServerCommandTypes.{ sort_results; input; files_to_search } -> begin
    try
      (env, CstSearchService.go genv env ~sort_results ~files_to_search input)
    with
    | MultiThreadedCall.Coalesced_failures failures ->
      let failures =
        failures
        |> List.map ~f:WorkerController.failure_to_string
        |> String.concat ~sep:"\n"
      in
      ( env,
        Error
          (Printf.sprintf
             "Worker failures - check the logs for more details:\n%s\n"
             failures) )
    | exn ->
      let e = Exception.wrap exn in
      (env, Error (Exception.to_string e))
  end
  | ServerCommandTypes.NO_PRECHECKED_FILES ->
    (ServerPrecheckedFiles.expand_all env, ())
  | ServerCommandTypes.FUN_DEPS_BATCH positions ->
    (env, ServerFunDepsBatch.go genv.ServerEnv.workers positions env)
  | ServerCommandTypes.LIST_FILES_WITH_ERRORS ->
    (env, ServerEnv.list_files_with_errors env)
  | ServerCommandTypes.FILE_DEPENDENTS filenames ->
    let files = ServerFileDependents.go genv env filenames in
    (env, files)
  | ServerCommandTypes.IDENTIFY_TYPES (labelled_file, line, column) ->
    let (path, file_input) =
      ServerCommandTypesUtils.extract_labelled_file labelled_file
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      ServerTypeDefinition.go_quarantined ~ctx ~entry ~line ~column
    in
    (env, result)
  | ServerCommandTypes.VERBOSE verbose ->
    if verbose then
      Hh_logger.Level.set_min_level Hh_logger.Level.Debug
    else
      Hh_logger.Level.set_min_level
        genv.ServerEnv.local_config.ServerLocalConfig.min_log_level;
    (env, ())
  | ServerCommandTypes.DEPS_OUT_BATCH positions ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerDepsOutBatch.go ctx positions)
  | ServerCommandTypes.DEPS_IN_BATCH positions ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerDepsInBatch.go ~ctx ~genv ~env positions)
