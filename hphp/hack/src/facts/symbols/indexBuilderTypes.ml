(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(* Keep track of all references yet to scan *)
let files_scanned = ref 0
let error_count = ref 0

type index_builder_context = {
  repo_folder: string;
  sqlite_filename: string option;
  text_filename: string option;
  json_filename: string option;
}

type found_symbol_kind =
  | FSK_Class
  | FSK_Interface
  | FSK_Enum
  | FSK_Trait
  | FSK_Unknown
  | FSK_Mixed
  | FSK_Function
  | FSK_Typedef

type found_symbol_result = {
  found_symbol_name: string;
  found_symbol_kind: found_symbol_kind;
}

type index_builder_parse_result =
  found_symbol_result list

let kind_to_int (kind: found_symbol_kind): int =
  match kind with
  | FSK_Class -> 1
  | FSK_Interface -> 2
  | FSK_Enum -> 3
  | FSK_Trait -> 4
  | FSK_Unknown -> 5
  | FSK_Mixed -> 6
  | FSK_Function -> 7
  | FSK_Typedef -> 8
