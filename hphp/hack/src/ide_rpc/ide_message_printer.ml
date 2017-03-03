(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message
open Ide_rpc_protocol_parser_types

let get_subscription_id = function
  | Request (Server_notification
        (Diagnostics_notification {subscription_id; _})) ->
      (* Nuclide subscriptions are special snowflakes with different output
      * format *)
      Some subscription_id
  | _ ->
      None

(* Delegate to the right printing module based on protocol and version *)
let get_message_body_printer protocol version =
  match protocol, version with
  | Nuclide_rpc, V0 -> Nuclide_rpc_message_printer.to_json
  | JSON_RPC2, V0 -> Ide_rpc_V0_message_printer.to_json

let get_message_protocol_printer protocol message =
  let subscription_id = get_subscription_id message in
  match subscription_id, protocol with
  | Some id, Nuclide_rpc -> Nuclide_rpc_protocol_printer.subscription_to_json id
  | None, Nuclide_rpc -> Nuclide_rpc_protocol_printer.to_json
  | _, JSON_RPC2 -> Json_rpc_message_printer.to_json

let to_json ~id ~protocol ~version ~message =
  let message_body_to_json = get_message_body_printer protocol version in
  let add_protocol_json = get_message_protocol_printer protocol message in
  add_protocol_json ~id ~message
    ~message_as_json:(message_body_to_json ~message)
