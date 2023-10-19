(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception GetProjectMetadataError of string

let main (env : ClientEnv.client_check_env) (config : ServerLocalConfig.t) :
    Exit_status.t Lwt.t =
  let {
    ClientEnv.root;
    ignore_hh_version;
    saved_state_ignore_hhconfig = _;
    config = _;
    autostart = _;
    custom_hhi_path = _;
    custom_telemetry_data = _;
    error_format = _;
    force_dormant_start = _;
    from = _;
    show_spinner = _;
    gen_saved_ignore_type_errors = _;
    paths = _;
    log_inference_constraints = _;
    max_errors = _;
    mode = _;
    no_load = _;
    save_64bit = _;
    save_human_readable_64bit_dep_map = _;
    output_json = _;
    prechecked = _;
    mini_state = _;
    sort_results = _;
    stdin_name = _;
    deadline = _;
    watchman_debug_logging = _;
    allow_non_opt_build = _;
    desc = _;
  } =
    env
  in
  let%lwt result =
    State_loader_lwt.get_project_metadata
      ~progress_callback:(fun _ -> ())
      ~repo:root
      ~ignore_hh_version
      ~opts:config.ServerLocalConfig.saved_state
  in
  match result with
  | Error (error, _telemetry) ->
    raise
      (GetProjectMetadataError
         (Saved_state_loader.LoadError.debug_details_of_error error))
  | Ok (project_metadata, _telemetry) ->
    Printf.printf "%s\n%!" project_metadata;
    Lwt.return Exit_status.No_error
