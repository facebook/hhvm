(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCommandTypes

module type RefsType = sig
  val clear : unit -> unit

  val set_new_client_type : connection_type option -> unit

  val set_client_request : 'a ServerCommandTypes.t option -> unit

  val set_client_response : 'a option -> unit

  val set_unclean_disconnect : bool -> unit

  val set_persistent_client_request : 'a ServerCommandTypes.t option -> unit

  val set_persistent_client_response : 'a option -> unit

  val push_message : ServerCommandTypes.push -> unit

  val get_new_client_type : unit -> connection_type option

  val get_client_request : unit -> 'a ServerCommandTypes.t option

  val get_client_response : unit -> 'a option

  val get_unclean_disconnect : unit -> bool

  val get_persistent_client_request : unit -> 'b

  val get_persistent_client_response : unit -> 'a option

  val get_push_messages : unit -> ServerCommandTypes.push list
end

module Refs : RefsType = struct
  let new_client_type = ref None

  (* Those references are used for mocking the results of Marshal.from_channel
   * function, which is untypeable. Hence, Obj.magic *)
  let client_request = Obj.magic (ref None)

  let client_response = Obj.magic (ref None)

  let unclean_disconnect = ref false

  let persistent_client_request = Obj.magic (ref None)

  let persistent_client_response = Obj.magic (ref None)

  let push_messages : ServerCommandTypes.push list ref = ref []

  let set_new_client_type x = new_client_type := x

  let set_client_request x = client_request := x

  let set_client_response x = client_response := x

  let set_unclean_disconnect x = unclean_disconnect := x

  let set_persistent_client_request x = persistent_client_request := x

  let set_persistent_client_response x = persistent_client_response := x

  let clear_push_messages () = push_messages := []

  let push_message x = push_messages := x :: !push_messages

  let get_new_client_type () = !new_client_type

  let get_client_response () = !client_response

  let get_unclean_disconnect () = !unclean_disconnect

  let get_client_request () = !client_request

  let get_persistent_client_request () = !persistent_client_request

  let get_persistent_client_response () = !persistent_client_response

  let get_push_messages () =
    let push_messages = !push_messages in
    clear_push_messages ();
    push_messages

  let clear () =
    set_new_client_type None;
    set_client_request None;
    set_client_response None;
    set_unclean_disconnect false;
    set_persistent_client_request None;
    set_persistent_client_response None;
    set_persistent_client_response None;
    clear_push_messages ();
    ()
end

let clear = Refs.clear

let mock_new_client_type x = Refs.set_new_client_type (Some x)

let mock_client_request x = Refs.set_client_request (Some x)

let mock_unclean_disconnect () = Refs.set_unclean_disconnect true

let mock_persistent_client_request x =
  Refs.set_persistent_client_request (Some x)

let get_mocked_new_client_type () = Refs.get_new_client_type ()

let get_mocked_client_request = function
  | Non_persistent -> Refs.get_client_request ()
  | Persistent -> Refs.get_persistent_client_request ()

let get_mocked_unclean_disconnect = function
  | Non_persistent -> false
  | Persistent -> Refs.get_unclean_disconnect ()

let record_client_response x = function
  | Non_persistent -> Refs.set_client_response (Some x)
  | Persistent -> Refs.set_persistent_client_response (Some x)

let get_client_response = function
  | Non_persistent -> Refs.get_client_response ()
  | Persistent -> Refs.get_persistent_client_response ()

let push_message x = Refs.push_message x

let get_push_messages = Refs.get_push_messages

module ClientId : sig
  type t = int

  val make : unit -> t
end = struct
  type t = int

  let next_id : t ref = ref 0

  let make () =
    let id = !next_id in
    next_id := id + 1;
    id
end

type t = unit

type client = connection_type

type client_id = ClientId.t

type select_outcome =
  | Select_persistent
  | Select_new of client
  | Select_nothing

exception Client_went_away

let provider_from_file_descriptors _ = ()

let provider_for_test _ = ()

let sleep_and_check _ _ ~ide_idle:_ ~idle_gc_slice:_ _ =
  let client_opt = get_mocked_new_client_type () in
  let is_persistent = Option.is_some (get_mocked_client_request Persistent) in
  match (is_persistent, client_opt) with
  | (true, _) -> Select_persistent
  | (false, Some client) -> Select_new client
  | (false, None) -> Select_nothing

let has_persistent_connection_request _ =
  Option.is_some (get_mocked_client_request Persistent)

let priority_fd _ = None

let not_implemented () = failwith "not implemented"

let get_client_fd _ = not_implemented ()

let track ~key:_ ?time:_ _ = ()

let accept_client _ = Non_persistent

let read_connection_type _ = Utils.unsafe_opt (get_mocked_new_client_type ())

let send_response_to_client c x =
  if get_mocked_unclean_disconnect c then
    raise Client_went_away
  else
    record_client_response x c

let send_push_message_to_client _ x = push_message x

let client_has_message _ = Option.is_some (get_mocked_client_request Persistent)

let read_client_msg c =
  let metadata = { ServerCommandTypes.from = "test"; desc = "cmd" } in
  Rpc (metadata, Utils.unsafe_opt (get_mocked_client_request c))

let get_channels _ = not_implemented ()

let is_persistent = function
  | Persistent -> true
  | Non_persistent -> false

let priority_to_string (_client : client) : string = "mock"

let persistent_client : (client_id * client) option ref = ref None

let make_and_store_persistent _ =
  let client = ServerCommandTypes.Persistent in
  persistent_client := Some (ClientId.make (), client);
  client

let disconnect_persistent () = persistent_client := None

let get_persistent_client () = !persistent_client

let shutdown_client _ = ()

let ping _ = ()
