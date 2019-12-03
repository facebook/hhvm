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
open Core_kernel

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
    (error_list : Pos.absolute Errors.error_ list)
    ~(save_state_result : SaveStateServiceTypes.save_state_result option)
    (recheck_stats : ServerCommandTypes.Recheck_stats.t option) =
  let (error_list, did_pass) =
    match error_list with
    | [] -> ([], true)
    | error_list -> (List.map ~f:Errors.to_json error_list, false)
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
  ServerCommandTypes.Recheck_stats.(
    let properties =
      match recheck_stats with
      | None -> properties
      | Some stats ->
        let last_recheck_result =
          ( "last_recheck",
            Hh_json.JSON_Object
              [
                ("id", Hh_json.JSON_String stats.id);
                ("time", Hh_json.JSON_Number (string_of_float stats.time));
                ("count", Hh_json.JSON_Number (string_of_int stats.count));
              ] )
        in
        last_recheck_result :: properties
    in
    Hh_json.JSON_Object properties)

let print_error_list_json
    (oc : Out_channel.t)
    (error_list : Pos.absolute Errors.error_ list)
    (save_state_result : SaveStateServiceTypes.save_state_result option)
    (recheck_stats : ServerCommandTypes.Recheck_stats.t option) =
  let res = get_error_list_json error_list ~save_state_result recheck_stats in
  Hh_json.json_to_output oc res;
  Out_channel.flush oc

let print_error_list
    (oc : Out_channel.t)
    ~(stale_msg : string option)
    ~(output_json : bool)
    ~(error_list : Pos.absolute Errors.error_ list)
    ~(save_state_result : SaveStateServiceTypes.save_state_result option)
    ~(recheck_stats : ServerCommandTypes.Recheck_stats.t option) =
  ( if output_json then
    print_error_list_json oc error_list save_state_result recheck_stats
  else if error_list = [] then
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
      sl );
  Option.iter stale_msg ~f:(fun msg -> Out_channel.output_string oc msg);
  Out_channel.flush oc
