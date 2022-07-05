(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module LMap = Local_id.Map
module KMap = Typing_continuations.Map

(** A generic exception for all refactor sound dynamic specific failures
   Relationship with shape_analysis: Shape_analysis_exn *)
exception Refactor_sd_exn of string

(** Relationship with shape_analysis: shape_result *)
type refactor_sd_result =
  | Exists_Upcast
  | No_Upcast
