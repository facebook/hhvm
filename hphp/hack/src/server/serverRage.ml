(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

let go (_genv : ServerEnv.genv) (env : ServerEnv.env) : ServerRageTypes.result =
  ServerRageTypes.(
    (* Gather up the contents of all files that hh_server believes are in the *)
    (* IDE different from what's on disk *)
    let ide_files_different_from_disk =
      ServerFileSync.get_unsaved_changes env
      |> Relative_path.Map.map ~f:fst
      |> Relative_path.Map.elements
      |> List.map ~f:(fun (relPath, data) ->
             {
               title = Some (Relative_path.to_absolute relPath ^ ":modified_hh");
               data;
             })
    in
    (* include PIDs that we know *)
    let pids_data =
      Printf.sprintf
        "hh_server pid=%d ppid=%d\n"
        (Unix.getpid ())
        (Unix.getppid ())
    in
    let pids = { title = None; data = pids_data } in
    (* is it paused? *)
    let paused_data =
      Printf.sprintf
        "\n%s... disk_needs_parsing:\n%s\n"
        (match env.ServerEnv.full_recheck_on_file_changes with
        | ServerEnv.Not_paused -> "hh"
        | ServerEnv.Paused _ -> "hh --pause"
        | ServerEnv.Resumed -> "hh --resume")
        ( Relative_path.Set.elements env.ServerEnv.disk_needs_parsing
        |> List.map ~f:Relative_path.to_absolute
        |> String.concat "\n" )
    in
    let paused = { title = None; data = paused_data } in
    (* include current state of diagnostics on client, as we know it *)
    ServerEnv.(
      let subscription_data =
        match (env.diag_subscribe, env.persistent_client) with
        | (None, None) -> "no diag subscription, no client"
        | (Some _, None) -> "?? diagnostics subscription but no client ??"
        | (None, Some _) -> "?? client but no diagnostics subscription ??"
        | (Some sub, Some client) ->
          let (is_truncated, count) =
            Diagnostic_subscription.get_pushed_error_length sub
          in
          let (_sub, errors) =
            Diagnostic_subscription.pop_errors sub env.errorl
          in
          let messages =
            [
              "hh_server view of diagnostics on client:";
              "client_has_message";
              ClientProvider.client_has_message client |> string_of_bool;
              "ide_needs_parsing";
              (not (Relative_path.Set.is_empty env.ide_needs_parsing))
              |> string_of_bool;
              "error_count";
              Errors.count env.errorl |> string_of_int;
              "errors_in_client";
              count |> string_of_int;
              is_truncated |> string_of_bool;
              "error_files_to_push";
              errors |> SMap.keys |> List.length |> string_of_int;
              "";
            ]
          in
          String.concat "\n" messages
      in
      let subscription = { title = None; data = subscription_data } in
      pids :: subscription :: paused :: ide_files_different_from_disk))
