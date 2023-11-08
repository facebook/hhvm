(** Positions embedded with full position info inside its
 * data structure. See "type 'a pos"
 *
 * Note: This module can only be used with the Lexbuf Pos_source
 * module, as it will not compile with other Pos_source modules. So,
 * when choosing a different Pos module, you must also choose its
 * compatible Pos_source module. *)

open Hh_prelude
open Lexing

type b = Pos_source.t

(* Note: While Pos.string and Pos.info_pos return positions as closed intervals,
 * pos_start and pos_end actually form a half-open interval (i.e. pos_end points
 * to the character *after* the last character of the relevant lexeme.) *)
type 'a pos =
  | Pos_small of {
      pos_file: 'a;
      pos_start: File_pos_small.t;
      pos_end: File_pos_small.t;
    }
  | Pos_large of {
      pos_file: 'a;
      pos_start: File_pos_large.t;
      pos_end: File_pos_large.t;
    }
  | Pos_tiny of {
      pos_file: 'a;
      pos_span: Pos_span_tiny.t;
    }
  | Pos_from_reason of 'a pos
[@@deriving eq, hash, show]

type t = Relative_path.t pos [@@deriving eq, hash, show]

type absolute = string pos [@@deriving eq, show]

let none =
  Pos_tiny { pos_file = Relative_path.default; pos_span = Pos_span_tiny.dummy }

(* Avoid warning about unused function *)
let _ = pp

let rec pp fmt pos =
  if equal pos none then
    Format.pp_print_string fmt "[Pos.none]"
  else (
    Format.pp_print_string fmt "[";
    begin
      match pos with
      | Pos_small { pos_start; pos_end; _ } ->
        File_pos_small.pp fmt pos_start;
        Format.pp_print_string fmt "-";
        if File_pos_small.line pos_start = File_pos_small.line pos_end then
          Format.pp_print_int fmt @@ (File_pos_small.column pos_end + 1)
        else
          File_pos_small.pp fmt pos_end
      | Pos_large { pos_start; pos_end; _ } ->
        File_pos_large.pp fmt pos_start;
        Format.pp_print_string fmt "-";
        if File_pos_large.line pos_start = File_pos_large.line pos_end then
          Format.pp_print_int fmt @@ (File_pos_large.column pos_end + 1)
        else
          File_pos_large.pp fmt pos_end
      | Pos_tiny { pos_span; pos_file = _ } -> Pos_span_tiny.pp fmt pos_span
      | Pos_from_reason p -> pp fmt p
    end;
    Format.pp_print_string fmt "]"
  )

let rec filename p =
  match p with
  | Pos_small { pos_file; _ }
  | Pos_large { pos_file; _ }
  | Pos_tiny { pos_file; _ } ->
    pos_file
  | Pos_from_reason p -> filename p

