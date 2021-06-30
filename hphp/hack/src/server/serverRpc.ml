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

let remove_dead_fixme_warning =
  "hh_server was started without '--no-load', which is required when removing dead fixmes.\n"
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
      (Errors.get_error_list env.errorl |> List.map ~f:Errors.get_code);
    let error_list = Errors.get_sorted_error_list env.errorl in
    let error_list = List.map ~f:Errors.to_absolute error_list in
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
        ~f:ServerEnv.recheck_loop_stats_to_user_telemetry
    in
    ( env,
      {
        Server_status.liveness;
        has_unsaved_changes;
        error_list;
        dropped_count;
        last_recheck_stats;
      } )
  | STATUS_SINGLE (fn, max_errors) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, take_max_errors (ServerStatusSingle.go fn ctx) max_errors)
  | STATUS_SINGLE_REMOTE_EXECUTION fn ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let (errors, dep_edges) = ServerStatusSingleRemoteExecution.go fn ctx in
    (env, (errors, dep_edges))
  | STATUS_REMOTE_EXECUTION (_, max_errors) ->
    (* let ctx = Provider_utils.ctx_from_server_env env in *)
    let errors = ServerStatusRemoteExecution.go env in
    let (error_list, dropped_count) = take_max_errors errors max_errors in
    (env, (error_list, dropped_count))
  | STATUS_MULTI_REMOTE_EXECUTION fns ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let (errors, dep_edges) = ServerStatusMultiRemoteExecution.go fns ctx in
    (env, (errors, dep_edges))
  | COVERAGE_LEVELS (path, file_input) ->
    let path = Relative_path.create_detect_prefix path in
    let (ctx, entry) = single_ctx env path file_input in
    (env, ServerColorFile.go_quarantined ~ctx ~entry)
  | INFER_TYPE (file_input, line, column, dynamic_view) ->
    let path =
      match file_input with
      | FileName fn -> Relative_path.create_detect_prefix fn
      | FileContent _ -> Relative_path.create_detect_prefix ""
    in
    let (ctx, entry) = single_ctx env path file_input in
    let ctx =
      Provider_context.map_tcopt ctx ~f:(fun tcopt ->
          { tcopt with GlobalOptions.tco_dynamic_view = dynamic_view })
    in
    let result =
      Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
          ServerInferType.go_ctx ~ctx ~entry ~line ~column)
    in
    (env, result)
  | INFER_TYPE_BATCH (positions, dynamic_view) ->
    let tcopt = env.ServerEnv.tcopt in
    let tcopt = { tcopt with GlobalOptions.tco_dynamic_view = dynamic_view } in
    let env = { env with tcopt } in
    (env, ServerInferTypeBatch.go genv.workers positions env)
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
        is_xhp_classname = false;
        is_instance_member = false;
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
        ~disable_nontoplevel_declarations:false
        ~disable_legacy_soft_typehints:false
        ~allow_new_attribute_syntax:false
        ~disable_legacy_attribute_syntax:false
        ~enable_xhp_class_modifier:false
        ~disable_xhp_element_mangling:false
        ~disallow_hash_comments:true
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
    in
    (env, result.With_complete_flag.value)
  | IDENTIFY_SYMBOL arg ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let get_def_opt type_ name =
      ServerSymbolDefinition.go
        ctx
        None
        SymbolOccurrence.{ type_; name; is_declaration = false; pos = Pos.none }
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
            get_def_opt (SymbolOccurrence.Method (c_name, member)) "";
            get_def_opt (SymbolOccurrence.Property (c_name, member)) "";
            get_def_opt (SymbolOccurrence.XhpLiteralAttr (c_name, member)) "";
            get_def_opt (SymbolOccurrence.ClassConst (c_name, member)) "";
            get_def_opt (SymbolOccurrence.Typeconst (c_name, member)) "";
          ]
      | [name] ->
        let name = Utils.add_ns name in
        List.concat
          [
            get_def_opt (SymbolOccurrence.Class SymbolOccurrence.ClassId) name;
            (* SymbolOccurrence.Record and Class find the same things *)
            get_def_opt SymbolOccurrence.Function name;
            get_def_opt SymbolOccurrence.GConst name;
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
  | REFACTOR refactor_action ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        ServerRefactor.go ctx refactor_action genv env)
  | IDE_REFACTOR
      { ServerCommandTypes.Ide_refactor_type.filename; line; char; new_name } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f:(fun () ->
        let open Done_or_retry in
        match
          ServerRefactor.go_ide ctx (filename, line, char) new_name genv env
        with
        | Error e -> (env, Done (Error e))
        | Ok r -> map_env r ~f:(fun x -> Ok x))
  | REMOVE_DEAD_FIXMES codes ->
    if genv.ServerEnv.options |> ServerArgs.no_load then (
      HackEventLogger.check_response
        (Errors.get_error_list env.errorl |> List.map ~f:Errors.get_code);
      (env, `Ok (ServerRefactor.get_fixme_patches codes env))
    ) else
      (env, `Error remove_dead_fixme_warning)
  | REWRITE_LAMBDA_PARAMETERS files ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerRefactor.get_lambda_parameter_rewrite_patches ctx files)
  | REWRITE_TYPE_PARAMS_TYPE files ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerRefactor.get_type_params_type_rewrite_patches ctx files)
  | DUMP_SYMBOL_INFO file_list ->
    (env, SymbolInfoService.go genv.workers file_list env)
  | IN_MEMORY_DEP_TABLE_SIZE ->
    (env, SaveStateService.get_in_memory_dep_table_entry_count ())
  | SAVE_NAMING filename ->
    (env, SaveStateService.go_naming env.naming_table filename)
  | SAVE_STATE
      (filename, gen_saved_ignore_type_errors, replace_state_after_saving) ->
    if Errors.is_empty env.errorl || gen_saved_ignore_type_errors then
      let save_decls =
        genv.local_config.ServerLocalConfig.store_decls_in_saved_state
      in
      ( env,
        SaveStateService.go
          ~save_decls
          genv
          env
          filename
          ~replace_state_after_saving )
    else
      (env, Error "There are typecheck errors; cannot generate saved state.")
  | SEARCH (query, type_) ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let lst = env.ServerEnv.local_symbol_table in
    (env, ServerSearch.go ctx query ~kind_filter:type_ lst)
  | COVERAGE_COUNTS path -> (env, ServerCoverageMetric.go path genv env)
  | LINT fnl ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go genv ctx fnl)
  | LINT_STDIN { filename; contents } ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.go_stdin ctx ~filename ~contents)
  | LINT_ALL code ->
    let ctx = Provider_utils.ctx_from_server_env env in
    (env, ServerLint.lint_all genv ctx code)
  | LINT_XCONTROLLER fnl -> (env, ServerLint.go_xcontroller genv env fnl)
  | CREATE_CHECKPOINT x -> (env, ServerCheckpoint.create_checkpoint x)
  | RETRIEVE_CHECKPOINT x -> (env, ServerCheckpoint.retrieve_checkpoint x)
  | DELETE_CHECKPOINT x -> (env, ServerCheckpoint.delete_checkpoint x)
  | STATS -> (env, Stats.get_stats ())
  | FORMAT (content, from, to_) ->
    let legacy_format_options =
      { Lsp.DocumentFormatting.tabSize = 2; insertSpaces = true }
    in
    (env, ServerFormat.go ~content from to_ legacy_format_options)
  | AI_QUERY _ -> (env, "Ai_query is deprecated")
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
            ~column:pos.File_content.column)
    in
    (env, results)
  | IDE_FFP_AUTOCOMPLETE (path, pos) ->
    let pos = pos |> Ide_api_types.ide_pos_to_fc in
    let contents =
      ServerFileSync.get_file_content (ServerCommandTypes.FileName path)
    in
    let offset = File_content.get_offset contents pos in
    (* will raise if out of bounds *)
    let char_at_pos = File_content.get_char contents offset in
    let ctx = Provider_utils.ctx_from_server_env env in
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx
        ~path:(Relative_path.create_detect_prefix path)
        ~contents
    in
    let result =
      FfpAutocompleteService.auto_complete
        ctx
        entry
        pos
        ~filter_by_token:false
        ~sienv:env.ServerEnv.local_symbol_table
    in
    ( env,
      {
        AutocompleteTypes.completions = result;
        char_at_pos;
        is_complete = true;
      } )
  | DISCONNECT -> (ServerFileSync.clear_sync_data env, ())
  | SUBSCRIBE_DIAGNOSTIC { id; error_limit } ->
    if TypecheckerOptions.stream_errors env.tcopt then
      (env, ())
    else
      let initial_errors =
        if is_full_check_done env.full_check_status then
          env.errorl
        else
          Errors.empty
      in
      let new_env =
        {
          env with
          diag_subscribe =
            Some (Diagnostic_subscription.of_id id ~initial_errors ?error_limit);
        }
      in
      let () = Hh_logger.log "Diag_subscribe: SUBSCRIBE %d" id in
      (new_env, ())
  | UNSUBSCRIBE_DIAGNOSTIC id ->
    let diag_subscribe =
      match env.diag_subscribe with
      | Some x when Diagnostic_subscription.get_id x = id ->
        let () = Hh_logger.log "Diag_subscribe: UNSUBSCRIBE %d" id in
        None
      | x ->
        let () = Hh_logger.log "Diag_subscribe: UNSUBSCRIBE %d no effect" id in
        x
    in
    let new_env = { env with diag_subscribe } in
    (new_env, ())
  | OUTLINE path ->
    ( env,
      ServerCommandTypes.FileName path
      |> ServerFileSync.get_file_content
      |> FileOutline.outline env.popt )
  | IDE_IDLE -> ({ env with ide_idle = true }, ())
  | RAGE -> (env, ServerRage.go genv env)
  | DYNAMIC_VIEW toggle -> (ServerFileSync.toggle_dynamic_view env toggle, ())
  | CST_SEARCH { sort_results; input; files_to_search } ->
    begin
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
      | e ->
        let msg = Exn.to_string e in
        let stack = Printexc.get_backtrace () in
        (env, Error (Printf.sprintf "%s\n%s" msg stack))
    end
  | NO_PRECHECKED_FILES -> (ServerPrecheckedFiles.expand_all env, ())
  | GEN_HOT_CLASSES threshold ->
    (env, ServerHotClasses.go genv.workers env threshold)
  | FUN_DEPS_BATCH (positions, dynamic_view) ->
    let tcopt = env.ServerEnv.tcopt in
    let tcopt = { tcopt with GlobalOptions.tco_dynamic_view = dynamic_view } in
    let env = { env with tcopt } in
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
  | BIGCODE path ->
    let (ctx, entry) = single_ctx_path env path in
    let result = ServerBigCode.go_ctx ~ctx ~entry in
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
