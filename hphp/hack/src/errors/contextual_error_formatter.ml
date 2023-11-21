(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open String_utils

let column_width line_number = max 3 (Errors.num_digits line_number)

let line_margin (line_num : int option) col_width : string =
  let padded_num =
    match line_num with
    | Some line_num -> Printf.sprintf "%*d" col_width line_num
    | None -> String.make col_width ' '
  in
  Tty.apply_color (Tty.Normal Tty.Cyan) (padded_num ^ " |")

(* Get the lines of source code associated with this position. *)
let load_context_lines (pos : Pos.absolute) : string list =
  let path = Pos.filename pos in
  let line = Pos.line pos in
  let end_line = Pos.end_line pos in

  let lines = Errors.read_lines path in
  (* Line numbers are 1-indexed. *)
  List.filteri lines ~f:(fun i _ -> i + 1 >= line && i + 1 <= end_line)

let format_context_lines (pos : Pos.absolute) (lines : string list) col_width :
    string =
  let lines =
    match lines with
    | [] -> [Tty.apply_color (Tty.Dim Tty.Default) "No source found"]
    | ls -> ls
  in
  let line_num = Pos.line pos in
  let format_line i (line : string) =
    Printf.sprintf "%s %s" (line_margin (Some (line_num + i)) col_width) line
  in
  let formatted_lines = List.mapi ~f:format_line lines in
  (* TODO: display all the lines, showing the underline on all of them. *)
  List.hd_exn formatted_lines

let format_filename (pos : Pos.absolute) : string =
  let relative_path path =
    let cwd = Filename.concat (Sys.getcwd ()) "" in
    lstrip path cwd
  in
  let filename = relative_path (Pos.filename pos) in
  Printf.sprintf
    "   %s %s"
    (Tty.apply_color (Tty.Normal Tty.Cyan) "-->")
    (Tty.apply_color (Tty.Normal Tty.Green) filename)

(* Format this message as "  ^^^ You did something wrong here". *)
let format_substring_underline
    (pos : Pos.absolute)
    (msg : string)
    (first_context_line : string option)
    is_first
    col_width : string =
  let (start_line, start_column) = Pos.line_column pos in
  let (end_line, end_column) = Pos.end_line_column pos in
  let underline_width =
    match first_context_line with
    | None -> 4 (* Arbitrary choice when source isn't available. *)
    | Some first_context_line ->
      if start_line = end_line then
        end_column - start_column
      else
        String.length first_context_line - start_column
  in
  let underline = String.make (max underline_width 1) '^' in
  let underline_padding =
    if Option.is_some first_context_line then
      String.make start_column ' '
    else
      ""
  in
  let color =
    if is_first then
      Tty.Bold Tty.Red
    else
      Tty.Dim Tty.Default
  in
  Printf.sprintf
    "%s %s%s"
    (line_margin None col_width)
    underline_padding
    (Tty.apply_color
       color
       (if is_first then
         underline
       else
         underline ^ " " ^ msg))

(* Format the line of code associated with this message, and the message itself. *)
let format_message (msg : string) (pos : Pos.absolute) ~is_first ~col_width :
    string * string =
  let col_width =
    Option.value col_width ~default:(column_width (Pos.line pos))
  in
  let context_lines = load_context_lines pos in
  let pretty_ctx = format_context_lines pos context_lines col_width in
  let pretty_msg =
    format_substring_underline
      pos
      msg
      (List.hd context_lines)
      is_first
      col_width
  in
  (pretty_ctx, pretty_msg)

(* Work out the column width needed for each file. Files with many
   lines need a wider column due to the higher line numbers. *)
let col_widths (msgs : Pos.absolute Message.t list) : int Core.String.Map.t =
  (* Find the longest line number for every file in msgs. *)
  let longest_lines =
    List.fold msgs ~init:String.Map.empty ~f:(fun acc msg ->
        let filename = Pos.filename (Message.get_message_pos msg) in
        let current_max = Option.value (Map.find acc filename) ~default:0 in
        Map.set
          acc
          ~key:filename
          ~data:(max current_max (Pos.line (Message.get_message_pos msg))))
  in
  String.Map.map longest_lines ~f:column_width

(** Format the list of messages in a given error with context.
    The list may not be ordered, and multiple messages may occur on one line.
 *)
let format_error (error : Errors.finalized_error) : string =
  (* Sort messages such that messages in the same file are together.
     Does not reorder the files or messages within a file. *)
  let msgs =
    User_error.get_messages error
    |> Errors.combining_sort ~f:(fun msg ->
           Message.get_message_pos msg |> Pos.filename)
  in
  (* The first message is the 'primary' message, so add a boolean to distinguish it. *)
  let rec label_first msgs is_first =
    match msgs with
    | msg :: msgs -> (msg, is_first) :: label_first msgs false
    | [] -> []
  in
  let labelled_msgs = label_first msgs true in
  (* Sort messages by line number, so we can display with context. *)
  let cmp (m1, _) (m2, _) =
    match
      String.compare
        (Message.get_message_pos m1 |> Pos.filename)
        (Message.get_message_pos m2 |> Pos.filename)
    with
    | 0 ->
      Int.compare
        (Message.get_message_pos m1 |> Pos.line)
        (Message.get_message_pos m2 |> Pos.line)
    | _ -> 0
  in
  let sorted_msgs = List.stable_sort ~compare:cmp labelled_msgs in
  (* For every message, show it alongside the relevant line. If there
     are multiple messages associated with the line, only show it once. *)
  let col_widths = col_widths msgs in
  let rec aux msgs prev : string list =
    match msgs with
    | (msg, is_first) :: msgs ->
      let pos = Message.get_message_pos msg in
      let filename = Pos.filename pos in
      let line = Pos.line pos in
      let col_width = Map.find col_widths filename in
      let (pretty_ctx, pretty_msg) =
        format_message (Message.get_message_str msg) pos ~is_first ~col_width
      in
      let formatted : string list =
        match prev with
        | Some (prev_filename, prev_line)
          when String.equal prev_filename filename && prev_line = line ->
          (* Previous message was on this line too, just show the message itself*)
          [pretty_msg]
        | Some (prev_filename, _) when String.equal prev_filename filename ->
          (* Previous message was this file, but an earlier line. *)
          [pretty_ctx; pretty_msg]
        | _ -> [format_filename pos; pretty_ctx; pretty_msg]
      in
      formatted @ aux msgs (Some (filename, line))
    | [] -> []
  in
  String.concat ~sep:"\n" (aux sorted_msgs None) ^ "\n"

let to_string
    ?(claim_color : Tty.raw_color option) (error : Errors.finalized_error) :
    string =
  let error_code = User_error.get_code error in
  let custom_msgs = error.User_error.custom_msgs in
  let msgl = User_error.to_list error in
  let buf = Buffer.create 50 in
  let color = Option.value claim_color ~default:Tty.Red in
  (match msgl with
  | [] -> failwith "Impossible: an error always has non-empty list of messages"
  | (_, msg) :: _ ->
    Buffer.add_string
      buf
      (Printf.sprintf
         "%s %s\n"
         (Tty.apply_color
            (Tty.Bold color)
            (User_error.error_code_to_string error_code))
         (Tty.apply_color (Tty.Bold Tty.Default) msg)));
  (try Buffer.add_string buf (format_error error) with
  | _ ->
    Buffer.add_string
      buf
      "Error could not be pretty-printed. Please file a bug.");
  Buffer.add_string buf "\n";
  if not @@ List.is_empty custom_msgs then
    Buffer.add_string
      buf
      (String.concat ~sep:"\n" error.User_error.custom_msgs ^ "\n");
  Buffer.contents buf
