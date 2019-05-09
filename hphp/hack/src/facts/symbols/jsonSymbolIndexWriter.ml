(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
open Core_kernel
open SearchUtils
open Hh_json

let record_one_jsonfile
    (filename_prefix: string)
    (filenum: int)
    (elems: json list): string =

  (* Open the file *)
  let filename = Printf.sprintf "%s%s.json"
    filename_prefix (string_of_int filenum) in

  (* Write all results in json format *)
  let json_export = JSON_Array [
    JSON_Object [
      ("predicate", JSON_String "hack.identifier.1");
      ("facts", JSON_Array elems);
    ]
  ] in

  (* Close the json file *)
  let json_str = json_to_string ~pretty:true json_export in
  let channel = Out_channel.create filename in
  Out_channel.output_string channel json_str;
  Out_channel.close channel;

  (* Here's your file *)
  filename

(* Save a list of symbols into a range of json files split into chunk_size *)
let record_in_jsonfiles
    (chunk_size: int)
    (filename_prefix: string)
    (symbols: si_item list) =

  (* Note that glean elements must start with high ID numbers *)
  let json_element_id = ref 500_000 in

  (* Create an array of all JSON elements *)
  let json_array = List.map symbols ~f:(fun symbol -> begin
        json_element_id := !json_element_id + 1;
        let kind_int = kind_to_int symbol.si_kind in
        JSON_Object [ ("key", JSON_Object [
            ("kind", JSON_Number (string_of_int kind_int));
            ("name", JSON_String symbol.si_name);
          ]);
            ("id", JSON_Number (string_of_int !json_element_id));
          ]
      end) in

  (* Save them into multiple files of 1000 elements each *)
  (*let chunk_id = ref 0 in
  let filenames = ref [] in*)
  let array_chunk_list = List.chunks_of json_array chunk_size in
  let filenames = List.mapi array_chunk_list ~f:(fun chunk_id chunk ->
        record_one_jsonfile filename_prefix chunk_id chunk
  ) in
  filenames
