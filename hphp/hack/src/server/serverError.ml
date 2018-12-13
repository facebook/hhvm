(**
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
open Utils

let get_error_list_json
    (error_list: (Pos.absolute Errors.error_ list))
    ~(edges_added: int option) =
  let error_list, did_pass = match error_list with
  | [] -> [], true
  | error_list -> (List.map ~f:Errors.to_json error_list), false
  in
  let (properties: (string * Hh_json.json) list) =
    [ "passed", Hh_json.JSON_Bool did_pass;
      "errors", Hh_json.JSON_Array error_list;
      "version", Hh_json.JSON_String Build_id.build_id_ohai;
    ]
  in
  let properties = match edges_added with
  | None -> properties
  | Some edges_added ->
    let saved_state_result =
      "saved_state_result", Hh_json.JSON_Object
        [
          "edges_added", Hh_json.int_ edges_added
        ]
    in
    saved_state_result :: properties in

  Hh_json.JSON_Object properties

let print_error_list_json
    (oc: Out_channel.t)
    (error_list: (Pos.absolute Errors.error_ list))
    (edges_added: int option) =
  let res = get_error_list_json error_list ~edges_added in
  Hh_json.json_to_output oc res;
  Out_channel.flush oc

let print_error_list
    (oc: Out_channel.t)
    ~(stale_msg: string option)
    ~(output_json: bool)
    ~(error_list: (Pos.absolute Errors.error_ list))
    ~(edges_added: int option) =
  if output_json then
    print_error_list_json oc error_list edges_added
  else begin
    if error_list = []
    then Out_channel.output_string oc "No errors!\n"
    else
      let sl = List.map ~f:Errors.to_string error_list in
      let sl = List.dedup_and_sort ~compare:String.compare sl in
      List.iter ~f:begin fun s ->
        if !debug then begin
          Out_channel.output_string stdout s;
          Out_channel.flush stdout;
        end;
        Out_channel.output_string oc s;
        Out_channel.output_string oc "\n";
      end sl
  end;
  Option.iter stale_msg ~f:(fun msg -> Out_channel.output_string oc msg);
  Out_channel.flush oc
