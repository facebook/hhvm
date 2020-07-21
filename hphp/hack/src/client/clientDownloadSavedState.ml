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
}

let changed_files_to_json (changed_files : Saved_state_loader.changed_files) :
    Hh_json.json =
  Hh_json.JSON_Array
    ( changed_files
    |> List.map ~f:Relative_path.to_absolute
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

let main (env : env) : Exit_status.t Lwt.t =
  Relative_path.set_path_prefix Relative_path.Root env.root;
  let watchman_opts =
    Saved_state_loader.Watchman_options.{ root = env.root; sockname = None }
  in
  let ignore_hh_version = false in
  match env.saved_state_type with
  | Naming_and_dep_table ->
    let%lwt result =
      State_loader_lwt.load
        ~watchman_opts
        ~ignore_hh_version
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
          manifold_path = _ (* TODO: use *);
        } ->
      let json =
        Hh_json.JSON_Object
          [
            ("changed_files", changed_files_to_json changed_files);
            ( "naming_table_path",
              naming_table_path |> Path.to_string |> Hh_json.string_ );
            ( "dep_table_path",
              dep_table_path |> Path.to_string |> Hh_json.string_ );
            ( "hot_decls_path",
              hot_decls_path |> Path.to_string |> Hh_json.string_ );
            ("errors_path", errors_path |> Path.to_string |> Hh_json.string_);
          ]
      in
      Hh_json.json_to_multiline_output stdout json;
      Lwt.return Exit_status.No_error)
  | Naming_table ->
    let%lwt result =
      State_loader_lwt.load
        ~watchman_opts
        ~ignore_hh_version
        ~saved_state_type:Saved_state_loader.Naming_table
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
          manifold_path = _ (* TODO: use *);
        } ->
      let json =
        Hh_json.JSON_Object
          [
            ("changed_files", changed_files_to_json changed_files);
            ( "naming_table_path",
              naming_table_path |> Path.to_string |> Hh_json.string_ );
          ]
      in
      Hh_json.json_to_multiline_output stdout json;
      Lwt.return Exit_status.No_error)
