(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  config: (string * string) list;
}

exception GetProjectMetadataError of string

let main (env : env) (config : ServerLocalConfig.t) : Exit_status.t Lwt.t =
  let { root; ignore_hh_version; saved_state_ignore_hhconfig = _; config = _ } =
    env
  in
  let%lwt result =
    State_loader_lwt.get_project_metadata
      ~progress_callback:(fun _ -> ())
      ~saved_state_type:
        (Saved_state_loader.Naming_and_dep_table
           { naming_sqlite = config.ServerLocalConfig.use_hack_64_naming_table })
      ~repo:root
      ~saved_state_manifold_api_key:
        config.ServerLocalConfig.saved_state_loading
          .GlobalOptions.saved_state_manifold_api_key
      ~ignore_hh_version
  in
  match result with
  | Error (error, _telemetry) ->
    raise
      (GetProjectMetadataError
         (Saved_state_loader.LoadError.debug_details_of_error error))
  | Ok (project_metadata, _telemetry) ->
    Printf.printf "%s\n%!" project_metadata;
    Lwt.return Exit_status.No_error
