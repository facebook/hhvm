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

(*****************************************************************************)
(* TextBlocks.make produces a list of indivisible blocks for a given file.
 *
 * Let's consider the following input:
 * 1   <?hh
 * 2   $x = 0;
 * 3   $y =
 * 4     1;
 *
 * The result will look like this:
 * [(1, 1, "<?hh\n");
 *  (2, 2, "$x = 0;\n");
 *  (3, 4, "$y = 1;\n");
 * ]
 *
 * The interesting case is the third one (3, 4, "$y = 1;\n"). It means that
 * we should consider the lines [3;4] as an indivisible block. In other words,
 * if we want to replace any line between 3 and 4, we would replace the
 * entire block.
 *
 * The formatted blocks will be interleaved with the existing code at line
 * boundaries, so it is important that the starts and ends of these blocks
 * line up with newlines in the original source (to avoid duplicated / missing
 * tokens). Format_hack.add_block_tag ensures this.
 *)
(*****************************************************************************)

module TextBlocks: sig

  type line_start = int
  type line_end = int
  type content = string

  type t = (line_start * line_end * content)

  (* Takes the output of the formatter with the "info" option on and
   * produces a list of indivisible text blocks.
   *)
  val make: (string * Format_hack.source_pos list) -> t list

end = struct

  type line_start = int
  type line_end = int
  type content = string

  type t = (line_start * line_end * content)

  type token =
    | Eof
    | Block
    | Line of int
    | Text of string

  (* Merges blocks when they overlap. *)
  let rec merge acc = function
    | [] -> List.rev acc
    | (start1, end1, content1) :: (start2, end2, content2) :: rl
      when start2 <= end1 ->
        merge acc ((start1, end2, content1^content2) :: rl)
    | x :: rl -> merge (x :: acc) rl

  let rec make_token_list text source_info acc i =
    match source_info with
    | [] -> List.rev (Text (String.sub text i (String.length text - i)) :: acc)
    | (char_pos, tag) :: source_info ->
        let acc = Text (String.sub text i (char_pos - i)) :: acc in
        let acc =
          (match tag with
          | Format_hack.Block -> Block
          | Format_hack.Line n -> Line n)
            :: acc
        in
        make_token_list text source_info acc char_pos

  let make_next text source_info =
    let list = ref (make_token_list text source_info [] 0) in
    fun () ->
      match !list with
      | [] -> Eof
      | tok :: rl -> list := rl; tok

  let rec make (text, source_info) =
    let next = make_next text source_info in
    let buffer = Buffer.create 256 in
    let list = loop 1 1 buffer [] next in
    merge [] list

  and loop line_start line_end buffer acc next =
    match next() with
    | Eof -> List.rev acc
    | Block ->
        let content = Buffer.contents buffer in
        if content = ""
        then loop line_start line_end buffer acc next
        else
          let block = line_start, line_end, content in
          Buffer.clear buffer;
          let line_end = line_end + 1 in
          loop line_end line_end buffer (block :: acc) next
    | Text str ->
        Buffer.add_string buffer str;
        loop line_start line_end buffer acc next
    | Line nbr ->
        let line_end = nbr in
        loop line_start line_end buffer acc next

end

(*****************************************************************************)
(* Given a list of intervals and a list of blocks, determine which
 * indivisible blocks are matching.
 *
 * 'intervals' correspond to the ranges that have been modified by a diff.
 * intervals = [(1, 3); (4; 4)] should be read as: the lines 1 to 3 and the
 * line 4 have been modified (cf Parse_diff module).
 *
 * 'blocks' is the list of indivisible blocks produced by TextBlocks.
 *
 * A block is selected if any of the intervals overlap with it.
 *)
(*****************************************************************************)

let rec matching_blocks acc intervals blocks =
  match intervals, blocks with
  | [], _ | _, [] -> List.rev acc
  | (modif_start, modif_end) :: rest_intervals,
    (block_start, block_end, _ as block) :: rest_blocks ->
      if modif_end < block_start
      then matching_blocks acc rest_intervals blocks
      else if block_end < modif_start
      then matching_blocks acc intervals rest_blocks
      else matching_blocks (block :: acc) intervals rest_blocks

(*
 * Diff output helpers
 *)

let mk_time_string {Unix.tm_sec; tm_min; tm_hour; tm_mday; tm_mon; tm_year; _} =
  Printf.sprintf "%d-%02d-%02d %02d:%02d:%02d +0000"
    (1900 + tm_year) tm_mon tm_mday tm_hour tm_min tm_sec

let print_file_header filename =
  let mtime = Unix.gmtime ((Unix.stat filename).Unix.st_mtime) in
  let now = Unix.gmtime (Unix.time ()) in
  Tty.(cprintf (Bold Default)) "--- %s\t%s\n" filename (mk_time_string mtime);
  Tty.(cprintf (Bold Default)) "+++ %s\t%s\n" filename (mk_time_string now)

