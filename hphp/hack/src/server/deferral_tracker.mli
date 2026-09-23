(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

type decision =
  | Ok
  | Deferred_by of string list
      (** Nonempty list of blocking asserted states, sorted by name. *)

(** Hg states only block when [hg_aware] is enabled; disabling
    [block_client_connections_while_deferring] bypasses all deferral blocking. *)
val should_accept_client_connection :
  local_config:Server_local_config.t -> Server_notifier.t -> decision
