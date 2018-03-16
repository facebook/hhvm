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

let open_multiline = Str.regexp "^/\\*\\(\\*?\\) *"
let close_multiline = Str.regexp " *\\*/$"
let line_prefix = Str.regexp "^// ?"

(** Tidies up a comment.

    # Multiline comment
    When given a multiline comment (`/* */`), this will do the following tidying:
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

    # Single line comment
    When given a list of single line comments (`//`), just strip the leading
    forward slashes and the first space after the second slash.
*)
let tidy_comment comment =
  let is_line_comment = Str.string_match line_prefix comment 0 in
  let comment =
    if is_line_comment
    then comment
    else
      comment
      |> Str.replace_first open_multiline ""
      |> Str.replace_first close_multiline ""
  in
  let lines = String_utils.split_into_lines comment in
  if is_line_comment
  then
    lines
    |> List.map ~f:(Str.replace_first line_prefix "")
    |> String.concat "\n"
  else
    let line_trimmer = match lines with
      | []
      | [_] -> String.trim
      | _hd :: tail ->
        let get_whitespace_count x =
          String_utils.index_not_opt x " "
        in
        let counts = List.filter_map ~f:get_whitespace_count tail in
        let min =
          List.min_elt counts ~cmp:(fun a b -> a - b)
          |> Option.value ~default:0
        in
        let removal = Str.regexp (Printf.sprintf "^%s\\(\\* ?\\)?" (String.make min ' ')) in
        Str.replace_first removal ""
    in
    lines
    |> List.map ~f:line_trimmer
    |> String.concat "\n"

let find_docblock (finder : finder)
                  ?(last_line = 0)
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

let find_single_docblock ?(tidy = false)
                         (relative_path : Relative_path.t)
                         (line : int) : string option =
  let open Option.Monad_infix in
  File_heap.get_contents relative_path
  >>= begin fun contents ->
    let env = Full_fidelity_ast.make_env ~include_line_comments:true relative_path in
    let results = Full_fidelity_ast.from_text_with_legacy env contents in
    let finder = make_docblock_finder (results.Parser_hack.comments) in
    find_docblock finder line
  end
  >>| (fun docblock -> if tidy then tidy_comment docblock else docblock)
  |> Option.filter ~f:begin fun comment ->
    (* Make sure to not add the `// strict` part of `<?hh // strict`
       if we're looking for something at the start of the file. *)
    not (line = 2 && comment = "strict")
  end

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
