(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Type info has additional optional user type *)
type t = {
  type_info_user_type: string option;
  type_info_type_constraint: Hhas_type_constraint.t;
}

let make type_info_user_type type_info_type_constraint =
  { type_info_user_type; type_info_type_constraint }

let user_type ti = ti.type_info_user_type

let type_constraint ti = ti.type_info_type_constraint

let has_type_constraint ti =
  Hhas_type_constraint.name ti.type_info_type_constraint <> None
