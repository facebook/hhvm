(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ServerCommandTypes

module type RefsType = sig

  val clear: unit -> unit

  val set_new_client_type: connection_type option -> unit

  val set_client_request: 'a ServerCommandTypes.t option -> unit
  val set_client_response: 'a option -> unit

  val set_persistent_client_request: 'a ServerCommandTypes.t option -> unit
  val set_persistent_client_response: 'a option  -> unit
  val set_push_message: ServerCommandTypes.push option -> unit

  val get_new_client_type: unit -> connection_type option

  val get_client_request: unit -> 'a ServerCommandTypes.t option
  val get_client_response: unit -> 'a option

  val get_persistent_client_request: unit -> 'b
  val get_persistent_client_response: unit -> 'a option
  val get_push_message: unit -> ServerCommandTypes.push option
end

module Refs : RefsType = struct

  let new_client_type = ref None

  (* Those references are used for mocking the results of Marshal.from_channel
   * function, which is untypeable. Hence, Obj.magic *)
  let client_request = Obj.magic (ref None)
  let client_response = Obj.magic (ref None)
  let persistent_client_request = Obj.magic (ref None)
  let persistent_client_response = Obj.magic (ref None)

  let push_message = ref None

  let set_new_client_type x = new_client_type := x
  let set_client_request x = client_request := x
  let set_client_response x = client_response := x
  let set_persistent_client_request x = persistent_client_request := x
  let set_persistent_client_response x = persistent_client_response := x
  let set_push_message x = push_message := x

  let get_new_client_type () = !new_client_type
  let get_client_response () = !client_response
  let get_client_request () = !client_request
  let get_persistent_client_request () = !persistent_client_request
  let get_persistent_client_response () = !persistent_client_response
  let get_push_message () = !push_message

  let clear () =
    set_new_client_type None;
    set_client_request None;
    set_client_response None;
    set_persistent_client_request None;
    set_persistent_client_response None;
    set_persistent_client_response None;
    set_push_message None;
end

let clear = Refs.clear

let mock_new_client_type x = Refs.set_new_client_type (Some x)

let mock_client_request x = Refs.set_client_request (Some x)

let mock_persistent_client_request x =
  Refs.set_persistent_client_request (Some x)

let get_mocked_new_client_type () = Refs.get_new_client_type ()

let get_mocked_client_request = function
  | Non_persistent -> Refs.get_client_request ()
  | Persistent-> Refs.get_persistent_client_request ()

let record_client_response x = function
  | Non_persistent -> Refs.set_client_response (Some x)
  | Persistent ->  Refs.set_persistent_client_response (Some x)

let get_client_response = function
  | Non_persistent -> Refs.get_client_response ()
  | Persistent -> Refs.get_persistent_client_response ()

let record_push_message x = Refs.set_push_message (Some x)

let get_push_message = Refs.get_push_message

type t = unit
type client = connection_type

exception Client_went_away

let provider_from_file_descriptor _ = ()
let provider_for_test _ = ()

let sleep_and_check _ _ =
  Option.is_some (get_mocked_new_client_type ()),
  Option.is_some (get_mocked_client_request Persistent)

let not_implemented () = failwith "not implemented"

let accept_client _ = Non_persistent

let say_hello _ = not_implemented ()

let read_connection_type _ = Utils.unsafe_opt (get_mocked_new_client_type ())

let send_response_to_client c x = record_client_response x c

let send_push_message_to_client _ x = record_push_message x

let read_client_msg c = Rpc (Utils.unsafe_opt (get_mocked_client_request c))

let get_channels _ = not_implemented ()

let is_persistent = function
  | Persistent -> true
  | Non_persistent -> false

let make_persistent _ = ServerCommandTypes.Persistent

let shutdown_client _ = ()
