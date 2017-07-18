(**
 * Copyright (c) 2013-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "flow" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Asttypes
open Parsetree
open Ast_mapper
open Ast_helper

let with_in_channel filename f =
  let ic = open_in_bin filename in
  try let res = f ic in close_in ic; res
  with exn -> close_in ic; raise exn

(* Helper to extract the file contents *)
let string_of_file filename =
  with_in_channel filename @@ fun ic ->
  let s = Bytes.create 32759 in
  let b = Buffer.create 1000 in
  let rec iter ic b s =
    let nread = input ic s 0 32759 in
    if nread > 0 then begin
      Buffer.add_substring b s 0 nread;
      iter ic b s
    end in
  iter ic b s;
  Buffer.contents b

(* Normalize the directory by removing a trailing directory separator *)
let normalize_dir dir =
  let sep = Filename.dir_sep in
  let sep_len = String.length sep in
  (* Check the last sep_len characters *)
  let trailing = String.sub dir ((String.length dir) - sep_len) sep_len in
  (* Strip the trailing separators if necessary *)
  if trailing = sep then String.sub dir 0 ((String.length dir) - sep_len)
  else dir

(* Read in all the files below the hhi directory *)
let get_recursive_files root =
  let rec loop dirs files = match dirs with
    | [] -> files
    | d::ds -> begin
      let curr_files = Sys.readdir d in
      (* Process the files in the next directory *)
      let (dirs', files') = Array.fold_left begin fun (d_acc, f_acc) f ->
        let f = Filename.concat d f in
        if (Sys.is_directory f) then (f::d_acc, f_acc)
        else (d_acc, f::f_acc)
      end (ds, files) curr_files in
      (* And then process the rest, eventually we'll exhaust the dirs list *)
      loop dirs' files'
    end
  in loop [root] []

let get_hhis dir =
  (* Chop off the trailing slash, since the full filename from the recursive
   * walk will always join paths *)
  let dir = normalize_dir dir in
  let dir_offset = (String.length dir) + (String.length Filename.dir_sep) in
  get_recursive_files dir
  |> List.fold_left (fun acc file ->
     (* Skip non-hhi in the directory *)
     if not (Filename.check_suffix file "hhi") then acc
     else begin
       let contents = string_of_file file in
       let file = String.sub file dir_offset ((String.length file) - dir_offset) in
       (file, contents)::acc
     end
  ) []

(* Turn the (name, contents) list into a PPX ast (string * string) array
 * expression *)
let contents =
  let hhi_dir = Sys.argv.(1) in
  get_hhis hhi_dir
  |> List.map (fun (name, contents) -> Exp.tuple [
      Exp.constant (Const.string name); Exp.constant (Const.string contents);
    ])
  |> Exp.array

(* Whenever we see [%hhi_contents], replace it with all of the hhis *)
let ppx_gen_hhi_mapper _argv =
 { default_mapper with
   expr = fun mapper expr ->
     match expr with
     | { pexp_desc = Pexp_extension ({ txt = "hhi_contents"; _ }, PStr []); _} ->
       contents
     | other -> default_mapper.expr mapper other; }

let () =
  register "ppx_gen_hhi" ppx_gen_hhi_mapper
