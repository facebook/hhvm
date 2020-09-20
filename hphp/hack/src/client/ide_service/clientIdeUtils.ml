(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let make_error_internal
    ~(message : string)
    ~(data : Hh_json.json option)
    ~(exn : Exception.t option) : string * Hh_json.json option * string =
  let open Hh_json in
  let (message, stack) =
    match exn with
    | None -> (message, Exception.get_current_callstack_string 99)
    | Some exn ->
      ( message ^ ": " ^ Exception.get_ctor_string exn,
        Exception.get_backtrace_string exn )
  in
  let stack = ("stack", stack |> Exception.clean_stack |> string_) in
  let elems =
    match data with
    | None -> [stack]
    | Some (JSON_Object elems) ->
      if List.Assoc.mem ~equal:String.equal elems "stack" then
        elems
      else
        stack :: elems
    | Some data -> [("data", data); stack]
  in
  let data = Hh_json.JSON_Object elems in
  let data_str = Hh_json.json_to_string data in
  (message, Some data, data_str)

let log_bug
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    ~(telemetry : bool)
    (message : string) : unit =
  let (message, data, data_str) = make_error_internal ~message ~exn ~data in
  Hh_logger.error "%s\n%s" message data_str;
  if telemetry then HackEventLogger.serverless_ide_bug ~message ~data;
  ()

let make_bug_error
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    (message : string) : Lsp.Error.t =
  let (message, data, _) = make_error_internal ~message ~exn ~data in
  { Lsp.Error.code = Lsp.Error.UnknownErrorCode; message; data }

let make_bug_reason
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    (message : string) : ClientIdeMessage.stopped_reason =
  let (message, _, data_str) = make_error_internal ~message ~exn ~data in
  {
    ClientIdeMessage.short_user_message = "failed";
    medium_user_message = "Hack IDE has failed.";
    long_user_message =
      "Hack IDE has failed.\nThis is unexpected.\nPlease file a bug within your IDE.";
    debug_details = message ^ "\nDETAILS:\n" ^ data_str;
    is_actionable = false;
  }
