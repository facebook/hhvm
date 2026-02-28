(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type insert_patch = {
  pos: Pos.absolute;
  text: string;
}

type patch =
  | Insert of insert_patch
  | Remove of Pos.absolute
  | Replace of insert_patch

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
      pos: File_content.Position.t;
      new_name: string;
    }

type deprecated_wrapper_function_ref =
  | DeprecatedStaticMethodRef
  | DeprecatedNonStaticMethodRef
  | DeprecatedFunctionRef

let get_pos = function
  | Insert patch
  | Replace patch ->
    patch.pos
  | Remove p -> p

let compare_pos pos1 pos2 =
  let (char_start1, char_end1) = Pos.info_raw pos1 in
  let (char_start2, char_end2) = Pos.info_raw pos2 in
  if char_end1 <= char_start2 then
    -1
  else if char_end2 <= char_start1 then
    1
  else
    0

let compare_result res1 res2 = compare_pos (get_pos res1) (get_pos res2)

let write_string_to_file fn str =
  let oc = Out_channel.create fn in
  Out_channel.output_string oc str;
  Out_channel.close oc

let is_whitespace (c : char) =
  match c with
  | '\n'
  | ' '
  | '\012'
  | '\r'
  | '\t' ->
    true
  | _ -> false

let write_patches_to_buffer buf original_content patch_list =
  let len = String.length original_content in
  let rec advance_skip_whitespaces (i : int) : int =
    if i < len && is_whitespace original_content.[i] then
      advance_skip_whitespaces (i + 1)
    else
      i
  in
  (* Takes the substring of original_content between `i` and `j`
     and add it to the `buf` buffer.
     Advance cursor `i` to `j`. *)
  let add_original_content ~start:i ~end_:j : int =
    if j <= i then
      i
    else
      let size = j - i in
      let size = min (max 0 (max (len - i) 0)) size in
      let str_to_write = String.sub original_content ~pos:i ~len:size in
      Buffer.add_string buf str_to_write;
      i + size
  in
  let i =
    List.fold patch_list ~init:0 ~f:(fun i patch ->
        let pos = get_pos patch in
        let (char_start, char_end) = Pos.info_raw pos in
        let i = add_original_content ~start:i ~end_:char_start in
        match patch with
        | Insert { text; pos = _ } ->
          Buffer.add_string buf text;
          i
        | Replace { text; pos = _ } ->
          Buffer.add_string buf text;
          char_end
        | Remove _pos ->
          let i = char_end in
          advance_skip_whitespaces i)
  in
  let _i = add_original_content ~start:i ~end_:len in
  ()

let map_patches_to_filename acc patch =
  let pos = get_pos patch in
  let fn = Pos.filename pos in
  match SMap.find_opt fn acc with
  | Some lst -> SMap.add fn (patch :: lst) acc
  | None -> SMap.add fn [patch] acc

let apply_patches_to_string old_content patch_list =
  let buf = Buffer.create (String.length old_content) in
  let patch_list = List.sort ~compare:compare_result patch_list in
  write_patches_to_buffer buf old_content patch_list;
  Buffer.contents buf

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
