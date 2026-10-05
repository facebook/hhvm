(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** What a run reports about its own memory. An isolation run is bounded by
    memory rather than by time, so every progress line carries these. *)

(** Resident set size in GiB. [0.] when the platform does not report one, which
    a caller cannot tell from a genuine zero. *)
val rss_gib : unit -> float

(** The shared heap in GiB. *)
val heap_gib : unit -> float
