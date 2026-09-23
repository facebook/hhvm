(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Mercurial state names shared by Eden and Watchman notifications.
  Asserted by hg while working. *)

val update : string

val transaction : string

val is_hg_state : string -> bool
