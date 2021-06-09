(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open String_utils

type error_code = int

type marker = int * Tty.raw_color

type marked_message = {
  original_index: int;
  marker: marker;
  message: Pos.absolute Errors.message;
}

(* A position_group is a record composed of a position (we'll call
   the aggregate_position) and a list of marked_message. Each
   marked_message contains a position, and all of these positions are
   closed enough together to be printed in one coalesced code context.

   For example, say we have the following code:
      1 | <?hh
      2 |
      3 | function f(): dict<int,
      4 |   int>
      5 | {
      6 |   return "hello";
      7 | }
      8 |

   We would be given three individual positions (one corresponding to line 2
   and two corresponding to lines 6), i.e.

      Typing[4110] Invalid return type [1]
      -> Expected dict<int, int> [2]
      -> But got string [1]

   They are close enough together so that code only needs to be printed once
   for all of them since their context overlaps. Thus the position_group would
   be composed of a list of the three individual aforementioned positions, as
   well as the aggregate_position whose first line is 1 and last line 7, and
   the final output would look like this.

      1 | <?hh
      2 |
  [2] 3 | function f(): dict<int,
  [2] 4 |   int>
      5 | {
  [1] 6 |   return "hello";
      7 | }
*)
type position_group = {
  aggregate_position: Pos.absolute;
  messages: marked_message list;
}

let n_extra_lines_hl = 2

(* 'char' because this is intended to be a string of length 1 *)
let out_of_bounds_char = "_"

let background_highlighted = Tty.apply_color (Tty.Dim Tty.Default)

let default_highlighted s = s

let line_num_highlighted line_num =
  background_highlighted (string_of_int line_num)

(* Checks if a line (or line/col pair) is inside a position,
   taking care to handle zero-width positions and positions
   spanning multiple lines.

   Some examples:
                  0         1         2
                  01234567890123456789012
     Example 1: 1 class Foo extends    {}

     Example 2: 1 function }

     Example 3: 1 function {
                2   return 3;
                3 }

   In example 1, the error position is [1,21] - [1,21) with length 1;
    column 21 of line 1 is within that error position.
   In example 2, error position is [1,0] - [1,0) with length 0;
    column 0 of line 1 is within that error position.
   In Example 2, the error position is [1,10] - [2,0) with length 1;
    column 10 of line 1 (but nothing in line 2) is within that error position.
*)
let pos_contains (pos : Pos.absolute) ?(col_num : int option) (line_num : int) :
    bool =
  let (first_line, first_col) = Pos.line_column pos in
  let (last_line, last_col) = Pos.end_line_column pos in
  let is_col_okay = Option.value_map col_num ~default:true in
  let last_col =
    match Pos.length pos with
    | 0 -> last_col + 1
    | _ -> last_col
  in
  (* A position's first line, first column, and last line
     are all checked inclusively. The position's last column
     is checked inclusively if the length of the position is zero,
     and exclusively otherwise, in order to correctly highlight
     zero-width positions. Line_num is 1-indexed, col_num is 0-indexed. *)
  if first_line = last_line then
    first_line = line_num
    && is_col_okay ~f:(fun cn -> first_col <= cn && cn < last_col)
  else if line_num = first_line then
    is_col_okay ~f:(fun cn -> first_col <= cn)
  else if line_num = last_line then
    (* When only checking for a line's inclusion in a position
      (i.e. when col_num = None), special care must be taken when
      the line we're checking is the last line of the position:
      since the end_column is exclusive, the end_column's value
      must be non-zero signifying there is at least one character
      on it to be highlighted. *)
    last_col > 0 && is_col_okay ~f:(fun cn -> cn < last_col)
  else
    first_line < line_num && line_num < last_line

(* Gets the list of unique marker/position tuples associated with a line. *)
let markers_and_positions_for_line
    (position_group : position_group) (line_num : int) :
    (marker * Pos.absolute) list =
  List.filter position_group.messages ~f:(fun mm ->
      pos_contains (Errors.get_message_pos mm.message) line_num)
  |> List.dedup_and_sort ~compare:(fun mm1 mm2 ->
         let (mk1, _) = mm1.marker in
         let (mk2, _) = mm2.marker in
         Int.compare mk1 mk2)
  |> List.map ~f:(fun mm -> (mm.marker, Errors.get_message_pos mm.message))

(* Gets the string containing the markers that should be displayed next to this line,
   e.g. something like "[1,4,5]" *)
