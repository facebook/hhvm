(*
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* This allows one to fake having multiple files in one file. This
 * is used only in unit test files.
 * Indeed, there are some features that require mutliple files to be tested.
 * For example, newtype has a different meaning depending on the file.
 *)
let rec make_files = function
  | [] -> []
  | Str.Delim header :: Str.Text content :: rl ->
    let pattern = Str.regexp "////" in
    let header = Str.global_replace pattern "" header in
    let pattern = Str.regexp "[ |\n]*" in
    let filename = Str.global_replace pattern "" header in
    (filename, content) :: make_files rl
  | _ -> assert false

(* We have some hacky "syntax extensions" to have one file contain multiple
 * files, which can be located at arbitrary paths. This is useful e.g. for
 * testing lint rules, some of which activate only on certain paths. It's also
 * useful for testing abstract types, since the abstraction is enforced at the
 * file boundary.
 * Takes the path to a single file, returns a map of filenames to file contents.
 *)
let file_to_file_list file =
  let join_path p1 p2 =
    if p1 <> "" && p2 <> "" then
      if p1.[String.length p1 - 1] = '/' && p2.[0] = '/' then
        p1 ^ Str.string_after p2 1
      else if p1.[String.length p1 - 1] <> '/' && p2.[0] <> '/' then
        p1 ^ "/" ^ p2
      else
        p1 ^ p2
    else
      p1 ^ p2
  in
  let abs_fn = Relative_path.to_absolute file in
  let content = Sys_utils.cat abs_fn in
  let delim = Str.regexp "////.*\n" in
  if Str.string_match delim content 0 then
    let contentl = Str.full_split delim content in
    let files = make_files contentl in
    List.map files ~f:(fun (sub_fn, c) ->
        (Relative_path.create Relative_path.Dummy (abs_fn ^ "--" ^ sub_fn), c))
  else if String.is_prefix content ~prefix:"// @directory " then (
    let contentl = Str.split (Str.regexp "\n") content in
    let first_line = List.hd_exn contentl in
    let regexp =
      Str.regexp "^// @directory *\\([^ ]*\\) *\\(@file *\\([^ ]*\\)*\\)?"
    in
    let has_match = Str.string_match regexp first_line 0 in
    assert has_match;
    let dir = Str.matched_group 1 first_line in
    let file_name =
      (try Str.matched_group 3 first_line with Caml.Not_found -> abs_fn)
    in
    let file =
      Relative_path.create Relative_path.Dummy (join_path dir file_name)
    in
    let content = String.concat ~sep:"\n" (List.tl_exn contentl) in
    [(file, content)]
  ) else
    [(file, content)]

let file_to_files file = file_to_file_list file |> Relative_path.Map.of_list
