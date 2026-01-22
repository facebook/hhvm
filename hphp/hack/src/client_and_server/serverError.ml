(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Error module                                                              *)
(*****************************************************************************)
open Hh_prelude

let get_error_list_json
    (error_format : Diagnostics.format option)
    (error_list : Diagnostics.finalized_diagnostic list)
    ~(recheck_stats : Telemetry.t option) =
  let error_format = Diagnostics.format_or_default error_format in
  (* for extended reasons, we produce a human-readable format. *)
  let human_formatter =
    Diagnostics.(
      match error_format with
      | Extended -> Some Extended_diagnostic_formatter.to_string
      | Context
      | Raw
      | Highlighted
      | Plain ->
        None)
  in
  let (error_list, did_pass) =
    match error_list with
    | [] -> ([], true)
    | error_list ->
      let passed =
        not
          (List.exists error_list ~f:(fun error ->
               match error.severity with
               | Err -> true
               | Warning -> false))
      in
      ( List.map
          ~f:
            (User_diagnostic.to_json ~human_formatter ~filename_to_string:Fn.id)
          error_list,
        passed )
  in
  let (properties : (string * Hh_json.json) list) =
    [
      ("passed", Hh_json.JSON_Bool did_pass);
      ("errors", Hh_json.JSON_Array error_list);
      ("version", Hh_version.version_json);
    ]
  in
  let properties =
    match recheck_stats with
    | None -> properties
    | Some telemetry ->
      ("last_recheck", Telemetry.to_json telemetry) :: properties
  in
  Hh_json.JSON_Object properties

let print_error_list_json
    (oc : Out_channel.t)
    (error_format : Diagnostics.format option)
    (error_list : Diagnostics.finalized_diagnostic list)
    (recheck_stats : Telemetry.t option) =
  let res = get_error_list_json error_format error_list ~recheck_stats in
  Hh_json.json_to_output oc res;
  Out_channel.flush oc

let print_error_list
    (oc : Out_channel.t)
    ~(stale_msg : string option)
    ~(output_json : bool)
    ~(error_format : Diagnostics.format option)
    ~(error_list : Diagnostics.finalized_diagnostic list)
    ~(recheck_stats : Telemetry.t option) =
  (if output_json then
    print_error_list_json oc error_format error_list recheck_stats
  else if List.is_empty error_list then
    Out_channel.output_string oc "No errors!\n"
  else
    let sl = List.map ~f:Diagnostics.to_string error_list in
    let sl = List.dedup_and_sort ~compare:String.compare sl in
    List.iter
      ~f:
        begin
          fun s ->
            Out_channel.output_string oc s;
            Out_channel.output_string oc "\n"
        end
      sl);
  Option.iter stale_msg ~f:(fun msg -> Out_channel.output_string oc msg);
  Out_channel.flush oc;
  ()
