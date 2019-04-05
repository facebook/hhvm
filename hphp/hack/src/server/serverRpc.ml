(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerEnv
open ServerCommandTypes
open Utils

let remove_dead_fixme_warning = (
  "hh_server was started without '--no-load', which is required when removing dead fixmes.\n" ^
  "Please run 'hh_client restart --no-load' to restart it."
)

let handle : type a. genv -> env -> is_stale:bool -> a t -> env * a =
  fun genv env ~is_stale -> function
    | STATUS _ ->
        HackEventLogger.check_response (Errors.get_error_list env.errorl);
        let error_list = Errors.get_sorted_error_list env.errorl in
        let error_list = List.map ~f:Errors.to_absolute error_list in
        let liveness = (if is_stale then Stale_status else Live_status) in
        let has_unsaved_changes = ServerFileSync.has_unsaved_changes env in
        env, { Server_status.liveness; has_unsaved_changes; error_list; }
    | STATUS_SINGLE fn -> env, ServerStatusSingle.go fn env.tcopt
    | COVERAGE_LEVELS fn ->
        env, ServerColorFile.go env.ServerEnv.tcopt env.ServerEnv.naming_table fn
    | INFER_TYPE (fn, line, char, dynamic_view) ->
        env, ServerInferType.go env (fn, line, char, dynamic_view)
    | INFER_TYPE_BATCH (positions, dynamic_view) ->
        let tcopt = env.ServerEnv.tcopt in
        let tcopt = { tcopt with GlobalOptions.tco_dynamic_view=dynamic_view } in
        let env = { env with tcopt } in
        env, ServerInferTypeBatch.go genv.workers positions env
    | TYPED_AST (filename) ->
        env, ServerTypedAst.go env filename
    | IDE_HOVER (fn, line, char) ->
        let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        env, ServerHover.go env (fn, line, char) ~basic_only
    | DOCBLOCK_AT (filename, line, char, base_class_name) ->
        let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        env, ServerDocblockAt.go_location env (filename, line, char) ~base_class_name
          ~basic_only
    | IDE_SIGNATURE_HELP (fn, line, char) ->
        let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        env, ServerSignatureHelp.go env (fn, line, char) ~basic_only
    | AUTOCOMPLETE content ->
      let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        let autocomplete_context = { AutocompleteTypes.
          is_manually_invoked = false;
          is_xhp_classname = false;
          is_instance_member = false;
          is_after_single_colon = false;
          is_after_double_right_angle_bracket = false;
          is_after_open_square_bracket = false;
          is_after_quote = false;
        } in (* feature not implemented here; it only works for LSP *)
        let result = ServerAutoComplete.auto_complete
          ~tcopt:env.tcopt ~delimit_on_namespaces:false ~autocomplete_context ~basic_only content in
        env, result.With_complete_flag.value
    | IDENTIFY_FUNCTION (file_input, line, char) ->
        let content = ServerFileSync.get_file_content file_input in
        env, ServerIdentifyFunction.go_absolute content line char env.tcopt
    | METHOD_JUMP (class_, filter, find_children) ->
      env, MethodJumps.get_inheritance class_ ~filter ~find_children
        env.naming_table genv.workers
    | METHOD_JUMP_BATCH (classes, filter) ->
      env, ServerMethodJumpsBatch.go genv.workers classes filter
    | FIND_REFS find_refs_action ->
        let open Done_or_retry in
        let include_defs = false in
        ServerFindRefs.(
          go find_refs_action include_defs genv env |>
          map_env ~f:to_absolute
        )
    | IDE_FIND_REFS (labelled_file, line, char, include_defs) ->
        let open Done_or_retry in
        ServerFindRefs.(
          match go_from_file (labelled_file, line, char) env with
          | None -> env, Done None
          | Some (name, action) -> map_env ~f:(to_ide name) (go action include_defs genv env)
        )
    | IDE_HIGHLIGHT_REFS (input, line, char) ->
        let content = ServerFileSync.get_file_content input in
        env, ServerHighlightRefs.go (content, line, char) env.tcopt
    | REFACTOR refactor_action ->
        ServerRefactor.go refactor_action genv env
    | IDE_REFACTOR { ServerCommandTypes.Ide_refactor_type.filename; line; char; new_name } ->
        let open Done_or_retry in
        begin match ServerRefactor.go_ide (filename, line, char) new_name genv env with
          | Error e -> env, Done (Error e)
          | Ok r -> map_env r ~f:(fun x -> Ok x)
        end
    | REMOVE_DEAD_FIXMES codes ->
      if genv.ServerEnv.options |> ServerArgs.no_load then begin
        HackEventLogger.check_response (Errors.get_error_list env.errorl);
        env, `Ok (ServerRefactor.get_fixme_patches codes env)
      end else
        env, (`Error remove_dead_fixme_warning)
    | REWRITE_LAMBDA_PARAMETERS files ->
        env, ServerRefactor.get_lambda_parameter_rewrite_patches env files
    | DUMP_SYMBOL_INFO file_list ->
        env, SymbolInfoService.go genv.workers file_list env
    | IN_MEMORY_DEP_TABLE_SIZE ->
      env, (SaveStateService.get_in_memory_dep_table_entry_count ())
    | SAVE_STATE (
        filename,
        gen_saved_ignore_type_errors,
        file_info_on_disk,
        replace_state_after_saving) ->
      if Errors.is_empty env.errorl || gen_saved_ignore_type_errors then
        let save_decls =
          genv.local_config.ServerLocalConfig.store_decls_in_saved_state in
        env,
        SaveStateService.go
          ~file_info_on_disk
          ~save_decls
          env.ServerEnv.naming_table
          env.errorl
          filename
          ~replace_state_after_saving
      else
        env, Error "There are typecheck errors; cannot generate saved state."
    | SEARCH (query, type_) ->
       env, ServerSearch.go genv.workers query type_
    | COVERAGE_COUNTS path -> env, ServerCoverageMetric.go path genv env
    | LINT fnl -> env, ServerLint.go genv env fnl
    | LINT_STDIN { filename; contents } ->
        env, ServerLint.go_stdin env ~filename ~contents
    | LINT_ALL code -> env, ServerLint.lint_all genv env code
    | LINT_XCONTROLLER fnl -> env, ServerLint.go_xcontroller genv env fnl
    | CREATE_CHECKPOINT x -> env, ServerCheckpoint.create_checkpoint x
    | RETRIEVE_CHECKPOINT x -> env, ServerCheckpoint.retrieve_checkpoint x
    | DELETE_CHECKPOINT x -> env, ServerCheckpoint.delete_checkpoint x
    | STATS -> env, Stats.get_stats ()
    | FORMAT (content, from, to_) ->
        let legacy_format_options =
          { Lsp.DocumentFormatting.
            tabSize = 2;
            insertSpaces = true;
          } in
        env, ServerFormat.go content from to_ legacy_format_options
    | AI_QUERY json ->
        env, Ai.QueryService.go json
    | DUMP_FULL_FIDELITY_PARSE file ->
        env, FullFidelityParseService.go file
    | OPEN_FILE (path, contents) ->
        let predeclare = genv.local_config.ServerLocalConfig.predeclare_ide in
        ServerFileSync.open_file ~predeclare env path contents, ()
    | CLOSE_FILE path ->
        ServerFileSync.close_file env path, ()
    | EDIT_FILE (path, edits) ->
        let predeclare = genv.local_config.ServerLocalConfig.predeclare_ide in
        let edits = List.map edits ~f:Ide_api_types.ide_text_edit_to_fc in
        ServerFileSync.edit_file ~predeclare env path edits, ()
    | IDE_AUTOCOMPLETE (path, pos, delimit_on_namespaces, is_manually_invoked) ->
        let open With_complete_flag in
        let pos = pos |> Ide_api_types.ide_pos_to_fc in
        let file_content = ServerFileSync.get_file_content (ServerCommandTypes.FileName path) in
        let offset = File_content.get_offset file_content pos in (* will raise if out of bounds *)
        let char_at_pos = File_content.get_char file_content offset in
        let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        let results = ServerAutoComplete.auto_complete_at_position
          ~delimit_on_namespaces ~is_manually_invoked ~file_content ~basic_only ~pos
          ~tcopt:env.tcopt
        in
        let completions = results.value in
        let is_complete = results.is_complete in
        env, { AutocompleteTypes.completions; char_at_pos; is_complete; }
    | IDE_FFP_AUTOCOMPLETE (path, pos) ->
        let pos = pos |> Ide_api_types.ide_pos_to_fc in
        let content = ServerFileSync.get_file_content (ServerCommandTypes.FileName path) in
        let offset = File_content.get_offset content pos in (* will raise if out of bounds *)
        let char_at_pos = File_content.get_char content offset in
        let basic_only = genv.local_config.ServerLocalConfig.basic_autocomplete_only in
        let result =
          FfpAutocompleteService.auto_complete env.tcopt content pos
            ~basic_only ~filter_by_token:false
        in
        env, { AutocompleteTypes.completions = result; char_at_pos; is_complete = true; }
    | DISCONNECT ->
        ServerFileSync.clear_sync_data env, ()
    | SUBSCRIBE_DIAGNOSTIC id ->
        let init = if env.full_check = Full_check_done
          then env.errorl else Errors.empty in
        let new_env = { env with
          diag_subscribe = Some (Diagnostic_subscription.of_id id init)
        } in
        new_env, ()
    | UNSUBSCRIBE_DIAGNOSTIC id ->
        let diag_subscribe = match env.diag_subscribe with
          | Some x when Diagnostic_subscription.get_id x = id -> None
          | x -> x
        in
        let new_env = { env with diag_subscribe } in
        new_env, ()
    | OUTLINE path ->
      env, ServerCommandTypes.FileName path |>
      ServerFileSync.get_file_content |>
      FileOutline.outline env.popt
    | IDE_IDLE ->
      {env with ide_idle = true;}, ()
    | RAGE ->
      env, ServerRage.go genv env
    | DYNAMIC_VIEW toggle ->
      ServerFileSync.toggle_dynamic_view env toggle, ()
    | INFER_RETURN_TYPE id_info ->
      let open ServerCommandTypes.Infer_return_type in
      begin match id_info with
      | Function fun_name ->
        env, InferReturnTypeService.get_fun_return_ty
          env.tcopt fun_name
      | Method (class_name, meth_name) ->
        env, InferReturnTypeService.get_meth_return_ty
          env.tcopt class_name meth_name
      end
    | CST_SEARCH { sort_results; input; files_to_search }->
      begin try
        env, CstSearchService.go genv env ~sort_results ~files_to_search input
      with
      | MultiThreadedCall.Coalesced_failures failures ->
        let failures = failures
          |> List.map ~f:WorkerController.failure_to_string
          |> String.concat ~sep:"\n"
        in
        env, Error (Printf.sprintf
          "Worker failures - check the logs for more details:\n%s\n" failures)
      | e ->
        let msg = Exn.to_string e in
        let stack = Printexc.get_backtrace () in
        env, Error (Printf.sprintf "%s\n%s" msg stack)
      end
    | NO_PRECHECKED_FILES ->
      ServerPrecheckedFiles.expand_all env, ()
    | GEN_HOT_CLASSES threshold ->
      env, ServerHotClasses.go genv.workers env threshold
    | FUN_DEPS_BATCH (positions, dynamic_view) ->
        let tcopt = env.ServerEnv.tcopt in
        let tcopt = { tcopt with GlobalOptions.tco_dynamic_view=dynamic_view } in
        let env = { env with tcopt } in
        env, ServerFunDepsBatch.go genv.workers positions env
    | FUN_IS_LOCALLABLE_BATCH positions ->
        let env = { env with tcopt = env.ServerEnv.tcopt } in
        env, ServerFunIsLocallableBatch.go genv.workers positions env
    | LIST_FILES_WITH_ERRORS ->
      env, ServerEnv.list_files env
