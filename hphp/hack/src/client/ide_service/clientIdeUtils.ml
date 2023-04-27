(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type error = {
  message: string;
  data: Hh_json.json;
}

let make_error_internal
    ~(message : string)
    ~(data : Hh_json.json option)
    ~(exn : Exception.t option) : error =
  let open Hh_json in
  let (message, stack) =
    match exn with
    | None -> (message, Exception.get_current_callstack_string 99)
    | Some exn ->
      let details =
        match Exception.unwrap exn with
        | Decl_class.Decl_heap_elems_bug details ->
          Decl_class.show_decl_heap_elems_bug details
        | _ -> ""
      in
      ( message ^ ": " ^ Exception.get_ctor_string exn ^ details,
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
  { message; data }

let log_bug
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    ~(telemetry : bool)
    (message : string) : unit =
  let { message; data } = make_error_internal ~message ~exn ~data in
  Hh_logger.error "%s\n%s" message (Hh_json.json_to_string data);
  if telemetry then HackEventLogger.serverless_ide_bug ~message ~data;
  ()

let make_bug_error
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    (message : string) : Lsp.Error.t =
  let { message; data } = make_error_internal ~message ~exn ~data in
  { Lsp.Error.code = Lsp.Error.UnknownErrorCode; message; data = Some data }

let make_bug_reason
    ?(data : Hh_json.json option = None)
    ?(exn : Exception.t option)
    (message : string) : ClientIdeMessage.stopped_reason =
  let { message; data } = make_error_internal ~message ~exn ~data in
  {
    ClientIdeMessage.short_user_message = "failed";
    medium_user_message = "hh_client has failed.";
    long_user_message =
      "hh_client has failed.\nThis is unexpected.\nPlease file a bug within your IDE, and try restarting it.";
    debug_details = message ^ "\nDETAILS:\n" ^ Hh_json.json_to_string data;
    is_actionable = false;
  }
