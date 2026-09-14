(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Client_env

let get_pos = Server_rename_types.get_pos

let compare_result = Server_rename_types.compare_result

let apply_patches_to_string old_content patch_list =
  let buf = Buffer.create (String.length old_content) in
  let patch_list = List.sort ~compare:compare_result patch_list in
  Server_rename_types.write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

let apply_patches_to_file fn patch_list =
  let old_content = Sys_utils.cat fn in
  let new_file_contents = apply_patches_to_string old_content patch_list in
  Server_rename_types.write_string_to_file fn new_file_contents

let list_to_file_map =
  List.fold_left ~f:Server_rename_types.map_patches_to_filename ~init:SMap.empty

let plural count one many =
  let obj =
    if count = 1 then
      one
    else
      many
  in
  string_of_int count ^ " " ^ obj

let apply_patches patches =
  let file_map = list_to_file_map patches in
  SMap.iter apply_patches_to_file file_map;
  print_endline
    ("Rewrote " ^ plural (SMap.cardinal file_map) "file" "files" ^ ".")

let patch_to_json res =
  let (type_, replacement) =
    match res with
    | Server_rename_types.Insert patch ->
      ("insert", patch.Server_rename_types.text)
    | Server_rename_types.Replace patch ->
      ("replace", patch.Server_rename_types.text)
    | Server_rename_types.Remove _ -> ("remove", "")
  in
  let pos = get_pos res in
  let (char_start, char_end) = Pos.info_raw pos in
  let (line, start, end_) = Pos.info_pos pos in
  `Assoc
    [
      ("char_start", `Int char_start);
      ("char_end", `Int char_end);
      ("line", `Int line);
      ("col_start", `Int start);
      ("col_end", `Int end_);
      ("patch_type", `String type_);
      ("replacement", `String replacement);
    ]

let patches_to_json_string patches =
  let file_map = list_to_file_map patches in
  let entries =
    SMap.fold
      begin
        fun fn patch_list acc ->
          `Assoc
            [
              ("filename", `String fn);
              ("patches", `List (List.map patch_list ~f:patch_to_json));
            ]
          :: acc
      end
      file_map
      []
  in
  Hh_json_helpers.Out.to_string (`List entries)

let print_patches_json patches = print_endline (patches_to_json_string patches)

let go_ide_from_patches patches json =
  if json then
    print_patches_json patches
  else
    apply_patches patches

let go
    (conn : unit -> Client_connect.conn Lwt.t)
    ~(desc : string)
    (args : client_check_env)
    (mode : rename_mode)
    ~(before : string)
    ~(after : string) : unit Lwt.t =
  let command =
    match mode with
    | Class -> Server_rename_types.ClassRename (before, after)
    | Function ->
      Server_rename_types.FunctionRename { old_name = before; new_name = after }
    | Method ->
      let befores = Str.split (Str.regexp "::") before in
      if List.length befores <> 2 then
        failwith "Before string should be of the format class::method";
      let afters = Str.split (Str.regexp "::") after in
      if List.length afters <> 2 then
        failwith "After string should be of the format class::method";
      let before_class = List.hd_exn befores in
      let before_method = List.hd_exn (List.tl_exn befores) in
      let after_class = List.hd_exn afters in
      let after_method = List.hd_exn (List.tl_exn afters) in
      if not (String.equal before_class after_class) then (
        Printf.printf "%s %s\n" before_class after_class;
        failwith "Before and After classname must match"
      ) else
        Server_rename_types.MethodRename
          {
            class_name = before_class;
            old_name = before_method;
            new_name = after_method;
          }
    | _ -> failwith "Unexpected Mode"
  in
  let%lwt patches =
    Client_connect.rpc_with_retry conn ~desc
    @@ Server_command_types.RENAME command
  in
  if args.output_json then
    print_patches_json patches
  else
    apply_patches patches;
  Lwt.return_unit
