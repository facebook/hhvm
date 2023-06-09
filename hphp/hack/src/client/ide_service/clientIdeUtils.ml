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
  let (message, stack, desc) =
    match exn with
    | None -> (message, Exception.get_current_callstack_string 99, "")
    | Some e -> begin
      let backtrace = Exception.get_backtrace_string e in
      match Exception.to_exn e with
      | Decl_class.Decl_heap_elems_bug details ->
        (message ^ ": Decl_heap_elems_bug", backtrace, details)
      | Decl_defs.Decl_not_found details ->
        (message ^ ": Decl_not_found", backtrace, details)
      | _ -> (message ^ ": " ^ Exception.get_ctor_string e, backtrace, "")
    end
  in
  let stack = ("stack", stack |> Exception.clean_stack |> string_) in
  let desc = ("desc", string_ desc) in
  let elems =
    match data with
    | None -> [stack]
    | Some (JSON_Object elems) ->
      if List.Assoc.mem ~equal:String.equal elems "stack" then
        elems
      else
        stack :: elems
    | Some data -> [("data", data); stack; desc]
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
    medium_user_message = "Hack IDE support has failed.";
    long_user_message =
      "Hack IDE support has failed.\nThis is unexpected.\nPlease file a bug within your IDE, and try restarting it.";
    debug_details = message ^ "\nDETAILS:\n" ^ Hh_json.json_to_string data;
    is_actionable = false;
  }
