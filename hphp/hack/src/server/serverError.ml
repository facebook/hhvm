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

let get_save_state_result_props_json
    (save_state_result : SaveStateServiceTypes.save_state_result) :
    (string * Hh_json.json) list =
  SaveStateServiceTypes.
    [
      ( "dep_table_edges_added",
        Hh_json.int_ save_state_result.dep_table_edges_added );
    ]

let get_save_state_result_json
    (save_state_result : SaveStateServiceTypes.save_state_result) :
    string * Hh_json.json =
  ( "save_state_result",
    Hh_json.JSON_Object (get_save_state_result_props_json save_state_result) )

let get_error_list_json
    (error_list : Errors.finalized_error list)
    ~(save_state_result : SaveStateServiceTypes.save_state_result option)
    ~(recheck_stats : Telemetry.t option) =
  let (error_list, did_pass) =
    match error_list with
    | [] -> ([], true)
    | error_list ->
      ( List.map ~f:(User_error.to_json ~filename_to_string:Fn.id) error_list,
        false )
  in
  let (properties : (string * Hh_json.json) list) =
    [
      ("passed", Hh_json.JSON_Bool did_pass);
      ("errors", Hh_json.JSON_Array error_list);
      ("version", Hh_version.version_json);
    ]
  in
  let properties =
    match save_state_result with
    | None -> properties
    | Some save_state_result ->
      let save_state_result_json =
        get_save_state_result_json save_state_result
      in
      save_state_result_json :: properties
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
    (error_list : Errors.finalized_error list)
    (save_state_result : SaveStateServiceTypes.save_state_result option)
    (recheck_stats : Telemetry.t option) =
  let res = get_error_list_json error_list ~save_state_result ~recheck_stats in
  Hh_json.json_to_output oc res;
  Out_channel.flush oc

let print_error_list
    (oc : Out_channel.t)
    ~(stale_msg : string option)
    ~(output_json : bool)
    ~(error_list : Errors.finalized_error list)
    ~(save_state_result : SaveStateServiceTypes.save_state_result option)
    ~(recheck_stats : Telemetry.t option) =
  (if output_json then
    print_error_list_json oc error_list save_state_result recheck_stats
  else if List.is_empty error_list then
    Out_channel.output_string oc "No errors!\n"
  else
    let sl = List.map ~f:Errors.to_string error_list in
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
  Out_channel.flush oc
