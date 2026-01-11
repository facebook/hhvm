(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Result.Monad_infix

(* TODO t14922604: Further improve error handling *)
let call_external_formatter
    (cmd : Exec_command.t) (content : string) (args : string list) :
    (string list, string) result =
  let args = Array.of_list (Exec_command.to_string cmd :: args) in
  let reader _timeout (ic, pid) oc =
    Out_channel.output_string oc content;
    Out_channel.close oc;
    let lines = ref [] in
    begin
      try
        while true do
          lines := Stdlib.input_line ic :: !lines
        done
      with
      | End_of_file -> ()
    end;
    match Timeout.close_process_in (ic, pid) with
    | Unix.WEXITED 0 -> Ok (List.rev !lines)
    | Unix.WEXITED v -> Error (Hackfmt_error.get_error_string_from_exit_value v)
    | _ -> Error "Call to hackfmt was killed"
  in
  Timeout.read_process
    ~timeout:2
    ~on_timeout:(fun _ ->
      Hh_logger.log "Formatter timeout";
      Error "Call to hackfmt timed out")
    ~reader
    cmd
    args

let formatting_options_to_args
    (options : Lsp.DocumentFormatting.formattingOptions) =
  Lsp.DocumentFormatting.(
    let args = ["--indent-width"; string_of_int options.tabSize] in
    if not options.insertSpaces then
      args @ ["--tabs"]
    else
      args)

let range_offsets_to_args from to_ =
  ["--range"; string_of_int from; string_of_int to_]

(** returns lines of stdout we get from shelling out to hackfmt *)
let go_hackfmt ?filename_for_logging ~content args :
    (string list, string) Result.t =
  let args =
    match filename_for_logging with
    | Some filename -> args @ ["--filename-for-logging"; filename]
    | None -> args
  in
  Hh_logger.log "%s" (String.concat ~sep:" " args);
  let paths = [Path.to_string (Path.make BuildOptions.default_hackfmt_path)] in
  let paths =
    match Sys.getenv_opt "HACKFMT_TEST_PATH" with
    | Some p -> [p] @ paths
    | None -> paths
  in
  let path = List.find ~f:Sys.file_exists paths in
  match path with
  | Some path ->
    call_external_formatter (Exec_command.Hackfmt path) content args
  | _ ->
    Hh_logger.log "Formatter not found";
    Error
      ("Could not locate formatter - looked in: " ^ String.concat ~sep:" " paths)

(* Our formatting engine can only handle ranges that span entire rows.  *)
(* This is signified by a range that starts at column 1 on one row,     *)
(* and ends at column 1 on another (because it's half-open).            *)
(* Nuclide always provides correct ranges, but other editors might not. *)
let expand_range_to_whole_rows content (range : File_content.range) :
    File_content.range * int * int =
  File_content.(
    (* It's easy to expand the start of the range if necessary, but to expand *)
    (* the end of the range requires more work... *)
    let range =
      { range with st = File_content.Position.beginning_of_line range.st }
    in
    let (from0, to0) = get_offsets content (range.st, range.ed) in
    if
      File_content.Position.is_beginning_of_line range.ed
      || to0 = String.length content
    then
      (range, from0, to0)
    (* common case is performant. *)
    else
      (* First guess: we'll extend range.ed to the end of the line. *)
      let ed = File_content.Position.beginning_of_next_line range.ed in
      (* But if this is longer than the length of the file, pull back. *)
      let ed_file = offset_to_position content (String.length content) in
      let ed =
        if Position.compare ed ed_file <= 0 then
          ed
        else
          ed_file
      in
      let range = { range with ed } in
      let (from0, to0) = get_offsets content (range.st, range.ed) in
      (range, from0, to0))

(* Two integers separated by a space. *)
let range_regexp = Str.regexp "^\\([0-9]+\\) \\([0-9]+\\)$"

let zip_with_index (xs : 'a list) (ys : 'b list) : (int * 'a * 'b option) list =
  let rec aux acc i xs ys =
    match (xs, ys) with
    | (x :: xs, y :: ys) -> aux ((i, x, Some y) :: acc) (i + 1) xs ys
    | (x :: xs, []) -> aux ((i, x, None) :: acc) (i + 1) xs ys
    | ([], _) -> acc
  in
  List.rev (aux [] 0 xs ys)

let opt_string_eq (x : string) (y : string option) : bool =
  match y with
  | Some y -> String.equal x y
  | None -> false

(** 0-indexed (line count starts from 0) *)
let first_changed_line_idx (old_lines : string list) (new_lines : string list) :
    int option =
  let numbered_lines = zip_with_index old_lines new_lines in
  let first_novel_line =
    List.find numbered_lines ~f:(fun (_, x, y) -> not (opt_string_eq x y))
  in
  match first_novel_line with
  | Some (i, _, _) -> Some i
  | None -> None

let last_changed_line_idx (old_lines : string list) (new_lines : string list) :
    int option =
  match first_changed_line_idx (List.rev old_lines) (List.rev new_lines) with
  | Some rev_idx -> Some (List.length old_lines - rev_idx)
  | None -> None

(* Drop the last [n] items in [xs]. *)
let drop_tail (xs : 'a list) (n : int) : 'a list =
  List.rev (List.drop (List.rev xs) n)

(* An empty edit that has no effect. *)
let noop_edit =
  let range =
    {
      File_content.st = File_content.Position.beginning_of_file;
      ed = File_content.Position.beginning_of_file;
    }
  in
  {
    ServerFormatTypes.new_text = "";
    range = Ide_api_types.ide_range_from_fc range;
  }

(* Return an IDE text edit that doesn't include unchanged leading or trailing lines. *)
let minimal_edit (old_src : string) (new_src : string) :
    ServerFormatTypes.ide_response =
  let old_src_lines = String.split_lines old_src in
  let new_src_lines = String.split_lines new_src in

  (* 0-indexed (line count starts from 0) *)
  let range_start_line = first_changed_line_idx old_src_lines new_src_lines in
  let range_end_line = last_changed_line_idx old_src_lines new_src_lines in

  match (range_start_line, range_end_line) with
  | (Some range_start_line, Some range_end_line) ->
    let new_src_novel_lines =
      drop_tail
        (List.drop new_src_lines range_start_line)
        (List.length old_src_lines - range_end_line)
    in
    let (new_text, start_line_adjustment) =
      if List.is_empty new_src_novel_lines && range_end_line < List.length new_src_lines then
        (*
          Empty `new_src_novel_lines` does not mean that there was no change
          (otherwise we wouldn't have reached here),
          it means some lines have been deleted. `-1` here allows to delete those lines.
        *)
        ("", -1)
      else
        (String.concat ~sep:"\n" new_src_novel_lines ^ "\n", 0)
    in
    let range_start_line = range_start_line + start_line_adjustment in
    let range =
      {
        File_content.st =
          File_content.Position.from_zero_based range_start_line 0;
        ed = File_content.Position.from_zero_based range_end_line 0;
      }
    in
    {
      ServerFormatTypes.new_text;
      range = Ide_api_types.ide_range_from_fc range;
    }
  | _ -> noop_edit

let go_ide
    ~(filename_for_logging : string)
    ~(content : string)
    ~(action : ServerFormatTypes.ide_action)
    ~(options : Lsp.DocumentFormatting.formattingOptions) :
    ServerFormatTypes.ide_result =
  let open File_content in
  let open ServerFormatTypes in
  let convert_to_ide_result
      (old_format_result : ServerFormatTypes.result)
      ~(range : File_content.range) : ServerFormatTypes.ide_result =
    let range = Ide_api_types.ide_range_from_fc range in
    old_format_result |> Result.map ~f:(fun new_text -> { new_text; range })
  in
  let formatting_args = formatting_options_to_args options in
  match action with
  | Document ->
    (* hackfmt currently takes one-indexed integers for range formatting. *)
    go_hackfmt ~filename_for_logging ~content formatting_args
    |> Result.map ~f:(String.concat ~sep:"\n")
    |> Result.map ~f:(fun new_text -> minimal_edit content new_text)
  | Range range ->
    let fc_range = Ide_api_types.ide_range_to_fc range in
    let (range, from0, to0) = expand_range_to_whole_rows content fc_range in
    let args = formatting_args @ range_offsets_to_args (from0 + 1) (to0 + 1) in
    go_hackfmt ~filename_for_logging ~content args
    |> Result.map ~f:(fun lines -> String.concat lines ~sep:"\n" ^ "\n")
    |> convert_to_ide_result ~range
  | Position position ->
    (* `get_offset` returns a zero-based index, and `--at-char` takes a
       zero-based index. *)
    let offset = get_offset content position in
    let args = ["--at-char"; string_of_int offset] in
    let args = args @ formatting_args in
    go_hackfmt ~filename_for_logging ~content args >>= fun lines ->
    (* `hackfmt --at-char` returns the range that was formatted, as well as the
       contents of that range. For example, it might return

           10 12
           }

       signifying that we should replace the character under the cursor with the
       following content, starting at index 10. We need to extract the range
       from the first line and forward it to the client so that it knows where
       to apply the edit. *)
    begin
      match lines with
      | range_line :: lines -> Ok (range_line, lines)
      | _ -> Error "Got no lines in at-position formatting"
    end
    >>= fun (range_line, lines) ->
    (* Extract the offsets in the first line that form the range.
       NOTE: `Str.string_match` sets global state to be consumed immediately
       afterwards by `Str.matched_group`. *)
    let does_range_match = Str.string_match range_regexp range_line 0 in
    if not does_range_match then
      Error "Range not found on first line of --at-char output"
    else
      let from0 = int_of_string (Str.matched_group 1 range_line) in
      let to0 = int_of_string (Str.matched_group 2 range_line) in
      let range =
        {
          st = offset_to_position content from0;
          ed = offset_to_position content to0;
        }
        |> Ide_api_types.ide_range_from_fc
      in
      let new_text = String.concat ~sep:"\n" lines in
      Ok { new_text; range }
