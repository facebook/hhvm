(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A program analysis to find locations of upcasts to dynamic type *)

open Refactor_sd_types

exception Refactor_sd_exn of string

val do_ : string -> options -> Provider_context.t -> Tast.program -> unit

(** Relationship with shape_analysis: is_shape_like_dict *)
val contains_upcast : refactor_sd_result -> bool
