(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type saved_state_type =
  | Naming_and_dep_table of { is_64bit: bool }
  | Naming_table

type env = {
  root: Path.t;
  from: string;
  saved_state_type: saved_state_type;
  should_save_replay: bool;
  replay_token: string option;
}

type 'additional_info replay_info = {
  manifold_path: string;
  changed_files: Saved_state_loader.changed_files;
  corresponding_rev: Hg.hg_rev;
  mergebase_rev: Hg.hg_rev;
  is_cached: bool;
  additional_info: 'additional_info;
}

let changed_files_to_absolute_paths_json
    (changed_files : Saved_state_loader.changed_files) : Hh_json.json =
  changed_files
  |> List.map ~f:Relative_path.to_absolute
  |> Hh_json_helpers.Jprint.string_array

let changed_files_to_relative_paths_json
    (changed_files : Saved_state_loader.changed_files) : Hh_json.json =
  changed_files
  |> List.map ~f:Relative_path.suffix
  |> Hh_json_helpers.Jprint.string_array

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

let additional_info_of_json
    (type main_artifacts additional_info)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    (json : Hh_json.json option) : additional_info =
  let open Hh_json_helpers in
  match saved_state_type with
  | Saved_state_loader.Naming_table -> ()
  | Saved_state_loader.Symbol_index -> ()
  | Saved_state_loader.Naming_and_dep_table { is_64bit = _ } ->
    let mergebase_global_rev = Jget.int_opt json "mergebase_global_rev" in
    let dep_table_is_64bit = Jget.bool_exn json "dep_table_is_64bit" in
    let dirty_files = Jget.obj_exn json "dirty_files" in
    let master_changes =
      Jget.string_array_exn dirty_files "master_changes"
      |> List.map ~f:(fun suffix -> Relative_path.from_root ~suffix)
      |> Relative_path.Set.of_list
    in
    let local_changes =
      Jget.string_array_exn dirty_files "local_changes"
      |> List.map ~f:(fun suffix -> Relative_path.from_root ~suffix)
      |> Relative_path.Set.of_list
    in
    Saved_state_loader.Naming_and_dep_table_info.
      {
        mergebase_global_rev;
        dep_table_is_64bit;
        dirty_files_promise = Future.of_value { master_changes; local_changes };
      }

let replay_info_of_json
    (type main_artifacts additional_info)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    (json : Hh_json.json option) : additional_info replay_info =
  let open Hh_json_helpers in
  let manifold_path = Jget.string_exn json "manifold_path" in
  let changed_files =
    Jget.string_array_exn json "changed_files"
    |> List.map ~f:(fun suffix -> Relative_path.from_root ~suffix)
  in
  let corresponding_rev = Jget.string_exn json "corresponding_rev" in
  let mergebase_rev = Jget.string_exn json "mergebase_rev" in
  let is_cached = Jget.bool_exn json "is_cached" in
  let additional_info =
    additional_info_of_json
      ~saved_state_type
      (Jget.obj_opt json "additional_info")
  in
  {
    manifold_path;
    changed_files;
    corresponding_rev;
    mergebase_rev;
    is_cached;
    additional_info;
  }

let get_replay_info
    (type main_artifacts additional_info)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    (replay_token : string) : additional_info replay_info Lwt.t =
  let%lwt replay_info = Clowder_paste.clowder_download replay_token in
  match replay_info with
  | Ok stdout ->
    let json = Some (Hh_json.json_of_string stdout) in
    Lwt.return (replay_info_of_json ~saved_state_type json)
  | Error message ->
    failwith
      (Printf.sprintf
         "Could not get replay info from Clowder handle %s: %s"
         replay_token
         message)

let make_replay_token_of_additional_info
    (type main_artifacts additional_info)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    ~(additional_info : additional_info) : Hh_json.json =
  match saved_state_type with
  | Saved_state_loader.Naming_table -> Hh_json.JSON_Null
  | Saved_state_loader.Symbol_index -> Hh_json.JSON_Null
  | Saved_state_loader.Naming_and_dep_table { is_64bit = _ } ->
    let Saved_state_loader.Naming_and_dep_table_info.
          { mergebase_global_rev; dep_table_is_64bit; dirty_files_promise } =
      additional_info
    in
    let Saved_state_loader.Naming_and_dep_table_info.
          { master_changes; local_changes } =
      Future.get_exn dirty_files_promise
    in
    let open Hh_json in
    JSON_Object
      [
        ("mergebase_global_rev", opt_int_to_json mergebase_global_rev);
        ("dep_table_is_64bit", JSON_Bool dep_table_is_64bit);
        ( "dirty_files",
          JSON_Object
            [
              ( "master_changes",
                changed_files_to_relative_paths_json
                @@ Relative_path.Set.elements master_changes );
              ( "local_changes",
                changed_files_to_relative_paths_json
                @@ Relative_path.Set.elements local_changes );
            ] );
      ]

