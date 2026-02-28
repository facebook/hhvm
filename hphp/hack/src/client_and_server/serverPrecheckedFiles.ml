(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

let should_use options local_config =
  Option.value
    (ServerArgs.prechecked options)
    ~default:local_config.ServerLocalConfig.prechecked_files

let set env prechecked_files = { env with prechecked_files }

let init env ~dirty_local_deps ~dirty_master_deps =
  set
    env
    (Initial_typechecking
       {
         rechecked_files = Relative_path.Set.empty;
         dirty_local_deps;
         dirty_master_deps;
         clean_local_deps = Typing_deps.(DepSet.make ());
       })

(** Update env.needs_recheck and return new value for [dirty_master_deps]
    using the following pseudo-code:
        needs_recheck = files(fanout(fanout(deps) /\ dirty_master_deps)) \ rechecked_files
        dirty_master_deps = dirty_master_deps \ fanout(deps)
  *)
let intersect_with_master_deps
    ~ctx ~deps ~dirty_master_deps ~rechecked_files genv env =
  let t0 = Unix.gettimeofday () in
  let deps_mode = Provider_context.get_deps_mode ctx in
  (* Compute maximum fan-out of input dep set *)
  let deps = Typing_deps.add_all_deps deps_mode deps in
  let t1 = Unix.gettimeofday () in
  (* See if it intersects in any way with dirty_master_deps *)
  let common_deps = Typing_deps.DepSet.inter deps dirty_master_deps in
  let t2 = Unix.gettimeofday () in
  (* Expand the common part *)
  let more_deps = Typing_deps.add_all_deps deps_mode common_deps in
  let t3 = Unix.gettimeofday () in
  (* Remove the common part from dirty_master_deps (because after expanding it's
   * no longer dirty. *)
  let dirty_master_deps =
    Typing_deps.DepSet.diff dirty_master_deps common_deps
  in
  let t4 = Unix.gettimeofday () in
  (* Translate the dependencies to files that need to be rechecked. *)
  let needs_recheck0 = Naming_provider.get_files ctx more_deps in
  let t5 = Unix.gettimeofday () in
  let needs_recheck = Relative_path.Set.diff needs_recheck0 rechecked_files in
  let t6 = Unix.gettimeofday () in
  let size = Relative_path.Set.cardinal needs_recheck in
  let env =
    if size = 0 then
      env
    else (
      ServerRevisionTracker.typing_changed genv.local_config size;
      Hh_logger.log "Adding %d files to recheck" size;
      let needs_recheck =
        Relative_path.Set.union env.needs_recheck needs_recheck
      in
      { env with needs_recheck }
    )
  in
  let duration key tend tstart telemetry =
    Telemetry.int_
      telemetry
      ~key
      ~value:(int_of_float ((tend -. tstart) *. 1000.))
  in
  let telemetry =
    Telemetry.create ()
    |> duration "t1" t1 t0
    |> duration "t2" t2 t1
    |> duration "t3" t3 t2
    |> duration "t4" t4 t3
    |> duration "t5" t5 t4
    |> duration "t6" t6 t5
    |> Telemetry.int_ ~key:"deps" ~value:(Typing_deps.DepSet.cardinal deps)
    |> Telemetry.int_
         ~key:"common_deps"
         ~value:(Typing_deps.DepSet.cardinal common_deps)
    |> Telemetry.int_
         ~key:"more_deps"
         ~value:(Typing_deps.DepSet.cardinal more_deps)
    |> Telemetry.int_
         ~key:"needs_recheck0"
         ~value:(Relative_path.Set.cardinal needs_recheck0)
    |> Telemetry.int_
         ~key:"needs_recheck"
         ~value:(Relative_path.Set.cardinal needs_recheck)
  in
  (env, dirty_master_deps, size, telemetry)

(** Update env.prechecked_files by adding to the recheck_files field of each variant *)
let update_rechecked_files env rechecked =
  let t = Unix.gettimeofday () in
  let add_rechecked (dirty_deps : dirty_deps) : dirty_deps =
    {
      dirty_deps with
      rechecked_files =
        Relative_path.Set.union rechecked dirty_deps.rechecked_files;
    }
  in
  let env =
    set env
    @@
    match env.prechecked_files with
    | Prechecked_files_disabled -> Prechecked_files_disabled
    | Initial_typechecking dirty_deps ->
      Initial_typechecking (add_rechecked dirty_deps)
    | Prechecked_files_ready dirty_deps ->
      Prechecked_files_ready (add_rechecked dirty_deps)
  in
  HackEventLogger.prechecked_update_rechecked t;
  env

(** Update:
      - env.needs_recheck,
      - env.full_check_status,
      - env.prechecked_files,
      - env.init_env.why_needed_full_check
    in esoteric ways
  *)
let update_after_recheck genv env rechecked ~start_time =
  let ctx = Provider_utils.ctx_from_server_env env in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.duration ~key:"start" ~start_time
    |> Telemetry.int_
         ~key:"rechecked"
         ~value:(Relative_path.Set.cardinal rechecked)
  in
  let env = update_rechecked_files env rechecked in
  let telemetry = Telemetry.duration telemetry ~key:"end" ~start_time in
  match (env.full_check_status, env.prechecked_files) with
  | ( Full_check_done,
      Initial_typechecking
        {
          dirty_local_deps;
          dirty_master_deps;
          rechecked_files;
          clean_local_deps;
        } ) ->
    let t = Unix.gettimeofday () in
    assert (Typing_deps.DepSet.is_empty clean_local_deps);
    Hh_logger.log "Finished rechecking dirty files, evaluating their fanout";

    (* Take any prechecked files that could have been affected by local changes
     * and expand them too *)
    let (env, dirty_master_deps, size, _telemetry) =
      intersect_with_master_deps
        ~ctx
        ~deps:dirty_local_deps
        ~dirty_master_deps
        ~rechecked_files
        genv
        env
    in
    let env =
      if size = 0 then
        env
      else
        let full_check_status = Full_check_started in
        let why_needed_full_check =
          ServerEnv.Init_telemetry.make
            ServerEnv.Init_telemetry.Init_prechecked_fanout
            (Telemetry.create ()
            |> Telemetry.float_ ~key:"time" ~value:(Unix.gettimeofday ())
            |> Telemetry.string_ ~key:"reason" ~value:"prechecked_fanout"
            |> Telemetry.object_opt
                 ~key:"prev"
                 ~value:
                   (Option.map
                      env.init_env.why_needed_full_check
                      ~f:ServerEnv.Init_telemetry.get))
          |> Option.some
        in
        let init_env = { env.init_env with why_needed_full_check } in
        { env with init_env; full_check_status }
    in
    let clean_local_deps = dirty_local_deps in
    let dirty_local_deps = Typing_deps.DepSet.make () in
    HackEventLogger.prechecked_evaluate_init t size;
    let telemetry =
      telemetry
      |> Telemetry.duration ~key:"fanout_end" ~start_time
      |> Telemetry.int_ ~key:"size" ~value:size
    in

    let env =
      set
        env
        (Prechecked_files_ready
           {
             dirty_local_deps;
             dirty_master_deps;
             rechecked_files;
             clean_local_deps;
           })
    in
    (env, telemetry)
  | _ -> (env, telemetry)

(** Update:
      - env.needs_recheck,
      - env.full_check_status,
      - env.prechecked_files,
    in esoteric ways
  *)
let update_after_local_changes genv env changes ~start_time =
  let ctx = Provider_utils.ctx_from_server_env env in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.duration ~key:"start" ~start_time
    |> Telemetry.int_
         ~key:"changes"
         ~value:(Typing_deps.DepSet.cardinal changes)
  in
  let (env, telemetry) =
    match env.prechecked_files with
    | Prechecked_files_disabled ->
      let telemetry =
        telemetry |> Telemetry.string_ ~key:"mode" ~value:"disabled"
      in
      (env, telemetry)
    | Initial_typechecking dirty_deps ->
      (* Add [changes] dep set to [dirty_local_deps] *)
      let env =
        set
          env
          (Initial_typechecking
             {
               dirty_deps with
               dirty_local_deps =
                 Typing_deps.DepSet.union changes dirty_deps.dirty_local_deps;
             })
      in
      let telemetry =
        telemetry |> Telemetry.string_ ~key:"mode" ~value:"initial"
      in
      (env, telemetry)
    | Prechecked_files_ready dirty_deps ->
      (* This is cleared during transition from Initial_typechecking to
       * Prechecked_files_ready and should not be populated again *)
      assert (Typing_deps.DepSet.is_empty dirty_deps.dirty_local_deps);
      let changes =
        Typing_deps.DepSet.diff changes dirty_deps.clean_local_deps
      in
      if Typing_deps.DepSet.is_empty changes then
        let telemetry =
          Telemetry.string_ telemetry ~key:"mode" ~value:"ready_empty"
        in
        (env, telemetry)
      else
        let t = Unix.gettimeofday () in
        let clean_local_deps =
          Typing_deps.DepSet.union dirty_deps.clean_local_deps changes
        in
        let (env, dirty_master_deps, size, intersect_telemetry) =
          intersect_with_master_deps
            ~ctx
            ~deps:changes
            ~dirty_master_deps:dirty_deps.dirty_master_deps
            ~rechecked_files:dirty_deps.rechecked_files
            genv
            env
        in
        let env =
          if size = 0 then
            env
          else
            let full_check_status =
              match env.full_check_status with
              | Full_check_done -> Full_check_needed
              | x -> x
            in
            { env with full_check_status }
        in
        let telemetry =
          telemetry
          |> Telemetry.string_ ~key:"mode" ~value:"ready_changes"
          |> Telemetry.int_ ~key:"size" ~value:size
          |> Telemetry.int_
               ~key:"changes"
               ~value:(Typing_deps.DepSet.cardinal changes)
          |> Telemetry.int_
               ~key:"dirty_local_deps"
               ~value:(Typing_deps.DepSet.cardinal dirty_deps.dirty_local_deps)
          |> Telemetry.int_
               ~key:"dirty_master_deps"
               ~value:(Typing_deps.DepSet.cardinal dirty_deps.dirty_master_deps)
          |> Telemetry.int_
               ~key:"dirty_master_deps_new"
               ~value:(Typing_deps.DepSet.cardinal dirty_master_deps)
          |> Telemetry.int_
               ~key:"rechecked_files"
               ~value:(Relative_path.Set.cardinal dirty_deps.rechecked_files)
          |> Telemetry.int_
               ~key:"clean_local_deps"
               ~value:(Typing_deps.DepSet.cardinal dirty_deps.clean_local_deps)
          |> Telemetry.int_
               ~key:"clean_local_deps_new"
               ~value:(Typing_deps.DepSet.cardinal clean_local_deps)
          |> Telemetry.int_
               ~key:"needs_recheck_new"
               ~value:(Relative_path.Set.cardinal env.needs_recheck)
          |> Telemetry.string_
               ~key:"full_check_status"
               ~value:(ServerEnv.show_full_check_status env.full_check_status)
          |> Telemetry.object_ ~key:"intersect" ~value:intersect_telemetry
        in
        HackEventLogger.prechecked_evaluate_incremental t size;
        let env =
          set
            env
            (Prechecked_files_ready
               { dirty_deps with dirty_master_deps; clean_local_deps })
        in
        (env, telemetry)
  in
  let telemetry = Telemetry.duration telemetry ~key:"end" ~start_time in
  (env, telemetry)

let expand_all env =
  match env.prechecked_files with
  | Prechecked_files_disabled -> env
  | Initial_typechecking dirty_deps
  | Prechecked_files_ready dirty_deps ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let deps_mode = Provider_context.get_deps_mode ctx in
    let deps =
      Typing_deps.add_all_deps deps_mode dirty_deps.dirty_master_deps
    in
    let needs_recheck = Naming_provider.get_files ctx deps in
    let needs_recheck =
      Relative_path.Set.diff needs_recheck dirty_deps.rechecked_files
    in
    let env =
      if Relative_path.Set.is_empty needs_recheck then
        env
      else (
        Hh_logger.log
          "Adding %d files to recheck after expanding all master deps"
          (Relative_path.Set.cardinal needs_recheck);
        let needs_recheck =
          Relative_path.Set.union env.needs_recheck needs_recheck
        in
        { env with needs_recheck; full_check_status = Full_check_started }
      )
    in
    set
      env
      (Prechecked_files_ready
         { dirty_deps with dirty_master_deps = Typing_deps.DepSet.make () })
