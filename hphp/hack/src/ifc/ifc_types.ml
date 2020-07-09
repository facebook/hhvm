(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module KMap = Typing_continuations.Map
module LMap = Local_id.Map
module Scope = Ifc_scope
module Type = Typing_defs

(* Most types should live here. *)

type purpose = string [@@deriving ord, eq, show]

(* A policy variable *)
type policy_var = string [@@deriving ord, eq, show]

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

(* Types with policies *)
type ptype =
  | Tprim of policy
  | Ttuple of ptype list
  | Tunion of ptype list
  | Tinter of ptype list
  | Tclass of class_

and class_ = {
  c_name: string;
  c_self: policy;
  c_lump: policy;
  c_property_map: ptype Lazy.t SMap.t;
  c_tparams: (ptype * Ast_defs.variance) list;
}

type fun_proto = {
  fp_name: string;
  fp_pc: policy;
  fp_this: ptype option;
  fp_args: ptype list;
  fp_ret: ptype;
}

(* Flow constraints with quantifiers and implication *)
type prop =
  | Ctrue
  | Cquant of quant * int * prop
  (* if policy <= purpose then prop0 else prop1 *)
  | Ccond of (policy * purpose) * prop * prop
  | Cconj of prop * prop
  | Cflow of (policy * policy)
  (* holes are introduced by calls to functions for which
     we do not have a flow type at hand *)
  | Chole of fun_proto

module Flow = struct
  type t = policy * policy

  let compare = compare
end

module FlowSet = Set.Make (Flow)

type security_lattice = FlowSet.t

module Policy = struct
  type t = policy

  let compare = compare
end

module PCSet = Set.Make (Policy)

type program_counter = PCSet.t

type local_env = {
  le_vars: ptype LMap.t;
  (* Policy tracking local effects, these effects
     are not observable outside the current function.
     Assignments to local variables fall into this
     category. *)
  le_pc: program_counter;
}

(* The environment is mutable data that
   has to be threaded through *)
type env = {
  (* Constraints accumulator. *)
  e_acc: prop list;
  (* Maps storing the type of local variables; one
     per continuation, for flow-sensitive typing.  *)
  e_cont: local_env KMap.t;
  (* Callable on which the current function depends. *)
  e_deps: SSet.t;
}

type policied_property = {
  pp_name: string;
  pp_type: Type.locl_ty;
  pp_purpose: purpose option;
}

type class_decl = {
  (* the list of policied properties in the class *)
  cd_policied_properties: policied_property list;
  cd_tparam_variance: Ast_defs.variance list;
}

type fun_decl_kind =
  | FDPublic
  | FDCIPP
  | FDInferFlows

type fun_decl = { fd_kind: fun_decl_kind }

type decl_env = {
  (* policy decls for classes indexed by class name *)
  de_class: class_decl SMap.t;
  (* policy decls for functions indexed by function name *)
  de_fun: fun_decl SMap.t;
}

(* Read-only environment containing just enough information to compute flow
   types from Hack types *)
type proto_renv = {
  (* during flow inference, types are always given relative to a scope. *)
  pre_scope: Scope.t;
  (* hash table keeping track of counters to generate variable names *)
  pre_pvar_counters: (string, int ref) Hashtbl.t;
  (* extended decls for IFC *)
  pre_decl: decl_env;
  (* Hack type environment *)
  pre_tenv: Tast.saved_env;
}

(* Read-only environment information managed following a stack discipline
   when walking the Hack syntax *)
type renv = {
  (* Section of renv needed to initialize full renv *)
  re_proto: proto_renv;
  (* Policy type of $this. *)
  re_this: ptype option;
  (* Return type of the function being checked. *)
  re_ret: ptype;
  (* The initial program counter for the function *)
  re_gpc: policy;
}

(* The analysis result for a callable *)
type callable_result = {
  (* The callable signature, with flow types *)
  res_proto: fun_proto;
  (* The scope of the free policy variables in res_proto
     and res_constraint *)
  res_scope: Scope.t;
  (* Constraints abstracting the callable body; the
     constrain the policies appearing in res_proto *)
  res_constraint: prop;
  (* The set of callable that appear in holes of
     res_constraint *)
  res_deps: SSet.t;
}

type options = {
  (* Verbosity level for the IFC output.
   * Each level includes everything below it as well.
   * 0: What the user is meant to see
   * 1: Details constraints after solving
   * 2: Results of IFC analysis on functions/methods
   * 3: Declaration analysis results
   *)
  verbosity: int;
  (* String representation of a security lattice. *)
  security_lattice: string;
}
