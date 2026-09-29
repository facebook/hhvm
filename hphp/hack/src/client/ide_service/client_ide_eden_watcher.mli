(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Starts an Eden watcher synchronously, then polls for small commit transitions.
    Transitions exceeding [max_changed_files] after filtering and deduplication
    are skipped.
    [on_changes] receives filtered, deduplicated Hack source paths; returning
    [false] stops polling. Native initialization errors leave the watcher
    disabled; subsequent watcher failures stop polling without propagating to
    the caller. Whenever polling ends, the native watcher is destroyed.

    This module owns the watcher for the lifetime of the daemon and registers
    synchronous cleanup with [Stdlib.at_exit]. Callers do not stop it when IDE
    initialization fails: it may keep polling until process exit, with its
    events ignored by the failed daemon. *)
val start :
  root:Path.t ->
  max_changed_files:int ->
  on_changes:(Relative_path.Set.t -> bool) ->
  unit
