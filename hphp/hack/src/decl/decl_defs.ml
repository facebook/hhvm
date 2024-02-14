(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

(** Exception representing not finding a class during decl *)
exception Decl_not_found of string

let raise_decl_not_found (path : Relative_path.t option) (name : string) : 'a =
  HackEventLogger.decl_consistency_bug ?path ~data:name "Decl_not_found";
  let err_str =
    Printf.sprintf
      "%s not found in %s"
      name
      (Option.value_map path ~default:"_" ~f:Relative_path.to_absolute)
  in
  Hh_logger.log
    "Decl_not_found: %s\n%s"
    err_str
    (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
  raise (Decl_not_found err_str)

(** A substitution context contains all the information necessary for
 * changing the type of an inherited class element to the class that is
 * inheriting the class element. It's best illustrated via an example.
 *
 * ```
 * class A<Ta1, Ta2> { public function test(Ta1 $x, Ta2 $y): void {} }
 *
 * class B<Tb> extends A<Tb, int> {}
 *
 * class C extends B<string> {}
 * ```
 *
 * The method `A::test()` has the type (function(Ta1, Ta2): void) in the
 * context of class A. However in the context of class B, it will have type
 * (function(Tb, int): void).

 * The substitution that leads to this change is [Ta1 -> Tb, Ta2 -> int],
 * which will produce a new type in the context of class B. It's subst_context
 * would then be:
 *
 * ```
 * { sc_subst            = [Ta1 -> Tb, Ta2 -> int];
 *   sc_class_context    = 'B';
 *   sc_from_req_extends = false;
 * }
 * ```
 *
 * The `sc_from_req_extends` field is set to true if the context was inherited
 * via a require extends type. This information is relevant when folding
 * `dc_substs` during inheritance. See Decl_inherit module.
 *)
type subst_context = {
  sc_subst: decl_ty SMap.t;
  sc_class_context: string;
  sc_from_req_extends: bool;
}
[@@deriving show, ord]

type source_type =
  | Child
  | Parent
  | Trait
  | XHPAttr
  | Interface
  | IncludedEnum
  | ReqImpl
  | ReqExtends
  | ReqClass
[@@deriving eq, show]

type decl_error =
  | Wrong_extend_kind of {
      pos: Pos.t;
      kind: Ast_defs.classish_kind;
      name: string;
      parent_pos: Pos_or_decl.t;
      parent_kind: Ast_defs.classish_kind;
      parent_name: string;
    }
  | Wrong_use_kind of {
      pos: Pos.t;
      name: string;
      parent_pos: Pos_or_decl.t;
      parent_name: string;
    }
  | Cyclic_class_def of {
      pos: Pos.t;
      stack: SSet.t;
    }
[@@deriving show]

type element = {
  elt_flags: Typing_defs_flags.ClassElt.t;
  elt_origin: string;
  elt_visibility: ce_visibility;
  elt_deprecated: string option;
  elt_sort_text: string option;
}
[@@deriving show]

type decl_class_type = {
  dc_need_init: bool;
  dc_abstract: bool;
  dc_final: bool;
  dc_const: bool;
  dc_internal: bool;
  dc_deferred_init_members: SSet.t;
  dc_kind: Ast_defs.classish_kind;
  dc_is_xhp: bool;
  dc_has_xhp_keyword: bool;
  dc_module: Ast_defs.id option;
  dc_is_module_level_trait: bool;
  dc_name: string;
  dc_pos: Pos_or_decl.t;
  dc_tparams: decl_tparam list;
  dc_where_constraints: decl_where_constraint list;
  dc_substs: subst_context SMap.t;
      (** class name to the subst_context that must be applied to that class *)
  dc_consts: class_const SMap.t;
  dc_typeconsts: typeconst_type SMap.t;
  dc_props: element SMap.t;
  dc_sprops: element SMap.t;
  dc_methods: element SMap.t;
  dc_smethods: element SMap.t;
  dc_construct: element option * consistent_kind;
  dc_ancestors: decl_ty SMap.t;
  dc_support_dynamic_type: bool;
  dc_req_ancestors: requirement list;
      (** All the `require extends` and `require implements`,
          possibly inherited from interface or trait ancestors.
          Does not include `require class` *)
  dc_req_ancestors_extends: SSet.t;
      (** All the `require extends` and `require implements`,
          possibly inherited from interface or trait ancestors,
          plus some extends and other ancestors of these.
          Does not include `require class` *)
  dc_req_class_ancestors: requirement list;
      (** dc_req_class_ancestors gathers all the `require class`
          requirements declared in ancestors.  Remark that `require class`
          requirements are _not_ stored in `dc_req_ancestors` or
         `dc_req_ancestors_extends` fields. *)
  dc_extends: SSet.t;
  dc_sealed_whitelist: SSet.t option;
  dc_xhp_attr_deps: SSet.t;
  dc_xhp_enum_values: Ast_defs.xhp_enum_value list SMap.t;
  dc_xhp_marked_empty: bool;
  dc_enum_type: enum_type option;
  dc_decl_errors: decl_error list;
  dc_docs_url: string option;
  dc_allow_multiple_instantiations: bool;
      (** Wether this interface has attribute __UNSAFE_AllowMultipleInstantiations. *)
  dc_sort_text: string option;
      (** The string provided by the __AutocompleteSortText attribute used for sorting
          autocomplete results. *)
}
[@@deriving show]

let get_elt_abstract elt = Typing_defs_flags.ClassElt.is_abstract elt.elt_flags

let get_elt_final elt = Typing_defs_flags.ClassElt.is_final elt.elt_flags

let get_elt_lsb elt = Typing_defs_flags.ClassElt.has_lsb elt.elt_flags

(** Whether a class element comes from a `require extends`. *)
let get_elt_synthesized elt =
  Typing_defs_flags.ClassElt.is_synthesized elt.elt_flags

let get_elt_xhp_attr elt = Typing_defs_flags.ClassElt.get_xhp_attr elt.elt_flags

let get_elt_needs_init elt = Typing_defs_flags.ClassElt.needs_init elt.elt_flags

let set_elt_synthesized elt =
  {
    elt with
    elt_flags = Typing_defs_flags.ClassElt.set_synthesized elt.elt_flags;
  }

let reset_elt_superfluous_override elt =
  {
    elt with
    elt_flags =
      Typing_defs_flags.ClassElt.reset_superfluous_override elt.elt_flags;
  }
