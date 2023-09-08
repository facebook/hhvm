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

  val get_new_client_type : unit -> connection_type option

  val get_client_request : unit -> 'a ServerCommandTypes.t option

  val get_client_response : unit -> 'a option
end

module Refs : RefsType = struct
  let new_client_type = ref None

  (* Those references are used for mocking the results of Marshal.from_channel
   * function, which is untypeable. Hence, Obj.magic *)
  let client_request = Obj.magic (ref None)

  let client_response = Obj.magic (ref None)

  let set_new_client_type x = new_client_type := x

  let set_client_request x = client_request := x

  let set_client_response x = client_response := x

  let get_new_client_type () = !new_client_type

  let get_client_response () = !client_response

  let get_client_request () = !client_request

  let clear () =
    set_new_client_type None;
    set_client_request None;
    set_client_response None;
    ()
end

let clear = Refs.clear

let mock_new_client_type x = Refs.set_new_client_type (Some x)

let mock_client_request x = Refs.set_client_request (Some x)

let get_mocked_new_client_type () = Refs.get_new_client_type ()

let get_mocked_client_request = function
  | Non_persistent -> Refs.get_client_request ()

let record_client_response x = function
  | Non_persistent -> Refs.set_client_response (Some x)

let get_client_response = function
  | Non_persistent -> Refs.get_client_response ()

type t = unit

type client = connection_type

type handoff = {
  client: client;
  m2s_sequence_number: int;
}

type select_outcome =
  | Select_persistent
  | Select_new of handoff
  | Select_nothing
  | Select_exception of Exception.t
  | Not_selecting_hg_updating

exception Client_went_away

let provider_from_file_descriptors _ = ()

let provider_for_test _ = ()

let sleep_and_check _ _ ~ide_idle:_ ~idle_gc_slice:_ _ =
  let client_opt = get_mocked_new_client_type () in
  match client_opt with
  | Some client -> Select_new { client; m2s_sequence_number = 0 }
  | None -> Select_nothing

let has_persistent_connection_request _ = false

let priority_fd _ = None

let not_implemented () = failwith "not implemented"

let get_client_fd _ = not_implemented ()

let track ~key:_ ?time:_ ?log:_ ?msg:_ ?long_delay_okay:_ _ = ()

let accept_client _ = Non_persistent

let read_connection_type _ = Utils.unsafe_opt (get_mocked_new_client_type ())

let send_response_to_client c x = record_client_response x c

let client_has_message _ = false

let read_client_msg c =
  let metadata = { ServerCommandTypes.from = "test"; desc = "cmd" } in
  Rpc (metadata, Utils.unsafe_opt (get_mocked_client_request c))

let get_channels _ = not_implemented ()

let is_persistent = function
  | Non_persistent -> false

let priority_to_string (_client : client) : string = "mock"

let shutdown_client _ = ()

let ping _ = ()
