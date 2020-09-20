(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type saved_state_type =
  | Naming_and_dep_table
  | Naming_table

type env = {
  root: Path.t;
  from: string;
  saved_state_type: saved_state_type;
  should_save_replay: bool;
  replay_token: string option;
}

type replay_info = {
  manifold_path: string;
  changed_files: Saved_state_loader.changed_files;
}

let changed_files_to_absolute_paths_json
    (changed_files : Saved_state_loader.changed_files) : Hh_json.json =
  Hh_json.JSON_Array
    ( changed_files
    |> List.map ~f:Relative_path.to_absolute
    |> List.map ~f:Hh_json.string_ )

let changed_files_to_relative_paths_json
    (changed_files : Saved_state_loader.changed_files) : Hh_json.json =
  Hh_json.JSON_Array
    ( changed_files
    |> List.map ~f:Relative_path.suffix
    |> List.map ~f:Hh_json.string_ )

let print_load_error (load_error : Saved_state_loader.load_error) : unit =
  let json =
    Hh_json.JSON_Object
      [
        ( "error",
          Hh_json.string_ (Saved_state_loader.debug_details_of_error load_error)
        );
      ]
  in
  Hh_json.json_to_multiline_output stdout json

let get_replay_info (replay_token : string) : replay_info Lwt.t =
  let%lwt replay_info = Clowder_paste.clowder_download replay_token in
  match replay_info with
  | Ok stdout ->
    let json = Hh_json.json_of_string stdout in

    let open Hh_json.Access in
    let accessor = return json in
    let result =
      accessor >>= get_string "manifold_path" >>= fun (manifold_path, _) ->
      accessor >>= get_array "changed_files" >>= fun (changed_files, _) ->
      let changed_files =
        List.map changed_files ~f:(fun changed_file ->
            let changed_file = Hh_json.get_string_exn changed_file in
            Relative_path.from_root ~suffix:changed_file)
      in
      return { manifold_path; changed_files }
    in
    (match result with
    | Ok (result, _) -> Lwt.return result
    | Error access_failure ->
      failwith
        (Printf.sprintf
           "Could not parse JSON: %s on file: %s"
           (access_failure_to_string access_failure)
           stdout))
  | Error message ->
    failwith
      (Printf.sprintf
         "Could not get replay info from Clowder handle %s: %s"
         replay_token
         message)

let make_replay_token
    ~(env : env)
    ~(manifold_path : string)
    ~(changed_files : Saved_state_loader.changed_files) : string option Lwt.t =
  match (env.should_save_replay, env.replay_token) with
  | (false, (Some _ | None)) -> Lwt.return_none
  | (true, Some replay_token) ->
    (* No need to generate a new replay token in this case, as it would
    contain the same data as we already have. *)
    Lwt.return_some replay_token
  | (true, None) ->
    let json =
      Hh_json.JSON_Object
        [
          ("manifold_path", Hh_json.JSON_String manifold_path);
          ("changed_files", changed_files_to_relative_paths_json changed_files);
        ]
    in
    let%lwt clowder_result =
      Clowder_paste.clowder_upload_and_get_handle
        (Hh_json.json_to_multiline json)
    in
    (match clowder_result with
    | Ok handle -> Lwt.return_some handle
    | Error message ->
      Hh_logger.error "Failed to generate replay token from Clowder: %s" message;
      Lwt.return_none)

let load_saved_state :
    type info.
    env:env ->
    saved_state_type:info Saved_state_loader.saved_state_type ->
    ( info Saved_state_loader.load_result,
      Saved_state_loader.load_error )
    Lwt_result.t =
 fun ~env ~saved_state_type ->
  match env.replay_token with
  | None ->
    let watchman_opts =
      Saved_state_loader.Watchman_options.{ root = env.root; sockname = None }
    in
    let%lwt result =
      State_loader_lwt.load
        ~watchman_opts
        ~ignore_hh_version:false
        ~saved_state_type
    in
    Lwt.return result
  | Some replay_token ->
    let%lwt { manifold_path; changed_files } = get_replay_info replay_token in
    let download_dir = State_loader_lwt.prepare_download_dir () in
    let target_path =
      State_loader_lwt.get_saved_state_target_path ~download_dir ~manifold_path
    in
    let%lwt result =
      State_loader_lwt.download_and_unpack_saved_state_from_manifold
        ~manifold_path
        ~target_path
        ~saved_state_type
    in
    (match result with
    | Ok (saved_state_info, _telemetry) ->
      let load_result =
        { Saved_state_loader.saved_state_info; changed_files; manifold_path }
      in
      Lwt.return_ok load_result
    | Error (load_error, _telemetry) -> Lwt.return_error load_error)

let main (env : env) : Exit_status.t Lwt.t =
  Relative_path.set_path_prefix Relative_path.Root env.root;
  match env.saved_state_type with
  | Naming_and_dep_table ->
    let%lwt result =
      load_saved_state
        ~env
        ~saved_state_type:Saved_state_loader.Naming_and_dep_table
    in
    (match result with
    | Error load_error ->
      print_load_error load_error;
      Lwt.return Exit_status.Failed_to_load_should_abort
    | Ok
        {
          Saved_state_loader.saved_state_info =
            {
              Saved_state_loader.Naming_and_dep_table_info.naming_table_path;
              dep_table_path;
              hot_decls_path;
              errors_path;
            };
          changed_files;
          manifold_path;
        } ->
      let%lwt replay_token =
        make_replay_token ~env ~manifold_path ~changed_files
      in
      let json =
        Hh_json.JSON_Object
          [
            ("changed_files", changed_files_to_absolute_paths_json changed_files);
            ( "naming_table_path",
              naming_table_path |> Path.to_string |> Hh_json.string_ );
            ( "dep_table_path",
              dep_table_path |> Path.to_string |> Hh_json.string_ );
            ( "hot_decls_path",
              hot_decls_path |> Path.to_string |> Hh_json.string_ );
            ("errors_path", errors_path |> Path.to_string |> Hh_json.string_);
            ( "replay_token",
              Option.value_map
                replay_token
                ~f:Hh_json.string_
                ~default:Hh_json.JSON_Null );
          ]
      in
      Hh_json.json_to_multiline_output stdout json;
      Out_channel.output_char stdout '\n';
      Lwt.return Exit_status.No_error)
  | Naming_table ->
    let%lwt result =
      load_saved_state ~env ~saved_state_type:Saved_state_loader.Naming_table
    in
    (match result with
    | Error load_error ->
      print_load_error load_error;
      Lwt.return Exit_status.Failed_to_load_should_abort
    | Ok
        {
          Saved_state_loader.saved_state_info =
            { Saved_state_loader.Naming_table_info.naming_table_path };
          changed_files;
          manifold_path;
        } ->
      let%lwt replay_token =
        make_replay_token ~env ~manifold_path ~changed_files
      in
      let json =
        Hh_json.JSON_Object
          [
            ("changed_files", changed_files_to_absolute_paths_json changed_files);
            ( "naming_table_path",
              naming_table_path |> Path.to_string |> Hh_json.string_ );
            ( "replay_token",
              Option.value_map
                replay_token
                ~f:Hh_json.string_
                ~default:Hh_json.JSON_Null );
          ]
      in
      Hh_json.json_to_multiline_output stdout json;
      Out_channel.output_char stdout '\n';
      Lwt.return Exit_status.No_error)
