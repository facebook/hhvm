(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open ServerEnv
open ServerCommandTypes

let handle : type a. genv -> env -> is_stale:bool -> a t -> env * a =
  fun genv env ~is_stale -> function
    | STATUS ->
      HackEventLogger.check_response (Errors.get_error_list env.errorl);
        let el = Errors.get_sorted_error_list env.errorl in
        let el = List.map ~f:Errors.to_absolute el in
        env, ((if is_stale then Stale_status else Live_status), el)
    | COVERAGE_LEVELS fn -> env, ServerColorFile.go env fn
    | INFER_TYPE (fn, line, char) ->
        env, ServerInferType.go env (fn, line, char)
    | AUTOCOMPLETE content ->
        let result = try
          ServerAutoComplete.auto_complete env.tcopt content
          with Decl.Decl_not_found s ->
            let s = s ^ "-- Autocomplete File contents: " ^ content in
            Printexc.print_backtrace stderr;
            raise (Decl.Decl_not_found s)
        in
        env, result
    | IDENTIFY_FUNCTION (file_input, line, char) ->
        let content = ServerFileSync.get_file_content file_input in
        env, ServerIdentifyFunction.go_absolute content line char env.tcopt
    | GET_DEFINITION_BY_ID id ->
        env, Option.map (ServerSymbolDefinition.from_symbol_id env.tcopt id)
          SymbolDefinition.to_absolute
    | METHOD_JUMP (class_, find_children) ->
      env, MethodJumps.get_inheritance env.tcopt class_ ~find_children
        env.files_info genv.workers
    | FIND_DEPENDENT_FILES file_list ->
        env, Ai.ServerFindDepFiles.go genv.workers file_list
          (ServerArgs.ai_mode genv.options)
    | FIND_REFS find_refs_action ->
        if ServerArgs.ai_mode genv.options = None then
          env, ServerFindRefs.go find_refs_action genv env
        else
          env, Ai.ServerFindRefs.go find_refs_action genv env
    | IDE_FIND_REFS (input, line, char) ->
        let content = ServerFileSync.get_file_content input in
        env, ServerFindRefs.go_from_file (content, line, char) genv env
    | IDE_HIGHLIGHT_REFS (input, line, char) ->
        let content = ServerFileSync.get_file_content input in
        env, ServerHighlightRefs.go (content, line, char) env.tcopt
    | REFACTOR refactor_action ->
        env, ServerRefactor.go refactor_action genv env
    | REMOVE_DEAD_FIXMES codes ->
      HackEventLogger.check_response (Errors.get_error_list env.errorl);
      env, ServerRefactor.get_fixme_patches codes env
    | IGNORE_FIXMES files ->
      let paths = List.map files (Relative_path.concat Relative_path.Root) in
      let disk_needs_parsing =
        List.fold_left
          paths
          ~init:env.disk_needs_parsing
          ~f:Relative_path.Set.add
      in
      Errors.set_ignored_fixmes (Some paths);
      let original_env = env in
      let env = {env with disk_needs_parsing} in
      (* Everything should happen on the master process *)
      let genv = {genv with workers = None} in
      let env, _, _ = ServerTypeCheck.(check genv env Full_check) in
      let el = Errors.get_sorted_error_list env.errorl in
      let el = List.map ~f:Errors.to_absolute el in
      Errors.set_ignored_fixmes None;
      original_env, el
    | DUMP_SYMBOL_INFO file_list ->
        env, SymbolInfoService.go genv.workers file_list env
    | DUMP_AI_INFO file_list ->
        env, Ai.InfoService.go Typing_check_utils.check_defs genv.workers
          file_list (ServerArgs.ai_mode genv.options) env.tcopt
    | SEARCH (query, type_) ->
        env, ServerSearch.go env.tcopt genv.workers query type_
    | COVERAGE_COUNTS path -> env, ServerCoverageMetric.go path genv env
    | LINT fnl -> env, ServerLint.go genv env fnl
    | LINT_ALL code -> env, ServerLint.lint_all genv env code
    | CREATE_CHECKPOINT x -> env, ServerCheckpoint.create_checkpoint x
    | RETRIEVE_CHECKPOINT x -> env, ServerCheckpoint.retrieve_checkpoint x
    | DELETE_CHECKPOINT x -> env, ServerCheckpoint.delete_checkpoint x
    | STATS -> env, Stats.get_stats ()
    | KILL -> env, ()
    | FORMAT (content, from, to_) ->
        env, ServerFormat.go genv content from to_
    | IDE_FORMAT {Ide_api_types.range_filename; file_range} ->
        let content = ServerFileSync.get_file_content
          (ServerUtils.FileName range_filename) in
        env, ServerFormat.go_ide genv content file_range
    | TRACE_AI action ->
        env, Ai.TraceService.go action Typing_check_utils.check_defs
           (ServerArgs.ai_mode genv.options) env.tcopt
    | AI_QUERY json ->
        env, Ai.QueryService.go json
    | DUMP_FULL_FIDELITY_PARSE file ->
        env, FullFidelityParseService.go file
    | OPEN_FILE (path, contents) ->
        ServerFileSync.open_file env path contents, ()
    | CLOSE_FILE path ->
        ServerFileSync.close_file env path, ()
    | EDIT_FILE (path, edits) ->
        ServerFileSync.edit_file env path edits, ()
    | IDE_AUTOCOMPLETE (path, pos) ->
        let open Ide_api_types in
        let fc = ServerFileSync.get_file_content (ServerUtils.FileName path) in
        let edits = [{range = Some {st = pos; ed = pos}; text = "AUTO332"}] in
        let content = File_content.edit_file_unsafe fc edits in
        env, ServerAutoComplete.auto_complete env.tcopt content
    | DISCONNECT ->
        ServerFileSync.clear_sync_data env, ()
    | SUBSCRIBE_DIAGNOSTIC id ->
        let new_env = { env with
          diag_subscribe = Some (Diagnostic_subscription.of_id id env.errorl)
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
      env, ServerUtils.FileName path |>
      ServerFileSync.get_file_content |>
      FileOutline.outline env.popt
