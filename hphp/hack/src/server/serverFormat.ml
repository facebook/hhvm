(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
 open Result.Monad_infix

(* TODO t14922604: Further improve error handling *)
let call_external_formatter
  (cmd : string)
  (content : string)
  (args : string list)
  : (string list, string) Result.t =
  let args = Array.of_list (cmd :: args) in
  let reader timeout ic oc =
    output_string oc content;
    close_out oc;
    let lines = ref [] in
    begin
      try
        while true
        do lines := Timeout.input_line ~timeout ic :: !lines
        done
      with End_of_file -> ()
    end;
    match Timeout.close_process_in ic with
    | Unix.WEXITED 0 -> Result.Ok (List.rev !lines)
    | Unix.WEXITED v -> Result.Error (Hackfmt_error.get_error_string_from_exit_value v)
    | _ -> Result.Error "Call to hackfmt was killed"
  in
  Timeout.read_process
    ~timeout:2
    ~on_timeout:(fun _ ->
      Hh_logger.log "Formatter timeout";
      Result.Error "Call to hackfmt timed out"
    )
    ~reader
    cmd
    args

let range_offsets_to_args from to_ =
  ["--range"; string_of_int from; string_of_int to_]

let go_hackfmt genv ?filename ~content args =
  let args = match filename with
    | Some filename -> args @ ["--filename-for-logging"; filename]
    | None -> args
  in
  Hh_logger.log "%s" (String.concat " " args);
  let path = match ServerConfig.formatter_override genv.ServerEnv.config with
    | None -> Path.make "/usr/local/bin/hackfmt"
    | Some p -> p
  in
  let path = Path.to_string path in
  if Sys.file_exists path
  then call_external_formatter path content args
  else begin
    Hh_logger.log "Formatter not found";
    Result.Error ("Could not locate formatter on provided path: " ^ path)
  end

let hh_format_result_to_response x =
  let open Format_hack in
  match x with
  | Disabled_mode -> Result.Error ("Not a Hack file")
  | Parsing_error _ -> Result.Error ("File has parse errors")
  | Internal_error -> Result.Error ("Formatter internal error")
  | Success s -> Result.Ok s

let go_hh_format _ content from to_ =
    let modes = [Some FileInfo.Mstrict; Some FileInfo.Mpartial] in
    hh_format_result_to_response @@
      Format_hack.region modes Path.dummy_path from to_ content

(* This function takes 1-based offsets, and 'to_' is exclusive. *)
let go genv ?filename ~content from to_ =
  if genv.ServerEnv.local_config.ServerLocalConfig.use_hackfmt
  then
    let args = range_offsets_to_args from to_ in
    go_hackfmt genv ?filename ~content args >>| fun lines ->
    (String.concat "\n" lines) ^ "\n"
  else go_hh_format genv content from to_

(* Our formatting engine can only handle ranges that span entire rows.  *)
(* This is signified by a range that starts at column 1 on one row,     *)
(* and ends at column 1 on another (because it's half-open).            *)
(* Nuclide always provides correct ranges, but other editors might not. *)
let expand_range_to_whole_rows content range =
  let open Ide_api_types in
  let open File_content in
  (* It's easy to expand the start of the range if necessary, but to expand *)
  (* the end of the range requres more work... *)
  let range = {range with st = {range.st with column = 1}} in
  let from0, to0 = get_offsets content (range.st, range.ed) in
  if range.ed.column = 1 || to0 = String.length content then
    (range, from0, to0)  (* common case is performant. *)
  else
    (* First guess: we'll extend range.ed to the end of the line. *)
    let ed = {line = range.ed.line + 1; column = 1} in
    (* But if this is longer than the length of the file, pull back. *)
    let ed_file = offset_to_position content (String.length content) in
    let ed =
      if ed.line < ed_file.line ||
        (ed.line = ed_file.line && ed.column <= ed_file.column)
      then ed
      else ed_file in
    let range = {range with ed} in
    let from0, to0 = get_offsets content (range.st, range.ed) in
    (range, from0, to0)

(* Two integers separated by a space. *)
let range_regexp = Str.regexp "^\\([0-9]+\\) \\([0-9]+\\)$"

let go_ide
  (genv: ServerEnv.genv)
  (action: ServerFormatTypes.ide_action)
  : ServerFormatTypes.ide_result =
  let open File_content in
  let open Ide_api_types in
  let open ServerFormatTypes in
  let filename = match action with
    | Document filename -> filename
    | Range range -> range.range_filename
    | Position position -> position.filename
  in
  let content =
    ServerFileSync.get_file_content (ServerUtils.FileName filename)
  in

  let convert_to_ide_result
    (old_format_result: ServerFormatTypes.result)
    ~(range : Ide_api_types.range)
    : ServerFormatTypes.ide_result =
    old_format_result
      |> Result.map ~f:(fun new_text -> {new_text; range;})
  in

  match action with
  | Document _filename ->
    (* `from0` and `to0` are zero-indexed, hence the name. *)
    let from0 = 0 in
    let to0 = String.length content in
    let ed = offset_to_position content to0 in
    let range = {st = {line = 1; column = 1;}; ed;} in
    (* hackfmt currently takes one-indexed integers for range formatting. *)
    go genv ~filename ~content (from0 + 1) (to0 + 1)
      |> convert_to_ide_result ~range

  | Range range ->
    let (range, from0, to0) =
      expand_range_to_whole_rows content range.file_range in
    go genv ~filename ~content (from0 + 1) (to0 + 1)
      |> convert_to_ide_result ~range

  | Position {position; _} ->
    (* `get_offset` returns a zero-based index, and `--at-char` takes a
       zero-based index. *)
    let offset = get_offset content position in
    let args = ["--at-char"; string_of_int offset] in
    go_hackfmt genv ~filename ~content args >>= fun lines ->

    (* `hackfmt --at-char` returns the range that was formatted, as well as the
       contents of that range. For example, it might return

           10 12
           }

       signifying that we should replace the character under the cursor with the
       following content, starting at index 10. We need to extract the range
       from the first line and forward it to the client so that it knows where
       to apply the edit. *)
    begin match lines with
    | range_line :: lines -> Result.Ok (range_line, lines)
    | _ -> Result.Error "Got no lines in at-position formatting"
    end >>= fun (range_line, lines) ->

    (* Extract the offsets in the first line that form the range.
       NOTE: `Str.string_match` sets global state to be consumed immediately
       afterwards by `Str.matched_group`. *)
    let does_range_match = Str.string_match range_regexp range_line 0 in
    if not does_range_match
    then Result.Error "Range not found on first line of --at-char output"
    else

    let from0 = int_of_string (Str.matched_group 1 range_line) in
    let to0 = int_of_string (Str.matched_group 2 range_line) in
    let range = {
      st = offset_to_position content from0;
      ed = offset_to_position content to0;
    } in
    let new_text = String.concat "\n" lines in
    Result.Ok {new_text; range;}
