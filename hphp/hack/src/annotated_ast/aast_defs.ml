(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs_visitors_ancestors

module ShapeMap = Ast.ShapeMap

type 'a shape_map = 'a ShapeMap.t [@@deriving show]

type pos = Ast.pos [@@deriving show]

type local_id = Local_id.t [@visitors.opaque]
and lid = pos * local_id
and sid = Ast.id

and is_terminal = bool

and call_type =
  | Cnormal    [@visitors.name "call_type_Cnormal"] (* when the call looks like f() *)
  | Cuser_func [@visitors.name "call_type_Cuser_func"] (* when the call looks like call_user_func(...) *)

and is_coroutine = bool
and func_reactive = FReactive | FLocal | FShallow | FNonreactive

and hint = pos * hint_
and variadic_hint =
  | Hvariadic of hint option
  | Hnon_variadic
and hint_ =
  | Hoption of hint
  | Hfun of func_reactive * is_coroutine * hint list * Ast.param_kind option list * variadic_hint * hint
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
  | Hnonnull
  | Habstr of string
  | Harray of hint option * hint option
  | Hdarray of hint * hint
  | Hvarray of hint
  | Hvarray_or_darray of hint
  | Hprim of tprim
  | Hthis
  | Hdynamic


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
  nsi_field_map : shape_field_info shape_map;
}

and kvc_kind = [
  | `Map
  | `ImmMap
  | `Dict ]
  [@visitors.opaque]


and vc_kind = [
  | `Vector
  | `ImmVector
  | `Vec
  | `Set
  | `ImmSet
  | `Pair
  | `Keyset ]
  [@visitors.opaque]

and tparam =
  Ast.variance * sid * (Ast.constraint_kind * hint) list * Ast.reified

and visibility =
  | Private [@visitors.name "visibility_Private"]
  | Public [@visitors.name "visibility_Public"]
  | Protected [@visitors.name "visibility_Protected"]

and typedef_visibility =
  | Transparent
  | Opaque

and enum_ = {
  e_base       : hint;
  e_constraint : hint option;
}


and instantiated_sid = sid * hint list

and where_constraint = hint * Ast.constraint_kind * hint
[@@deriving
  show { with_path = false },
  visitors {
    name="iter_defs";
    variety = "iter";
    nude=true;
    visit_prefix="on_";
    ancestors=["iter_defs_base"];
  },
  visitors {
    name="reduce_defs";
    variety = "reduce";
    nude=true;
    visit_prefix="on_";
    ancestors=["reduce_defs_base"];
  },
  visitors {
    name="map_defs";
    variety = "map";
    nude=true;
    visit_prefix="on_";
    ancestors=["map_defs_base"];
  },
  visitors {
    name="endo_defs";
    variety = "endo";
    nude=true;
    visit_prefix="on_";
    ancestors=["endo_defs_base"];
  }]

type id = lid [@@deriving show]
type pstring = Ast.pstring [@@deriving show]
type shape_field_name = Ast.shape_field_name [@@deriving show]

type og_null_flavor = Ast.og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe

let pp_og_null_flavor fmt flavor =
  Format.pp_print_string fmt @@
    match flavor with
    | OG_nullthrows -> "OG_nullthrows"
    | OG_nullsafe -> "OG_nullsafe"

let pp_kvc_kind fmt _ = Format.pp_print_string fmt "<kvc_kind>"
let pp_vc_kind fmt _ = Format.pp_print_string fmt "<vc_kind>"
