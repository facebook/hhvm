(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ServerEnv
open Utils

type file_info = Typing_check_service.file

(* Return all the files that we need to typecheck *)
let make_next_files (genv: ServerEnv.genv) : Relative_path.t list Bucket.next =
  let next_files_root = compose
    (List.map ~f:(Relative_path.(create Root)))
    (genv.indexer FindUtils.file_filter) in
  let hhi_root = Hhi.get_hhi_root () in
  let hhi_filter = FindUtils.is_php in
  let next_files_hhi = compose
    (List.map ~f:(Relative_path.(create Hhi)))
    (Find.make_next_files
       ~name:"hhi" ~filter:hhi_filter hhi_root) in
  let rec concat_next_files l () =
    begin match l with
    | [] -> []
    | hd::tl -> begin match hd () with
      | [] -> concat_next_files tl ()
      | x -> x
      end
    end
  in
  let extra_roots = ServerConfig.extra_paths genv.config in
  let next_files_extra = List.map extra_roots
    (fun root -> compose
      (List.map ~f:Relative_path.create_detect_prefix)
      (Find.make_next_files
        ~filter:FindUtils.file_filter
        root)
    ) |> concat_next_files
  in
  fun () ->
    let next = concat_next_files [next_files_hhi; next_files_extra; next_files_root] () in
    Bucket.of_list next

let is_check_mode (options: ServerArgs.options) : bool =
  ServerArgs.check_mode options &&
  (* Note: we need to run update_files to get an accurate saved state *)
  ServerArgs.save_filename options = None &&
  ServerArgs.save_with_spec options = None

let indexing (genv: ServerEnv.genv) : Relative_path.t list Bucket.next * float =
  ServerProgress.send_progress_to_monitor "indexing";
  let t = Unix.gettimeofday () in
  let get_next = make_next_files genv in
  HackEventLogger.indexing_end t;
  let t = Hh_logger.log_duration "indexing" t in
  get_next, t

let parsing
    ~(lazy_parse: bool)
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    ~(get_next: Relative_path.t list Bucket.next)
    ?(count: int option)
    (t: float)
    ~(trace: bool)
  : ServerEnv.env * float =
  begin match count with
    | None -> ServerProgress.send_progress_to_monitor "%s" "parsing"
    | Some c -> ServerProgress.send_progress_to_monitor "parsing %d files" c
  end;
  let quick = lazy_parse in
  let fast, errorl, _=
    Parsing_service.go
      ~quick
      ~show_all_errors:true
      genv.workers
      Relative_path.Set.empty
      ~get_next
      ~trace
      env.popt in
  let naming_table = Naming_table.update_many env.naming_table fast in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  Stats.(stats.init_parsing_heap_size <- hs);
  (* TODO: log a count of the number of files parsed... 0 is a placeholder *)
  HackEventLogger.parsing_end t hs  ~parsed_count:0;
  let env = { env with
    naming_table;
    errorl = Errors.merge errorl env.errorl;
  } in
  env, (Hh_logger.log_duration "Parsing" t)

let update_files
    (genv: ServerEnv.genv)
    (naming_table: Naming_table.t)
    (t: float)
  : float =
  if is_check_mode genv.options then t else begin
    Naming_table.iter naming_table Typing_deps.update_file;
    HackEventLogger.updating_deps_end t;
    Hh_logger.log_duration "Updating deps" t
  end

let naming (env: ServerEnv.env) (t: float) : ServerEnv.env * float =
  ServerProgress.send_progress_to_monitor "resolving symbol references";
  let env =
    Naming_table.fold env.naming_table ~f:begin fun k v env ->
      let errorl, failed_naming = NamingGlobal.ndecl_file k v in
      { env with
        errorl = Errors.merge errorl env.errorl;
        failed_naming =
          Relative_path.Set.union env.failed_naming failed_naming;
      }
    end ~init:env
  in
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  HackEventLogger.global_naming_end t;
  env, (Hh_logger.log_duration "Naming" t)

let is_path_in_range (path: string) (range: ServerArgs.files_to_check_range): bool =
  (* TODO: not sure how to include the prefix - should we just have these as strings? *)
  let is_from_prefix_incl = match range.ServerArgs.from_prefix_incl with
  | None -> true
  | Some (from_prefix_incl: Relative_path.t) ->
    path >= (Relative_path.suffix from_prefix_incl) in

  let is_to_prefix_excl = match range.ServerArgs.to_prefix_excl with
  | None -> true
  | Some (to_prefix_excl: Relative_path.t) ->
    path < (Relative_path.suffix to_prefix_excl) in

  is_from_prefix_incl && is_to_prefix_excl

let does_path_match_spec path (spec: ServerArgs.files_to_check_spec): bool =
  match spec with
  | ServerArgs.Range range -> is_path_in_range path range
  | ServerArgs.Prefix prefix -> String_utils.string_starts_with path (Relative_path.suffix prefix)

let does_path_match_specs path (specs: ServerArgs.files_to_check_spec list): bool =
  match List.find specs ~f:(fun spec -> does_path_match_spec path spec) with
  | None -> false
  | Some _ -> true

let filter_filenames_by_spec
    (fnl: file_info list)
    (spec: ServerArgs.save_state_spec_info) : file_info list =

    let filtered_filenames =
      List.filter fnl ~f:(fun ((fn: Relative_path.t), _names) ->
        (* TODO: not sure how to include the prefix *)
        does_path_match_specs (Relative_path.suffix fn) spec.ServerArgs.files_to_check) in
    Hh_logger.log "Filtered filenames to %d" (List.length filtered_filenames);
    filtered_filenames

let type_check
    (genv: ServerEnv.genv)
    (env: ServerEnv.env)
    (fast: FileInfo.names Relative_path.Map.t)
    (t: float)
  : ServerEnv.env * float =
  if ServerArgs.ai_mode genv.options <> None then env, t
  else if
    is_check_mode genv.options ||
    (ServerArgs.save_filename genv.options <> None) ||
    (ServerArgs.save_with_spec genv.options <> None)
  then begin
    (* Prechecked files are not supported in AI/check/saving-state modes, we
     * should always recheck everything necessary up-front.*)
    assert (env.prechecked_files = Prechecked_files_disabled);

    let count = Relative_path.Map.cardinal fast in
    let logstring = Printf.sprintf "Filter %d files" count in
    Hh_logger.log "Begin %s" logstring;

    let (files_to_check: file_info list) = Relative_path.Map.elements fast in
    let (files_to_check: file_info list) =
      match ServerArgs.save_with_spec genv.options with
      | None -> files_to_check
      | Some (spec: ServerArgs.save_state_spec_info) ->
        filter_filenames_by_spec files_to_check spec in

    let (_new_t: float) = Hh_logger.log_duration logstring t in

    let count = (List.length files_to_check) in
    let logstring = Printf.sprintf "Type-check %d files" count in
    Hh_logger.log "Begin %s" logstring;

    let (errorl: Decl_service.lazy_decl_result) =
      let memory_cap = genv.local_config.ServerLocalConfig.max_typechecker_worker_memory_mb in
      Typing_check_service.go
        genv.workers
        env.tcopt
        Relative_path.Set.empty
        files_to_check
        ~memory_cap
    in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    HackEventLogger.type_check_end count count t;
    let env = { env with
      errorl = Errors.merge errorl env.errorl;
    } in
    env, (Hh_logger.log_duration logstring t)
  end
  else begin
    let needs_recheck = Relative_path.Map.fold fast
      ~init:Relative_path.Set.empty
      ~f:(fun fn _ acc -> Relative_path.Set.add acc fn)
    in
    let env = { env with
      needs_recheck = Relative_path.Set.union env.needs_recheck needs_recheck;
      (* eagerly start rechecking after init *)
      full_check = Full_check_started;
      init_env = { env.init_env with
        needs_full_init = true;
      };
    } in
    env, t
  end
