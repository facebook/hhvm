(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs_visitors_ancestors
module ShapeMap = Ast_defs.ShapeMap

type 'a shape_map = 'a ShapeMap.t [@@deriving eq, show]

type 'a local_id_map = 'a Local_id.Map.t [@@deriving eq]

let pp_local_id_map _ fmt map =
  Format.fprintf fmt "@[<hov 2>{";
  ignore
    (Local_id.Map.fold
       (fun key _ sep ->
         if sep then Format.fprintf fmt "@ ";
         Local_id.pp fmt key;
         true)
       map
       false);
  Format.fprintf fmt "}@]"

type pos = Ast_defs.pos [@@deriving eq, show, ord]

type byte_string = Ast_defs.byte_string [@@deriving eq, show, ord]

type visibility = Ast_defs.visibility =
  | Private [@visitors.name "visibility_Private"]
  | Public [@visitors.name "visibility_Public"]
  | Protected [@visitors.name "visibility_Protected"]
  | Internal [@visitors.name "visibility_Internal"]
[@@deriving eq, ord, show { with_path = false }]

type local_id = (Local_id.t[@visitors.opaque])

and lid = pos * local_id

and sid = Ast_defs.id

and is_reified = bool

and import_flavor =
  | Include
  | Require
  | IncludeOnce
  | RequireOnce

and xhp_child =
  | ChildName of sid
  | ChildList of xhp_child list
  | ChildUnary of xhp_child * xhp_child_op
  | ChildBinary of xhp_child * xhp_child

and xhp_child_op =
  | ChildStar
  | ChildPlus
  | ChildQuestion

and hint = pos * hint_

and variadic_hint = hint option

and contexts = pos * hint list

and hf_param_info = {
  hfparam_kind: Ast_defs.param_kind option;
  hfparam_readonlyness: Ast_defs.readonly_kind option;
}

and hint_fun = {
  hf_is_readonly: Ast_defs.readonly_kind option;
  hf_param_tys: hint list;
  (* hf_param_info is None when all three are none, for perf optimization reasons.
   It is not semantically incorrect for the record to appear with 3 None values,
   but in practice we shouldn't lower to that, since it wastes CPU/space *)
  hf_param_info: hf_param_info option list;
  hf_variadic_ty: variadic_hint;
  hf_ctxs: contexts option;
  hf_return_ty: hint;
  hf_is_readonly_return: Ast_defs.readonly_kind option;
}

and hint_ =
  | Hoption of hint
  | Hlike of hint
  | Hfun of hint_fun
  | Htuple of hint list
  | Happly of sid * hint list
  | Hshape of nast_shape_info
  | Haccess of hint * sid list
      (** This represents the use of a type const. Type consts are accessed like
       * regular consts in Hack, i.e.
       *
       * [$x | self | static | Class]::TypeConst
       *
       * Class  => Happly "Class"
       * self   => Happly of the class of definition
       * static => Habstr ("static",
       *           Habstr ("this", (Constraint_as, Happly of class of definition)))
       * $x     => Hvar "$x"
       *
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
  | Hsoft of hint
  (* The following constructors don't exist in the AST hint type *)
  | Hany
  | Herr
  | Hmixed
  | Hnonnull
  | Habstr of string * hint list
  | Hdarray of hint * hint
  | Hvarray of hint
  | Hvarray_or_darray of hint option * hint
  | Hvec_or_dict of hint option * hint
  | Hprim of tprim
  | Hthis
  | Hdynamic
  | Hnothing
  | Hunion of hint list
  | Hintersection of hint list
  | Hfun_context of string
  | Hvar of string

(** AST types such as Happly("int", []) are resolved to Hprim values *)
and tprim =
  | Tnull
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
  sfi_hint: hint;
  sfi_name: Ast_defs.shape_field_name;
}

and nast_shape_info = {
  nsi_allows_unknown_fields: bool;
  nsi_field_map: shape_field_info list;
}

and kvc_kind =
  | Map
  | ImmMap
  | Dict
[@@visitors.opaque]

and vc_kind =
  | Vector
  | ImmVector
  | Vec
  | Set
  | ImmSet
  | Keyset
[@@visitors.opaque]

and use_as_visibility =
  | UseAsPublic
  | UseAsPrivate
  | UseAsProtected
  | UseAsFinal

and typedef_visibility =
  | Transparent
  | Opaque
  | Tinternal

and enum_ = {
  e_base: hint;
  e_constraint: hint option;
  e_includes: hint list;
  e_enum_class: bool;
}

and where_constraint_hint = hint * Ast_defs.constraint_kind * hint

and reify_kind =
  | Erased
  | SoftReified
  | Reified
[@@deriving
  show { with_path = false },
    eq,
    ord,
    visitors
      {
        name = "iter_defs";
        variety = "iter";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["iter_defs_base"];
      },
    visitors
      {
        name = "reduce_defs";
        variety = "reduce";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["reduce_defs_base"];
      },
    visitors
      {
        name = "map_defs";
        variety = "map";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["map_defs_base"];
      },
    visitors
      {
        name = "endo_defs";
        variety = "endo";
        nude = true;
        visit_prefix = "on_";
        ancestors = ["endo_defs_base"];
      }]

let string_of_visibility vis =
  match vis with
  | Private -> "private"
  | Public -> "public"
  | Protected -> "protected"
  | Internal -> "internal"

let string_of_use_as_visibility vis =
  match vis with
  | UseAsPublic -> "public"
  | UseAsPrivate -> "private"
  | UseAsProtected -> "protected"
  | UseAsFinal -> "final"

type pstring = Ast_defs.pstring [@@deriving eq, show]

type positioned_byte_string = Ast_defs.positioned_byte_string
[@@deriving eq, show]

type og_null_flavor = Ast_defs.og_null_flavor =
  | OG_nullthrows
  | OG_nullsafe
[@@deriving eq]

let pp_og_null_flavor fmt flavor =
  Format.pp_print_string fmt
  @@
  match flavor with
  | OG_nullthrows -> "OG_nullthrows"
  | OG_nullsafe -> "OG_nullsafe"