let make_replay_token_json
    (type main_artifacts additional_info)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    ~(manifold_path : string)
    ~(changed_files : Saved_state_loader.changed_files)
    ~(corresponding_rev : Hg.hg_rev)
    ~(mergebase_rev : Hg.hg_rev)
    ~(is_cached : bool)
    ~(additional_info : additional_info) : Hh_json.json =
  let open Hh_json in
  JSON_Object
    [
      ("manifold_path", JSON_String manifold_path);
      ("changed_files", changed_files_to_relative_paths_json changed_files);
      ("corresponding_rev", JSON_String corresponding_rev);
      ("mergebase_rev", JSON_String mergebase_rev);
      ("is_cached", JSON_Bool is_cached);
      ( "additional_info",
        make_replay_token_of_additional_info ~saved_state_type ~additional_info
      );
    ]

let make_replay_token
    (type main_artifacts additional_info)
    ~(env : env)
    ~(saved_state_type :
       (main_artifacts * additional_info) Saved_state_loader.saved_state_type)
    ~(manifold_path : string)
    ~(changed_files : Saved_state_loader.changed_files)
    ~(corresponding_rev : Hg.hg_rev)
    ~(mergebase_rev : Hg.hg_rev)
    ~(is_cached : bool)
    ~(additional_info : additional_info) : string option Lwt.t =
  match (env.should_save_replay, env.replay_token) with
  | (false, (Some _ | None)) -> Lwt.return_none
  | (true, Some replay_token) ->
    (* No need to generate a new replay token in this case, as it would
    contain the same data as we already have. *)
    Lwt.return_some replay_token
  | (true, None) ->
    let json =
      make_replay_token_json
        ~saved_state_type
        ~manifold_path
        ~changed_files
        ~corresponding_rev
        ~mergebase_rev
        ~is_cached
        ~additional_info
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
    type main_artifacts additional_info.
    env:env ->
    saved_state_type:
      (main_artifacts * additional_info) Saved_state_loader.saved_state_type ->
    ( (main_artifacts, additional_info) Saved_state_loader.load_result,
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
    let%lwt {
          manifold_path;
          changed_files;
          corresponding_rev;
          mergebase_rev;
          is_cached;
          additional_info;
        } =
      get_replay_info ~saved_state_type replay_token
    in
    let download_dir =
      State_loader_lwt.prepare_download_dir ~saved_state_type
    in
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
    | Ok (main_artifacts, _telemetry) ->
      let load_result =
        {
          Saved_state_loader.main_artifacts;
          additional_info;
          changed_files;
          manifold_path;
          corresponding_rev;
          mergebase_rev;
          is_cached;
        }
      in
      Lwt.return_ok load_result
    | Error (load_error, _telemetry) -> Lwt.return_error load_error)

let main (env : env) : Exit_status.t Lwt.t =
  Relative_path.set_path_prefix Relative_path.Root env.root;
  match env.saved_state_type with
  | Naming_and_dep_table { is_64bit } ->
    let saved_state_type =
      Saved_state_loader.Naming_and_dep_table { is_64bit }
    in
    let%lwt result = load_saved_state ~env ~saved_state_type in
    (match result with
    | Error load_error ->
      print_load_error load_error;
      Lwt.return Exit_status.Failed_to_load_should_abort
    | Ok
        Saved_state_loader.
          {
            main_artifacts =
              {
                Naming_and_dep_table_info.naming_table_path;
                dep_table_path;
                legacy_hot_decls_path;
                shallow_hot_decls_path;
                errors_path;
              };
            additional_info;
            changed_files;
            manifold_path;
            corresponding_rev;
            mergebase_rev;
            is_cached;
          } ->
      let%lwt replay_token =
        make_replay_token
          ~env
          ~saved_state_type
          ~manifold_path
          ~changed_files
          ~corresponding_rev
          ~mergebase_rev
          ~is_cached
          ~additional_info
      in
      let json =
        Hh_json.JSON_Object
          [
            ("changed_files", changed_files_to_absolute_paths_json changed_files);
            ( "naming_table_path",
              naming_table_path |> Path.to_string |> Hh_json.string_ );
            ( "dep_table_path",
              dep_table_path |> Path.to_string |> Hh_json.string_ );
            ( "legacy_hot_decls_path",
              legacy_hot_decls_path |> Path.to_string |> Hh_json.string_ );
            ( "shallow_hot_decls_path",
              shallow_hot_decls_path |> Path.to_string |> Hh_json.string_ );
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
    let saved_state_type = Saved_state_loader.Naming_table in
    let%lwt result = load_saved_state ~env ~saved_state_type in
    (match result with
    | Error load_error ->
      print_load_error load_error;
      Lwt.return Exit_status.Failed_to_load_should_abort
    | Ok
        {
          Saved_state_loader.main_artifacts =
            { Saved_state_loader.Naming_table_info.naming_table_path };
          additional_info = ();
          changed_files;
          manifold_path;
          corresponding_rev;
          mergebase_rev;
          is_cached;
        } ->
      let%lwt replay_token =
        make_replay_token
          ~env
          ~saved_state_type
          ~manifold_path
          ~changed_files
          ~corresponding_rev
          ~mergebase_rev
          ~is_cached
          ~additional_info:()
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
