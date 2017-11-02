(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(**
 * This is a simple data structure that allows querying for the docblock
 * given a line in the source code. Rough description:
 *
 * 1. Find the last comment preceding the line of the definition.
 *    We also make sure this doesn't overlap with the preceding definition.
 *    If the last comment is more than 1 line away, it is ignored.
 *
 * 2. If the last comment is a block-style comment (/* */) just return it.
 *
 * 3. Otherwise (if it is a line style comment //) attempt to merge it with
 *    preceding line comments, if they exist.
 *      NOTE: We also enforce that line comments must be on the definition's
 *            immediately preceding line.
 *)

(* line, string, is_line_comment *)
type comment = int * string * bool

type finder = {
  comments: comment array;
}

let make_docblock_finder (comments: (Pos.t * Prim_defs.comment) list) : finder =
  (* The Hack parser produces comments in reverse but sorted order. *)
  let comments = Array.of_list (
    List.rev_map comments ~f:begin fun (pos, cmt) ->
      let str = Prim_defs.string_of_comment cmt in
      (File_pos.line (Pos.pos_end pos), str, Prim_defs.is_line_comment cmt)
    end
  ) in
  { comments }

(* Binary search for the index of the last comment before a given line. *)

(* Merge all consecutive line comments preceding prev_line.
 * Stop when we reach last_line. *)
let rec merge_line_comments (finder : finder)
                            (idx : int)
                            (last_line : int)
                            (prev_line : int)
                            (acc : string list) : string list =
  if idx < 0 then acc
  else begin
    let (line, str, is_line_comment) = Array.get finder.comments idx in
    if is_line_comment && line > last_line && line = prev_line - 1 then
      merge_line_comments finder (idx - 1) last_line line (("//" ^ str) :: acc)
    else acc
  end

let find_last_comment_index finder line =
  Utils.infimum finder.comments line
    (fun (comment_line, _, _) line -> comment_line - line)

let find_docblock (finder : finder)
                  (last_line : int)
                  (line : int) : string option =
  match find_last_comment_index finder line with
  | Some comment_index ->
      let (comment_line, str, is_line_comment) =
        Array.get finder.comments comment_index in
      if is_line_comment then begin
        match merge_line_comments finder comment_index last_line line [] with
        | [] -> None
        | lines -> Some (String.trim (String.concat "" lines))
      end
      else if comment_line > last_line && comment_line >= line - 2 then
        Some ("/*" ^ str ^ "*/")
      else
        None
  | None -> None

(* Find the last comment on `line` if it exists. *)
let find_inline_comment (finder : finder) (line : int) : string option =
  match find_last_comment_index finder (line + 1) with
  | Some last_comment_index ->
    let (comment_line, str, is_line_comment) =
      Array.get finder.comments last_comment_index in
    if comment_line = line then begin
      if is_line_comment then
        Some (String.trim ("//" ^ str))
      else
        Some ("/*" ^ str ^ "*/")
    end
    else
      None
  | None -> None
