(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

type decision =
  | Ok
  | Deferred_by of string list

let should_block_on ~(local_config : Server_local_config.t) state =
  if Hg_states.is_hg_state state then
    local_config.hg_aware
  else
    true

let should_accept_client_connection
    ~(local_config : Server_local_config.t) notifier =
  if not local_config.block_client_connections_while_deferring then
    Ok
  else
    let asserted_states = Server_notifier.get_deferring_states notifier in
    let blocking_states =
      List.filter asserted_states ~f:(should_block_on ~local_config)
    in
    match blocking_states with
    | [] -> Ok
    | states -> Deferred_by states
