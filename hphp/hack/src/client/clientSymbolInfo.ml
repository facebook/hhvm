(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go
    (conn : ClientConnect.conn)
    ~(desc : string)
    (files : string)
    (expand_path : string -> string) : unit Lwt.t =
  let file_list =
    match files with
    | "-" ->
      let content = Sys_utils.read_stdin_to_string () in
      Str.split (Str.regexp "\n") content
    | _ -> Str.split (Str.regexp ";") files
  in
  let expand_path_list file_list =
    List.rev_map file_list ~f:(fun file_path -> expand_path file_path)
  in
  let command =
    ServerCommandTypes.DUMP_SYMBOL_INFO (expand_path_list file_list)
  in
  let%lwt (result, _telemetry) = ClientConnect.rpc conn ~desc command in
  let result_json = ServerCommandTypes.Symbol_info_service.to_json result in
  print_endline (Hh_json.json_to_string result_json);
  Lwt.return_unit
