(*
 * Copyright (c) 2015-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** This allows one to fake having multiple files in one file. This
  is used only in unit test files.
  Indeed, there are some features that require mutliple files to be tested.
  For example, newtype has a different meaning depending on the file. *)

type path = string

type content = string

let delim_regexp = "////.*\n"

let delim = Str.regexp delim_regexp

let short_suffix path =
  Relative_path.suffix path |> Str.replace_first (Str.regexp "^.*--") ""

let split_multifile_content content =
  let rec make_files = function
    | [] -> []
    | Str.Delim header :: Str.Text content :: rl ->
      let pattern = Str.regexp "\n////" in
      let header = Str.global_replace pattern "" header in
      let pattern = Str.regexp "[ |\n]*" in
      let filename = Str.global_replace pattern "" header in
      (filename, content) :: make_files rl
    | _ -> assert false
  in
  let contents =
    Str.full_split (Str.regexp ("\n" ^ delim_regexp)) ("\n" ^ content)
  in
  make_files contents

(* We have some hacky "syntax extensions" to have one file contain multiple
 * files, which can be located at arbitrary paths. This is useful e.g. for
 * testing lint rules, some of which activate only on certain paths. It's also
 * useful for testing abstract types, since the abstraction is enforced at the
 * file boundary.
 * Takes the path to a single file, returns a map of filenames to file contents.
 *)
let file_to_file_list file =
  let join_path p1 p2 =
    let open Char in
    if String.(p1 <> "" && p2 <> "") then
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
  if Str.string_match delim content 0 then
    let files = split_multifile_content content in
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
      try Str.matched_group 3 first_line with
      | Caml.Not_found -> abs_fn
    in
    let file =
      Relative_path.create Relative_path.Dummy (join_path dir file_name)
    in
    let content = String.concat ~sep:"\n" (List.tl_exn contentl) in
    [(file, content)]
  ) else
    [(file, content)]

let file_to_files file = file_to_file_list file |> Relative_path.Map.of_list

(** Given a path of the form `sample/file/name.php--another/file/name.php`,
  read in the portion of multifile `sample/file/name.php` corresponding
  to `another/file/name.php`. *)
let read_file_from_multifile path : string list =
  let splitter = ".php--" in
  let ix = String_utils.substring_index splitter path in
  if ix >= 0 then
    let abs_fn =
      String_utils.string_before path (ix + String.length ".php")
      |> Relative_path.create Relative_path.Dummy
    in
    let rel_path = Relative_path.create Relative_path.Dummy path in
    let files = file_to_files abs_fn in
    match Relative_path.Map.find_opt files rel_path with
    | None -> []
    | Some content -> Str.split (Str.regexp "\n") content
  else
    []

let print_files_as_multifile files =
  if Int.equal (Relative_path.Map.cardinal files) 1 then
    Out_channel.output_string stdout (Relative_path.Map.choose files |> snd)
  else
    Relative_path.Map.fold files ~init:[] ~f:(fun fn content acc ->
        content
        :: Printf.sprintf "//// %s" (Relative_path.to_absolute fn)
        :: acc)
    |> List.rev
    |> String.concat ~sep:"\n"
    |> Out_channel.output_string stdout

(** This module handles multifiles with internal file names like
  'base-xxx.php' and 'changed-xxx.php' *)
module States : sig
  val base_files : path -> content Relative_path.Map.t

  val changed_files : path -> content Relative_path.Map.t
end = struct
  let files path ~prefix =
    let content = Sys_utils.cat path in
    let files = split_multifile_content content in
    List.filter_map files ~f:(fun (sub_fn, content) ->
        match String.chop_prefix sub_fn ~prefix:(prefix ^ "-") with
        | None -> None
        | Some sub_fn ->
          Some
            ( Relative_path.create Relative_path.Dummy (path ^ "--" ^ sub_fn),
              content ))
    |> Relative_path.Map.of_list

  let base_files = files ~prefix:"base"

  let changed_files = files ~prefix:"changed"
end