let markers_string
    (position_group : position_group) ?(apply_color = true) (line_num : int) =
  let add_color f s =
    if apply_color then
      f s
    else
      s
  in
  let markers =
    markers_and_positions_for_line position_group line_num |> List.map ~f:fst
  in
  match markers with
  | [] -> ""
  | markers ->
    let ms =
      List.map markers ~f:(fun (marker, color) ->
          let marker_str = string_of_int marker in
          add_color (Tty.apply_color (Tty.Normal color)) marker_str)
    in
    let lbracket = add_color background_highlighted "[" in
    let rbracket = add_color background_highlighted "]" in
    let comma = add_color background_highlighted "," in
    let prefix =
      Printf.sprintf "%s%s%s" lbracket (String.concat ~sep:comma ms) rbracket
    in
    prefix

let line_margin_highlighted position_group line_num col_width_raw : string =
  (* Separate the margin into several sections:
    |markers|space(s)|line_num|space|vertical_bar|
    i.e.
    [1,2]  9 |
    [3]   10 |
    We need to do this because each has its own color and length
  *)
  let nspaces =
    let markers_raw_len =
      markers_string position_group line_num ~apply_color:false |> String.length
    in
    let line_num_raw_len = string_of_int line_num |> String.length in
    max (col_width_raw - (markers_raw_len + line_num_raw_len)) 1
  in
  let markers_pretty = markers_string position_group line_num in
  let spaces = String.make nspaces ' ' in
  let line_num_pretty = line_num_highlighted line_num in
  let prefix =
    Printf.sprintf
      "%s%s%s%s"
      markers_pretty
      spaces
      line_num_pretty
      (background_highlighted " |")
  in
  prefix

