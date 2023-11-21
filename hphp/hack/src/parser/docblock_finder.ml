(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
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
      Stdlib.String.trim
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
      | lines -> Some (Stdlib.String.trim (String.concat ~sep:"" lines))
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
        Some (Stdlib.String.trim ("//" ^ str))
      else
        Some ("/*" ^ str ^ "*/")
    else
      None
  | None -> None

(* Regexp matching single-line comments, either # foo or // foo . *)
let line_comment_prefix = Str.regexp "^\\(//\\|#\\) ?"

(**
 * Extract a // comment docblock that is directly before this node.
 *
 *     // A function.
 *     // It is useful.
 *     function foo(): void {}
 *
 * In this example, we return "A function.\nIt is useful."
 *)
let get_single_lines_docblock (node : Syntax.t) : string option =
  let rec aux trivia_list acc (seen_newline : bool) : string list =
    match trivia_list with
    | [] -> acc
    | hd :: tail ->
      (match Trivia.kind hd with
      | TriviaKind.SingleLineComment -> aux tail (Trivia.text hd :: acc) false
      | TriviaKind.WhiteSpace ->
        (* Step over whitespace. *)
        aux tail acc seen_newline
      | TriviaKind.EndOfLine ->
        (* Stop if we've seen consecutive newlines, as that means
           we've reached a blank line. *)
        if seen_newline then
          acc
        else
          aux tail acc true
      | TriviaKind.FixMe
      | TriviaKind.IgnoreError ->
        (* Step over HH_FIXME comments. *)
        aux tail acc false
      | TriviaKind.DelimitedComment
      | TriviaKind.FallThrough
      | TriviaKind.ExtraTokenError ->
        (* Stop if we see a /* ... */ comment, a FALLTHROUGH
           comment, or a syntax error. *)
        acc)
  in

  match aux (List.rev (Syntax.leading_trivia node)) [] false with
  | [] -> None
  | comment_lines ->
    Some
      (comment_lines
      |> List.map ~f:(Str.replace_first line_comment_prefix "")
      |> String.concat ~sep:"\n"
      |> Stdlib.String.trim)

(**
 * Extract a /* ... */ comment docblock that is before this node,
 * even if there are blank lines present.
 *
 *)
let get_delimited_docblock (node : Syntax.t) : string option =
  let rec aux trivia_list : string option =
    match trivia_list with
    | [] -> None
    | hd :: tail ->
      (match Trivia.kind hd with
      | TriviaKind.DelimitedComment ->
        (* We've found the comment. *)
        Some (tidy_delimited_comment (Trivia.text hd))
      | TriviaKind.WhiteSpace
      | TriviaKind.EndOfLine
      | TriviaKind.FixMe
      | TriviaKind.IgnoreError ->
        (* Step over whitespace and HH_FIXME comments. *)
        aux tail
      | TriviaKind.SingleLineComment ->
        (* Give up if we have a // comment. *)
        None
      | TriviaKind.FallThrough
      | TriviaKind.ExtraTokenError ->
        (* Give up if we have a // FALLTHROUGH comment or syntax error. *)
        None)
  in
  aux (List.rev (Syntax.leading_trivia node))

let get_docblock (node : Syntax.t) : string option =
  Option.first_some
    (get_single_lines_docblock node)
    (get_delimited_docblock node)
