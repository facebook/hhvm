(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ClientEnv

let get_pos = ServerRenameTypes.get_pos

let compare_result = ServerRenameTypes.compare_result

let apply_patches_to_string old_content patch_list =
  let buf = Buffer.create (String.length old_content) in
  let patch_list = List.sort ~compare:compare_result patch_list in
  ServerRenameTypes.write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

let apply_patches_to_file fn patch_list =
  let old_content = Sys_utils.cat fn in
  let new_file_contents = apply_patches_to_string old_content patch_list in
  ServerRenameTypes.write_string_to_file fn new_file_contents

let list_to_file_map =
  List.fold_left ~f:ServerRenameTypes.map_patches_to_filename ~init:SMap.empty

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
    | ServerRenameTypes.Insert patch -> ("insert", patch.ServerRenameTypes.text)
    | ServerRenameTypes.Replace patch ->
      ("replace", patch.ServerRenameTypes.text)
    | ServerRenameTypes.Remove _ -> ("remove", "")
  in
  let pos = get_pos res in
  let (char_start, char_end) = Pos.info_raw pos in
  let (line, start, end_) = Pos.info_pos pos in
  Hh_json.JSON_Object
    [
      ("char_start", Hh_json.int_ char_start);
      ("char_end", Hh_json.int_ char_end);
      ("line", Hh_json.int_ line);
      ("col_start", Hh_json.int_ start);
      ("col_end", Hh_json.int_ end_);
      ("patch_type", Hh_json.JSON_String type_);
      ("replacement", Hh_json.JSON_String replacement);
    ]

let patches_to_json_string patches =
  let file_map = list_to_file_map patches in
  let entries =
    SMap.fold
      begin
        fun fn patch_list acc ->
          Hh_json.JSON_Object
            [
              ("filename", Hh_json.JSON_String fn);
              ( "patches",
                Hh_json.JSON_Array (List.map patch_list ~f:patch_to_json) );
            ]
          :: acc
      end
      file_map
      []
  in
  Hh_json.json_to_string (Hh_json.JSON_Array entries)

let print_patches_json patches = print_endline (patches_to_json_string patches)

let go_sound_dynamic
    (conn : unit -> ClientConnect.conn Lwt.t)
    (args : client_check_env)
    (mode : rename_mode)
    (name : string) =
  let command =
    match mode with
    | Class -> ServerRenameTypes.ClassRename (name, "")
    | Function ->
      let filename = None in
      let definition = None in
      ServerRenameTypes.FunctionRename
        { filename; definition; old_name = name; new_name = "" }
    | _ -> failwith "Unexpected Mode"
  in
  ClientConnect.rpc_with_retry conn ~desc:args.desc
  @@ ServerCommandTypes.RENAME_CHECK_SD command

let go_ide
    (conn : unit -> ClientConnect.conn Lwt.t)
    ~(desc : string)
    (args : client_check_env)
    (filename : string)
    (line : int)
    (char : int)
    (new_name : string) : unit Lwt.t =
  let%lwt patches =
    ClientConnect.rpc_with_retry conn ~desc
    @@ ServerCommandTypes.IDE_RENAME
         { ServerCommandTypes.Ide_rename_type.filename; line; char; new_name }
  in
  let patches =
    match patches with
    | Ok patches -> patches
    | Error message -> failwith message
  in
  if args.output_json then
    print_patches_json patches
  else
    apply_patches patches;
  Lwt.return_unit

let go
    (conn : unit -> ClientConnect.conn Lwt.t)
    ~(desc : string)
    (args : client_check_env)
    (mode : rename_mode)
    (before : string)
    (after : string) : unit Lwt.t =
  let command =
    match mode with
    | Class -> ServerRenameTypes.ClassRename (before, after)
    | Function ->
      (*
        We set these to `None` here because we don't want to add a deprecated
          wrapper after the rename. Likewise for `MethodRename`
      *)
      let filename = None in
      let definition = None in
      ServerRenameTypes.FunctionRename
        { filename; definition; old_name = before; new_name = after }
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
        let filename = None in
        let definition = None in
        ServerRenameTypes.MethodRename
          {
            filename;
            definition;
            class_name = before_class;
            old_name = before_method;
            new_name = after_method;
          }
    | _ -> failwith "Unexpected Mode"
  in
  let%lwt patches =
    ClientConnect.rpc_with_retry conn ~desc @@ ServerCommandTypes.RENAME command
  in
  if args.output_json then
    print_patches_json patches
  else
    apply_patches patches;
  Lwt.return_unit
