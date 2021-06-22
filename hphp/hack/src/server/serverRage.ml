(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go (_genv : ServerEnv.genv) (env : ServerEnv.env) : ServerRageTypes.result =
  let open ServerRageTypes in
  let open ServerEnv in
  (* Gather up the contents of all files that hh_server believes are in the
    IDE different from what's on disk *)
  let unsaved_items =
    ServerFileSync.get_unsaved_changes env
    |> Relative_path.Map.map ~f:fst
    |> Relative_path.Map.elements
    |> List.map ~f:(fun (relPath, data) ->
           { title = "unsaved:" ^ Relative_path.to_absolute relPath; data })
  in

  (* include PIDs that we know *)
  let pids_data =
    Printf.sprintf
      "hh_server pid=%d ppid=%d\n"
      (Unix.getpid ())
      (Unix.getppid ())
  in

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
      |> String.concat ~sep:"\n" )
  in

  (* include current state of diagnostics on client, as we know it *)
  let subscription_data =
    match (env.diag_subscribe, env.persistent_client) with
    | (None, None) -> "no diag subscription, no client"
    | (Some _, None) -> "?? diagnostics subscription but no client ??"
    | (None, Some _) -> "?? client but no diagnostics subscription ??"
    | (Some sub, Some client) ->
      let (_is_truncated, count) =
        Diagnostic_subscription.get_pushed_error_length sub
      in
      let (_sub, errors, is_truncated) =
        Diagnostic_subscription.pop_errors sub ~global_errors:env.errorl
      in
      Printf.sprintf
        ( "client_has_message: %b\nide_needs_parsing: %b\n"
        ^^ "error_count: %d\nerrors_in_client: %d\nis_truncated: %b\n"
        ^^ "error_files_to_push: %s\n" )
        (ClientProvider.client_has_message client)
        (not (Relative_path.Set.is_empty env.ide_needs_parsing))
        (Errors.count env.errorl)
        count
        (Option.is_some is_truncated)
        (errors |> SMap.keys |> String.concat ~sep:",")
  in

  (* that's it! *)
  let data =
    Printf.sprintf
      "PIDS\n%s\n\nPAUSED\n%s\n\nSUBSCRIPTION\n%s\n\nIDE FILES\n%s\n"
      pids_data
      paused_data
      subscription_data
      ( List.map unsaved_items ~f:(fun item -> item.title)
      |> String.concat ~sep:"\n" )
  in
  { title = "status"; data } :: unsaved_items
