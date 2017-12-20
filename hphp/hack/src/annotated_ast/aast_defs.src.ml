(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type id = Pos.t * Local_id.t [@@deriving show]
type sid = Ast.id [@@deriving show]
type pstring = Ast.pstring [@@deriving show]

type is_terminal = bool [@@deriving show]

type call_type =
  | Cnormal    (* when the call looks like f() *)
  | Cuser_func (* when the call looks like call_user_func(...) *)
  [@@deriving show]

type shape_field_name = Ast.shape_field_name

module ShapeMap = Ast.ShapeMap

type is_coroutine = bool [@@deriving show]
type is_reactive = bool [@@deriving show]

type hint = Pos.t * hint_
and variadic_hint =
  | Hvariadic of hint option
  | Hnon_variadic
and hint_ =
  | Hoption of hint
  | Hfun of is_reactive * is_coroutine * hint list * Ast.param_kind option list * variadic_hint * hint
  | Htuple of hint list
  | Happly of sid * hint list
  | Hshape of nast_shape_info

 (* This represents the use of a type const. Type consts are accessed like
  * regular consts in Hack, i.e.
  *
  * [self | static | Class]::TypeConst
  *
  * Class  => Happly "Class"
  * self   => Happly of the class of definition
  * static => Habstr ("static",
  *           Habstr ("this", (Constraint_as, Happly of class of definition)))
  * Type const access can be chained such as
  *
  * Class::TC1::TC2::TC3
  *
  * We resolve the root of the type access chain as a type as follows.
  *
  * This will result in the following representation
  *
  * Haccess (Happly "Class", ["TC1", "TC2", "TC3"])
  *)
  | Haccess of hint * sid list

  (* The following constructors don't exist in the AST hint type *)
  | Hany
  | Hmixed
  | Habstr of string
  | Harray of hint option * hint option
  | Hdarray of hint * hint
  | Hvarray of hint
  | Hvarray_or_darray of hint
  | Hprim of tprim
  | Hthis

(* AST types such as Happly("int", []) are resolved to Hprim values *)
and tprim =
  | Tvoid
  | Tint
  | Tbool
  | Tfloat
  | Tstring
  | Tresource
  | Tnum
  | Tarraykey
  | Tnoreturn

and shape_field_info = {
  sfi_optional: bool;
  sfi_hint : hint;
}

and nast_shape_info = {
  nsi_allows_unknown_fields : bool;
  nsi_field_map : shape_field_info ShapeMap.t;
}
[@@deriving show]

type og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
  [@@deriving show]

type kvc_kind = [
  | `Map
  | `ImmMap
  | `Dict ]

  let pp_kvc_kind fmt _ = Format.pp_print_string fmt "<kvc_kind>"

type vc_kind = [
  | `Vector
  | `ImmVector
  | `Vec
  | `Set
  | `ImmSet
  | `Pair
  | `Keyset ]

let pp_vc_kind fmt _ = Format.pp_print_string fmt "<vc_kind>"

type tparam = Ast.variance * sid * (Ast.constraint_kind * hint) list [@@deriving show]

type visibility =
  | Private
  | Public
  | Protected
  [@@deriving show]

type typedef_visibility =
  | Transparent
  | Opaque
  [@@deriving show]

type enum_ = {
  e_base       : hint;
  e_constraint : hint option;
} [@@deriving show]


type instantiated_sid = sid * hint list [@@deriving show]

type where_constraint = hint * Ast.constraint_kind * hint [@@deriving show]
