(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Reason = Typing_reason

(* Check that a property type is a subtype of dynamic *)
let check_property_sound_for_dynamic_read ~on_error env classname id ty =
  if
    not
      (Typing_utils.is_sub_type_for_union
         ~coerce:(Some Typing_logic.CoerceToDynamic)
         env
         ty
         (mk (Reason.Rnone, Tdynamic)))
  then
    on_error
      (fst id)
      (snd id)
      classname
      (get_pos ty, Typing_print.full_strip_ns env ty)

let check_property_sound_for_dynamic_write ~on_error env classname id ty =
  let te_check = Typing_enforceability.is_enforceable env ty in
  if not te_check then
    on_error
      (fst id)
      (snd id)
      classname
      (get_pos ty, Typing_print.full_strip_ns_decl env ty)
