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

type content = string [@@deriving show]

type repo = content Relative_path.Map.t [@@deriving show]

let delim_regexp = "////.*\n"

let delim = Str.regexp delim_regexp

let short_suffix path =
  Relative_path.suffix path |> Str.replace_first (Str.regexp "^.*--") ""

let split_multifile_content content : (string * content) list =
  let file_name_of_header header =
    let pattern = Str.regexp "\n////" in
    let header = Str.global_replace pattern "" header in
    let pattern = Str.regexp "[ |\n]*" in
    let filename = Str.global_replace pattern "" header in
    filename
  in
  let rec make_files = function
    | [] -> []
    | Str.Delim header :: Str.Text content :: rl ->
      let filename = file_name_of_header header in
      (filename, content) :: make_files rl
    | Str.Delim header :: rl ->
      (* We can have a Delim not followed by a Text if the file content is empty. *)
      let filename = file_name_of_header header in
      (filename, "") :: make_files rl
    | Str.Text txt :: _ ->
      failwith (Printf.sprintf "unexpected split result `Text (%s)`" txt)
  in
  let contents =
    Str.full_split (Str.regexp ("\n" ^ delim_regexp)) ("\n" ^ content)
  in
  make_files contents

(** Concatenate both provided strings with "--" in between
  and make a Relative_path out of the result. *)
let full_path (real_filename : string) (sub_filename : string) : Relative_path.t
    =
  Relative_path.create Relative_path.Dummy (real_filename ^ "--" ^ sub_filename)

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
    List.map files ~f:(fun (sub_fn, c) -> (full_path abs_fn sub_fn, c))
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
      | Stdlib.Not_found -> abs_fn
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

module States = struct
  type change =
    | Modified of content
    | Deleted
  [@@deriving show]

  type repo_change = change Relative_path.Map.t [@@deriving show]

  type t = {
    base: repo;
    changes: repo_change list;
  }
  [@@deriving show]

  let apply_repo_change (base : repo) (repo_change : repo_change) : repo =
    Relative_path.Map.fold repo_change ~init:base ~f:(fun path change repo ->
        match change with
        | Deleted -> Relative_path.Map.remove repo path
        | Modified content -> Relative_path.Map.add repo ~key:path ~data:content)

  let is_only_slashes s = String.to_list s |> List.for_all ~f:(Char.equal '/')

  (** The file we parse here starts as a regular multifile to represent the base state of the repo,
    then each repo change is separated by a line of only slashes, like

    ////////////

    Between those delimiters, we have other multifiles representing either file changes
    (or file addition if the file wasn't present in the base state),
    or file deletion if the file name is prefixed with "deleted-"
    (in which case the file content is ignored and assumed to be empty. *)
  let parse_series_of_repo_changes path file_contents : t =
    let rec parse_bases file_contents base_acc =
      match file_contents with
      | [] -> ([], base_acc)
      | (filename, content) :: file_contents ->
        if is_only_slashes filename then
          (file_contents, base_acc)
        else
          let base_acc =
            Relative_path.Map.add
              base_acc
              ~key:(full_path path filename)
              ~data:content
          in
          parse_bases file_contents base_acc
    in
    let (file_contents, base) =
      parse_bases file_contents Relative_path.Map.empty
    in
    let (last_repo_change, past_repo_changes) =
      List.fold
        file_contents
        ~init:(Relative_path.Map.empty, [])
        ~f:(fun (current_repo_change, past_repo_changes) (filename, content) ->
          if is_only_slashes filename then
            (Relative_path.Map.empty, current_repo_change :: past_repo_changes)
          else
            let (filename, change) =
              match String.chop_prefix ~prefix:"deleted-" filename with
              | None -> (filename, Modified content)
              | Some filename -> (filename, Deleted)
            in
            let current_repo_change =
              Relative_path.Map.add
                current_repo_change
                ~key:(full_path path filename)
                ~data:change
            in
            (current_repo_change, past_repo_changes))
    in
    let changes = List.rev (last_repo_change :: past_repo_changes) in
    { base; changes }

  let files path file_contents ~prefix =
    List.filter_map file_contents ~f:(fun (sub_fn, content) ->
        match String.chop_prefix sub_fn ~prefix:(prefix ^ "-") with
        | None -> None
        | Some sub_fn ->
          Some
            ( Relative_path.create Relative_path.Dummy (path ^ "--" ^ sub_fn),
              content ))
    |> Relative_path.Map.of_list

  (** Get the base files from a multifile, simarly to {!file_to_files),
    stripping the "base-" prefix for internal file names. *)
  let base_files = files ~prefix:"base"

  (** Get the base files from a multifile, simarly to {!file_to_files),
    stripping the "changed-" prefix for internal file names. *)
  let changed_files = files ~prefix:"changed"

  let parse path : t =
    let content = Sys_utils.cat path in
    let file_contents = split_multifile_content content in
    let has_multi_repo_changes =
      List.exists file_contents ~f:(fun (filename, _) ->
          is_only_slashes filename)
    in
    if has_multi_repo_changes then
      parse_series_of_repo_changes path file_contents
    else
      {
        base = base_files path file_contents;
        changes =
          [
            changed_files path file_contents
            |> Relative_path.Map.map ~f:(fun content -> Modified content);
          ];
      }
end
