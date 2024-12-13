(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Watchman_process_helpers : sig
  module J = Hh_json_helpers.AdhocJsonHelpers

  val debug : bool

  val timeout_to_secs : Watchman_sig.Types.timeout -> float option

  exception Read_payload_too_long

  val assert_no_error : Hh_json.json -> unit

  val sanitize_watchman_response : debug_logging:bool -> string -> Hh_json.json
end

include Watchman_sig.S
