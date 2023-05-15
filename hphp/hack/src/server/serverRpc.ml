(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
open ServerCommandTypes
open Utils

let remove_dead_warning name =
  "hh_server was started without '--no-load', which is required when removing dead "
  ^ name
  ^ "s.\n"
  ^ "Please run 'hh_client restart --no-load' to restart it."

let take_max_errors error_list max_errors =
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

let single_ctx_path env path =
  single_ctx
    env
    (Relative_path.create_detect_prefix path)
    (ServerCommandTypes.FileName path)

(* Might raise {!Naming_table.File_info_not_found} *)
let handle : type a. genv -> env -> is_stale:bool -> a t -> env * a =
 fun genv env ~is_stale -> function
  | STATUS { max_errors; _ } ->
    HackEventLogger.check_response
      (Errors.get_error_list env.errorl |> List.map ~f:User_error.get_code);
    let error_list = Errors.sort_and_finalize env.errorl in
    let (error_list, dropped_count) = take_max_errors error_list max_errors in
    let liveness =
      if is_stale then
        Stale_status
      else
        Live_status
    in
    let has_unsaved_changes = ServerFileSync.has_unsaved_changes env in
    let last_recheck_stats =
      Option.map
        env.ServerEnv.last_recheck_loop_stats_for_actual_work
        ~f:ServerEnv.RecheckLoopStats.to_user_telemetry
    in
    ( env,
      {
        Server_status.liveness;
        has_unsaved_changes;
        error_list;
        dropped_count;
        last_recheck_stats;
      } )
  | STATUS_SINGLE { file_names; max_errors } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, take_max_errors (ServerStatusSingle.go file_names ctx) max_errors)
  | INFER_TYPE (file_input, line, column) ->
    let path =
      match file_input with
      | FileName fn -> Relative_path.create_detect_prefix fn
      | FileContent _ -> Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerInferType.go_ctx ~ctx ~entry ~line ~column)
    in
    (env, result)
  | INFER_TYPE_BATCH positions ->
    (env, ServerInferTypeBatch.go genv.workers positions env)
  | IS_SUBTYPE stdin -> (env, ServerIsSubtype.check genv.workers stdin env)
  | TAST_HOLES (file_input, hole_filter) ->
    let path =
      match file_input with
      | FileName fn -> Relative_path.create_detect_prefix fn
      | FileContent _ -> Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerCollectTastHoles.go_ctx ~ctx ~entry ~hole_filter)
    in
    (env, result)
  | TAST_HOLES_BATCH files ->
    (env, ServerTastHolesBatch.go genv.workers files env)
  | INFER_TYPE_ERROR (file_input, line, column) ->
    let path =
      match file_input with
      | FileName fn -> Relative_path.create_detect_prefix fn
      | FileContent _ -> Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerInferTypeError.go_ctx ~ctx ~entry ~line ~column)
    in
    (env, result)
  | IDE_HOVER (path, line, column) ->
    let (ctx, entry) = single_ctx_path env path in
    let result = ServerHover.go_quarantined ~ctx ~entry ~line ~column in
    (env, result)
  | DOCBLOCK_AT (path, line, column, _, kind) ->
    let (ctx, entry) = single_ctx_path env path in
    let r = ServerDocblockAt.go_docblock_ctx ~ctx ~entry ~line ~column ~kind in
    (env, r)
  | DOCBLOCK_FOR_SYMBOL (symbol, kind) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let r = ServerDocblockAt.go_docblock_for_symbol ~ctx ~symbol ~kind in
    (env, r)
  | IDE_SIGNATURE_HELP (path, line, column) ->
    let (ctx, entry) = single_ctx_path env path in
    (env, ServerSignatureHelp.go_quarantined ~ctx ~entry ~line ~column)
  | COMMANDLINE_AUTOCOMPLETE contents ->
    (* For command line autocomplete, we assume the AUTO332 text has
       already been inserted, and we fake the rest of this information. *)
    let autocomplete_context =
      {
        AutocompleteTypes.is_manually_invoked = false;
        is_after_single_colon = false;
        is_after_double_right_angle_bracket = false;
        is_after_open_square_bracket = false;
        is_after_quote = false;
        is_before_apostrophe = false;
        is_open_curly_without_equals = false;
        char_at_pos = ' ';
      }
    in
    (* Since this is being executed from the command line,
     * let's turn on the flags that increase accuracy but slow it down *)
    let sienv =
      {
        env.ServerEnv.local_symbol_table with
        SearchUtils.sie_resolve_signatures = true;
        SearchUtils.sie_resolve_positions = true;
        SearchUtils.sie_resolve_local_decl = true;
      }
    in
    (* feature not implemented here; it only works for LSP *)
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx:(Provider_utils.ctx_from_server_env env)
        ~path:(Relative_path.create_detect_prefix "")
        ~contents
    in
    (* Update the symbol index from this file *)
    Facts_parser.mangle_xhp_mode := false;
    let facts_opt =
      Facts_parser.from_text
        ~php5_compat_mode:false
        ~hhvm_compat_mode:true
        ~disable_legacy_soft_typehints:false
        ~allow_new_attribute_syntax:false
        ~disable_legacy_attribute_syntax:false
        ~enable_xhp_class_modifier:false
        ~disable_xhp_element_mangling:false
        ~auto_namespace_map:
          env.ServerEnv.popt.GlobalOptions.po_auto_namespace_map
        ~filename:Relative_path.default
        ~text:contents
    in
    let sienv =
      match facts_opt with
      | None -> sienv
      | Some facts ->
        SymbolIndexCore.update_from_facts
          ~sienv
          ~path:Relative_path.default
          ~facts
    in
    let result =
      ServerAutoComplete.go_at_auto332_ctx
        ~ctx
        ~entry
        ~sienv
        ~autocomplete_context
        ~naming_table:env.naming_table
    in
    (env, result.With_complete_flag.value)
  | IDENTIFY_SYMBOL arg ->
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
  | IDENTIFY_FUNCTION (filename, file_input, line, column) ->
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
  | METHOD_JUMP (class_, filter, find_children) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    ( env,
      MethodJumps.get_inheritance
        ctx
        class_
        ~filter
        ~find_children
        env.naming_table
        genv.workers )
  | METHOD_JUMP_BATCH (classes, filter) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerMethodJumpsBatch.go ctx genv.workers classes filter)
  | FIND_REFS find_refs_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open Done_or_retry in
        let include_defs = false in
        ServerFindRefs.(
          go ctx find_refs_action include_defs genv env
          |> map_env ~f:to_absolute))
  | GO_TO_IMPL go_to_impl_action ->
    Done_or_retry.(
      ServerGoToImpl.go ~action:go_to_impl_action ~genv ~env
      |> map_env ~f:ServerFindRefs.to_absolute)
  | IDE_FIND_REFS (labelled_file, line, column, include_defs) ->
    Done_or_retry.(
      let (path, file_input) =
        ServerCommandTypesUtils.extract_labelled_file labelled_file
      in
      let (ctx, entry) = single_ctx env path file_input in
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          match ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column with
          | None -> (env, Done None)
          | Some (name, action) ->
            map_env
              ~f:(ServerFindRefs.to_ide name)
              (ServerFindRefs.go ctx action include_defs genv env)))
  | IDE_FIND_REFS_BY_SYMBOL (find_refs_action, name) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open Done_or_retry in
        let include_defs = false in
        map_env
          ~f:(ServerFindRefs.to_ide name)
          (ServerFindRefs.go ctx find_refs_action include_defs genv env))
  | IDE_GO_TO_IMPL (labelled_file, line, column) ->
    Done_or_retry.(
      let (path, file_input) =
        ServerCommandTypesUtils.extract_labelled_file labelled_file
      in
      let (ctx, entry) = single_ctx env path file_input in
      (match ServerFindRefs.go_from_file_ctx ~ctx ~entry ~line ~column with
      | None -> (env, Done None)
      | Some (name, action) ->
        map_env
          ~f:(ServerFindRefs.to_ide name)
          (ServerGoToImpl.go ~action ~genv ~env)))
  | IDE_HIGHLIGHT_REFS (path, file_input, line, column) ->
    let (ctx, entry) =
      single_ctx env (Relative_path.create_detect_prefix path) file_input
    in
    let r =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          (env, ServerHighlightRefs.go_quarantined ~ctx ~entry ~line ~column))
    in
    r
  | RENAME rename_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        ServerRename.go ctx rename_action genv env)
  | RENAME_CHECK_SD rename_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    ServerRename.go_sound_dynamic ctx rename_action genv env
  | IDE_RENAME
      { ServerCommandTypes.Ide_rename_type.filename; line; char; new_name } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open Done_or_retry in
        match
          ServerRename.go_ide ctx (filename, line, char) new_name genv env
        with
        | Error e -> (env, Done (Error e))
        | Ok r -> map_env r ~f:(fun x -> Ok x))
  | IDE_RENAME_BY_SYMBOL (action, new_name, filename, symbol_definition) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open Done_or_retry in
        match
          ServerRename.go_ide_with_find_refs_action
            ctx
            ~find_refs_action:action
            ~new_name
            ~filename
            ~symbol_definition
            genv
            env
        with
        | Error e -> (env, Done (Error e))
        | Ok r -> map_env r ~f:(fun x -> Ok x))
  | CODEMOD_SDT codemod_line ->
    let patches = Sdt_analysis.patches_of_codemod_line codemod_line in
    (env, patches)
  | REMOVE_DEAD_FIXMES codes ->
    if genv.ServerEnv.options |> ServerArgs.no_load then (
      HackEventLogger.check_response
        (Errors.get_error_list env.errorl |> List.map ~f:User_error.get_code);
      (env, `Ok (ServerRename.get_fixme_patches codes env))
    ) else
      (env, `Error (remove_dead_warning "fixme"))
  | REMOVE_DEAD_UNSAFE_CASTS ->
    if genv.ServerEnv.options |> ServerArgs.no_load then (
      HackEventLogger.check_response
        (Errors.get_error_list env.errorl |> List.map ~f:User_error.get_code);
      (env, `Ok (ServerRename.get_dead_unsafe_cast_patches env))
    ) else
      (env, `Error (remove_dead_warning "unsafe cast"))
  | REWRITE_LAMBDA_PARAMETERS files ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerRename.get_lambda_parameter_rewrite_patches ctx files)
  | REWRITE_TYPE_PARAMS_TYPE files ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerRename.get_type_params_type_rewrite_patches ctx files)
  | DUMP_SYMBOL_INFO file_list ->
    (env, SymbolInfoService.go genv.workers file_list env)
  | IN_MEMORY_DEP_TABLE_SIZE ->
    (* TODO(hverr): Clean up 32-bit/migrate *)
    (env, Ok 0)
  | SAVE_NAMING filename ->
    (env, SaveStateService.go_naming env.naming_table filename)
  | SAVE_STATE (filename, gen_saved_ignore_type_errors) ->
    if Errors.is_empty env.errorl || gen_saved_ignore_type_errors then
      (env, SaveStateService.go env filename)
    else
      (env, Error "There are typecheck errors; cannot generate saved state.")
  | SEARCH (query, type_) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let lst = env.ServerEnv.local_symbol_table in
    (env, ServerSearch.go ctx query ~kind_filter:type_ lst)
  | LINT fnl ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go genv ctx fnl)
  | LINT_STDIN { filename; contents } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go_stdin ctx ~filename ~contents)
  | LINT_ALL code ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.lint_all genv ctx code)
  | CREATE_CHECKPOINT x -> (env, ServerCheckpoint.create_checkpoint x)
  | RETRIEVE_CHECKPOINT x -> (env, ServerCheckpoint.retrieve_checkpoint x)
  | DELETE_CHECKPOINT x -> (env, ServerCheckpoint.delete_checkpoint x)
  | STATS -> (env, Stats.get_stats ())
  | FORMAT (content, from, to_) ->
    let legacy_format_options =
      { Lsp.DocumentFormatting.tabSize = 2; insertSpaces = true }
    in
    (env, ServerFormat.go ~content from to_ legacy_format_options)
  | DUMP_FULL_FIDELITY_PARSE file -> (env, FullFidelityParseService.go file)
  | OPEN_FILE (path, contents) ->
    let predeclare = genv.local_config.ServerLocalConfig.predeclare_ide in
    (ServerFileSync.open_file ~predeclare env path contents, ())
  | CLOSE_FILE path -> (ServerFileSync.close_file env path, ())
  | EDIT_FILE (path, edits) ->
    let predeclare = genv.local_config.ServerLocalConfig.predeclare_ide in
    let edits = List.map edits ~f:Ide_api_types.ide_text_edit_to_fc in
    (ServerFileSync.edit_file ~predeclare env path edits, ())
  | IDE_AUTOCOMPLETE (filename, pos, is_manually_invoked) ->
    let pos = pos |> Ide_api_types.ide_pos_to_fc in
    let contents =
      ServerFileSync.get_file_content (ServerCommandTypes.FileName filename)
    in
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx:(Provider_utils.ctx_from_server_env env)
        ~path:(Relative_path.create_detect_prefix filename)
        ~contents
    in
    let results =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerAutoComplete.go_ctx
            ~ctx
            ~entry
            ~sienv:env.ServerEnv.local_symbol_table
            ~is_manually_invoked
            ~line:pos.File_content.line
            ~column:pos.File_content.column
            ~naming_table:env.naming_table)
    in
    (env, results)
  | CODE_ACTIONS (path, range) ->
    let (ctx, entry) = single_ctx_path env path in
    let actions = CodeActionsService.go ~ctx ~entry ~path ~range in
    (env, actions)
  | DISCONNECT -> (ServerFileSync.clear_sync_data env, ())
  | OUTLINE path ->
    ( env,
      ServerCommandTypes.FileName path
      |> ServerFileSync.get_file_content
      |> FileOutline.outline env.popt )
  | IDE_IDLE -> ({ env with ide_idle = true }, ())
  | RAGE -> (env, ServerRage.go genv env)
  | CST_SEARCH { sort_results; input; files_to_search } -> begin
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
  | NO_PRECHECKED_FILES -> (ServerPrecheckedFiles.expand_all env, ())
  | GEN_PREFETCH_DIR dir ->
    (* TODO(bobren) remove dir entirely from saved state job invocation *)
    let _ = dir in
    (env, ServerGenPrefetchDir.go env genv genv.workers)
  | FUN_DEPS_BATCH positions ->
    (env, ServerFunDepsBatch.go genv.workers positions env)
  | LIST_FILES_WITH_ERRORS -> (env, ServerEnv.list_files env)
  | FILE_DEPENDENTS filenames ->
    let files = ServerFileDependents.go genv env filenames in
    (env, files)
  | IDENTIFY_TYPES (labelled_file, line, column) ->
    let (path, file_input) =
      ServerCommandTypesUtils.extract_labelled_file labelled_file
    in
    let (ctx, entry) = single_ctx env path file_input in
    let result =
      ServerTypeDefinition.go_quarantined ~ctx ~entry ~line ~column
    in
    (env, result)
  | EXTRACT_STANDALONE target ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerExtractStandalone.go ctx target)
  | CONCATENATE_ALL paths -> (env, ServerConcatenateAll.go genv env paths)
  | GO_TO_DEFINITION (labelled_file, line, column) ->
    let (path, file_input) =
      ServerCommandTypesUtils.extract_labelled_file labelled_file
    in
    let (ctx, entry) = single_ctx env path file_input in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        (env, ServerGoToDefinition.go_quarantined ~ctx ~entry ~line ~column))
  | PREPARE_CALL_HIERARCHY (labelled_file, line, column) ->
    let (path, file_input) =
      ServerCommandTypesUtils.extract_labelled_file labelled_file
    in
    let (ctx, entry) = single_ctx env path file_input in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        ( env,
          ServerPrepareCallHierarchy.go_quarantined ~ctx ~entry ~line ~column ))
  | CALL_HIERARCHY_INCOMING_CALLS call_item ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let result =
      ServerCallHierarchyIncomingCalls.go call_item ~ctx ~genv ~env
    in
    (env, result)
  | CALL_HIERARCHY_OUTGOING_CALLS call_item ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let result = ServerCallHierarchyOutgoingCalls.go call_item ~ctx in
    (env, result)
  | VERBOSE verbose ->
    if verbose then
      Hh_logger.Level.set_min_level Hh_logger.Level.Debug
    else
      Hh_logger.Level.set_min_level
        genv.local_config.ServerLocalConfig.min_log_level;
    (env, ())
  | PAUSE pause ->
    let env =
      if pause then
        {
          env with
          full_recheck_on_file_changes =
            Paused { paused_recheck_id = env.init_env.recheck_id };
        }
      else
        { env with full_recheck_on_file_changes = Resumed }
    in
    (env, ())
  | GLOBAL_INFERENCE (submode, files) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (* We are getting files in the reverse order*)
    let files = List.rev files in
    (env, ServerGlobalInference.execute ctx submode files)
  | DEPS_OUT_BATCH positions ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerDepsOutBatch.go ctx positions)
  | DEPS_IN_BATCH positions ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerDepsInBatch.go ~ctx ~genv ~env positions)
