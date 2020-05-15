(* Copyright (c) 2020, Facebook, Inc.
   All rights reserved. *)
module KMap = Typing_continuations.Map
module LMap = Local_id.Map
module Scope = Ifc_scope

(* Most types should live here. *)

type purpose = string [@@deriving ord, eq, show]

(* A policy variable *)
type policy_var = int [@@deriving ord, eq, show]

(* In policies, variables are handled using a locally-nameless
   representation. This means that variables bound in a
   constraints use de Bruijn indices while free variables use
   a name. The scope in a Pvar is used to store inline the
   creation point of a purpose variable. *)
type policy =
  (* Bound variable; represented with a de Bruijn index *)
  | Pbound_var of int
  (* Free variable; relative to a scope *)
  | Pfree_var of policy_var * Ifc_scope.t
  (* A policy allowing use for a single purpose *)
  | Ppurpose of purpose
  (* Bottom policy; public *)
  | Pbot
  (* Top policy; private *)
  | Ptop
[@@deriving ord, eq, show]

(* Two kinds of quantification in constraints, universal and
   existential *)
type quant =
  | Qforall
  | Qexists

(* Flow constraints with quantifiers and implication *)
type prop =
  | Ctrue
  | Cquant of quant * int * prop
  (* if policy <= purpose then prop0 else prop1 *)
  | Ccond of (policy * purpose) * prop * prop
  | Cconj of prop * prop
  | Cflow of (policy * policy)

(* Types with policies *)
type ptype =
  | Tprim of policy
  | Ttuple of ptype list
  | Tunion of ptype list
  | Tinter of ptype list

type local_env = { le_vars: ptype LMap.t }

(* The environment is a mix of global immutable
   information (e.g., e_ret) and mutable data that
   has to be threaded through (e.g., e_acc) *)
type env = {
  e_tenv: Tast.saved_env;
  (* Constraints accumulator. *)
  e_acc: prop list;
  (* Maps storing the type of local variables; one
     per continuation, for flow-sensitive typing.  *)
  e_cont: local_env KMap.t;
  (* Return type of the function being checked. *)
  e_ret: ptype;
}

(* We aggregate in renv the information managed following
   a stack discipline when walking the Hack syntax (it is
   a "read-only" env) *)
type renv = {
  (* During flow inference, types are always given
     relative to a scope. *)
  s_scope: Scope.t;
  (* Policy tracking local effects, these effects
     are not observable outside the current function.
     Assignments to local variables fall into this
     category. *)
  s_lpc: policy list;
  (* Policy tracking effects with global visibility,
     like assignments to fields of an argument. *)
  s_gpc: policy list;
}
