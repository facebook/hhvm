(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils

(* This exception will be raised when we want to ignore an error.
 * It is useful in the following case:
 * Somebody has code in strict mode, and is using a class A.
 * A has a naming error, and because of that, the type of A cannot
 * be found. When that happens, we want to stop trying to type-check
 * the class. Because all the errors that are going to be thrown
 * are just the result of the missing type definition A.
 *)
exception Ignore

module Reason = Typing_reason

type visibility =
  | Vpublic
  | Vprivate of string
  | Vprotected of string

type is_local_array = bool

(* All the possible types, reason is a trace of why a type
   was infered in a certain way.
*)
type ty = Reason.t * ty_
and ty_ =
  | Tany                          (* Unifies with anything *)
  | Tmixed                        (* ' with Nothing (mixed type) *)
  | Tarray        of is_local_array * ty option * ty option
  | Tgeneric      of string * ty option (* A generic type *)
  | Toption       of ty
  | Tprim         of Nast.tprim   (* All the primitive types: int, string, void, etc. *)
  | Tvar          of Ident.t      (* Type variable *)
  | Tfun          of fun_type
  (* Abstract types are "opaque", which means that they only unify with themselves.
   * However, it is possible to have a constraint that allows us to relax this.
   * Example:
   * newtype my_type as int = ...
   * Outside of the file where the type was defined, this translates to:
   * Tabstract ((pos, "my_type"), [], Some (Tprim Tint))
   * Which means that my_type is abstract, but is subtype of int as well.
   *)
  | Tabstract     of Nast.sid * ty list * ty option
  | Tapply        of Nast.sid * ty list (* Object type, ty list are the arguments *)
  | Ttuple        of ty list
  (* an anonymous function, the number of mandatory arguments,
   * the number of arguments in total, the identifier to type the body of the
   * function. (The actual closure is stored in Typing_env.env.genv.anons)
   *)
  | Tanon         of int * int * Ident.t
  (* This is in the case where we are looking for an intersection
   * basically without this type, we could never infer that an array
   * is an array of mixed for example.
   * if I write $x = 0; and then $x = true; well technically, it is
   * correct, there is a type that $x could have, and that's the type
   * mixed. Thanks to the inter type we can now make this happen.
   * On some operations, a type is allowed to "grow". That is,
   * we don't know what the type of that thing is (yet). So we
   * delay the moment where we are going to check on this type.
   * Once it is unified with a "true" type (one that cannot grow),
   * then we "fold" the intersection, that is we verify that every
   * member of the intersection is a subtype of that type. If
   * this is the case, Yay! we have found a common ancestor for the
   * intersection. However, note that once the type has been "folded",
   * it cannot grow anymore ...
   *)
  | Tunresolved        of ty list

  (* Tobject is an object type compatible with all objects. This type is also
   * compatible with some string operations (since a class might implement __toString), but
   * not with string type hints. In a similar way, Tobject is compatible with some
   * array operations (since a class might implement ArrayAccess), but not with
   * array type hints.
   *
   * Tobject is currently used to type code like: ../test/more_tests/return_unknown_class.php
   *)
  | Tobject
  | Tshape of ty SMap.t

(* The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and fun_type = {
  ft_pos       : Pos.t;
  ft_unsafe    : bool            ;
  ft_abstract  : bool            ;
  ft_arity_min : int             ;
  ft_arity_max : int             ;
  ft_tparams   : tparam list     ;
  ft_params    : fun_params      ;
  ft_ret       : ty              ;
}

and fun_params = (string option * ty) list

and class_elt = {
  ce_final      : bool;
  ce_override   : bool;
  ce_visibility : visibility;
  ce_type       : ty;
  (* classname where this elt originates from *)
  ce_origin     : string;
}

and class_type = {
  tc_need_init           : bool;
  (* Whether the typechecker knows of all (non-interface) ancestors
   * and thus known all accessible members of this class *)
  tc_members_fully_known : bool;
  tc_abstract            : bool;
  tc_final               : bool;
  (* When a class is abstract (or in a trait) the initialization of
   * a protected member can be delayed *)
  tc_members_init        : SSet.t;
  tc_kind                : Ast.class_kind;
  tc_name                : string    ;
  tc_tparams             : tparam list   ;
  tc_consts              : class_elt SMap.t;
  tc_cvars               : class_elt SMap.t;
  tc_scvars              : class_elt SMap.t;
  tc_methods             : class_elt SMap.t;
  tc_smethods            : class_elt SMap.t;
  tc_construct           : class_elt option ;
  (* This includes all the classes, interfaces and traits this class is
   * using. *)
  tc_ancestors           : ty SMap.t ;
  (* Ancestors that have to be checked when the class becomes
   * concrete. *)
  tc_ancestors_checked_when_concrete  : ty SMap.t;
  tc_req_ancestors       : SSet.t;
  tc_req_ancestors_extends         : SSet.t; (* the extends of req_ancestors *)
  tc_extends             : SSet.t;
  tc_user_attributes     : Ast.user_attribute SMap.t;
  (* These are approximations of what the class depends on,
   * So that we can prefetch these classes to heat-up the caches *)
  tc_prefetch_classes    : SSet.t;
  tc_prefetch_funs       : SSet.t;

  tc_mtime               : float;
}

and tparam = Ast.id * ty option

(* The identifier for this *)
let this = Ident.make "$this"

(*****************************************************************************)
(* Code filtering the private members (useful for inheritance) *)
(*****************************************************************************)

let filter_private x =
  SMap.fold begin fun name class_elt acc ->
    match class_elt.ce_visibility with
    | Vprivate _ -> acc
    | Vpublic | Vprotected _ -> SMap.add name class_elt acc
  end x SMap.empty

let chown_private owner =
  SMap.map begin fun class_elt ->
    match class_elt.ce_visibility with 
      | Vprivate _ -> {class_elt with ce_visibility = Vprivate owner}
      | _ -> class_elt end

let apply_fn_to_class_elts fn class_type = {
  class_type with
  tc_consts = fn class_type.tc_consts;
  tc_cvars = fn class_type.tc_cvars;
  tc_scvars = fn class_type.tc_scvars;
  tc_methods = fn class_type.tc_methods;
  tc_smethods = fn class_type.tc_smethods;
}

let filter_privates = apply_fn_to_class_elts filter_private
let chown_privates owner = apply_fn_to_class_elts (chown_private owner)

(*****************************************************************************)
(* Infer-type-at-point mode *)
(*****************************************************************************)

let (infer_target: (int * int) option ref) = ref None
let (infer_type: string option ref) = ref None
let (infer_pos: Pos.t option ref) = ref None

(*****************************************************************************)
(* Accumulate method calls mode *)
(*****************************************************************************)

let accumulate_method_calls = ref false
let (accumulate_method_calls_result: (Pos.t * string) list ref) = ref []

(*****************************************************************************)
(* Suggest mode *)
(*****************************************************************************)

(* Set to true when we are trying to infer the missing type hints. *)
let is_suggest_mode = ref false

(*****************************************************************************)
(* Print types mode *)
(*****************************************************************************)
let accumulate_types = ref false
let (type_acc: (Pos.t * ty) list ref) = ref []
