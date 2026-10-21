(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Start prefetching [files] with Sapling, then run [f] without waiting for
    prefetch to finish. Stop and reap the prefetch process when [f] returns,
    raises, or exits the server.
    Prefetch failures are logged and do not change the result of [f].

    Returns [f]'s result and prefetch telemetry, including duration when
    available. *)
val with_prefetch :
  root:Path.t ->
  files:Relative_path.Set.t ->
  workers:int ->
  f:(unit -> 'a) ->
  'a * Telemetry.t
