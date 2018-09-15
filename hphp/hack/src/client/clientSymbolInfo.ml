(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Hh_json

let fun_call_to_json fun_call_results =
  let  open ServerCommandTypes.Symbol_info_service in
  List.map fun_call_results begin fun item ->
    let item_type =
      match item.type_ with
      | Function        -> "Function"
      | Method          -> "Method"
      | Constructor     -> "Constructor" in
    JSON_Object [
      "name",           JSON_String item.name;
      "type",           JSON_String item_type;
      "pos",            Pos.json item.pos;
      "caller",         JSON_String item.caller;
    ]
  end

let symbol_type_to_json symbol_type_results =
  let open ServerCommandTypes.Symbol_type in
  List.rev_map symbol_type_results begin fun item ->
    JSON_Object [
      "pos",    Pos.json item.pos;
      "type",   JSON_String item.type_;
      "ident",  int_ item.ident_;
    ]
  end

let to_json result =
  let open ServerCommandTypes.Symbol_info_service in
  let fun_call_json =
    fun_call_to_json result.fun_calls in
  let symbol_type_json =
    symbol_type_to_json result.symbol_types in
  JSON_Object [
    "function_calls",   JSON_Array fun_call_json;
    "symbol_types",     JSON_Array symbol_type_json;
  ]

let go conn (files:string) expand_path =
  let file_list = match files with
  | "-" ->
      let content = Sys_utils.read_stdin_to_string () in
      Str.split (Str.regexp "\n") content
  | _ ->
      Str.split (Str.regexp ";") files
  in
  let expand_path_list file_list =
    List.rev_map file_list begin fun file_path ->
      expand_path file_path
    end in
  let command =
    ServerCommandTypes.DUMP_SYMBOL_INFO (expand_path_list file_list) in
  let result = ClientConnect.rpc conn command in
  let result_json = to_json result in
  print_endline (Hh_json.json_to_string result_json)