let print_hunk (old_start_line, new_start_line, old_lines, new_lines) =
  let old_range = List.length old_lines in
  let new_range = List.length new_lines in
  Tty.cprintf Tty.(Normal Cyan) "@@ -%d,%d +%d,%d @@\n"
    old_start_line old_range new_start_line new_range;
  List.iter old_lines (Tty.cprintf Tty.(Normal Red) "-%s\n");
  List.iter new_lines (Tty.cprintf Tty.(Normal Green) "+%s\n")

(*****************************************************************************)
(* Applies the changes to a list of lines:
 * -) 'blocks' the list of indivisible blocks of formatted content
 *     (cf TextBlocks)
 * -) 'lines' the lines in the pre-formatted file
 *)
(*****************************************************************************)
let apply_blocks ~should_patch filename blocks lines =
  let buf = Buffer.create 1024 in
  let add_line buf s = Buffer.add_string buf s; Buffer.add_char buf '\n' in
  let first_hunk = ref true in
  let rec loop old_lineno new_lineno blocks lines =
    match blocks, lines with
    (* We reached the end of the input file. *)
    | _, [] -> ()

    (* We have no more modifications for that file. *)
    | [], line :: lines ->
        add_line buf line;
        loop (old_lineno + 1) (new_lineno + 1) blocks lines

    (* We have not reached the beginning of the block yet. *)
    | (start_block, _, _) :: _, line :: lines when old_lineno < start_block ->
        add_line buf line;
        loop (old_lineno + 1) (new_lineno + 1) blocks lines

    (* We found a block that must be replaced! *)
    | (start_block, end_block, new_content) :: blocks, lines ->
        (* Cut the old content. *)
        let old_lines, lines =
          List.split_n lines (end_block - start_block + 1) in
        let new_lines = String_utils.split_on_newlines new_content in
        let hunk = (start_block, new_lineno, old_lines, new_lines) in
        let new_lineno = new_lineno + (List.length new_lines) in
        (* only show nonempty hunks *)
        if old_lines = new_lines
        then Buffer.add_string buf new_content
        else begin
          if !first_hunk then begin
            print_file_header filename;
            first_hunk := false;
          end;
          print_hunk hunk;
          if should_patch (old_lines, new_lines)
          then Buffer.add_string buf new_content
          else List.iter old_lines (add_line buf);
        end;
        loop (end_block + 1) new_lineno blocks lines
  in
  loop 1 1 blocks lines;
  Buffer.contents buf

(*****************************************************************************)
(* Formats a diff (in place) *)
(*****************************************************************************)

type file_diff = Path.t * (int * int) list

let parse_diff prefix diff_text =
  List.map (Parse_diff.go diff_text) (fun (filename, modified_lines) ->
    Path.concat prefix filename, modified_lines
  )

let rec apply modes apply_mode ~diff:file_and_lines_modified =
  List.iter file_and_lines_modified begin fun (filepath, modified_lines) ->
    let file_content = Path.cat filepath in
    apply_file modes apply_mode filepath file_content modified_lines
  end

and apply_file modes apply_mode filepath file_content modified_lines =
  let filename = Path.to_string filepath in
  let result =
    Format_hack.program_with_source_metadata modes filepath file_content in
  match result with
  | Format_hack.Success formatted_content ->
      apply_formatted apply_mode filepath formatted_content file_content
        modified_lines
  | Format_hack.Disabled_mode ->
      Printf.fprintf stderr "PHP FILE: skipping %s\n" filename
  | Format_hack.Parsing_error _ ->
      Printf.fprintf stderr "Parsing error: %s\n" filename
  | Format_hack.Internal_error ->
      Printf.fprintf stderr "*** PANIC *** Internal error!: %s\n" filename

and apply_formatted apply_mode filepath formatted_content file_content
    modified_lines =
  let blocks = TextBlocks.make formatted_content in
  let blocks = matching_blocks [] modified_lines blocks in
  let lines = String_utils.split_on_newlines file_content in
  let filename = Path.to_string filepath in
  let should_patch =
    if apply_mode <> Format_mode.Patch
    then fun _ -> true
    else fun (old_lines, new_lines) ->
      let choice = Tty.read_choice "Apply this patch?" ['y'; 'n'; 'q'] in
      FormatEventLogger.patch_choice filename (old_lines, new_lines) choice;
      match choice with
      | 'y' -> true
      | 'n' -> false
      | 'q' -> raise Exit
      | _ -> assert false
  in
  let formatted =
    try Some (apply_blocks ~should_patch filename blocks lines)
    with Exit -> None
  in
  match formatted with
  | None -> ()
  | Some _ when apply_mode = Format_mode.Print -> ()
  | Some content ->
      try
        let outc = open_out filename in
        output_string outc content;
        close_out outc
      with Sys_error _ ->
        Printf.eprintf "Error: could not modify file %s\n" filename
