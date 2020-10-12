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

module PosSet = Set.Make (Pos)

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
  | Ppurpose of
      (PosSet.t[@equal (fun _ _ -> true)] [@compare (fun _ _ -> 0)]) * purpose
  (* Bottom policy; public *)
  | Pbot of (PosSet.t[@equal (fun _ _ -> true)] [@compare (fun _ _ -> 0)])
  (* Top policy; private *)
  | Ptop of (PosSet.t[@equal (fun _ _ -> true)] [@compare (fun _ _ -> 0)])
[@@deriving eq, ord]

let pbot = Pbot PosSet.empty

let pos_set_of_policy = function
  | Ppurpose (poss, _)
  | Ptop poss
  | Pbot poss ->
    poss
  | _ -> PosSet.empty

let set_pos_set_of_policy pos = function
  | Ppurpose (_, name) -> Ppurpose (pos, name)
  | Ptop _ -> Ptop pos
  | Pbot _ -> Pbot pos
  | pol -> pol

(* Two kinds of quantification in constraints, universal and
   existential *)
type quant =
  | Qforall
  | Qexists

type class_ = {
  c_name: string;
  c_self: policy;
  c_lump: policy;
}

type array_kind =
  | Avec
  | Adict
  | Akeyset
[@@deriving eq]

(* Types with policies *)
type ptype =
  | Tprim of policy
  | Tgeneric of policy
  | Ttuple of ptype list
  | Tunion of ptype list
  | Tinter of ptype list
  | Tclass of class_
  | Tfun of fun_
  | Tcow_array of cow_array

(* Copy-on-write indexed collection used for Hack arrays i.e. vec, dict, and
   keyset *)
and cow_array = {
  a_kind: array_kind;
  a_key: ptype;
  a_value: ptype;
  a_length: policy;
}

and fun_ = {
  (* The PC guards a function's effects *)
  f_pc: policy;
  (* Policy that the function's computational contents depend on *)
  f_self: policy;
  f_args: ptype list;
  f_ret: ptype;
  f_exn: ptype;
}

type fun_proto = {
  fp_name: string;
  fp_this: ptype option;
  fp_type: fun_;
}

(* A flow between two policies with positions justifying it *)
type pos_flow = PosSet.t * policy * policy

(* Flow constraints with quantifiers and implication *)
type prop =
  | Ctrue
  | Cquant of quant * int * prop
  (* if policy <= purpose then prop0 else prop1 *)
  | Ccond of (Pos.t * policy * purpose) * prop * prop
  | Cconj of prop * prop
  | Cflow of pos_flow
  (* holes are introduced by calls to functions for which
     we do not have a flow type at hand *)
  | Chole of (Pos.t * fun_proto)

let unique_pos_of_prop =
  let is_real pos = not @@ Pos.equal pos Pos.none in
  function
  | Cflow (posset, _, _) ->
    begin
      match PosSet.elements posset with
      | [pos] when is_real pos -> Some pos
      | _ -> None
    end
  | Ccond ((pos, _, _), _, _) when is_real pos -> Some pos
  | Chole (pos, _) when not @@ is_real pos -> Some pos
  | _ -> None

type fun_scheme = Fscheme of Scope.t * fun_proto * prop

module Flow = struct
  type t = policy * policy

  let compare (a, b) (c, d) =
    match compare_policy a c with
    | 0 -> compare_policy b d
    | x -> x
end

module FlowSet = Set.Make (Flow)

type security_lattice = FlowSet.t

module Policy = struct
  type t = policy

  let compare = compare_policy
end

module PSet = Set.Make (Policy)

module Var = struct
  type t = string * Ifc_scope.t

  let compare = compare
end

module VarSet = Set.Make (Var)

(* A cont represents the typing environment for one
   outcome (fallthrough, break, throw, ...) of a
   statement *)
type cont = {
  k_vars: ptype LMap.t;
  (* Policy tracking the dependencies of the current
     outcome. NB: only dependencies *local* to the
     function are tracked here (i.e., the function's
     pc policy is not included) *)
  k_pc: PSet.t;
}

type policied_property = {
  pp_pos: Pos.t;
  pp_name: string;
  pp_purpose: purpose;
  (* Visibility is not needed beyond the decl phase, but OCaml makes
   * it difficult to map between collections, so it is carried to the analysis. *)
  pp_visibility: Aast.visibility;
}

type class_decl = {
  (* the list of policied properties in the class *)
  cd_policied_properties: policied_property list;
}

type magic_decl = {
  ma_class_decl: class_decl;
  ma_tparams: Type.locl_ty list;
  ma_variances: Ast_defs.variance list;
}

type fun_decl_kind =
  | FDGovernedBy of policy option
  | FDInferFlows

type arg_kind =
  | AKDefault
  | AKExternal of Pos.t
  | AKCallable of Pos.t

type fun_decl = {
  fd_kind: fun_decl_kind;
  fd_args: arg_kind list;
}

type decl_env = {
  (* policy decls for classes indexed by class name *)
  de_class: class_decl SMap.t;
  (* policy decls for functions indexed by function name *)
  de_fun: fun_decl SMap.t;
}

(* Mode of operation.
 * The constructors should be topologically sorted with respect to the
 * dependency partial order between different modes. *)
type mode =
  (* Constructs the security lattice. *)
  | Mlattice
  (* Performs declaration analysis *)
  | Mdecl
  (* Analyses function/method bodies for flux constraints *)
  | Manalyse
  (* Invokes the constraint solver simplifying constraints *)
  | Msolve
  (* Checks simplified constraints against a security lattice *)
  | Mcheck
  (* Run and print everything for debugging *)
  | Mdebug
[@@deriving eq]

(* Structured/parsed/sanitised options. *)
type options = {
  (* Mode of operation that determines how much of the analysis is executed
   * and what to printout. *)
  opt_mode: mode;
  (* Security lattice to check results against. *)
  opt_security_lattice: security_lattice;
}

(* Read-only environment information managed following a stack discipline
   when walking the Hack syntax *)
type 'ptype renv_ = {
  (* during flow inference, types are always given relative to a scope. *)
  re_scope: Scope.t;
  (* hash table keeping track of counters to generate variable names *)
  re_pvar_counters: (string, int ref) Hashtbl.t;
  (* extended decls for IFC *)
  re_decl: decl_env;
  (* Hack type environment *)
  re_tenv: Tast.saved_env;
  (* policy type of $this *)
  re_this: 'ptype option;
  (* return type of the function being checked *)
  re_ret: 'ptype;
  (* the program counter policy of the current function *)
  re_gpc: policy;
  (* Exception thrown from the callable *)
  re_exn: 'ptype;
}

type proto_renv = unit renv_

type renv = ptype renv_

(* The analysis result for a callable *)
type callable_result = {
  (* Position of the callable the result pertains to *)
  res_span: Pos.t;
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
  (* Entailment based on the function's assumed prototype *)
  res_entailment: prop -> pos_flow list;
}

type adjustment =
  | Astrengthen
  | Aweaken

type call_type =
  | Cglobal of string
  | Clocal of fun_
