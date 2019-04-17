(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open SearchUtils

let record_in_jsonfile
    (filename: string)
    (symbols: si_results) =

  (* Open the file *)
  let open Core_kernel in
  let channel = Out_channel.create filename in
  let id = ref 0 in

  (* Write all results in json format *)
  let open Hh_json in
  let json_array = List.map symbols ~f:(fun symbol -> begin
        id := !id + 1;
        let kind_int = kind_to_int symbol.si_kind in
        JSON_Object [ ("key", JSON_Object [
              ("kind", JSON_Number (string_of_int kind_int));
              ("name", JSON_String symbol.si_name);
            ]);
          ("id", JSON_Number (string_of_int !id));
        ]
      end) in
  let json_export = JSON_Array json_array in

  (* Close the json file *)
  let json_str = json_to_string ~pretty:true json_export in
  Out_channel.output_string channel json_str;
  Out_channel.close channel;
;;
