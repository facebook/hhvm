(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open Pp_type

(* Exception representing not finding a class during decl *)
exception Decl_not_found of string

(* A substitution context contains all the information necessary for
 * changing the type of an inherited class element to the class that is
 * inheriting the class element. It's best illustrated via an example.
 *
 * class A<Ta1, Ta2> { public function test(Ta1 $x, Ta2 $y): void {} }
 *
 * class B<Tb> extends A<Tb, int> {}
 *
 * class C extends B<string> {}
 *
 * The method `A::test()` has the type (function(Ta1, Ta2): void) in the
 * context of class A. However in the context of class B, it will have type
 * (function(Tb, int): void).

 * The substitution that leads to this change is [Ta1 -> Tb, Ta2 -> int],
 * which will produce a new type in the context of class B. It's subst_context
 * would then be:
 *
 * { sc_subst            = [Ta1 -> Tb, Ta2 -> int];
 *   sc_class_context    = 'B';
 *   sc_from_req_extends = false;
 * }
 *
 * The `sc_from_req_extends` field is set to true if the context was inherited
 * via a require extends type. This information is relevant when folding
 * `dc_substs` during inheritance. See Decl_inherit module.
 *)
type subst_context = {
  sc_subst            : decl ty SMap.t;
  sc_class_context    : string;
  sc_from_req_extends : bool;
} [@@deriving show]

type source_type = Child | Parent | Trait | XHPAttr | Interface | ReqImpl | ReqExtends
  [@@deriving show]

type mro_element = {
  (* The class's name *)
  mro_name : string;
  (* The type arguments with which this ancestor class was instantiated. The
     first class in the linearization (the one which was linearized) will have
     an empty list here, even when it takes type parameters. *)
  mro_type_args : decl ty list;
  (* True if this element is included in the linearization because of a require
     extends or require implements relationship. *)
  mro_synthesized : bool;
  (* True if this element is included in the linearization because of any
     XHP-attribute-inclusion relationship, and thus, the linearized class
     inherits only the XHP attributes from this element. *)
  mro_xhp_attrs_only : bool;
  (* True if this element is included in the linearization because of a
     interface-implementation relationship, and thus, the linearized class
     inherits only the class constants and type constants from this element. *)
  mro_consts_only : bool;
  (* True if this element is included in the linearization via an unbroken chain
     of trait-use relationships, and thus, the linearized class inherits the
     private members of this element (on account of the runtime behavior where
     they are effectively copied into the linearized class). *)
  mro_copy_private_members : bool;
} [@@deriving show]

type linearization = mro_element Sequence.t

type decl_class_type = {
  dc_need_init           : bool;
  dc_members_fully_known : bool;
  dc_abstract            : bool;
  dc_final               : bool;
  dc_is_disposable       : bool;
  dc_const               : bool;
  dc_ppl                 : bool;
  dc_deferred_init_members : SSet.t;
  dc_kind                : Ast.class_kind;
  dc_is_xhp              : bool;
  dc_name                : string ;
  dc_pos                 : Pos.t ;
  dc_tparams             : decl tparam list ;
  (* class name to the subst_context that must be applied to that class *)
  dc_substs              : subst_context SMap.t;
  dc_consts              : class_const SMap.t;
  dc_typeconsts          : typeconst_type SMap.t;
  dc_props               : element SMap.t;
  dc_sprops              : element SMap.t;
  dc_methods             : element SMap.t;
  dc_smethods            : element SMap.t;
  dc_construct           : element option * consistent_kind;
  dc_ancestors           : decl ty SMap.t ;
  dc_req_ancestors       : requirement list;
  dc_req_ancestors_extends : SSet.t;
  dc_extends             : SSet.t;
  dc_sealed_whitelist    : SSet.t option;
  dc_xhp_attr_deps       : SSet.t;
  dc_enum_type           : enum_type option;
  dc_decl_errors         : Errors.t option [@opaque];
  (* this field is used to prevent condition types being filtered
       in Decl_redecl_service.is_dependent_class_of_any *)
  dc_condition_types     : SSet.t;
} [@@deriving show]

(* name of condition type for conditional reactivity of methods.
   If None - method is unconditionally reactive *)
and condition_type_name = string option

and method_reactivity =
| Method_reactive of condition_type_name
| Method_shallow of condition_type_name
| Method_local of condition_type_name

and element = {
  elt_final : bool;
  elt_synthesized : bool;

  (* Only relevant for methods *)
  elt_override : bool;
  elt_memoizelsb : bool;
  elt_abstract : bool;
  elt_reactivity : method_reactivity option;

  (* Only relevant for properties *)
  elt_is_xhp_attr : bool;
  elt_const: bool;
  elt_lateinit: bool;
  elt_lsb: bool;

  elt_origin : string;
  elt_visibility : visibility;
}
