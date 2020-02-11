(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Typing_defs
module T = Aast
module S = ServerRxApiShared
module BRC = Basic_reactivity_check
module Env = Tast_env

let result_to_string result (fn, line, char) =
  Hh_json.(
    let fields =
      [("position", S.pos_to_json fn line char)]
      @
      match result with
      | Ok (Some []) -> [("result", JSON_Bool true)]
      | Ok (Some errors) ->
        let errors =
          errors |> List.map ~f:(fun e -> Errors.(to_json (to_absolute e)))
        in
        [("result", JSON_Bool false); ("errors", JSON_Array errors)]
      | Ok None -> [("error", JSON_String "Function/method not found")]
      | Error e -> [("error", JSON_String e)]
    in
    json_to_string (JSON_Object fields))

let process_body env body =
  let ctx = BRC.new_ctx_for_is_locallable_pass (Local None) in
  let (errors, _) = Errors.do_ (fun () -> BRC.check#handle_body env ctx body) in
  Errors.get_error_list errors

let walker =
  {
    S.plus = List.rev_append;
    S.on_fun =
      begin
        fun env f ->
        let env = Env.restore_fun_env env f in
        process_body env f.T.f_body
      end;
    S.on_method =
      begin
        fun env m ->
        let env = Env.restore_method_env env m in
        process_body env m.T.m_body
      end;
  }

let handlers =
  {
    S.result_to_string;
    S.walker;
    S.get_state = (fun _ _ -> ());
    S.map_result = (fun _ () r -> r);
  }

(* Entry Point *)
let go :
    MultiWorker.worker list option ->
    (string * int * int) list ->
    ServerEnv.env ->
    _ =
 (fun workers pos_list env -> ServerRxApiShared.go workers pos_list env handlers)