(** This returns a closed interval that's incorrect for multi-line spans. *)
let rec info_pos p =
  match p with
  | Pos_small { pos_start; pos_end; _ } ->
    let (line, start_minus1, bol) = File_pos_small.line_column_beg pos_start in
    let start = start_minus1 + 1 in
    let end_offset = File_pos_small.offset pos_end in
    let end_ = end_offset - bol in
    (* To represent the empty interval, pos_start and pos_end are equal because
       end_offset is exclusive. Here, it's best for error messages to the user if
       we print characters N to N (highlighting a single character) rather than characters
       N to (N-1), which is very unintuitive.
    *)
    let end_ =
      if start = end_ + 1 then
        start
      else
        end_
    in
    (line, start, end_)
  | Pos_large { pos_start; pos_end; _ } ->
    let (line, start_minus1, bol) = File_pos_large.line_column_beg pos_start in
    let start = start_minus1 + 1 in
    let end_offset = File_pos_large.offset pos_end in
    let end_ = end_offset - bol in
    (* To represent the empty interval, pos_start and pos_end are equal because
       end_offset is exclusive. Here, it's best for error messages to the user if
       we print characters N to N (highlighting a single character) rather than characters
       N to (N-1), which is very unintuitive.
    *)
    let end_ =
      if start = end_ + 1 then
        start
      else
        end_
    in
    (line, start, end_)
  | Pos_tiny { pos_span = span; pos_file = _ } ->
    let line = Pos_span_tiny.start_line_number span in
    let start_column = Pos_span_tiny.start_column span + 1 in
    let end_column = Pos_span_tiny.end_column span in
    let end_column =
      if start_column = end_column + 1 then
        start_column
      else
        end_column
    in
    (line, start_column, end_column)
  | Pos_from_reason p -> info_pos p

(* This returns a closed interval. *)
let rec info_pos_extended p =
  let (line_begin, start, end_) = info_pos p in
  match p with
  | Pos_small { pos_end; _ } ->
    let (line_end, _, _) = File_pos_small.line_column_beg pos_end in
    (line_begin, line_end, start, end_)
  | Pos_large { pos_end; _ } ->
    let (line_end, _, _) = File_pos_large.line_column_beg pos_end in
    (line_begin, line_end, start, end_)
  | Pos_tiny { pos_span; pos_file = _ } ->
    let line_end = Pos_span_tiny.end_line_number pos_span in
    (line_begin, line_end, start, end_)
  | Pos_from_reason p -> info_pos_extended p

let rec info_raw p =
  match p with
  | Pos_small { pos_start; pos_end; _ } ->
    (File_pos_small.offset pos_start, File_pos_small.offset pos_end)
  | Pos_large { pos_start; pos_end; _ } ->
    (File_pos_large.offset pos_start, File_pos_large.offset pos_end)
  | Pos_tiny { pos_span; pos_file = _ } ->
    (Pos_span_tiny.start_offset pos_span, Pos_span_tiny.end_offset pos_span)
  | Pos_from_reason p -> info_raw p

let rec length p =
  match p with
  | Pos_small { pos_start; pos_end; _ } ->
    File_pos_small.offset pos_end - File_pos_small.offset pos_start
  | Pos_large { pos_start; pos_end; _ } ->
    File_pos_large.offset pos_end - File_pos_large.offset pos_start
  | Pos_tiny { pos_span; pos_file = _ } ->
    Pos_span_tiny.end_offset pos_span - Pos_span_tiny.start_offset pos_span
  | Pos_from_reason p -> length p

let rec start_offset p =
  match p with
  | Pos_small { pos_start; _ } -> File_pos_small.offset pos_start
  | Pos_large { pos_start; _ } -> File_pos_large.offset pos_start
  | Pos_tiny { pos_span; _ } -> Pos_span_tiny.start_offset pos_span
  | Pos_from_reason p -> start_offset p

let rec end_offset p =
  match p with
  | Pos_small { pos_end; _ } -> File_pos_small.offset pos_end
  | Pos_large { pos_end; _ } -> File_pos_large.offset pos_end
  | Pos_tiny { pos_span; _ } -> Pos_span_tiny.end_offset pos_span
  | Pos_from_reason p -> end_offset p

let rec line p =
  match p with
  | Pos_small { pos_start; _ } -> File_pos_small.line pos_start
  | Pos_large { pos_start; _ } -> File_pos_large.line pos_start
  | Pos_tiny { pos_span; _ } -> Pos_span_tiny.start_line_number pos_span
  | Pos_from_reason p -> line p

let rec end_line p =
  match p with
  | Pos_small { pos_end; _ } -> File_pos_small.line pos_end
  | Pos_large { pos_end; _ } -> File_pos_large.line pos_end
  | Pos_tiny { pos_span; _ } -> Pos_span_tiny.end_line_number pos_span
  | Pos_from_reason p -> end_line p

(* This returns a closed interval. *)
let string t =
  let (line, start, end_) = info_pos t in
  let path = filename t in

  let hhi_path =
    try Relative_path.path_of_prefix Relative_path.Hhi with
    | Invalid_argument _ ->
      (* .hhi path hasn't been set (e.g. when running tests). *)
      ""
  in
  (* Strip temporary .hhi directories from the path shown. *)
  let path =
    match String.chop_prefix path ~prefix:hhi_path with
    | Some file_path -> file_path
    | None -> path
  in
  Printf.sprintf "File %S, line %d, characters %d-%d:" path line start end_

(* Some positions, like those in buffers sent by IDE/created by unit tests might
 * not have a file specified.
 * This returns a closed interval. *)
let string_no_file t =
  let (line, start, end_) = info_pos t in
  Printf.sprintf "line %d, characters %d-%d" line start end_

(* This returns a closed interval. *)
let json pos =
  let (line, start, end_) = info_pos pos in
  let fn = filename pos in
  Hh_json.JSON_Object
    [
      ("filename", Hh_json.JSON_String fn);
      ("line", Hh_json.int_ line);
      ("char_start", Hh_json.int_ start);
      ("char_end", Hh_json.int_ end_);
    ]

let json_no_filename pos =
  let (line, start, end_) = info_pos pos in
  Hh_json.JSON_Object
    [
      ("line", Hh_json.int_ line);
      ("char_start", Hh_json.int_ start);
      ("char_end", Hh_json.int_ end_);
    ]

(*
 * !!! Be careful !!!
 * This method returns zero-based column numbers, but one-based line numbers.
 * Consider using info_pos instead.
 *)
let rec line_column p =
  match p with
  | Pos_small { pos_start; _ } -> File_pos_small.line_column pos_start
  | Pos_large { pos_start; _ } -> File_pos_large.line_column pos_start
  | Pos_tiny { pos_span; _ } ->
    ( Pos_span_tiny.start_line_number pos_span,
      Pos_span_tiny.start_column pos_span )
  | Pos_from_reason p -> line_column p

let rec end_line_column p =
  match p with
  | Pos_small { pos_end; _ } -> File_pos_small.line_column pos_end
  | Pos_large { pos_end; _ } -> File_pos_large.line_column pos_end
  | Pos_tiny { pos_span; _ } ->
    (Pos_span_tiny.end_line_number pos_span, Pos_span_tiny.end_column pos_span)
  | Pos_from_reason p -> end_line_column p

let inside p line char_pos =
  let (first_line, first_col) = line_column p in
  let (last_line, last_col) = end_line_column p in
  if first_line = last_line then
    first_line = line && first_col + 1 <= char_pos && char_pos <= last_col
  else if line = first_line then
    char_pos > first_col
  else if line = last_line then
    char_pos <= last_col
  else
    line > first_line && line < last_line

let exactly_matches_range p ~start_line ~start_col ~end_line ~end_col =
  let (p_start_line, p_start_col) = line_column p in
  let (p_end_line, p_end_col) = end_line_column p in
  p_start_line = start_line
  && p_start_col = start_col - 1
  && p_end_line = end_line
  && p_end_col = end_col - 1

let contains pos_container pos =
  let (cstart, cend) = info_raw pos_container in
  let (pstart, pend) = info_raw pos in
  Relative_path.equal (filename pos_container) (filename pos)
  && pstart >= cstart
  && pend <= cend

let overlaps pos1 pos2 =
  let (start1, end1) = info_raw pos1 in
  let (start2, end2) = info_raw pos2 in
  Relative_path.equal (filename pos1) (filename pos2)
  && end1 > start2
  && start1 < end2

let is_hhi pos = Relative_path.is_hhi (Relative_path.prefix (filename pos))

let rec compress p =
  match p with
  | Pos_tiny _ -> p
  | Pos_small { pos_file; pos_start; pos_end } ->
    (match
       Pos_span_tiny.make
         ~pos_start:(File_pos_small.as_large_pos pos_start)
         ~pos_end:(File_pos_small.as_large_pos pos_end)
     with
    | None -> p
    | Some pos_span -> Pos_tiny { pos_file; pos_span })
  | Pos_large { pos_file; pos_start; pos_end } ->
    (match Pos_span_tiny.make ~pos_start ~pos_end with
    | Some pos_span -> Pos_tiny { pos_file; pos_span }
    | None ->
      (match File_pos_small.of_large_pos pos_start with
      | None -> p
      | Some pos_start ->
        (match File_pos_small.of_large_pos pos_end with
        | None -> p
        | Some pos_end -> Pos_small { pos_file; pos_start; pos_end })))
  | Pos_from_reason p -> Pos_from_reason (compress p)

let make_from_lexing_pos pos_file pos_start pos_end =
  compress
  @@ Pos_large
       {
         pos_file;
         pos_start = File_pos_large.of_lexing_pos pos_start;
         pos_end = File_pos_large.of_lexing_pos pos_end;
       }

let make file (lb : b) =
  let pos_start = lexeme_start_p lb in
  let pos_end = lexeme_end_p lb in
  make_from_lexing_pos file pos_start pos_end

let make_from file =
  Pos_tiny { pos_file = file; pos_span = Pos_span_tiny.dummy }

let rec as_large_pos p =
  match p with
  | Pos_tiny { pos_file; pos_span } ->
    let (pos_start, pos_end) = Pos_span_tiny.as_large_span pos_span in
    Pos_large { pos_file; pos_start; pos_end }
  | Pos_small { pos_file; pos_start; pos_end } ->
    Pos_large
      {
        pos_file;
        pos_start = File_pos_small.as_large_pos pos_start;
        pos_end = File_pos_small.as_large_pos pos_end;
      }
  | Pos_large _ -> p
  | Pos_from_reason p -> Pos_from_reason (as_large_pos p)

let rec btw_nocheck x1 x2 =
  match (x1, x2) with
  | (Pos_small { pos_file; pos_start; _ }, Pos_small { pos_end; _ }) ->
    Pos_small { pos_file; pos_start; pos_end }
  | (Pos_large { pos_file; pos_start; _ }, Pos_large { pos_end; _ }) ->
    Pos_large { pos_file; pos_start; pos_end }
  | (Pos_small { pos_file; pos_start; _ }, Pos_large { pos_end; _ }) ->
    Pos_large
      { pos_file; pos_start = File_pos_small.as_large_pos pos_start; pos_end }
  | (Pos_large { pos_file; pos_start; _ }, Pos_small { pos_end; _ }) ->
    Pos_large
      { pos_file; pos_start; pos_end = File_pos_small.as_large_pos pos_end }
  | (Pos_from_reason p1, p2) -> btw_nocheck p1 p2
  | (p1, Pos_from_reason p2) -> btw_nocheck p1 p2
  | (Pos_tiny _, _)
  | (_, Pos_tiny _) ->
    let p1 = as_large_pos x1 in
    let p2 = as_large_pos x2 in
    btw_nocheck p1 p2 |> compress

let rec set_file pos_file pos =
  match pos with
  | Pos_small { pos_start; pos_end; _ } ->
    Pos_small { pos_file; pos_start; pos_end }
  | Pos_large { pos_start; pos_end; _ } ->
    Pos_large { pos_file; pos_start; pos_end }
  | Pos_tiny { pos_span; pos_file = _ } -> Pos_tiny { pos_file; pos_span }
  | Pos_from_reason p -> Pos_from_reason (set_file pos_file p)

let set_col_start pos_cnum pos =
  let pos = as_large_pos pos in
  match pos with
  | Pos_large { pos_file; pos_start; pos_end } ->
    let pos_start = File_pos_large.set_column pos_cnum pos_start in
    Pos_large { pos_file; pos_start; pos_end }
  | _ -> pos

let set_col_end pos_cnum pos =
  let pos = as_large_pos pos in
  match pos with
  | Pos_large { pos_file; pos_start; pos_end } ->
    let pos_end = File_pos_large.set_column pos_cnum pos_end in
    Pos_large { pos_file; pos_start; pos_end }
  | _ -> pos

let rec shrink_to_start pos =
  let pos = as_large_pos pos in
  match pos with
  | Pos_large { pos_file; pos_start; _ } ->
    Pos_large { pos_file; pos_start; pos_end = pos_start }
  | Pos_from_reason p -> Pos_from_reason (shrink_to_start p)
  | _ -> pos

let rec shrink_to_end pos =
  let pos = as_large_pos pos in
  match pos with
  | Pos_large { pos_file; pos_end; _ } ->
    Pos_large { pos_file; pos_start = pos_end; pos_end }
  | Pos_from_reason p -> Pos_from_reason (shrink_to_end p)
  | _ -> pos

let set_from_reason pos =
  match pos with
  | Pos_from_reason _ -> pos
  | _ -> Pos_from_reason pos

let get_from_reason pos =
  match pos with
  | Pos_from_reason _ -> true
  | _ -> false

let to_absolute p = set_file (Relative_path.to_absolute (filename p)) p

let to_relative p = set_file (Relative_path.create_detect_prefix (filename p)) p

let btw x1 x2 =
  if not (Relative_path.equal (filename x1) (filename x2)) then
    failwith "Position in separate files";
  if end_offset x1 > end_offset x2 then
    failwith
      (Printf.sprintf
         "btw: invalid positions %s and %s"
         (string (to_absolute x1))
         (string (to_absolute x2)));
  btw_nocheck x1 x2

let rec merge x1 x2 =
  match (x1, x2) with
  | ( Pos_small { pos_file = file1; pos_start = start1; pos_end = end1 },
      Pos_small { pos_file = _; pos_start = start2; pos_end = end2; _ } ) ->
    let pos_start =
      if File_pos_small.is_dummy start1 then
        start2
      else if File_pos_small.is_dummy start2 then
        start1
      else if start_offset x1 < start_offset x2 then
        start1
      else
        start2
    in
    let pos_end =
      if File_pos_small.is_dummy end1 then
        end2
      else if File_pos_small.is_dummy end2 then
        end1
      else if end_offset x1 < end_offset x2 then
        end2
      else
        end1
    in
    Pos_small { pos_file = file1; pos_start; pos_end }
  | ( Pos_large { pos_file = file1; pos_start = start1; pos_end = end1 },
      Pos_large { pos_file = _; pos_start = start2; pos_end = end2; _ } ) ->
    let pos_start =
      if File_pos_large.is_dummy start1 then
        start2
      else if File_pos_large.is_dummy start2 then
        start1
      else if start_offset x1 < start_offset x2 then
        start1
      else
        start2
    in
    let pos_end =
      if File_pos_large.is_dummy end1 then
        end2
      else if File_pos_large.is_dummy end2 then
        end1
      else if end_offset x1 < end_offset x2 then
        end2
      else
        end1
    in
    Pos_large { pos_file = file1; pos_start; pos_end }
  | (Pos_from_reason p1, Pos_from_reason p2) -> Pos_from_reason (merge p1 p2)
  | (Pos_from_reason p1, p2) -> Pos_from_reason (merge p1 p2)
  | (p1, Pos_from_reason p2) -> Pos_from_reason (merge p1 p2)
  | (_, _) -> merge (as_large_pos x1) (as_large_pos x2) |> compress

let rec last_char p =
  if equal p none then
    none
  else
    (match p with
    | Pos_small { pos_start = _; pos_end; pos_file } ->
      Pos_small { pos_start = pos_end; pos_end; pos_file }
    | Pos_large { pos_start = _; pos_end; pos_file } ->
      Pos_large { pos_start = pos_end; pos_end; pos_file }
    | Pos_tiny { pos_span; pos_file } ->
      let (_pos_start, pos_end) = Pos_span_tiny.as_large_span pos_span in
      Pos_large { pos_file; pos_start = pos_end; pos_end }
    | Pos_from_reason p -> last_char p)
    |> compress

let rec first_char_of_line p =
  if equal p none then
    none
  else
    (match p with
    | Pos_small { pos_start; pos_end = _; pos_file } ->
      let start = File_pos_small.set_column_unchecked 0 pos_start in
      Pos_small { pos_start = start; pos_end = start; pos_file }
    | Pos_large { pos_start; pos_end = _; pos_file } ->
      let start = File_pos_large.set_column 0 pos_start in
      Pos_large { pos_start = start; pos_end = start; pos_file }
    | Pos_tiny { pos_span; pos_file } ->
      let (pos_start, _pos_end) = Pos_span_tiny.as_large_span pos_span in
      let start = File_pos_large.set_column 0 pos_start in
      Pos_large { pos_file; pos_start = start; pos_end = start }
    | Pos_from_reason p -> first_char_of_line p)
    |> compress

let to_relative_string p = set_file (Relative_path.suffix (filename p)) p

let get_text_from_pos ~content pos =
  let pos_length = length pos in
  let offset = start_offset pos in
  String.sub content ~pos:offset ~len:pos_length

(** Compare by filename, then tie-break by start position, and finally by the
    end position *)
let compare_pos :
    type file. (file -> file -> int) -> file pos -> file pos -> int =
 fun compare_files x y ->
  let r = compare_files (filename x) (filename y) in
  if r <> 0 then
    r
  else
    let (xstart, xend) = info_raw x in
    let (ystart, yend) = info_raw y in
    let r = xstart - ystart in
    if r <> 0 then
      r
    else
      xend - yend

let compare = compare_pos Relative_path.compare

let compare_absolute = compare_pos String.compare

(* This returns a half-open interval. *)
let destruct_range (p : 'a pos) : int * int * int * int =
  let (line_start, col_start_minus1) = line_column p in
  let (line_end, col_end_minus1) = end_line_column p in
  (line_start, col_start_minus1 + 1, line_end, col_end_minus1 + 1)

let rec advance_one (p : 'a pos) : 'a pos =
  match p with
  | Pos_small { pos_file; pos_start; pos_end } ->
    let end_column = File_pos_small.column pos_end in
    (match File_pos_small.set_column (end_column + 1) pos_end with
    | Some pos_end -> Pos_small { pos_file; pos_start; pos_end }
    | None ->
      let pos_start = File_pos_small.as_large_pos pos_start in
      let pos_end = File_pos_small.as_large_pos pos_end in
      let pos_end = File_pos_large.set_column (end_column + 1) pos_end in
      Pos_large { pos_file; pos_start; pos_end })
  | Pos_large { pos_file; pos_start; pos_end } ->
    Pos_large
      {
        pos_file;
        pos_start;
        pos_end =
          (let column = File_pos_large.column pos_end in
           File_pos_large.set_column (column + 1) pos_end);
      }
  | Pos_tiny _ -> p |> as_large_pos |> advance_one |> compress
  | Pos_from_reason p -> Pos_from_reason (advance_one p)

(* This function is used when we have captured a position that includes
 * outside boundary characters like apostrophes.  If we need to remove these
 * apostrophes, this function shrinks by one character in each direction. *)
let rec shrink_by_one_char_both_sides (p : 'a pos) : 'a pos =
  match p with
  | Pos_small { pos_file; pos_start; pos_end } ->
    let pos_end =
      let column = File_pos_small.column pos_end in
      File_pos_small.set_column_unchecked (column - 1) pos_end
    in
    let start_column = File_pos_small.column pos_start in
    (match File_pos_small.set_column (start_column + 1) pos_start with
    | None ->
      let pos_start = File_pos_small.as_large_pos pos_start in
      let pos_start = File_pos_large.set_column (start_column + 1) pos_start in
      let pos_end = File_pos_small.as_large_pos pos_end in
      Pos_large { pos_file; pos_start; pos_end }
    | Some pos_start -> Pos_small { pos_file; pos_start; pos_end })
  | Pos_large { pos_file; pos_start; pos_end } ->
    let new_pos_start =
      let column = File_pos_large.column pos_start in
      File_pos_large.set_column (column + 1) pos_start
    in
    let new_pos_end =
      let column = File_pos_large.column pos_end in
      File_pos_large.set_column (column - 1) pos_end
    in
    Pos_large { pos_file; pos_start = new_pos_start; pos_end = new_pos_end }
  | Pos_tiny _ -> p |> as_large_pos |> shrink_by_one_char_both_sides |> compress
  | Pos_from_reason p -> Pos_from_reason (shrink_by_one_char_both_sides p)

(* This returns a half-open interval. *)
let multiline_string t =
  let (line_start, char_start, line_end, char_end) = destruct_range t in
  Printf.sprintf
    "File %S, line %d, character %d - line %d, character %d:"
    (String.strip (filename t))
    line_start
    char_start
    line_end
    (char_end - 1)

(* This returns a half-open interval. *)
let multiline_string_no_file t =
  let (line_start, char_start, line_end, char_end) = destruct_range t in
  Printf.sprintf
    "line %d, character %d - line %d, character %d"
    line_start
    char_start
    line_end
    (char_end - 1)

(* This returns a half-open interval. *)
let multiline_json t =
  let (line_start, char_start, line_end, char_end) = destruct_range t in
  let fn = filename t in
  Hh_json.JSON_Object
    [
      ("filename", Hh_json.JSON_String fn);
      ("line_start", Hh_json.int_ line_start);
      ("char_start", Hh_json.int_ char_start);
      ("line_end", Hh_json.int_ line_end);
      ("char_end", Hh_json.int_ (char_end - 1));
    ]

let multiline_json_no_filename t =
  let (line_start, char_start, line_end, char_end) = destruct_range t in
  Hh_json.JSON_Object
    [
      ("line_start", Hh_json.int_ line_start);
      ("char_start", Hh_json.int_ char_start);
      ("line_end", Hh_json.int_ line_end);
      ("char_end", Hh_json.int_ (char_end - 1));
    ]

let rec line_beg_offset p =
  match p with
  | Pos_small { pos_start; _ } -> File_pos_small.line_beg_offset pos_start
  | Pos_large { pos_start; _ } -> File_pos_large.line_beg_offset pos_start
  | Pos_tiny { pos_span; _ } ->
    let (pos_start, _pos_end) = Pos_span_tiny.as_large_span pos_span in
    File_pos_large.line_beg_offset pos_start
  | Pos_from_reason p -> line_beg_offset p

let rec end_line_beg_offset p =
  match p with
  | Pos_small { pos_end; _ } -> File_pos_small.line_beg_offset pos_end
  | Pos_large { pos_end; _ } -> File_pos_large.line_beg_offset pos_end
  | Pos_tiny { pos_span; _ } ->
    let (_pos_start, pos_end) = Pos_span_tiny.as_large_span pos_span in
    File_pos_large.line_beg_offset pos_end
  | Pos_from_reason p -> end_line_beg_offset p

let make_from_lnum_bol_offset ~pos_file ~pos_start ~pos_end =
  let (lnum_start, bol_start, offset_start) = pos_start in
  let (lnum_end, bol_end, offset_end) = pos_end in
  compress
  @@ Pos_large
       {
         pos_file;
         pos_start =
           File_pos_large.of_lnum_bol_offset
             ~pos_lnum:lnum_start
             ~pos_bol:bol_start
             ~pos_offset:offset_start;
         pos_end =
           File_pos_large.of_lnum_bol_offset
             ~pos_lnum:lnum_end
             ~pos_bol:bol_end
             ~pos_offset:offset_end;
       }

let pessimize_enabled pos pessimize_coefficient =
  let path = filename pos in
  let open Float in
  match Relative_path.prefix path with
  | Relative_path.Root when pessimize_coefficient > 0.0 ->
    let range = 2000000 in
    let filename = Relative_path.suffix path in
    let hash = Hashtbl.hash filename in
    let r = Int.( % ) hash range in
    Float.of_int r /. Float.of_int range <= pessimize_coefficient
  | _ -> Float.equal pessimize_coefficient 1.0

(* hack for test cases *)

let print_verbose_absolute p =
  let (a, b, c) = line_beg_offset p in
  let (d, e, f) = end_line_beg_offset p in
  Printf.sprintf "Pos('%s', <%d,%d,%d>, <%d,%d,%d>)" (filename p) a b c d e f

let print_verbose_relative p = print_verbose_absolute (to_absolute p)

module Pos = struct
  type path = t

  (* The definition below needs to refer to the t in the outer scope, but WrappedMap
   * expects a module with a type of name t, so we define t in a second step *)
  type t = path

  let compare = compare
end

module Map = WrappedMap.Make (Pos)

module AbsolutePosMap = WrappedMap.Make (struct
  type t = absolute

  let compare = compare_absolute
end)

module Set = Caml.Set.Make (Pos)
