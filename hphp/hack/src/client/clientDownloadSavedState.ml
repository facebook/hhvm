(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type saved_state_type = Naming_table

type env = {
  root: Path.t;
  from: string;
  saved_state_type: saved_state_type;
}

let main (env : env) : Exit_status.t Lwt.t =
  let%lwt result =
    State_loader_lwt.load
      ~repo:env.root
      ~ignore_hh_version:false
      ~saved_state_type:Saved_state_loader.Naming_table
  in
  match result with
  | Ok (info, _changed_files) ->
    let json =
      Hh_json.JSON_Object
        [
          ( "naming_table_path",
            info
              .Saved_state_loader.Naming_table_saved_state_info
               .naming_table_path
            |> Path.to_string
            |> Hh_json.string_ );
        ]
    in
    Hh_json.json_to_multiline_output stdout json;
    Lwt.return Exit_status.No_error
  | Error load_error ->
    let json =
      Hh_json.JSON_Object
        [
          ( "error",
            Hh_json.string_
              (Saved_state_loader.debug_details_of_error load_error) );
        ]
    in
    Hh_json.json_to_multiline_output stdout json;
    Lwt.return Exit_status.Failed_to_load_should_abort
