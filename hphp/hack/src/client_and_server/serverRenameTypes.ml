(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type patch =
  | Insert of insert_patch
  | Remove of Pos.absolute
  | Replace of insert_patch

and insert_patch = {
  pos: Pos.absolute;
  text: string;
}

type action =
  | ClassRename of string * string (* old_name * new_name *)
  | ClassConstRename of string * string * string
  (* class_name * old_name * new_name *)
  | MethodRename of {
      class_name: string;
      old_name: string;
      new_name: string;
    }
  | FunctionRename of {
      old_name: string;
      new_name: string;
    }
  | LocalVarRename of {
      filename: Relative_path.t;
      file_content: string;
      line: int;
      char: int;
      new_name: string;
    }
[@@deriving show]

type deprecated_wrapper_function_ref =
  | DeprecatedStaticMethodRef
  | DeprecatedNonStaticMethodRef
  | DeprecatedFunctionRef

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
  | Insert patch
  | Replace patch ->
    patch.pos
  | Remove p -> p

let compare_result res1 res2 = compare_pos (get_pos res1) (get_pos res2)

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
      let str_to_write = String.sub original_content ~pos:!i ~len:size in
      Buffer.add_string buf str_to_write;
      i := !i + size
  in
  List.iter patch_list ~f:(fun res ->
      let pos = get_pos res in
      let (char_start, char_end) = Pos.info_raw pos in
      add_original_content char_start;
      trim_leading_whitespace := false;
      match res with
      | Insert patch -> Buffer.add_string buf patch.text
      | Replace patch ->
        Buffer.add_string buf patch.text;
        i := char_end
      | Remove _ ->
        i := char_end;
        trim_leading_whitespace := true);
  add_original_content len

let map_patches_to_filename acc res =
  let pos = get_pos res in
  let fn = Pos.filename pos in
  match SMap.find_opt fn acc with
  | Some lst -> SMap.add fn (res :: lst) acc
  | None -> SMap.add fn [res] acc

let apply_patches_to_string old_content patch_list =
  let buf = Buffer.create (String.length old_content) in
  let patch_list = List.sort ~compare:compare_result patch_list in
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
    match SMap.find_opt (Relative_path.to_absolute fn) file_map with
    | Some patches -> apply_patches_to_string old_contents patches
    | None -> old_contents
  in
  Relative_path.Map.mapi ~f:apply file_contents
