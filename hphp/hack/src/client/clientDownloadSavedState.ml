(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Ppx_yojson_conv_lib.Yojson_conv.Primitives

type env = {
  root: Path.t;
  from: string;
  saved_state_manifold_api_key: string option;
  should_save_replay: bool;
  replay_token: string option;
}

type 'additional_info replay_info = {
  manifold_path: string;
  changed_files: Saved_state_loader.changed_files;
  is_cached: bool;
  additional_info: 'additional_info;
}
[@@deriving yojson]

let changed_files_to_absolute_paths_json
    (changed_files : Saved_state_loader.changed_files) : Hh_json.json =
  changed_files
  |> List.map ~f:Relative_path.to_absolute
  |> Hh_json_helpers.Jprint.string_array

let print_load_error (load_error : Saved_state_loader.LoadError.t) : unit =
  let json =
    Hh_json.JSON_Object
      [
        ( "error",
          Hh_json.string_
            (Saved_state_loader.LoadError.debug_details_of_error load_error) );
      ]
  in
  Hh_json.json_to_multiline_output stdout json

let get_replay_info (replay_token : string) :
    Saved_state_loader.Naming_and_dep_table_info.additional_info replay_info
    Lwt.t =
  let%lwt replay_info = Clowder_paste.clowder_download replay_token in
  match replay_info with
  | Ok stdout ->
    let json = Yojson.Safe.from_string stdout in
    Lwt.return
      (replay_info_of_yojson
         Saved_state_loader.Naming_and_dep_table_info.additional_info_of_yojson
         json)
  | Error message ->
    failwith
      (Printf.sprintf
         "Could not get replay info from Clowder handle %s: %s"
         replay_token
         message)

let make_replay_token_json
    ~(manifold_path : string)
    ~(changed_files : Saved_state_loader.changed_files)
    ~(is_cached : bool)
    ~(additional_info :
       Saved_state_loader.Naming_and_dep_table_info.additional_info) :
    Yojson.Safe.t =
  yojson_of_replay_info
    Saved_state_loader.Naming_and_dep_table_info.yojson_of_additional_info
    { manifold_path; changed_files; is_cached; additional_info }

let make_replay_token
    ~(env : env)
    ~(manifold_path : string)
    ~(changed_files : Saved_state_loader.changed_files)
    ~(is_cached : bool)
    ~(additional_info :
       Saved_state_loader.Naming_and_dep_table_info.additional_info) :
    string option Lwt.t =
  match (env.should_save_replay, env.replay_token) with
  | (false, (Some _ | None)) -> Lwt.return_none
  | (true, Some replay_token) ->
    (* No need to generate a new replay token in this case, as it would
       contain the same data as we already have. *)
    Lwt.return_some replay_token
  | (true, None) ->
    let json =
      make_replay_token_json
        ~manifold_path
        ~changed_files
        ~is_cached
        ~additional_info
    in
    let%lwt clowder_result =
      Clowder_paste.clowder_upload_and_get_handle
        (Yojson.Safe.pretty_to_string json)
    in
    (match clowder_result with
    | Ok handle -> Lwt.return_some handle
    | Error message ->
      Hh_logger.error "Failed to generate replay token from Clowder: %s" message;
      Lwt.return_none)

let load_saved_state ~(env : env) ~(local_config : ServerLocalConfig.t) :
    ( Saved_state_loader.load_result,
      Saved_state_loader.LoadError.t )
    Lwt_result.t =
  let ssopt =
    {
      local_config.ServerLocalConfig.saved_state with
      GlobalOptions.loading =
        {
          local_config.ServerLocalConfig.saved_state.GlobalOptions.loading with
          GlobalOptions.log_saved_state_age_and_distance = false;
          saved_state_manifold_api_key = env.saved_state_manifold_api_key;
        };
    }
  in
  match env.replay_token with
  | None ->
    let watchman_opts =
      Saved_state_loader.Watchman_options.{ root = env.root; sockname = None }
    in
    let%lwt result =
      State_loader_lwt.load
        ~ssopt
        ~progress_callback:(fun _ -> ())
        ~watchman_opts
        ~ignore_hh_version:false
    in
    Lwt.return result
  | Some replay_token ->
    let%lwt ({ manifold_path; changed_files; is_cached; additional_info }
              : _ replay_info) =
      get_replay_info replay_token
    in
    let download_dir = State_loader_lwt.prepare_download_dir () in
    let target_path =
      State_loader_lwt.get_saved_state_target_path ~download_dir ~manifold_path
    in
    let%lwt result =
      State_loader_lwt.download_and_unpack_saved_state_from_manifold
        ~ssopt:ssopt.GlobalOptions.loading
        ~progress_callback:(fun _ -> ())
        ~manifold_path
        ~target_path
    in
    (match result with
    | Ok (main_artifacts, _telemetry) ->
      let load_result : Saved_state_loader.load_result =
        {
          Saved_state_loader.main_artifacts;
          additional_info;
          changed_files_according_to_watchman = changed_files;
          manifold_path;
          is_cached;
        }
      in
      Lwt.return_ok load_result
    | Error (load_error, _telemetry) -> Lwt.return_error load_error)

let main (env : env) (local_config : ServerLocalConfig.t) : Exit_status.t Lwt.t
    =
  Relative_path.set_path_prefix Relative_path.Root env.root;
  let%lwt result = load_saved_state ~env ~local_config in
  match result with
  | Error load_error ->
    print_load_error load_error;
    Lwt.return Exit_status.Failed_to_load_should_abort
  | Ok
      Saved_state_loader.
        {
          main_artifacts =
            {
              Naming_and_dep_table_info.naming_table_path;
              Naming_and_dep_table_info.naming_sqlite_table_path;
              dep_table_path;
              compressed_dep_table_path;
              errors_path;
              warning_hashes_path;
            };
          additional_info;
          changed_files_according_to_watchman;
          manifold_path;
          is_cached;
        } ->
    let%lwt replay_token =
      make_replay_token
        ~env
        ~manifold_path
        ~changed_files:changed_files_according_to_watchman
        ~is_cached
        ~additional_info
    in
    let json =
      Hh_json.JSON_Object
        [
          ( "changed_files",
            changed_files_to_absolute_paths_json
              changed_files_according_to_watchman );
          ( "naming_table_path",
            naming_table_path |> Path.to_string |> Hh_json.string_ );
          ( "naming_sqlite_table_path",
            naming_sqlite_table_path |> Path.to_string |> Hh_json.string_ );
          ("dep_table_path", dep_table_path |> Path.to_string |> Hh_json.string_);
          ( "compressed_dep_table_path",
            compressed_dep_table_path |> Path.to_string |> Hh_json.string_ );
          ("errors_path", errors_path |> Path.to_string |> Hh_json.string_);
          ( "warning_hashes_path",
            warning_hashes_path |> Path.to_string |> Hh_json.string_ );
          ( "replay_token",
            Option.value_map
              replay_token
              ~f:Hh_json.string_
              ~default:Hh_json.JSON_Null );
        ]
    in
    Hh_json.json_to_multiline_output stdout json;
    Out_channel.output_char stdout '\n';
    Lwt.return Exit_status.No_error
