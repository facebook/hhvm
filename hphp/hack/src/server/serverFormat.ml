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

let go genv content from to_ =
  if genv.ServerEnv.local_config.ServerLocalConfig.use_hackfmt
  then go_hackfmt genv content from to_
  else go_hh_format genv content from to_

let go_ide genv content range =
  let open Ide_api_types in
  let from, to_ = File_content.get_offsets content (range.st, range.ed) in
  go genv content from to_
