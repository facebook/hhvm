(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ClientEnv

let compare_pos pos1 pos2 =
  let (char_start1, char_end1) = Pos.info_raw pos1 in
  let (char_start2, char_end2) = Pos.info_raw pos2 in
  if char_end1 <= char_start2 then
    -1
  else if char_end2 <= char_start1 then
    1
  else
    0

let get_pos = function
  | ServerRefactorTypes.Insert patch
  | ServerRefactorTypes.Replace patch ->
    patch.ServerRefactorTypes.pos
  | ServerRefactorTypes.Remove p -> p

let compare_result res1 res2 = compare_pos (get_pos res1) (get_pos res2)

let map_patches_to_filename acc res =
  let pos = get_pos res in
  let fn = Pos.filename pos in
  match SMap.find_opt fn acc with
  | Some lst -> SMap.add fn (res :: lst) acc
  | None -> SMap.add fn [res] acc

let write_string_to_file fn str =
  let oc = Out_channel.create fn in
  Out_channel.output_string oc str;
  Out_channel.close oc

let write_patches_to_buffer buf original_content patch_list =
  let i = ref 0 in
  let trim_leading_whitespace = ref false in
  let len = String.length original_content in
  let is_whitespace c =
    match c with
    | '\n'
    | ' '
    | '\012'
    | '\r'
    | '\t' ->
      true
    | _ -> false
  in
  (* advances to requested character and adds the original content
     from the current position to that point to the buffer *)
  let add_original_content j =
    while
      !trim_leading_whitespace
      && !i < len
      && is_whitespace original_content.[!i]
    do
      i := !i + 1
    done;
    if j <= !i then
      ()
    else
      let size = j - !i in
      let size = min (- !i + len) size in
      let str_to_write = String.sub original_content !i size in
      Buffer.add_string buf str_to_write;
      i := !i + size
  in
  List.iter patch_list (fun res ->
      let pos = get_pos res in
      let (char_start, char_end) = Pos.info_raw pos in
      add_original_content char_start;
      trim_leading_whitespace := false;
      match res with
      | ServerRefactorTypes.Insert patch ->
        Buffer.add_string buf patch.ServerRefactorTypes.text
      | ServerRefactorTypes.Replace patch ->
        Buffer.add_string buf patch.ServerRefactorTypes.text;
        i := char_end
      | ServerRefactorTypes.Remove _ ->
        (* We only expect `Remove` to be used with HH_FIXMEs, in which case
         * char_end will point to the last character. Consequently, we should
         * increment it by 1 *)
        i := char_end + 1;
        trim_leading_whitespace := true);
  add_original_content len

let apply_patches_to_string old_content patch_list =
  let buf = Buffer.create (String.length old_content) in
  let patch_list = List.sort compare_result patch_list in
  write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

let apply_patches_to_file fn patch_list =
  let old_content = Sys_utils.cat fn in
  let new_file_contents = apply_patches_to_string old_content patch_list in
  write_string_to_file fn new_file_contents

let list_to_file_map =
  List.fold_left ~f:map_patches_to_filename ~init:SMap.empty

let apply_patches_to_file_contents file_contents patches =
  let file_map = list_to_file_map patches in
  let apply fn old_contents =
    match SMap.find_opt fn file_map with
    | Some patches -> apply_patches_to_string old_contents patches
    | None -> old_contents
  in
  SMap.mapi apply file_contents

let apply_patches patches =
  let file_map = list_to_file_map patches in
  SMap.iter apply_patches_to_file file_map;
  print_endline ("Rewrote " ^ string_of_int (SMap.cardinal file_map) ^ " files.")

let patch_to_json res =
  let (type_, replacement) =
    match res with
    | ServerRefactorTypes.Insert patch ->
      ("insert", patch.ServerRefactorTypes.text)
    | ServerRefactorTypes.Replace patch ->
      ("replace", patch.ServerRefactorTypes.text)
    | ServerRefactorTypes.Remove _ -> ("remove", "")
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
            ("patches", Hh_json.JSON_Array (List.map patch_list patch_to_json));
          ]
        :: acc
      end
      file_map
      []
  in
  Hh_json.json_to_string (Hh_json.JSON_Array entries)

let print_patches_json patches = print_endline (patches_to_json_string patches)

let go_ide
    (conn : unit -> ClientConnect.conn Lwt.t)
    (args : client_check_env)
    (filename : string)
    (line : int)
    (char : int)
    (new_name : string) : unit Lwt.t =
  let%lwt patches =
    ClientConnect.rpc_with_retry conn
    @@ ServerCommandTypes.IDE_REFACTOR
         { ServerCommandTypes.Ide_refactor_type.filename; line; char; new_name }
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
    (args : client_check_env)
    (mode : string)
    (before : string)
    (after : string) : unit Lwt.t =
  let command =
    match mode with
    | "Class" -> ServerRefactorTypes.ClassRename (before, after)
    | "Function" ->
      (*
        We set these to `None` here because we don't want to add a deprecated
          wrapper after the rename. Likewise for `MethodRename`
      *)
      let filename = None in
      let definition = None in
      ServerRefactorTypes.FunctionRename
        { filename; definition; old_name = before; new_name = after }
    | "Method" ->
      let befores = Str.split (Str.regexp "::") before in
      if List.length befores <> 2 then
        failwith "Before string should be of the format class::method"
      else
        ();
      let afters = Str.split (Str.regexp "::") after in
      if List.length afters <> 2 then
        failwith "After string should be of the format class::method"
      else
        ();
      let before_class = List.hd_exn befores in
      let before_method = List.hd_exn (List.tl_exn befores) in
      let after_class = List.hd_exn afters in
      let after_method = List.hd_exn (List.tl_exn afters) in
      if before_class <> after_class then (
        Printf.printf "%s %s\n" before_class after_class;
        failwith "Before and After classname must match"
      ) else
        let filename = None in
        let definition = None in
        ServerRefactorTypes.MethodRename
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
    ClientConnect.rpc_with_retry conn @@ ServerCommandTypes.REFACTOR command
  in
  if args.output_json then
    print_patches_json patches
  else
    apply_patches patches;
  Lwt.return_unit
