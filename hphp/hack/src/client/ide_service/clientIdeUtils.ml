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
  category: string;
  data: Hh_json.json;
}

let make_error_internal
    ~(category : string) ~(data : Hh_json.json option) ~(e : Exception.t option)
    : error =
  let open Hh_json in
  let (category, stack, desc) =
    match e with
    | None -> (category, Exception.get_current_callstack_string 99, "")
    | Some e -> begin
      let backtrace = Exception.get_backtrace_string e in
      match Exception.to_exn e with
      | Decl_class.Decl_heap_elems_bug details ->
        (category ^ ": Decl_heap_elems_bug", backtrace, details)
      | Decl_defs.Decl_not_found details ->
        (category ^ ": Decl_not_found", backtrace, details)
      | _ -> (category ^ ": " ^ Exception.get_ctor_string e, backtrace, "")
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
  { category; data }

let log_bug
    ?(data : Hh_json.json option = None)
    ?(e : Exception.t option)
    ~(telemetry : bool)
    (category : string) : unit =
  let { category; data } = make_error_internal ~category ~e ~data in
  Hh_logger.error "%s\n%s" category (Hh_json.json_to_string data);
  if telemetry then HackEventLogger.serverless_ide_bug ~message:category ~data;
  ()

let make_rich_error
    ?(data : Hh_json.json option = None)
    ?(e : Exception.t option)
    (category : string) : ClientIdeMessage.rich_error =
  let { category; data } = make_error_internal ~category ~e ~data in
  {
    ClientIdeMessage.short_user_message = "failed";
    medium_user_message = "Hack IDE support has failed.";
    long_user_message =
      "Hack IDE support has failed.\nThis is unexpected.\nPlease file a bug within your IDE, and try restarting it.";
    is_actionable = false;
    category;
    data = Some data;
  }

let to_lsp_error (reason : ClientIdeMessage.rich_error) : Lsp.Error.t =
  {
    Lsp.Error.code = Lsp.Error.UnknownErrorCode;
    message = reason.ClientIdeMessage.category;
    data = reason.ClientIdeMessage.data;
  }
