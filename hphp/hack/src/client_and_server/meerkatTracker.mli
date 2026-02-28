(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
  TODO(T226505256)
  This module is part of a workaround to support deferring file change
  notifications in the Eden-backed version of ServerNotifier whenever Meerkat is
  running. It's a workaround in the sense that the Eden file watching API will
  eventually be able to do the deferral itself.

  In the meantime, this module tracks whether or not Meerkat is currently
  running, by being informed about state transitions for the "meerkat-build"
  state by ServerNotifier. The implementation is intentionally copied from
  ServerRevisionTracker with only minimal changes. *)

(** Called by Eden-backed ServerNotifier on any state enter transition *)
val on_state_enter : string -> unit

(** Called by Eden-backed ServerNotifier on any state leave transition *)
val on_state_leave : string -> unit

val is_meerkat_running : unit -> bool
