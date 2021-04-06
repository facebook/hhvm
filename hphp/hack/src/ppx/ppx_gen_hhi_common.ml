(*
 * Copyright (c) 2013-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "flow" directory of this source tree.
 *
 *)

let with_in_channel filename f =
  let ic = open_in_bin filename in
  try
    let res = f ic in
    close_in ic;
    res
  with exn ->
    close_in ic;
    raise exn

(* Helper to extract the file contents *)
let string_of_file filename =
  let string_size = 32759 and buffer_size = 1000 in
  with_in_channel filename @@ fun ic ->
  let s = Bytes.create string_size in
  let b = Buffer.create buffer_size in
  let rec iter ic b s =
    let nread = input ic s 0 string_size in
    if nread > 0 then (
      Buffer.add_substring b (Bytes.to_string s) 0 nread;
      iter ic b s
    )
  in
  iter ic b s;
  Buffer.contents b

(* Normalize the directory by removing a trailing directory separator *)
let normalize_dir dir =
  let sep = Filename.dir_sep in
  let sep_len = String.length sep in
  (* Check the last sep_len characters *)
  let trailing = String.sub dir (String.length dir - sep_len) sep_len in
  (* Strip the trailing separators if necessary *)
  if String.equal trailing sep then
    String.sub dir 0 (String.length dir - sep_len)
  else
    dir

(* Read in all the files below the hhi directory *)
let get_recursive_files root =
  let rec loop dirs files =
    match dirs with
    | [] -> files
    | d :: ds ->
      let curr_files = Sys.readdir d in
      (* Process the files in the next directory *)
      let (dirs', files') =
        Array.fold_left
          begin
            fun (d_acc, f_acc) f ->
            let f = Filename.concat d f in
            if Sys.is_directory f then
              (f :: d_acc, f_acc)
            else
              (d_acc, f :: f_acc)
          end
          (ds, files)
          curr_files
      in
      (* And then process the rest, eventually we'll exhaust the dirs list *)
      loop dirs' files'
  in
  loop [root] []

let get_hhis_in_dir dir =
  (* Chop off the trailing slash, since the full filename from the recursive
   * walk will always join paths *)
  let dir = normalize_dir dir in
  let dir_offset = String.length dir + String.length Filename.dir_sep in
  get_recursive_files dir
  |> List.fold_left
       (fun acc file ->
         (* Skip non-hhi in the directory *)
         if not (Filename.check_suffix file "hhi") then
           acc
         else
           let contents = string_of_file file in
           let file =
             String.sub file dir_offset (String.length file - dir_offset)
           in
           (file, contents) :: acc)
       []

let get_hhis hhi_dir hsl_dir =
  let handwritten_hhis = get_hhis_in_dir hhi_dir in
  let generated_hsl_hhis =
    get_hhis_in_dir hsl_dir
    |> List.map (fun (name, contents) -> ("hsl_generated/" ^ name, contents))
  in
  handwritten_hhis @ generated_hsl_hhis
