(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(*****************************************************************************)
(* Section building a list of intervals per file for a given diff.
 *
 * Typically, the output of git/hg looks like:
 * --- a/path/filename
 * +++ b/path/filename
 * @@ -line,length +line, length @@
 *
 * The information that we are interested in is the filename after +++
 * and the line number after '+' in the header section (the section after @@).
 * That's because we don't really care about what has been removed.
 * What we want is to format the new content, not the old one.
 * ParseDiff builds a list of intervals of modified lines for each file in
 * a diff.
 *
 * For example: ["myfile1", [4, 6; 7, 7]] means that the file named "myfile1"
 * has modified lines, from line 4 to 6 (inclusive) and the line 7.
 *)
(*****************************************************************************)

type filename = string
type interval = int * int
type file_diff = filename * interval list

type env = {
  (* The file we are currently parsing (None for '/dev/null') *)
  mutable file: string option;

  (* The list of lines that have been modified *)
  mutable modified: int list;

  (* The current line *)
  mutable line: int;

  (* The accumulator (for the result) *)
  mutable result: file_diff list;
}

(* The entry point *)
let rec go content =
  let env = { file = None; modified = []; line = 0; result = [] } in
  let lines = String_utils.split_on_newlines content in
  start env lines;
  List.rev env.result

(* Skip the text before the first +++ (to make things work with git show) *)
and start env = function
  | [] -> ()
  | line :: lines
    when String_utils.string_starts_with line "+++" ->
      header env line;
      modified env 0 lines
  | _ :: lines -> start env lines

(* Parses the content of a line starting with +++ (extracts the filename) *)
and header env line =
  add_file env;
  let filename = String.sub line 4 (String.length line - 4) in
  (* Getting rid of the prefix b/ *)
  let filename =
    if filename = Sys_utils.null_path
    then None
    else if String.length filename >= 2 && String.sub filename 0 2 = "b/"
    then Some (String.sub filename 2 (String.length filename - 2))
    else Some filename
  in
  env.file <- filename;
  env.modified <- []

(* Parses the lines *)
and modified env nbr = function
  | [] -> add_file env
  | line :: lines
    when String.length line > 4 && String.sub line 0 3 = "+++" ->
      header env line;
      modified env 0 lines
  | line :: lines
    when String.length line > 2 && String.sub line 0 2 = "@@" ->
      (* Find the position right after '+' in '@@ -line,len +line, len@@' *)
      let _ = Str.search_forward (Str.regexp "[+][0-9]+") line 0 in
      let matched = Str.matched_string line in
      let matched = String.sub matched 1 (String.length matched - 1) in
      let nbr = int_of_string matched in
      modified env nbr lines
  | line :: lines
    when String.length line >= 1 && String.sub line 0 1 = "+" ->
      (* Adds the line to the list of modified lines *)
      env.line <- env.line + 1;
      env.modified <- nbr :: env.modified;
      modified env (nbr+1) lines
  | line :: lines
    when String.length line >= 1 && String.sub line 0 1 = "-" ->
      (* Skips the line (we don't care about removed code) *)
      modified env nbr lines
  | _ :: lines ->
      modified env (nbr+1) lines

and add_file env =
  (* Given a list of modified lines => returns a list of intervals *)
  let lines_modified = List.rev_map env.modified (fun x -> (x, x)) in
  let lines_modified = normalize_intervals [] lines_modified in
  (* Adds the file to the list of results *)
  match env.file with
  | None -> ()
  | Some filename ->
      env.result <- (filename, lines_modified) :: env.result

(* Merges intervals when necessary.
 * For example: '[(1, 2), (2, 2), (2, 5); ...]' becomes '[(1, 5); ...]'.
 *)
and normalize_intervals acc = function
  | [] -> List.rev acc
  | (start1, end1) :: (start2, end2) :: rl when end1 + 1 >= start2 ->
      normalize_intervals acc ((min start1 start2, max end1 end2) :: rl)
  | x :: rl -> normalize_intervals (x :: acc) rl
