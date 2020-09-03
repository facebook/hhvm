(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Render the details about the fanout to be checked in a nice, colorized,
user-readable format. Prints to stdout. *)
val go : Calculate_fanout.result list -> unit Lwt.t
