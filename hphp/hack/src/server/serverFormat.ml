(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Result.Monad_infix

(* TODO t14922604: Further improve error handling *)
let call_external_formatter
    (cmd : Exec_command.t) (content : string) (args : string list) :
    (string list, string) result =
  let args = Array.of_list (Exec_command.to_string cmd :: args) in
  let reader timeout ic oc =
    Out_channel.output_string oc content;
    Out_channel.close oc;
    let lines = ref [] in
    begin
      try
        while true do
          lines := Timeout.input_line ~timeout ic :: !lines
        done
      with End_of_file -> ()
    end;
    match Timeout.close_process_in ic with
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

let go_hackfmt ?filename_for_logging ~content args =
  let args =
    match filename_for_logging with
    | Some filename -> args @ ["--filename-for-logging"; filename]
    | None -> args
  in
  Hh_logger.log "%s" (String.concat ~sep:" " args);
  let dirname = Filename.dirname Sys.argv.(0) in
  let paths =
    List.map
      ~f:(fun x -> Path.make x |> Path.to_string)
      [
        (* if running from build tree *)
        dirname ^ "/hackfmt";
        dirname ^ "/../hackfmt/hackfmt";
        (* look for system installation *)
        BuildOptions.default_hackfmt_path;
      ]
  in
  let path = List.find ~f:Sys.file_exists paths in
  match path with
  | Some path ->
    call_external_formatter (Exec_command.Hackfmt path) content args
  | _ ->
    Hh_logger.log "Formatter not found";
    Error
      ("Could not locate formatter - looked in: " ^ String.concat ~sep:" " paths)

(* This function takes 1-based offsets, and 'to_' is exclusive. *)
let go ?filename_for_logging ~content from to_ options =
  let format_args = formatting_options_to_args options in
  let range_args = range_offsets_to_args from to_ in
  let args = format_args @ range_args in
  go_hackfmt ?filename_for_logging ~content args >>| fun lines ->
  String.concat ~sep:"\n" lines ^ "\n"

(* Our formatting engine can only handle ranges that span entire rows.  *)
(* This is signified by a range that starts at column 1 on one row,     *)
(* and ends at column 1 on another (because it's half-open).            *)
(* Nuclide always provides correct ranges, but other editors might not. *)
let expand_range_to_whole_rows content (range : File_content.range) :
    File_content.range * int * int =
  File_content.(
    (* It's easy to expand the start of the range if necessary, but to expand *)
    (* the end of the range requres more work... *)
    let range = { range with st = { range.st with column = 1 } } in
    let (from0, to0) = get_offsets content (range.st, range.ed) in
    if range.ed.column = 1 || to0 = String.length content then
      (range, from0, to0)
    (* common case is performant. *)
    else
      (* First guess: we'll extend range.ed to the end of the line. *)
      let ed = { line = range.ed.line + 1; column = 1 } in
      (* But if this is longer than the length of the file, pull back. *)
      let ed_file = offset_to_position content (String.length content) in
      let ed =
        if
          ed.line < ed_file.line
          || (ed.line = ed_file.line && ed.column <= ed_file.column)
        then
          ed
        else
          ed_file
      in
      let range = { range with ed } in
      let (from0, to0) = get_offsets content (range.st, range.ed) in
      (range, from0, to0))

(* Two integers separated by a space. *)
let range_regexp = Str.regexp "^\\([0-9]+\\) \\([0-9]+\\)$"

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
  match action with
  | Document ->
    (* `from0` and `to0` are zero-indexed, hence the name. *)
    let from0 = 0 in
    let to0 = String.length content in
    let ed = offset_to_position content to0 in
    let range = { st = { line = 1; column = 1 }; ed } in
    (* hackfmt currently takes one-indexed integers for range formatting. *)
    go ~filename_for_logging ~content (from0 + 1) (to0 + 1) options
    |> convert_to_ide_result ~range
  | Range range ->
    let fc_range = Ide_api_types.ide_range_to_fc range in
    let (range, from0, to0) = expand_range_to_whole_rows content fc_range in
    go ~filename_for_logging ~content (from0 + 1) (to0 + 1) options
    |> convert_to_ide_result ~range
  | Position position ->
    (* `get_offset` returns a zero-based index, and `--at-char` takes a
 zero-based index. *)
    let fc_position = Ide_api_types.ide_pos_to_fc position in
    let offset = get_offset content fc_position in
    let args = ["--at-char"; string_of_int offset] in
    let args = args @ formatting_options_to_args options in
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
