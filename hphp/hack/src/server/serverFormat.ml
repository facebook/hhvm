(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* TODO t14922604: Further improve error handling *)
let call_external_formatter path content from to_ =
  let args = [|"hackfmt"; "--range"; string_of_int from; string_of_int to_|] in
  let lines = ref [] in
  let status = ref None in
  let reader timeout ic oc =
    output_string oc content;
    close_out oc;
    try while true do lines := Timeout.input_line ~timeout ic :: !lines done
      with End_of_file -> lines := "" :: !lines;
    status := Some (Timeout.close_process_in ic);
  in
  try
    Timeout.read_process
      ~timeout:2
      ~on_timeout:(fun _ -> ())
      ~reader
      path
      args;
    match !status with
      | Some (Unix.WEXITED 0) ->
          Result.Ok (String.concat "\n" @@ List.rev !lines)
      | Some (Unix.WEXITED v) ->
          Result.Error (Hackfmt_error.get_error_string_from_exit_value v)
      | None -> Result.Error "Call to hackfmt never terminated"
      | _ -> Result.Error "Call to hackfmt was killed"
  with Timeout.Timeout -> begin
    Hh_logger.log "Formatter timeout";
    Result.Error "Call to hackfmt timed out"
  end

let go_hackfmt genv content from to_ =
  Hh_logger.log "--range %d %d" from to_;
  let path = match ServerConfig.formatter_override genv.ServerEnv.config with
    | None -> Path.make "/usr/local/bin/hackfmt"
    | Some p -> p
  in
  let path = Path.to_string path in
  if Sys.file_exists path
  then call_external_formatter path content from to_
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
let go genv content from to_ =
  if genv.ServerEnv.local_config.ServerLocalConfig.use_hackfmt
  then go_hackfmt genv content from to_
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

let go_ide genv content range_opt =
  let open Ide_api_types in
  let open ServerFormatTypes in
  let open File_content in
  let range, from0, to0 = match range_opt with
    | None ->
        let from0 = 0 in
        let to0 = String.length content in
        let ed = offset_to_position content to0 in
        let range = {st = {line = 1; column = 1;}; ed;} in
        (range, from0, to0)
    | Some range ->
        expand_range_to_whole_rows content range
  in
  (* get_offsets returns 0-based offsets, but we need 1-based. *)
  let result = go genv content (from0 + 1) (to0 + 1) in
  match result with
    | Result.Ok new_text -> Result.Ok {new_text; range;}
    | Result.Error e -> Result.Error e
