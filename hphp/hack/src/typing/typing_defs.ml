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
  (* an anonymous function, the fun arity, the identifier to
   * type the body of the function. (The actual closure is stored in
   * Typing_env.env.genv.anons) *)
  | Tanon         of fun_arity * Ident.t
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
   * Tobject is currently used to type code like: ../test/typecheck/return_unknown_class.php
   *)
  | Tobject
  | Tshape of ty Nast.ShapeMap.t

(* The type of a function AND a method.
 * A function has a min and max arity because of optional arguments *)
and fun_type = {
  ft_pos       : Pos.t;
  ft_unsafe    : bool            ;
  ft_abstract  : bool            ;
  ft_arity     : fun_arity       ;
  ft_tparams   : tparam list     ;
  ft_params    : fun_params      ;
  ft_ret       : ty              ;
}

(* Arity informaton for a fun_type; indicating the minimum number of
 * args expected by the function and the maximum number of args for
 * standard, non-variadic functions or the type of variadic argument taken *)
and fun_arity =
  | Fstandard of int * int (* min ; max *)
  (* PHP5.6-style ...$args finishes the func declaration *)
  | Fvariadic of int * fun_param (* min ; variadic param type *)
  (* HH-style ... anonymous variadic arg; body presumably uses func_get_args *)
  | Fellipsis of int       (* min *)

and fun_param = (string option * ty)

and fun_params = fun_param list

and class_elt = {
  ce_final       : bool;
  ce_override    : bool;
  (* true if this elt arose from require-extends or other mechnaisms
     of hack "synthesizing" methods that were not written by the
     programmer. The eventual purpose of this is to make sure that
     elts that *are* written by the programmer take precedence over
     synthesized elts. *)
  ce_synthesized : bool;
  ce_visibility  : visibility;
  ce_type        : ty;
  (* classname where this elt originates from *)
  ce_origin      : string;
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
  tc_pos                 : Pos.t ;
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
  tc_req_ancestors       : ty SMap.t;
  tc_req_ancestors_extends : SSet.t; (* the extends of req_ancestors *)
  tc_extends             : SSet.t;
  tc_user_attributes     : Ast.user_attribute SMap.t;
}

and tparam = Ast.id * ty option

(* The identifier for this *)
let this = Ident.make "$this"

let arity_min ft_arity : int = match ft_arity with
  | Fstandard (min, _) | Fvariadic (min, _) | Fellipsis min -> min

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
