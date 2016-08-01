(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Typing_defs

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
}

type decl_class_type = {
  dc_need_init           : bool;
  dc_members_fully_known : bool;
  dc_abstract            : bool;
  dc_final               : bool;
  dc_deferred_init_members : SSet.t;
  dc_kind                : Ast.class_kind;
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
  dc_construct           : element option * bool;
  dc_ancestors           : decl ty SMap.t ;
  dc_req_ancestors       : requirement list;
  dc_req_ancestors_extends : SSet.t;
  dc_extends             : SSet.t;
  dc_enum_type           : enum_type option;
}

and element = {
  elt_final : bool;
  elt_synthesized : bool;

  (* Only relevant for methods *)
  elt_override : bool;
  elt_abstract : bool;

  (* Only relevant for properties *)
  elt_is_xhp_attr : bool;

  elt_origin : string;
  elt_visibility : visibility;
}
