(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Syntax = Full_fidelity_positioned_syntax
module Trivia = Full_fidelity_positioned_trivia
module TriviaKind = Full_fidelity_trivia_kind

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

type finder = { comments: comment array }

let make_docblock_finder (comments : (Pos.t * Prim_defs.comment) list) : finder
    =
  (* The Hack parser produces comments in reverse but sorted order. *)
  let comments =
    Array.of_list
      (List.rev_map comments ~f:(fun (pos, cmt) ->
           let str = Prim_defs.string_of_comment cmt in
           (Pos.end_line pos, str, Prim_defs.is_line_comment cmt)))
  in
  { comments }

(* Binary search for the index of the last comment before a given line. *)

(* Merge all consecutive line comments preceding prev_line.
 * Stop when we reach last_line. *)
let rec merge_line_comments
    (finder : finder)
    (idx : int)
    (last_line : int)
    (prev_line : int)
    (acc : string list) : string list =
  if idx < 0 then
    acc
  else
    let (line, str, is_line_comment) = finder.comments.(idx) in
    if is_line_comment && line > last_line && line = prev_line - 1 then
      merge_line_comments finder (idx - 1) last_line line (("//" ^ str) :: acc)
    else
      acc

let find_last_comment_index finder line =
  Utils.infimum finder.comments line (fun (comment_line, _, _) line ->
      comment_line - line)

let open_multiline = Str.regexp "^/\\*\\(\\*?\\) *"

let close_multiline = Str.regexp " *\\*/$"

(** Tidies up a delimited comment.

    1. Strip the leading `/*` and trailing `*/`.
    2. Remove leading whitespace equal to the least amount of whitespace
       before any comment lines after the first (since the first line is on
       the same line as the opening `/*`, it will almost always have only a
       single leading space).

       We remove leading whitespace equal to the least amount rather than
       removing all leading whitespace in order to preserve manual indentation
       of text in the doc block.

    3. Remove leading `*` characters to properly handle box-style multiline
       doc blocks. Without this they would be formatted as Markdown lists.

  Known failure cases:
    1. A doc block which is legitimately just a list of items will need at
       least one non-list item which is under-indented compared to the list in
       order to not strip all of the whitespace from the list items. The
       easiest way to do this is to write a little one line summary before the
       list and place it on its own line instead of directly after the `/*`.
*)
let tidy_delimited_comment comment =
  let comment =
    comment
    |> Str.replace_first open_multiline ""
    |> Str.replace_first close_multiline ""
  in
  let lines = String_utils.split_into_lines comment in
  let line_trimmer =
    match lines with
    | []
    | [_] ->
      Caml.String.trim
    | _hd :: tail ->
      let get_whitespace_count x = String_utils.index_not_opt x " " in
      let counts = List.filter_map ~f:get_whitespace_count tail in
      let min =
        List.min_elt counts ~compare:(fun a b -> a - b)
        |> Option.value ~default:0
      in
      let removal =
        Str.regexp (Printf.sprintf "^%s\\(\\* ?\\)?" (String.make min ' '))
      in
      Str.replace_first removal ""
  in
  lines |> List.map ~f:line_trimmer |> String.concat ~sep:"\n"

let find_docblock (finder : finder) (last_line : int) (line : int) :
    string option =
  match find_last_comment_index finder line with
  | Some comment_index ->
    let (comment_line, str, is_line_comment) =
      finder.comments.(comment_index)
    in
    if is_line_comment then
      match merge_line_comments finder comment_index last_line line [] with
      | [] -> None
      | lines -> Some (Caml.String.trim (String.concat ~sep:"" lines))
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
      finder.comments.(last_comment_index)
    in
    if comment_line = line then
      if is_line_comment then
        Some (Caml.String.trim ("//" ^ str))
      else
        Some ("/*" ^ str ^ "*/")
    else
      None
  | None -> None

(* Regexp matching single-line comments, either # foo or // foo . *)
let line_comment_prefix = Str.regexp "^\\(//\\|#\\) ?"

let get_docblock node =
  let rec helper trivia_list acc eols_until_exit =
    match (trivia_list, eols_until_exit) with
    | ([], _)
    | (Trivia.{ kind = TriviaKind.EndOfLine; _ } :: _, 0) ->
      begin
        match acc with
        | [] -> None
        | comments ->
          let comment =
            comments
            |> List.map ~f:(Str.replace_first line_comment_prefix "")
            |> String.concat ~sep:"\n"
            |> Caml.String.trim
          in
          Some comment
      end
    | (hd :: tail, _) ->
      begin
        match Trivia.kind hd with
        | TriviaKind.DelimitedComment when List.is_empty acc ->
          Some (tidy_delimited_comment (Trivia.text hd))
        | TriviaKind.SingleLineComment -> helper tail (Trivia.text hd :: acc) 1
        | TriviaKind.WhiteSpace -> helper tail acc eols_until_exit
        | TriviaKind.EndOfLine -> helper tail acc (eols_until_exit - 1)
        | TriviaKind.FixMe
        | TriviaKind.IgnoreError ->
          helper tail acc 1
        | TriviaKind.DelimitedComment
        | TriviaKind.FallThrough
        | TriviaKind.ExtraTokenError ->
          (* Short circuit immediately. *)
          helper [] acc 0
      end
  in
  let trivia_list = Syntax.leading_trivia node in
  (* Set the starting EOL count to 2 instead of 1 because it's valid to have up
     to one blank line between the end of a docblock and the start of its
     associated element. *)
  helper (List.rev trivia_list) [] 2
