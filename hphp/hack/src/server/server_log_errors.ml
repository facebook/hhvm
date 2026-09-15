(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Option.Monad_infix

let go
    (workers : Multi_worker.worker list option)
    (env : Server_env.env)
    (files : string list)
    (error_filter : Filter_diagnostics.Filter.t)
    (preexisting_warnings : bool) : Telemetry.t =
  let file_names =
    List.map files ~f:(fun filename -> Server_command_types.FileName filename)
  in

  let ctx = Provider_utils.ctx_from_server_env env in

  let tast_error_filter =
    {
      Tast_provider.ErrorFilter.error_filter;
      warnings_saved_state =
        Server_env.(env.init_env.mergebase_warning_hashes)
        >>= Option.some_if (not preexisting_warnings);
    }
  in

  let (errors, _tasts) =
    Server_status_single.go
      workers
      file_names
      ctx
      ~return_expanded_tast:false
      ~error_filter:tast_error_filter
  in

  (* Define error JSON serialization function.
     This differs from `hh --json` to align with the information sent to VSCode
     (similar structure, though not identical). The `hh --json` format includes
     unnecessary extra fields that aren't needed for this use case. *)
  let error_to_json : Diagnostics.diagnostic -> Yojson.Safe.t =
   fun err ->
    let {
      User_diagnostic.severity;
      code = _;
      claim = (pos, claim_msg);
      reasons;
      explanation = _;
      custom_msgs;
      quickfixes = _;
      is_fixmed = _;
      function_pos = _;
    } =
      User_diagnostic.to_absolute err
    in
    let msg_to_json msg = `String (Markdown_lite.render ~add_bold:false msg) in
    let reason_to_json (pos, msg) =
      `Assoc
        [("location", Pos.multiline_json pos); ("message", msg_to_json msg)]
    in
    `Assoc
      [
        ( "severity",
          `String (User_diagnostic.Severity.to_all_caps_string severity) );
        ("range", Pos.multiline_json_no_filename pos);
        ("message", msg_to_json claim_msg);
        ("relatedInformation", `List (List.map ~f:reason_to_json reasons));
        ("customErrors", `List (List.map ~f:msg_to_json custom_msgs));
        ( "lineAgnosticHash",
          `String
            (Printf.sprintf
               "%x"
               (User_diagnostic.hash_diagnostic_for_saved_state err)) );
      ]
  in
  let errors = Diagnostics.drop_fixmed_errors_in_files errors in
  let file_to_errors = Diagnostics.as_map errors in
  let file_to_error_json =
    Relative_path.Map.map ~f:(List.map ~f:error_to_json) file_to_errors
  in
  let compute_file_telemetry fn =
    let relpath = Relative_path.create_detect_prefix fn in
    Telemetry.create ()
    |> Telemetry.string_
         ~key:"filename"
         ~value:(Relative_path.to_absolute relpath)
    |> Telemetry.json
         ~key:"diagnostics"
         ~value:
           (`List
             (Relative_path.Map.find_opt file_to_error_json relpath
             |> Option.value ~default:[]))
  in
  Telemetry.create ()
  |> Telemetry.object_list
       ~key:"errors"
       ~value:(List.map files ~f:compute_file_telemetry)