(* line_num is 1-based *)
let line_highlighted position_group ~(line_num : int) ~(line : string option) =
  match line with
  | None -> Tty.apply_color (Tty.Dim Tty.Default) "No source found"
  | Some line ->
    (match markers_and_positions_for_line position_group line_num with
    | [] -> default_highlighted line
    | ms ->
      let get_markers_at_col col_num =
        List.filter ms ~f:(fun (_, pos) -> pos_contains pos ~col_num line_num)
      in
      let color_column ms c =
        (* Prefer shorter smaller positions so the boundaries between overlapping
          positions are visible. Take the lowest-number (presumably more important)
          to break ties. *)
        let ((_, color), _) =
          List.stable_sort ms ~compare:(fun ((m1, _), p1) ((m2, _), p2) ->
              match Int.compare (Pos.length p1) (Pos.length p2) with
              | 0 -> Int.compare m1 m2
              | v -> v)
          |> List.hd_exn
        in
        Tty.apply_color (Tty.Normal color) c
      in
      let highlighted_columns : string list =
        (* Not String.mapi because we don't want to return a char from each element *)
        List.mapi (String.to_list line) ~f:(fun i c ->
            match get_markers_at_col i with
            | [] -> default_highlighted (Char.to_string c)
            | ms -> color_column ms (Char.to_string c))
      in
      (* Add extra column when position spans multiple lines and we're on the last line. Handles
        the case where a position exists for after file but there is no character to highlight *)
      let extra_column =
        let (end_line, end_col) =
          Pos.end_line_column position_group.aggregate_position
        in
        if line_num = end_line || (line_num + 1 = end_line && end_col = 0) then
          match get_markers_at_col (String.length line) with
          | [] -> ""
          | ms -> color_column ms out_of_bounds_char
        else
          ""
      in
      String.concat highlighted_columns ^ extra_column)

(* Prefixes each line with its corresponding formatted margin *)
let format_context_lines_highlighted
    ~(position_group : position_group)
    ~(lines : (int * string option) list)
    ~(col_width : int) : string =
  let format_line (line_num, line) =
    Printf.sprintf
      "%s %s"
      (line_margin_highlighted position_group line_num col_width)
      (line_highlighted position_group ~line_num ~line)
  in
  let formatted_lines = List.map ~f:format_line lines in
  String.concat ~sep:"\n" formatted_lines

(* Gets lines from a position, including additional before/after
   lines from the current position; line numbers are 1-indexed.
   If the position references one extra line than actually
   in the file, we append a line with a sentinel character ('_')
   so that we have a line to highlight later. *)
let load_context_lines_for_highlighted ~before ~after ~(pos : Pos.absolute) :
    (int * string option) list =
  let path = Pos.filename pos in
  let (start_line, _start_col) = Pos.line_column pos in
  let (end_line, end_col) = Pos.end_line_column pos in
  let lines = Errors.read_lines path in
  match lines with
  | [] ->
    List.range ~start:`inclusive ~stop:`inclusive start_line end_line
    |> List.map ~f:(fun line_num -> (line_num, None))
  | lines ->
    let numbered_lines = List.mapi lines ~f:(fun i l -> (i + 1, Some l)) in
    let original_lines =
      List.filter numbered_lines ~f:(fun (i, _) ->
          i >= start_line - before && i <= end_line + after)
    in
    let additional_line =
      let last_line =
        match List.last original_lines with
        | Some (last_line, _) -> last_line
        | None -> 0
      in
      if end_line > last_line && end_col > 0 then
        Some (end_line, out_of_bounds_char)
      else
        None
    in
    (match additional_line with
    | None -> original_lines
    | Some (line_num, additional) ->
      original_lines @ [(line_num, Some additional)])

let format_context_highlighted
    (col_width : int) (position_group : position_group) =
  let lines =
    load_context_lines_for_highlighted
      ~before:n_extra_lines_hl
      ~after:n_extra_lines_hl
      ~pos:position_group.aggregate_position
  in
  format_context_lines_highlighted ~position_group ~lines ~col_width

(* The column width will be the length of the largest prefix before
   the " |", which is composed of the list of markers, a space, and
   the line number. The column size will be the same for all messages
   in this error, regardless of file, so that they are all aligned.
   Returns the length of the raw (uncolored) prefix string.

   For example:
    overlap_pos.php:11:10
           8 |
           9 |
    [2]   10 | function returns_int(): int {
    [1,3] 11 |   return 'foo';
          12 | }
          13 |

    The length of the column is the length of "[1,3] 11" = 8 *)
let col_width_for_highlighted (position_groups : position_group list) =
  let largest_line_length =
    let message_positions (messages : marked_message list) =
      List.map messages ~f:(fun mm -> Errors.get_message_pos mm.message)
    in
    let line_nums =
      List.map position_groups ~f:(fun pg -> pg.messages |> message_positions)
      |> List.concat_no_order
      |> List.map ~f:Pos.end_line
    in
    List.max_elt line_nums ~compare:Int.compare
    |> Option.value ~default:0
    |> Errors.num_digits
  in
  let max_marker_prefix_length =
    let markers_strs (pg : position_group) =
      let pos = pg.aggregate_position in
      let start_line = Pos.line pos in
      let end_line = Pos.end_line pos in
      List.range start_line (end_line + 1)
      |> List.map ~f:(fun line_num ->
             markers_string pg ~apply_color:false line_num)
    in
    let marker_lens =
      List.map position_groups ~f:markers_strs
      |> List.concat
      |> List.map ~f:String.length
    in
    List.max_elt marker_lens ~compare:Int.compare |> Option.value ~default:0
  in
  (* +1 for the space between them *)
  let col_width = max_marker_prefix_length + 1 + largest_line_length in
  col_width

(* Each list of position_groups in the returned list is from a different file.
   The first position_group in the list will contain _at least_ the first (i.e. main)
   message. The list of marked_messages in each position group is ordered by increasing
   line number (so the main message isn't necessarily first). *)
let position_groups_by_file marker_and_msgs : position_group list list =
  let msgs_by_file : marked_message list list =
    List.group marker_and_msgs ~break:(fun mm1 mm2 ->
        let p1 = Errors.get_message_pos mm1.message in
        let p2 = Errors.get_message_pos mm2.message in
        String.compare (Pos.filename p1) (Pos.filename p2) <> 0)
    (* Must make sure list of positions is ordered *)
    |> List.map ~f:(fun msgs ->
           List.sort
             ~compare:(fun mm1 mm2 ->
               let p1 = Errors.get_message_pos mm1.message in
               let p2 = Errors.get_message_pos mm2.message in
               Int.compare (Pos.line p1) (Pos.line p2))
             msgs)
  in
  let close_enough prev_pos curr_pos =
    let line1_end = Pos.end_line prev_pos in
    let line2_begin = Pos.line curr_pos in

    (* Example:
      line1_end = 10:
      [3] 10 |   $z = 3 * $x;
          11 |   if ($z is int) {
          12 |     echo 'int';

      line2_begin = 16
          14 |     echo 'not int';
          15 |   }
      [1] 16 |   return $z;

      Then they should be conjoined as such:
      [3] 10 |   $z = 3 * $x;
          11 |   if ($z is int) {
          12 |     echo 'int';
          13 |   } else
          14 |     echo 'not int';
          15 |   }
      [1] 16 |   return $z;

      If they were any farther away, the line
          13 |   } else
      would be replaced with
             :
      to signify more than one interposing line.
     *)
    line1_end + n_extra_lines_hl + 2 >= line2_begin - n_extra_lines_hl
  in
  (* Group marked messages that are sufficiently close. *)
  let grouped_messages (messages : marked_message list) :
      marked_message list list =
    List.group messages ~break:(fun prev_mm curr_mm ->
        let prev_pos = Errors.get_message_pos prev_mm.message in
        let curr_pos = Errors.get_message_pos curr_mm.message in
        not (close_enough prev_pos curr_pos))
  in
  List.map msgs_by_file ~f:(fun (mmsgl : marked_message list) ->
      grouped_messages mmsgl
      |> List.map ~f:(fun messages ->
             {
               aggregate_position =
                 List.map
                   ~f:(fun mm -> Errors.get_message_pos mm.message)
                   messages
                 |> List.reduce_exn ~f:Pos.merge;
               messages;
             }))

let format_file_name_and_pos (pgl : position_group list) =
  let relative_path path =
    let cwd = Filename.concat (Sys.getcwd ()) "" in
    lstrip path cwd
  in
  (* Primary position for the file is the one with the lowest-numbered
     marker across all positions in each position_group of the list *)
  let primary_pos =
    List.concat_map pgl ~f:(fun pg -> pg.messages)
    |> List.stable_sort ~compare:(fun mm1 mm2 ->
           let (mk1, _) = mm1.marker in
           let (mk2, _) = mm2.marker in
           Int.compare mk1 mk2)
    |> List.hd_exn
    |> fun { message; _ } -> message |> Errors.get_message_pos
  in
  let filename = relative_path (Pos.filename primary_pos) in
  let (line, col) = Pos.line_column primary_pos in
  let dirname =
    let dn = Filename.dirname filename in
    if String.equal Filename.current_dir_name dn then
      ""
    else
      background_highlighted (Filename.dirname filename ^ "/")
  in
  let filename = default_highlighted (Filename.basename filename) in
  let pretty_filename = Printf.sprintf "%s%s" dirname filename in
  let pretty_position =
    Printf.sprintf ":%d:%d" line (col + 1) |> background_highlighted
  in
  let line_info = pretty_filename ^ pretty_position in
  line_info

let format_all_contexts_highlighted (marker_and_msgs : marked_message list) =
  (* Create a set of position_groups (set of positions that can be written in the same snippet) *)
  let position_groups = position_groups_by_file marker_and_msgs in
  let col_width = col_width_for_highlighted (List.concat position_groups) in
  let sep =
    Printf.sprintf
      "\n%s %s\n"
      (String.make col_width ' ')
      (background_highlighted ":")
  in
  let contexts =
    List.map position_groups ~f:(fun pgl ->
        (* Each position_groups list (pgl) is for a single file, so separate each with ':' *)
        let fn_pos = format_file_name_and_pos pgl in
        let ctx_strs = List.map pgl ~f:(format_context_highlighted col_width) in
        fn_pos ^ "\n" ^ String.concat ~sep ctx_strs)
  in
  String.concat ~sep:"\n\n" contexts ^ "\n\n"

let single_marker_highlighted marker =
  let (n, color) = marker in
  let lbracket = background_highlighted "[" in
  let rbracket = background_highlighted "]" in
  let npretty = Tty.apply_color (Tty.Normal color) (string_of_int n) in
  lbracket ^ npretty ^ rbracket

let format_claim_highlighted
    (error_code : error_code)
    (marker : error_code * Tty.raw_color)
    (msg : string) : string =
  let suffix = single_marker_highlighted marker in
  let (_, color) = marker in
  let pretty_error_code =
    Tty.apply_color (Tty.Bold color) (Errors.error_code_to_string error_code)
  in
  (* The color of any highlighted text in the message itself should match
     the marker color, unless it's a Lint message (Yellow), in which case
     making it Red is fine because Red is not a color available on the color
     wheel for subsequent reason messages (in addition, Lint messages don't
     typically have subsequent reason messages anyway) *)
  let color =
    match color with
    | Tty.Yellow -> Tty.Red
    | _ -> color
  in
  let pretty_msg = Markdown_lite.render ~add_bold:true ~color msg in
  Printf.sprintf "%s %s %s" pretty_error_code pretty_msg suffix

let format_reason_highlighted marked_msg : string =
  let suffix = single_marker_highlighted marked_msg.marker in
  let pretty_arrow = background_highlighted "->" in
  let pretty_msg =
    Markdown_lite.render
      ~color:(snd marked_msg.marker)
      (Errors.get_message_str marked_msg.message)
  in
  Printf.sprintf "%s %s %s" pretty_arrow pretty_msg suffix

let make_marker n error_code : marker =
  (* Explicitly list out the various codes, rather than a catch-all
     for non 5 or 6 codes, to make the correspondence clear and to
     make updating this in the future more straightforward *)
  let claim_color =
    match error_code / 1000 with
    | 1 -> Tty.Red (* Parsing *)
    | 2 -> Tty.Red (* Naming *)
    | 3 -> Tty.Red (* NastCheck *)
    | 4 -> Tty.Red (* Typing *)
    | 5 -> Tty.Yellow (* Lint *)
    | 6 -> Tty.Yellow (* Zoncolan (AI) *)
    | 8 -> Tty.Red (* Init *)
    | _ -> Tty.Red
    (* Other *)
  in
  let color_wheel = [Tty.Cyan; Tty.Green; Tty.Magenta; Tty.Blue] in
  let color =
    if n <= 1 then
      claim_color
    else
      let ix = (n - 2) mod List.length color_wheel in
      List.nth_exn color_wheel ix
  in
  (n, color)

(* Convert a list of messages tuples into marked_message structs,
   by assigning each their original index in the list and a marker.
   Any messages that have the same position (meaning exact location
   and size) share the same marker. *)
let mark_messages
    (error_code : error_code) (msgl : Pos.absolute Errors.message list) :
    marked_message list =
  List.folding_mapi
    msgl
    ~init:(1, [])
    ~f:(fun original_index (next_marker_n, existing_markers) message ->
      let (next_marker_n, existing_markers, marker) =
        let curr_msg_pos = Errors.get_message_pos message in
        match
          List.find existing_markers ~f:(fun (_, pos) ->
              Pos.equal_absolute curr_msg_pos pos)
        with
        | None ->
          let marker = make_marker next_marker_n error_code in
          (next_marker_n + 1, (marker, curr_msg_pos) :: existing_markers, marker)
        | Some (marker, _) -> (next_marker_n, existing_markers, marker)
      in
      ((next_marker_n, existing_markers), { original_index; marker; message }))

let to_string (error : Errors.finalized_error) : string =
  let error_code = Errors.get_code error in
  (* Assign messages markers according to order of original error list
    and then sort these marked messages such that messages in the same
    file are together. Does not reorder the files or messages within a file. *)
  let marked_messages =
    Errors.get_messages error
    |> mark_messages error_code
    |> Errors.combining_sort ~f:(fun mm ->
           Errors.get_message_pos mm.message |> Pos.filename)
  in

  (* Typing[4110] Invalid return type [1]   <claim>
      -> Expected int [2]                   <reasons>
      -> But got string [1]                 <reasons>
  *)
  let (claim, reasons) =
    (* Present the reasons in _exactly_ the order given in the error,
       not in the marked_messages order, which is by file *)
    let mms =
      List.stable_sort marked_messages ~compare:(fun mm1 mm2 ->
          Int.compare mm1.original_index mm2.original_index)
    in
    match mms with
    | mm :: msgs ->
      (* This very vitally assumes that the first message in the error is the main one *)
      ( format_claim_highlighted
          error_code
          mm.marker
          (Errors.get_message_str mm.message),
        List.map msgs ~f:format_reason_highlighted )
    | [] ->
      failwith "Impossible: an error always has non-empty list of messages"
  in

  (* overlap_pos.php:4:10
          1 | <?hh
          2 |
      [2] 3 | function returns_int(): int {
      [1] 4 |   return 'foo';
          5 | }
  *)
  let all_contexts = format_all_contexts_highlighted marked_messages in

  let buf = Buffer.create 50 in
  Buffer.add_string buf (claim ^ "\n");
  if not (List.is_empty reasons) then
    Buffer.add_string buf (String.concat ~sep:"\n" reasons ^ "\n");
  Buffer.add_string buf "\n";
  Buffer.add_string buf all_contexts;
  Buffer.contents buf
