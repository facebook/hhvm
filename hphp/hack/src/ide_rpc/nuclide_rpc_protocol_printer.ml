(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json

let subscription_to_json subscription_id ~id:_ ~message:_ ~message_as_json =
   JSON_Object [
    ("protocol", JSON_String "service_framework3_rpc");
    ("type", JSON_String "next");
    ("id", int_ subscription_id);
    ("value", message_as_json);
  ]

let to_json ~id ~message:_ ~message_as_json =
  JSON_Object [
    ("protocol", JSON_String "service_framework3_rpc");
    ("type", JSON_String "response");
    ("id", opt_int_to_json id);
    ("result", message_as_json);
  ]
