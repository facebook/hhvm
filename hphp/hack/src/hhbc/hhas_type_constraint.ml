(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type type_constraint_flag =
  | Nullable
  | ExtendedHint
  | TypeVar
  | Soft
  | TypeConstant
  | DisplayNullable
  | UpperBound

let string_of_flag f =
  match f with
  | Nullable -> "nullable"
  | ExtendedHint -> "extended_hint"
  | TypeVar -> "type_var"
  | Soft -> "soft"
  | TypeConstant -> "type_constant"
  | DisplayNullable -> "display_nullable"
  | UpperBound -> "upper_bound"

(* A type constraint is just a name and flags *)
type t = {
  tc_name: string option;
  tc_flags: type_constraint_flag list;
}

let make tc_name tc_flags = { tc_name; tc_flags }

let flags tc = tc.tc_flags

let name tc = tc.tc_name
