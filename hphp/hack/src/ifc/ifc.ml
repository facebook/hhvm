(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Ifc_types
module Decl = Ifc_decl
module Env = Ifc_env
module Logic = Ifc_logic
module Pp = Ifc_pretty
module Lift = Ifc_lift
module Solver = Ifc_solver
module Utils = Ifc_utils
module A = Aast
module T = Typing_defs
module L = Logic.Infix
module K = Typing_cont_key
module LSet = Local_id.Set
module TClass = Decl_provider.Class

exception FlowInference of string

let should_print ~user_mode ~phase =
  equal_mode user_mode Mdebug || equal_mode user_mode phase

let fail fmt = Format.kasprintf (fun s -> raise (FlowInference s)) fmt

(* Returns the policies appearing in a type split in three sets
   of occurrences (covariant, invariant, contravariant) *)
let rec policy_occurrences pty =
  let emp = PSet.empty in
  let pol = PSet.singleton in
  let on_list =
    List.fold ~init:(emp, emp, emp) ~f:(fun (s1, s2, s3) (t1, t2, t3) ->
        (PSet.union s1 t1, PSet.union s2 t2, PSet.union s3 t3))
  in
  match pty with
  | Tprim p -> (pol p, emp, emp)
  | Tgeneric p -> (emp, pol p, emp)
  | Tinter tl
  | Tunion tl
  | Ttuple tl ->
    on_list (List.map ~f:policy_occurrences tl)
  | Tclass cls -> (pol cls.c_self, pol cls.c_lump, emp)
  | Tfun { f_pc; f_self; f_args; f_ret; f_exn } ->
    let swap_policy_occurrences t =
      let (cov, inv, cnt) = policy_occurrences t in
      (cnt, inv, cov)
    in
    on_list
      ( (pol f_self, emp, pol f_pc)
      :: policy_occurrences f_ret
      :: policy_occurrences f_exn
      :: List.map ~f:swap_policy_occurrences f_args )
  | Tcow_array { a_key; a_value; a_length; _ } ->
    on_list
      [
        (pol a_length, emp, emp);
        policy_occurrences a_key;
        policy_occurrences a_value;
      ]

exception SubtypeFailure of string * ptype * ptype

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  let err msg = raise (SubtypeFailure (msg, t1, t2)) in
  let rec first_ok ~f msg = function
    | [] -> err msg
    | x :: l ->
      begin
        try f x with SubtypeFailure (msg, _, _) -> first_ok ~f msg l
      end
  in
  match (t1, t2) with
  | (Tprim p1, Tprim p2) -> L.(p1 < p2) ~pos acc
  | (Tgeneric p1, _) ->
    let (cv, inv, cn) = policy_occurrences t2 in
    L.(
      [p1] <* PSet.elements cv
      && PSet.elements cn <* [p1]
      && [p1] =* PSet.elements inv)
      ~pos
      acc
  | (_, Tgeneric p2) ->
    let (cv, inv, cn) = policy_occurrences t1 in
    L.(
      PSet.elements cv <* [p2]
      && [p2] <* PSet.elements cn
      && [p2] =* PSet.elements inv)
      ~pos
      acc
  | (Ttuple tl1, Ttuple tl2) ->
    (match List.zip tl1 tl2 with
    | Some zip ->
      List.fold zip ~init:acc ~f:(fun acc (t1, t2) -> subtype t1 t2 acc)
    | None -> err "incompatible tuple types")
  | (Tclass cl1, Tclass cl2) ->
    (* We do not attempt to replicate the work Hack did and instead
       only act on policies. A bit of precision could be gained when
       dealing with disjunctive subtyping queries if we realize that,
       for example, cl1 and cl2 are incompatible; but let's keep it
       simple for now. *)
    L.(cl1.c_lump = cl2.c_lump && cl1.c_self < cl2.c_self) ~pos acc
  | (Tfun f1, Tfun f2) ->
    let zipped_args =
      match List.zip f1.f_args f2.f_args with
      | Some zip -> zip
      | None -> err "functions have different number of arguments"
    in
    (* Contravariant in argument types *)
    List.fold ~f:(fun acc (t1, t2) -> subtype t2 t1 acc) ~init:acc zipped_args
    (* Contravariant in PC *)
    |> L.(f2.f_pc < f1.f_pc) ~pos
    (* Covariant in its own identity *)
    |> L.(f1.f_self < f2.f_self) ~pos
    (* Covariant in return type and exceptions *)
    |> subtype f1.f_ret f2.f_ret
    |> subtype f1.f_exn f2.f_exn
  | (Tcow_array arr1, Tcow_array arr2) ->
    acc
    |> subtype arr1.a_key arr2.a_key
    |> subtype arr1.a_value arr2.a_value
    |> L.(arr1.a_length < arr2.a_length) ~pos
  | (Tcow_array _, Tclass cl) ->
    let (cov, inv, _con) = policy_occurrences t1 in
    acc
    |> L.(PSet.elements cov <* [cl.c_self]) ~pos
    |> L.(PSet.elements inv =* [cl.c_lump]) ~pos
  | (Tunion tl, _) ->
    List.fold tl ~init:acc ~f:(fun acc t1 -> subtype t1 t2 acc)
  | (_, Tinter tl) ->
    List.fold tl ~init:acc ~f:(fun acc t2 -> subtype t1 t2 acc)
  | (_, Tunion tl) ->
    (* It is not complete to try all the RHS elements in sequence
       but that's how typing_subtype.ml works today *)
    first_ok "nothing" tl ~f:(fun t2 -> subtype t1 t2 acc)
  | (Tinter tl, _) ->
    (* Same remark as for (_ <: Tunion _) *)
    first_ok "mixed" tl ~f:(fun t1 -> subtype t1 t2 acc)
  | _ -> err "unhandled subtyping query"

(* A constraint accumulator that registers that t1 = t2 *)
let equivalent ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  subtype t1 t2 (subtype t2 t1 acc)

(* Overwrite subtype and equivalent catching the SubtypeFailure
   exception *)
let (subtype, equivalent) =
  let wrap f ~pos t1 t2 acc =
    try f ~pos t1 t2 acc
    with SubtypeFailure (msg, tsub, tsup) ->
      fail "subtype: %s (%a <: %a)" msg Pp.ptype tsub Pp.ptype tsup
  in
  (wrap subtype, wrap equivalent)

(* Returns the list of free policy variables in a type *)
let free_pvars pty =
  let (cov, inv, cnt) = policy_occurrences pty in
  let all_vars =
    PSet.fold (fun pol vset ->
        match pol with
        | Pfree_var (v, s) -> VarSet.add (v, s) vset
        | _ -> vset)
  in
  VarSet.empty |> all_vars cov |> all_vars inv |> all_vars cnt

(* Type refinements happen when the typechecker realizes that a dynamic
   check ensures a code path is taken only when a local has a given type.
   The refined type Tref is in a subtyping relation with the original
   type Tori (Tref <: Tori).

   Refinement gets tricky when the reference type and original type
   are parameterized (either with type parameters, or with policy
   parameters). In plain Hack, type parameters are not constrained by
   dynamic checks (e.g., one can only say ($x is vec<_>)), so we have
   to ensure that the code type-checked under a refinement is valid
   regardless of the parameter's value. In practice, the Hack
   typechecker introduces a generic type on the spot (i.e., a rigid
   type variable); it is however improperly scoped, and that leads
   to soundness bugs.

   In Hack IFC, we will not bother too much with type parameters until
   Hack is fixed. However, we will properly handle policy variables.
   Doing so requires generating universally quantified policy variables
   in the constraint. Let's consider an example.

     $x: mixed<=pm>
     if ($x is A<+pcls, =pfld>) {
        /* some code */
     }

   The variances were marked with + and = for covariant and invariant,
   respectively. Inside the if statement, the type for $x is refined
   (from mixed). Hack IFC mints a new type A<pcls, pfld> for $x then
   generates the constraint Cif for the body of the conditional, the
   complete constraint is then assembled as such:

     (forall pcls pfld. (A<pcls, pfld> <: mixed<pm>) ==> Cif)

   which, expanding the action of subtyping, is the same as

     (forall pcls pfld. (pcls < pm && pfld = pm) ==> Cif)

   Note how pcls and pfld are local to the part of the constraint
   that deals with the body of the if conditional. This is how Hack
   IFC controls the (potentially unsound) escaping of the fresh
   parameters induced by refinements.

   It is clear that we can get rid of pfld, and replace it with pm
   everywhere it appears in Cif. But what about pcls? The key to
   eliminating it early is that pcls appears covariantly in the
   type of a live value accessible via $x. Consequently, all the
   constraints in Cif involving pcls must be of the form (pcls < X).
   Because of this observation, it is sound (and complete) to replace
   pcls in Cif by pm. In contrast, if there had been constraints of
   the form (X < pcls) in Cif, we'd have had to replace pcls with bot
   in those.
*)
let refine renv tyori (pos, ltyref) =
  let ref_scope = Scope.alloc () in
  let tyref = Lift.ty { renv with re_scope = ref_scope } ltyref in
  let acc = subtype ~pos tyref tyori [] in
  (* we know the three sets are disjoint, because Lift.ty above
     will not reuse the same variable multiple times *)
  let (cov, _inv, cnt) = policy_occurrences tyref in
  (* analyze the result of the subtyping call; we build a map
     that associates a bound to each policy variable in tyref.
     Depending on whether the refined variable is in cov/inv/cnt
     the bound is to be interpreted as an upper bound, an
     equality, or a lower bound.
  *)
  let rec collect vmap acc =
    let set_bound pref pori vmap =
      (* only store bounds that apply to the refined type and
         are from the origin type *)
      match pref with
      | Pfree_var (var, s) when Scope.equal s ref_scope ->
        begin
          match pori with
          | Pfree_var (_, s) when Scope.equal s ref_scope -> vmap
          | _ -> SMap.add var pori vmap
        end
      | _ -> vmap
    in
    match acc with
    | [] -> vmap
    | Ctrue :: acc -> collect vmap acc
    | Cconj (Cflow (_, p1, p2), Cflow (_, p3, p4)) :: acc
      when equal_policy p1 p4 && equal_policy p2 p3 ->
      collect (set_bound p1 p2 @@ set_bound p2 p1 @@ vmap) acc
    | Cflow (_, p1, p2) :: acc when PSet.mem p1 cov ->
      collect (set_bound p1 p2 vmap) acc
    | Cflow (_, p1, p2) :: acc when PSet.mem p2 cnt ->
      collect (set_bound p2 p1 vmap) acc
    | c :: acc ->
      (* soundness is not compromised by ignoring the constraint,
         however, for debugging purposes it is good to know this
         happened *)
      Format.eprintf "Duh?! unhandled subtype constraint: %a" Pp.prop c;
      collect vmap acc
  in
  let vmap = collect SMap.empty acc in
  (* replace policy variables in tyref with their bounds *)
  let rec replace_vars ty =
    let on_policy = function
      | Pfree_var (var, s) ->
        (* sanity check *)
        assert (Scope.equal s ref_scope);
        begin
          match SMap.find_opt var vmap with
          | None ->
            (* this is what happens when the refined type has a
               policy variable that is not related to anything in
               the original type; it happens, for instance, if the
               refined type is a class C<...> and the original
               type is a policy-free empty intersection (Tinter[])

               in this case, we cannot get rid of the universal
               quantification; for now we simply fail *)
            fail "univ refinement (%a ~> %a)" Pp.ptype tyori Pp.ptype tyref
          | Some bnd -> bnd
        end
      | pol -> pol
    in
    Ifc_mapper.ptype replace_vars on_policy ty
  in
  (* finally, we have the refined type!
     some examples of refine's behavior in common cases:
       original             refined locl     result
       (int<pi>|C<pc,pl>)   int          --> int<pi>
       (int<pi>|I<pc,pl>)   C            --> C<pc,pl>  (C implements I)
       mixed<pm>            C            --> C<pm,pm>  (policied mixed is TODO)
  *)
  replace_vars tyref

(* Generates a fresh sub/super policy of the argument polic *)
let adjust_policy ?(prefix = "weak") ~pos ~adjustment renv env policy =
  match (adjustment, policy) with
  | (Astrengthen, Pbot _)
  | (Aweaken, Ptop _) ->
    (env, policy)
  | (Astrengthen, _) ->
    let new_policy = Env.new_policy_var renv prefix in
    let prop = L.(new_policy < policy) ~pos in
    (Env.acc env prop, new_policy)
  | (Aweaken, _) ->
    let new_policy = Env.new_policy_var renv prefix in
    let prop = L.(policy < new_policy) ~pos in
    (Env.acc env prop, new_policy)

(* Generates a fresh sub/super ptype of the argument ptype *)
let adjust_ptype ?prefix ~pos ~adjustment renv env ty =
  let adjust_policy = adjust_policy ?prefix ~pos renv in
  let flip = function
    | Astrengthen -> Aweaken
    | Aweaken -> Astrengthen
  in
  let rec freshen adjustment env ty =
    let freshen_cov = freshen adjustment in
    let freshen_con = freshen @@ flip adjustment in
    let freshen_pol_cov = adjust_policy ~adjustment in
    let freshen_pol_con = adjust_policy ~adjustment:(flip adjustment) in
    let simple_freshen env f p =
      let (env, p) = freshen_pol_cov env p in
      (env, f p)
    in
    let on_list env mk tl =
      let (env, tl') = List.map_env env tl ~f:freshen_cov in
      (env, mk tl')
    in
    match ty with
    | Tprim p -> simple_freshen env (fun p -> Tprim p) p
    | Tgeneric p -> simple_freshen env (fun p -> Tgeneric p) p
    | Ttuple tl -> on_list env (fun l -> Ttuple l) tl
    | Tunion tl -> on_list env (fun l -> Tunion l) tl
    | Tinter tl -> on_list env (fun l -> Tinter l) tl
    | Tclass class_ ->
      let (env, super_pol) = freshen_pol_cov env class_.c_self in
      (env, Tclass { class_ with c_self = super_pol })
    | Tfun fun_ ->
      let (env, f_pc) = freshen_pol_con env fun_.f_pc in
      let (env, f_self) = freshen_pol_cov env fun_.f_self in
      let (env, f_args) = List.map_env env ~f:freshen_con fun_.f_args in
      let (env, f_ret) = freshen_cov env fun_.f_ret in
      let (env, f_exn) = freshen_cov env fun_.f_exn in
      (env, Tfun { f_pc; f_self; f_args; f_ret; f_exn })
    | Tcow_array arr ->
      let (env, a_key) = freshen_cov env arr.a_key in
      let (env, a_value) = freshen_cov env arr.a_value in
      let (env, a_length) = freshen_pol_cov env arr.a_length in
      (env, Tcow_array { a_kind = arr.a_kind; a_key; a_value; a_length })
  in
  freshen adjustment env ty

(* Returns the set of policies, to be understood as a join,
   that governs an object with the argument type *)
let rec object_policy = function
  | Tprim pol
  | Tgeneric pol
  | Tclass { c_self = pol; _ }
  | Tfun { f_self = pol; _ } ->
    PSet.singleton pol
  | Tinter tl
  | Tunion tl
  | Ttuple tl ->
    let f set t = PSet.union (object_policy t) set in
    List.fold tl ~init:PSet.empty ~f
  | Tcow_array { a_key; a_value; a_length; _ } ->
    PSet.union (object_policy a_key) (object_policy a_value)
    |> PSet.add a_length

let add_dependencies pl t = L.(pl <* PSet.elements (object_policy t))

(* TODO: make a list version of it *)
let policy_join ?(prefix = "join") renv p1 p2 =
  let id ~pos:_ acc = acc in
  match Logic.policy_join p1 p2 with
  | Some p -> (id, p)
  | None when equal_policy p1 p2 -> (id, p1)
  | _ ->
    let pv = Env.new_policy_var renv prefix in
    (L.(p1 < pv && p2 < pv), pv)

let policy_join_env ?prefix ~pos renv env p1 p2 =
  let (facc, p) = policy_join ?prefix renv p1 p2 in
  (Env.acc env (facc ~pos), p)

let get_local_type ~pos env lid =
  match Env.get_local_type env lid with
  | None ->
    let name = Local_id.get_name lid in
    (* TODO: deal with co-effect thingies *)
    ( if not (String.equal (String.sub name ~pos:0 ~len:2) "$#") then
      let msg = "local " ^ name ^ " missing from env" in
      Errors.unknown_information_flow pos msg );
    None
  | pty_opt -> pty_opt

(* Uses a Hack-inferred type to update the flow type of a local
   variable *)
let refresh_local_type ?(force = false) ~pos renv env lid lty =
  let is_simple pty =
    match pty with
    | Tunion _ -> false
    | _ -> true
  in
  let pty =
    match get_local_type ~pos env lid with
    | Some pty -> pty
    | None -> Lift.ty renv lty
  in
  if (not force) && is_simple pty then
    (* if the type is already simple, do not refresh it with
       what Hack found *)
    (env, pty)
  else
    let prefix = Local_id.to_string lid in
    let new_pty = Lift.ty renv lty ~prefix in
    let env = Env.acc env (subtype ~pos pty new_pty) in
    let env = Env.set_local_type env lid new_pty in
    (env, new_pty)

let lift_params renv =
  let lift_param p =
    let prefix = p.A.param_name in
    let pty = Lift.ty ~prefix renv (fst p.A.param_type_hint) in
    let lid = Local_id.make_unscoped p.A.param_name in
    (lid, pty)
  in
  List.map ~f:lift_param

let set_local_types env =
  List.fold ~init:env ~f:(fun env (lid, pty) -> Env.set_local_type env lid pty)

let binop ~pos renv env ty1 ty2 =
  match (ty1, ty2) with
  | (Tprim p1, Tprim p2) ->
    let (env, pj) = policy_join_env ~pos renv env ~prefix:"bop" p1 p2 in
    (env, Tprim pj)
  | _ -> fail "unexpected Binop types"

let receiver_of_obj_get obj_ptype property =
  match obj_ptype with
  | Tclass class_ -> class_
  | _ -> fail "couldn't find a class for the property '%s'" property

(* We generate a ptype out of the property type and fill it with either the
 * purpose of property or the lump policy of some object root. *)
let property_ptype renv obj_ptype property property_ty =
  let class_ = receiver_of_obj_get obj_ptype property in
  let prop_pol =
    match Decl.property_policy renv.re_decl class_.c_name property with
    | Some policy -> policy
    | None -> class_.c_lump
  in
  Lift.ty ~prefix:property ~lump:prop_pol renv property_ty

(* may_throw registers that the expression being checked may
   throw an exception of type exn_ty; the throwing is conditioned
   on data whose policy is in pc_deps.
   If pc_deps is empty an exception is unconditionally thrown *)
let may_throw ~pos renv env pc_deps exn_ty =
  let env = Env.throw env pc_deps in
  let deps = PSet.elements (Env.get_lpc env) in
  let env = Env.acc env (subtype exn_ty renv.re_exn ~pos) in
  let env = Env.acc env (add_dependencies deps renv.re_exn ~pos) in
  env

let call ~pos renv env call_type that_pty_opt args_pty ret_ty =
  let name =
    match call_type with
    | Cglobal callable_name -> callable_name
    | Clocal _ -> "anonymous"
  in
  let callee = Env.new_policy_var renv (name ^ "_self") in
  let ret_pty = Lift.ty ~prefix:(name ^ "_ret") renv ret_ty in
  let env = Env.acc env (add_dependencies ~pos [callee] ret_pty) in
  let (env, callee_exn) =
    let exn = Lift.class_ty ~prefix:(name ^ "_exn") renv Decl.exception_id in
    let env = may_throw ~pos renv env (object_policy exn) exn in
    (env, exn)
  in
  (* The PC of the function being called depends on the join of the current
   * PC dependencies, as well as the function's own self policy *)
  let (env, pc_joined) =
    let join pc' (env, pc) =
      policy_join_env ~pos renv env ~prefix:"pcjoin" pc pc'
    in
    PSet.fold join (Env.get_gpc renv env) (env, callee)
  in
  let hole_ty =
    {
      f_pc = pc_joined;
      f_self = callee;
      f_args = args_pty;
      f_ret = ret_pty;
      f_exn = callee_exn;
    }
  in
  match call_type with
  | Clocal fty ->
    let env = Env.acc env @@ subtype ~pos (Tfun fty) (Tfun hole_ty) in
    (env, ret_pty)
  | Cglobal callable_name ->
    let fp =
      { fp_name = callable_name; fp_this = that_pty_opt; fp_type = hole_ty }
    in
    let (env, call_constraint) =
      match SMap.find_opt callable_name renv.re_decl.de_fun with
      | Some { fd_kind = FDGovernedBy policy; fd_args } ->
        let scheme = Decl.make_callable_scheme renv policy fp fd_args in
        let prop =
          (* because cipp_scheme is created after fp they cannot
             mismatch and call_constraint will not fail *)
          Option.value_exn (Solver.call_constraint ~subtype ~pos fp scheme)
        in
        (env, prop)
      | Some { fd_kind = FDInferFlows; _ } ->
        let env = Env.add_dep env callable_name in
        (env, Chole (pos, fp))
      | None -> fail "unknown function '%s'" callable_name
    in
    let env = Env.acc env (fun acc -> call_constraint :: acc) in
    (env, ret_pty)

let cow_array ~pos renv ty =
  let rec f = function
    | Tcow_array arry -> Some arry
    | Tinter tys -> List.find_map tys f
    | _ -> None
  in
  match f ty with
  | Some arry -> arry
  | None ->
    Errors.unknown_information_flow pos "Hack array";
    (* The following CoW array is completely arbitrary. *)
    {
      a_kind = Adict;
      a_key = Tprim (Env.new_policy_var renv "fake_key");
      a_value = Tprim (Env.new_policy_var renv "fake_value");
      a_length = Env.new_policy_var renv "fake_length";
    }

(* Deals with a true assignment to either a local variable
   or an object property *)
let asn ~expr ~pos renv env ((_, lhs_ty), lhs_exp) rhs_pty =
  match lhs_exp with
  | A.Lvar (_, lid) ->
    let prefix = Local_id.to_string lid in
    let lhs_pty = Lift.ty ~prefix renv lhs_ty in
    (* set asn to true to mark the local as assigned in the
       current code branch *)
    let env = Env.set_local_type env lid lhs_pty in
    let deps = PSet.elements (Env.get_lpc env) in
    let env = Env.acc env (add_dependencies ~pos deps lhs_pty) in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_pty) = expr env obj in
    let obj_pol = (receiver_of_obj_get obj_pty property).c_self in
    let lhs_pty = property_ptype renv obj_pty property lhs_ty in
    let deps = obj_pol :: PSet.elements (Env.get_gpc renv env) in
    let env = Env.acc env (add_dependencies ~pos deps lhs_pty) in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | _ ->
    Errors.unknown_information_flow pos "lvalue";
    env

(* Hack array accesses and mutations may throw when the indexed
   element is not in the array. may_throw_out_of_bounds_exn is
   used to register this fact. *)
let may_throw_out_of_bounds_exn ~pos renv env arry ix_pty =
  let exn_ty = Lift.class_ty renv Decl.out_of_bounds_exception_id in
  (* both the indexing expression and the array length influence
     whether an exception is thrown or not; indeed, the check
     performed by the indexing is of the form:
       if ($ix >= $arry->length) { throw ...; } *)
  let pc_deps = PSet.add arry.a_length (object_policy ix_pty) in
  may_throw ~pos renv env pc_deps exn_ty

(* A wrapper for asn that deals with Hack arrays' syntactic
   sugar; it is important to get it right to account for the
   CoW semantics of Hack arrays:

     $x->p[4][] = "hi";

   does not mutate an array cell, but the property p of
   the object $x instead.  *)
let rec asn_top ~expr ~pos renv env lhs rhs_pty =
  match snd lhs with
  | A.Array_get (arry_exp, ix_opt) ->
    (* Evaluate the array *)
    let (env, arry_pty, arry) =
      let (env, old_arry_pty) = expr env arry_exp in
      (* Here weakening achieves copy-on-write because any new flow we
         register won't share the same flow destination as the earlier
         assignments. *)
      let (env, arry_pty) =
        adjust_ptype ~pos ~adjustment:Aweaken renv env old_arry_pty
      in
      let arry = cow_array ~pos renv arry_pty in
      (env, arry_pty, arry)
    in

    (* TODO(T68269878): track flows due to length changes *)
    (* Evaluate the index *)
    let (env, ix_pty_opt) =
      match ix_opt with
      | Some ix ->
        let (env, ix_pty) = expr env ix in
        (* The index flows to they key of the array *)
        let env = Env.acc env (subtype ~pos ix_pty arry.a_key) in
        (env, Some ix_pty)
      | None -> (env, None)
    in

    (* Potentially raise `OutOfBoundsException` *)
    let env =
      match ix_pty_opt with
      (* When there is no index, we add a new element, hence no exception. *)
      | None -> env
      (* Dictionaries don't throw on assignment, they register a new key. *)
      | Some _ when equal_array_kind arry.a_kind Adict -> env
      | Some ix_pty -> may_throw_out_of_bounds_exn ~pos renv env arry ix_pty
    in

    (* Do the assignment *)
    let env = Env.acc env (subtype ~pos rhs_pty arry.a_value) in

    (* assign the vector itself *)
    asn_top ~expr ~pos renv env arry_exp arry_pty
  | _ -> asn ~expr ~pos renv env lhs rhs_pty

let rec assign ~pos renv env op lhs_exp rhs_exp =
  let expr = expr ~pos renv in
  let (env, rhs_pty) =
    if Option.is_none op then
      expr env rhs_exp
    else
      (* increment-like operations (e.g., $a += $b) *)
      let (env, lhs_pty) = expr env lhs_exp in
      let (env, rhs_pty) = expr env rhs_exp in
      binop ~pos renv env lhs_pty rhs_pty
  in
  let env = asn_top ~expr ~pos renv env lhs_exp rhs_pty in
  (env, rhs_pty)

(* Generate flow constraints for an expression *)
and expr ~pos renv (env : Env.expr_env) (((_, ety), e) : Tast.expr) =
  let expr = expr ~pos renv in
  match e with
  | A.Null
  | A.True
  | A.False
  | A.Int _
  | A.Float _
  | A.String _ ->
    (* literals are public *)
    (env, Tprim (Env.new_policy_var renv "lit"))
  | A.Binop (Ast_defs.Eq op, e1, e2) -> assign ~pos renv env op e1 e2
  | A.Binop (_, e1, e2) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr env e2 in
    binop ~pos renv env ty1 ty2
  | A.Unop (op, e) ->
    begin
      match op with
      (* Operators that mutate *)
      | Ast_defs.Uincr
      | Ast_defs.Udecr
      | Ast_defs.Upincr
      | Ast_defs.Updecr ->
        assign ~pos renv env None e e
      (* Prim operators that don't mutate *)
      | Ast_defs.Utild
      | Ast_defs.Unot
      | Ast_defs.Uplus
      | Ast_defs.Uminus ->
        expr env e
      | Ast_defs.Usilence ->
        Errors.unknown_information_flow pos "silence (@) operator";
        expr env e
    end
  | A.Lvar (_pos, lid) -> refresh_local_type ~pos renv env lid ety
  | A.Obj_get (obj, (_, A.Id (_, property)), _) ->
    let (env, obj_ptype) = expr env obj in
    let prop_pty = property_ptype renv obj_ptype property ety in
    let prefix = "." ^ property in
    let (env, super_pty) =
      adjust_ptype ~pos ~prefix ~adjustment:Aweaken renv env prop_pty
    in
    let obj_pol = (receiver_of_obj_get obj_ptype property).c_self in
    let env = Env.acc env (add_dependencies ~pos [obj_pol] super_pty) in
    (env, super_pty)
  | A.This ->
    (match renv.re_this with
    | Some ptype -> (env, ptype)
    | None -> fail "encountered $this outside of a class context")
  | A.ET_Splice e
  | A.ExpressionTree (_, e, _)
  | A.ParenthesizedExpr e ->
    expr env e
  (* TODO(T68414656): Support calls with type arguments *)
  | A.Call (e, _type_args, args, _extra_args) ->
    let (env, args_pty) = List.map_env ~f:expr env args in
    let call env call_type this_pty =
      call ~pos renv env call_type this_pty args_pty ety
    in
    begin
      match e with
      | (_, A.Id (_, name)) ->
        let call_id = Decl.make_callable_name None name in
        call env (Cglobal call_id) None
      | (_, A.Obj_get (obj, (_, A.Id (_, meth_name)), _)) ->
        let (env, obj_pty) = expr env obj in
        begin
          match obj_pty with
          | Tclass { c_name; _ } ->
            let call_id = Decl.make_callable_name (Some c_name) meth_name in
            call env (Cglobal call_id) (Some obj_pty)
          | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
        end
      | (_, A.Class_const (((_, ty), cid), (_, meth_name))) ->
        let env =
          match cid with
          | A.CIexpr e -> fst @@ expr env e
          | A.CIstatic ->
            (* TODO(T72024862): Handle late static binding *)
            Errors.unknown_information_flow pos "late static binding";
            env
          | _ -> env
        in
        let rec find_class_name ty =
          match T.get_node ty with
          | T.Tdependent (T.DTthis, ty) -> find_class_name ty
          | T.Tclass ((_, class_name), _, _) -> class_name
          | _ -> fail "unhandled method call on a non-class"
        in
        let class_name = find_class_name ty in
        let call_id = Decl.make_callable_name (Some class_name) meth_name in
        let this_pty =
          if String.equal meth_name Decl.construct_id then begin
            (* The only legal class id is `parent` which is invoked as if it
               is invoked on the object being constructed, hence it uses the same
               `$this` as the caller. *)
            assert (Option.is_some renv.re_this);
            renv.re_this
          end else
            None
        in
        call env (Cglobal call_id) this_pty
      | _ ->
        let (env, func_ty) = expr env e in
        let fty =
          match func_ty with
          | Tfun fty -> fty
          | _ -> failwith "calling something that is not a function"
        in
        call env (Clocal fty) None
    end
  | A.ValCollection (((A.Vec | A.Keyset) as kind), _, exprs) ->
    (* Each element of the array is a subtype of the array's value parameter. *)
    let arry_pty = Lift.ty ~prefix:(A.show_vc_kind kind) renv ety in
    let element_pty = (cow_array ~pos renv arry_pty).a_value in
    let mk_element_subtype env exp =
      let (env, pty) = expr env exp in
      Env.acc env (subtype ~pos pty element_pty)
    in
    let env = List.fold ~f:mk_element_subtype ~init:env exprs in
    (env, arry_pty)
  | A.KeyValCollection (A.Dict, _, fields) ->
    (* Each field's key and value are subtypes of the array key and value
       policy types. *)
    let dict_pty = Lift.ty ~prefix:"dict" renv ety in
    let arr = cow_array ~pos renv dict_pty in
    let mk_element_subtype env (key, value) =
      let subtype = subtype ~pos in
      let (env, key_pty) = expr env key in
      let (env, value_pty) = expr env value in
      Env.acc env @@ fun acc ->
      acc |> subtype key_pty arr.a_key |> subtype value_pty arr.a_value
    in
    let env = List.fold ~f:mk_element_subtype ~init:env fields in
    (env, dict_pty)
  | A.Array_get (arry, ix_opt) ->
    (* Evaluate the array *)
    let (env, arry_pty) = expr env arry in
    let arry = cow_array ~pos renv arry_pty in

    (* Evaluate the index, it might have side-effects! *)
    let (env, ix_pty) =
      match ix_opt with
      | Some ix -> expr env ix
      | None -> fail "cannot have an empty index when reading"
    in

    (* The index flows into the array key which flows into the array value *)
    let env = Env.acc env @@ subtype ~pos ix_pty arry.a_key in

    let env = may_throw_out_of_bounds_exn ~pos renv env arry ix_pty in

    (env, arry.a_value)
  | A.New (((_, lty), cid), _targs, args, _extra_args, _) ->
    (* TODO(T70139741): support variadic functions and constructors
     * TODO(T70139893): support classes with type parameters
     *)
    let (env, args_pty) = List.map_env ~f:expr env args in
    let env =
      match cid with
      | A.CIexpr e -> fst @@ expr env e
      | A.CIstatic ->
        (* TODO(T72024862): Handle late static binding *)
        Errors.unknown_information_flow pos "late static binding";
        env
      | _ -> env
    in
    let obj_pty = Lift.ty renv lty in
    begin
      match obj_pty with
      | Tclass { c_name; _ } ->
        let call_id = Decl.make_callable_name (Some c_name) Decl.construct_id in
        let lty = T.mk (Typing_reason.Rnone, T.Tprim A.Tvoid) in
        let (env, _) =
          call ~pos renv env (Cglobal call_id) (Some obj_pty) args_pty lty
        in
        let pty = Lift.ty ~prefix:"constr" renv ety in
        (env, pty)
      | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
    end
  | A.Efun (fun_, captured_ids)
  | A.Lfun (fun_, captured_ids) ->
    (* weaken all the captured local variables and reset the local
       pc to create the initial continuation we'll use to check the
       lambda literal *)
    let (env, start_cont) =
      let (env, k_vars) =
        let lids = List.map ~f:snd captured_ids |> LSet.of_list in
        Env.get_locals env
        |> LMap.filter (fun lid _ -> LSet.mem lid lids)
        |> LMap.map_env
             (fun env _ -> adjust_ptype ~pos ~adjustment:Aweaken renv env)
             env
      in
      (env, { k_pc = PSet.empty; k_vars })
    in
    let pc = Env.new_policy_var renv "pc" in
    let self = Env.new_policy_var renv "lambda" in
    let exn = Lift.class_ty renv Decl.exception_id in
    let ret = Lift.ty ~prefix:"ret" renv (fst fun_.A.f_ret) in
    let ptys = lift_params renv fun_.A.f_params in
    let args = List.map ~f:snd ptys in
    let env =
      Env.analyze_lambda_body env @@ fun env ->
      let env = Env.prep_stmt env start_cont in
      let env = set_local_types env ptys in
      let renv = { renv with re_ret = ret; re_exn = exn; re_gpc = pc } in
      let (env, _out) = block renv env fun_.A.f_body.A.fb_ast in
      env
    in
    let ty =
      Tfun { f_pc = pc; f_self = self; f_args = args; f_ret = ret; f_exn = exn }
    in
    (env, ty)
  | A.Await e -> expr env e
  | A.List es ->
    let (env, ptys) = List.map_env env es ~f:(fun env e -> expr env e) in
    (env, Ttuple ptys)
  | A.Pipe ((_, dollardollar), e1, e2) ->
    let (env, t1) = expr env e1 in
    let dd_old = Env.get_local_type env dollardollar in
    let env = Env.set_local_type env dollardollar t1 in
    let (env, t2) = expr env e2 in
    let env = Env.set_local_type_opt env dollardollar dd_old in
    (env, t2)
  | A.Dollardollar (_, lid) -> refresh_local_type ~pos renv env lid ety
  (* --- expressions below are not yet supported *)
  | A.Darray (_, _)
  | A.Varray (_, _)
  | A.Shape _
  | A.ValCollection (_, _, _)
  | A.KeyValCollection (_, _, _)
  | A.Omitted
  | A.Id _
  | A.Clone _
  | A.Obj_get (_, _, _)
  | A.Class_get (_, _)
  | A.Class_const (_, _)
  | A.FunctionPointer (_, _)
  | A.String2 _
  | A.PrefixedString (_, _)
  | A.Yield _
  | A.Yield_break
  | A.Suspend _
  | A.Expr_list _
  | A.Cast (_, _)
  | A.Eif (_, _, _)
  | A.Is (_, _)
  | A.As (_, _, _)
  | A.Record (_, _)
  | A.Xml (_, _, _)
  | A.Callconv (_, _)
  | A.Lplaceholder _
  | A.Fun_id _
  | A.Method_id (_, _)
  | A.Method_caller (_, _)
  | A.Smethod_id (_, _)
  | A.Pair _
  | A.Assert _
  | A.PU_atom _
  | A.PU_identifier (_, _, _)
  | A.Any ->
    Errors.unknown_information_flow pos "expression";
    (env, Lift.ty renv ety)
  | A.Import _
  | A.Collection _
  | A.BracedExpr _ ->
    failwith "AST should not contain these nodes"

and stmt renv (env : Env.stmt_env) ((pos, s) : Tast.stmt) =
  let expr_ = expr
  and expr ?(pos = pos) renv env e =
    let (env, ety) = expr ~pos renv (Env.prep_expr env) e in
    let (env, ethrow) = Env.close_expr env in
    (env, ety, ethrow)
  in
  let untainted_pc = Env.get_lpc env in
  let clear_pc_deps out =
    (* if the outcome of a statement is unconditional, the
       outcome control flow has exactly the same dependencies
       as the statement itself *)
    if KMap.cardinal out = 1 then
      KMap.map (fun k -> { k with k_pc = untainted_pc }) out
    else
      out
  in
  let refresh_with_tymap ~pos renv env tymap =
    let refresh var (_, lty) env =
      fst @@ refresh_local_type ~force:true ~pos renv env var lty
    in
    Local_id.Map.fold refresh tymap env
  in
  let loop_the_env ~pos env out_blk beg_locals =
    (* issue a subtype call between the type of the locals at
       the end of loop and their invariant type; this constrains
       their IFC type to indeed be invariant and spares us from
       running a more classic fixpoint computation *)
    Env.acc env
    @@
    match KMap.find_opt K.Next out_blk with
    | None -> Utils.identity
    | Some { k_vars = end_locals; _ } ->
      let update_lid lid end_type =
        match LMap.find_opt lid beg_locals with
        | None -> Utils.identity
        | Some beg_type -> subtype ~pos end_type beg_type
      in
      LMap.fold update_lid end_locals
  in
  match s with
  | A.AssertEnv (A.Refinement, tymap) ->
    (* The typechecker refined the type of some locals due
       to a runtime check, update ifc types to match the
       refined types *)
    let env =
      Local_id.Map.fold
        (fun var hint env ->
          match get_local_type ~pos env var with
          | None -> env
          | Some pty ->
            let new_pty = refine renv pty hint in
            Env.set_local_type env var new_pty)
        tymap
        env
    in
    Env.close_stmt env K.Next
  | A.Expr e ->
    let (env, _ety, ethrow) = expr renv env e in
    Env.close_stmt ~merge:ethrow env K.Next
  | A.If (cond, b1, b2) ->
    let pos = fst (fst cond) in
    let (env, cty, cthrow) = expr ~pos renv env cond in
    (* use object_policy to account for both booleans
       and null checks *)
    Env.with_pc_deps env (object_policy cty) @@ fun env ->
    let beg_cont = Env.get_next env in
    let (env, out1) = block renv env b1 in
    let (env, out2) = block renv (Env.prep_stmt env beg_cont) b2 in
    let out = Env.merge_out out1 out2 in
    let out = Env.merge_out out cthrow in
    let out = clear_pc_deps out in
    (env, out)
  | A.While (cond, (_, A.AssertEnv (A.Join, tymap)) :: blk) ->
    let pos = fst (fst cond) in
    let env = refresh_with_tymap ~pos renv env tymap in
    let beg_locals = Env.get_locals env in
    (* TODO: pc_pols should also flow into cty because the condition is evaluated
       unconditionally only the first time around the loop. *)
    let (env, cty, cthrow) = expr ~pos renv env cond in

    let pc_policies = object_policy cty in
    Env.with_pc_deps env pc_policies @@ fun env ->
    let tainted_lpc = Env.get_lpc env in
    let (env, out_blk) = block renv env blk in
    let out_blk = Env.merge_out out_blk cthrow in
    let out_blk = Env.merge_in_next out_blk K.Continue in
    let env = loop_the_env ~pos env out_blk beg_locals in
    let out =
      (* overwrite the Next outcome to use the type for local
         variables as they are at the beginning of the loop,
         then merge the Break outcome in Next *)
      let next = { k_pc = tainted_lpc; k_vars = beg_locals } in
      Env.merge_in_next (KMap.add K.Next next out_blk) K.Break
    in
    let out = clear_pc_deps out in
    (env, out)
  | A.Foreach (collection, as_exp, (_, A.AssertEnv (A.Join, tymap)) :: blk) ->
    let pos = fst (fst collection) in
    let env = refresh_with_tymap ~pos renv env tymap in
    let beg_locals = Env.get_locals env in

    (* TODO: pc should also flow into cty because the condition is evaluated
       unconditionally only the first time around the loop. *)
    let (env, array_pty, cthrow) = expr ~pos renv env collection in
    let array = cow_array ~pos renv array_pty in

    let env = Env.prep_expr env in
    let expr = expr_ ~pos renv in
    let env =
      match as_exp with
      | Aast.As_v value
      | Aast.Await_as_v (_, value) ->
        asn ~expr ~pos renv env value array.a_value
      | Aast.As_kv (key, value)
      | Aast.Await_as_kv (_, key, value) ->
        let env = asn ~expr ~pos renv env value array.a_value in
        asn ~expr ~pos renv env key array.a_key
    in

    let pc_policies = PSet.singleton array.a_length in
    let (env, ethrow) = Env.close_expr env in
    if not @@ KMap.is_empty ethrow then fail "foreach collection threw";
    Env.with_pc_deps env pc_policies @@ fun env ->
    let tainted_lpc = Env.get_lpc env in
    let (env, out_blk) = block renv env blk in
    let out_blk = Env.merge_out out_blk cthrow in
    let out_blk = Env.merge_in_next out_blk K.Continue in
    let env = loop_the_env ~pos env out_blk beg_locals in
    let out =
      (* overwrite the Next outcome to use the type for local
         variables as they are at the beginning of the loop,
         then merge the Break outcome in Next *)
      let next = { k_pc = tainted_lpc; k_vars = beg_locals } in
      Env.merge_in_next (KMap.add K.Next next out_blk) K.Break
    in
    let out = clear_pc_deps out in
    (env, out)
  | A.Break -> Env.close_stmt env K.Break
  | A.Continue -> Env.close_stmt env K.Continue
  | A.Fallthrough -> Env.close_stmt env K.Fallthrough
  | A.Return e ->
    let (env, ethrow) =
      match e with
      | None -> (env, KMap.empty)
      | Some e ->
        let (env, ety, ethrow) = expr renv env e in
        let deps = PSet.elements (Env.get_lpc env) in
        let env = Env.acc env (subtype ety renv.re_ret ~pos) in
        let env = Env.acc env (add_dependencies deps renv.re_ret ~pos) in
        (env, ethrow)
    in
    Env.close_stmt ~merge:ethrow env K.Exit
  | A.Throw e ->
    let (env, ety) = expr_ ~pos renv (Env.prep_expr env) e in
    let env = may_throw ~pos renv env PSet.empty ety in
    let (env, ethrow) = Env.close_expr env in
    Env.close_stmt ~merge:ethrow env K.Catch
  | A.Try (try_blk, cs, finally) ->
    (* Note: unlike the Hack typechecker which accumulates the environments
       for exceptional outcomes in 'env' itself we have a more explicit
       handling of outcomes (stmt *returns* the outcome map). This explicit
       style makes it much easier to process the difficult semantics of
       finally blocks; in particular, we have no use of the K.Finally
       cont key that was used as some kind of temporary variable by the
       Hack typechecker. *)
    (* use a fresh exception type to check the try block so that we
       do not influence the current exception type in case there is
       a catch-all block (`catch (Exception ...) { ... }`) *)
    let fresh_exn = Lift.class_ty renv Decl.exception_id in
    let (env, out_try) =
      let renv = { renv with re_exn = fresh_exn } in
      block renv env try_blk
    in
    (* strip out_try from its Catch continuation; it will be used
       at the beginning of catch blocks *)
    let (out_try, catch_cont) = Env.strip_cont out_try K.Catch in
    (* in case there is no catch all, the exception still lingers *)
    let (env, out_try) =
      let is_Exception ((_, exn), _, _) = String.equal exn Decl.exception_id in
      match catch_cont with
      | Some catch_cont when not (List.exists ~f:is_Exception cs) ->
        let out_try = KMap.add K.Catch catch_cont out_try in
        let env = Env.acc env (subtype ~pos fresh_exn renv.re_exn) in
        (env, out_try)
      | _ -> (env, out_try)
    in
    (* merge the outcome of all the catch blocks started from the Catch
       outcome of the try block *)
    let (env, out_catch) =
      match catch_cont with
      | None ->
        (* the try block does not throw, we can just return an empty
           outcome for the catch blocks since they will never run *)
        (env, KMap.empty)
      | Some catch_cont ->
        let catch (env, out) (_, (_, exn_var), blk) =
          let env = Env.prep_stmt env catch_cont in
          let env = Env.set_local_type env exn_var fresh_exn in
          let (env, out_blk) = block renv env blk in
          (env, Env.merge_out out out_blk)
        in
        List.fold ~f:catch ~init:(env, KMap.empty) cs
    in
    (* now we simply merge the outcomes of all the catch blocks with the
       one of the try block *)
    let out_try_catch = Env.merge_out out_try out_catch in
    let out_try_catch = clear_pc_deps out_try_catch in
    (* for each continuation in out_try_catch we will perform an
       analysis of the finally block; this improves precision of
       the analysis a bit and mimicks what Hack does *)
    KMap.fold
      (fun k cont (env, out_finally) ->
        (* analyze the finally block for the outcome k *)
        let env = Env.prep_stmt env cont in
        (* we use the untainted pc when processing the finally
           block because it will run exactly as often as the
           Try statement itself *)
        Env.with_pc env untainted_pc @@ fun env ->
        let (env, out) = block renv env finally in
        let out = Env.merge_next_in out k in
        (* restore all the pc dependencies for the outcomes
           of the finally block *)
        let out =
          let add_deps pc = PSet.union pc cont.k_pc in
          KMap.map (fun c -> { c with k_pc = add_deps c.k_pc }) out
        in
        (env, Env.merge_out out_finally out))
      out_try_catch
      (env, KMap.empty)
  | A.Switch (e, cl) ->
    let pos = fst (fst e) in
    let (env, ety, ethrow) = expr ~pos renv env e in
    let (env, out_cond) = Env.close_stmt ~merge:ethrow env K.Fallthrough in
    let case (env, (out, deps)) c =
      let out = Env.merge_out out_cond out in
      let (out, ft_cont_opt) = Env.strip_cont out K.Fallthrough in
      (* out_cond has a Fallthrough so ft_cont_opt cannot be None *)
      let env = Env.prep_stmt env (Option.value_exn ft_cont_opt) in
      Env.with_pc_deps env deps @@ fun env ->
      let (env, out, new_deps, b) =
        match c with
        | A.Default (_, b) -> (env, out, PSet.empty, b)
        | A.Case (e, b) ->
          let pos = fst (fst e) in
          let (env, ety, ethrow) = expr ~pos renv env e in
          let out = Env.merge_out out ethrow in
          (env, out, object_policy ety, b)
      in
      Env.with_pc_deps env new_deps @@ fun env ->
      let (env, out_blk) = block renv env b in
      let out = Env.merge_out out out_blk in
      (* deps is accumulated monotonically because going
         through each 'case' reveals increasingly more
         information about the scrutinee and the cases *)
      (env, (out, PSet.union deps new_deps))
    in
    let (env, (out, _final_deps)) =
      let initial_deps = object_policy ety in
      List.fold ~f:case ~init:(env, (out_cond, initial_deps)) cl
    in
    let out = Env.merge_in_next out K.Continue in
    let out = Env.merge_in_next out K.Break in
    let out = Env.merge_in_next out K.Fallthrough in
    let out = clear_pc_deps out in
    (env, out)
  | A.Noop -> Env.close_stmt env K.Next
  | _ ->
    Errors.unknown_information_flow pos "statement";
    Env.close_stmt env K.Next

and block renv env (blk : Tast.block) =
  let seq (env, out) s =
    let (out, next_opt) = Env.strip_cont out K.Next in
    match next_opt with
    | None ->
      (* we are looking at dead code; skip the statement *)
      (env, out)
    | Some cont ->
      let env = Env.prep_stmt env cont in
      let (env, out_pst) = stmt renv env s in
      (* we have to merge the exceptional outcomes from
         before 's' with the ones after 's' *)
      (env, Env.merge_out out_pst out)
  in
  let init = Env.close_stmt env K.Next in
  List.fold_left ~f:seq ~init blk

(* Checks that two type schemes are in a subtyping relationship. The
 * skeletons of the two input type schemes are expected to be same.
 * A list of invalid flows is returned, if the list is empty the
 * type schemes are in a subtyping relationship. *)
let check_subtype_scheme ~pos sub_scheme sup_scheme : pos_flow list =
  let (Fscheme (sub_scope, sub_proto, sub_prop)) = sub_scheme in
  let (Fscheme (sup_scope, sup_proto, sup_prop)) = sup_scheme in
  assert (not (Scope.equal sub_scope sup_scope));
  (* To compare them, we need the two constraints to use the same set
     of free variables. For example, if sub_scheme and sup_scheme are
       - ((function():int{p1}), Csub) and,
       - ((function():int{p2}), Csup)
     we rename p2 into p1 by conjoining p2 = p1 to Csup and
     quantifying p2 away. *)
  let sup_props =
    let accum = ref [sup_prop] in
    let rec equate pt1 pt2 =
      let eqpol p1 p2 = accum := L.(p1 = p2) ~pos !accum in
      Ifc_mapper.iter_ptype2 equate eqpol pt1 pt2
    in
    equate (Tfun sub_proto.fp_type) (Tfun sup_proto.fp_type);
    begin
      match (sub_proto.fp_this, sup_proto.fp_this) with
      | (Some sub_this_ty, Some sup_this_ty) -> equate sub_this_ty sup_this_ty
      | (None, None) -> ()
      | _ -> invalid_arg "method/function mismatch"
    end;
    !accum
  in
  let sub_vars =
    let fp_vars = free_pvars (Tfun sub_proto.fp_type) in
    match sub_proto.fp_this with
    | Some sub_this_ty -> VarSet.union fp_vars (free_pvars sub_this_ty)
    | None -> fp_vars
  in
  let pred v = not @@ VarSet.mem v sub_vars in
  let sup_lattice =
    Logic.conjoin sup_props
    |> Logic.quantify ~pred ~quant:Qexists
    |> Logic.simplify
    |> Logic.flatten_prop
    |> Ifc_security_lattice.transitive_closure
  in
  let sub_prop =
    sub_prop |> Logic.quantify ~pred ~quant:Qexists |> Logic.simplify
  in
  Logic.entailment_violations sup_lattice sub_prop

let analyse_callable
    ?class_name
    ~pos
    ~opts
    ~decl_env
    ~is_static
    ~saved_env
    name
    params
    body
    return =
  try
    (* Setup the read-only environment *)
    let scope = Scope.alloc () in
    let renv = Env.new_renv scope decl_env saved_env in

    let global_pc = Env.new_policy_var renv "pc" in
    let exn = Lift.class_ty ~prefix:"exn" renv Decl.exception_id in

    (* Here, we ignore the type parameters of this because at the moment we
     * lack Tgeneric policy type. This will be fixed (T68414656) in the future.
     *)
    let this_ty =
      match class_name with
      | Some cname when not is_static -> Some (Lift.class_ty renv cname)
      | _ -> None
    in
    let ret_ty = Lift.ty ~prefix:"ret" renv return in
    let renv = Env.prep_renv renv this_ty ret_ty exn global_pc in

    (* Initialise the mutable environment *)
    let env = Env.prep_stmt Env.empty_env Env.empty_cont in
    let params = lift_params renv params in
    let env = set_local_types env params in

    (* Run the analysis *)
    let beg_env = env in
    let (env, out_blk) = block renv env body.A.fb_ast in
    let out_blk = Env.merge_next_in out_blk K.Exit in
    let end_env = env in

    (* Display the analysis results *)
    if should_print opts.opt_mode Manalyse then begin
      Format.printf "Analyzing %s:@." name;
      Format.printf "%a@." Pp.renv renv;
      Format.printf "* Params:@,  %a@." Pp.cont (Env.get_next beg_env);
      Format.printf "* Final environment:@,  %a@." Pp.env end_env;
      begin
        match KMap.find_opt K.Exit out_blk with
        | Some cont -> Format.printf "  Locals:@,    %a@." Pp.cont cont
        | None -> ()
      end;
      Format.printf "@."
    end;

    let callable_name = Decl.make_callable_name class_name name in
    let f_self = Env.new_policy_var renv callable_name in

    let proto =
      {
        fp_name = callable_name;
        fp_this = this_ty;
        fp_type =
          {
            f_pc = global_pc;
            f_self;
            f_args = List.map ~f:snd params;
            f_ret = ret_ty;
            f_exn = exn;
          };
      }
    in

    let entailment =
      match SMap.find_opt callable_name decl_env.de_fun with
      | Some { fd_kind = FDGovernedBy policy; fd_args } ->
        let scheme = Decl.make_callable_scheme renv policy proto fd_args in
        fun prop ->
          let fun_scheme = Fscheme (scope, proto, prop) in
          check_subtype_scheme ~pos fun_scheme scheme
      | _ -> const []
    in

    (* Return the results *)
    let res =
      {
        res_span = pos;
        res_proto = proto;
        res_scope = scope;
        res_constraint = Logic.conjoin (Env.get_constraints env);
        res_deps = Env.get_deps env;
        res_entailment = entailment;
      }
    in
    Some res
  with FlowInference s ->
    Format.printf "Analyzing %s:@.  Failure: %s@.@." name s;
    None

let walk_tast opts decl_env =
  let def = function
    | A.Fun
        {
          A.f_name = (_, name);
          f_annotation = saved_env;
          f_params = params;
          f_body = body;
          f_ret = (return, _);
          f_span = pos;
          _;
        } ->
      let is_static = false in
      let callable_res =
        analyse_callable
          ~opts
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          name
          params
          body
          return
      in
      Option.map ~f:(fun x -> [x]) callable_res
    | A.Class { A.c_name = (_, class_name); c_methods = methods; _ } ->
      let handle_method
          {
            A.m_name = (_, name);
            m_annotation = saved_env;
            m_params = params;
            m_body = body;
            m_ret = (return, _);
            m_span = pos;
            m_static = is_static;
            _;
          } =
        analyse_callable
          ~opts
          ~class_name
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          name
          params
          body
          return
      in
      Some (List.filter_map ~f:handle_method methods)
    | _ -> None
  in
  (fun tast -> List.concat (List.filter_map ~f:def tast))

let check opts tast =
  (* Declaration phase *)
  let decl_env = Decl.collect_sigs tast in
  if should_print ~user_mode:opts.opt_mode ~phase:Mdecl then
    Format.printf "%a@." Pp.decl_env decl_env;

  (* Flow analysis phase *)
  let results = walk_tast opts decl_env tast in

  (* Solver phase *)
  let results =
    try Solver.global_exn ~subtype results with
    | Solver.Error Solver.RecursiveCycle ->
      fail "solver error: cyclic call graph"
    | Solver.Error (Solver.MissingResults callable) ->
      fail "solver error: missing results for callable '%s'" callable
    | Solver.Error (Solver.InvalidCall (caller, callee)) ->
      fail "solver error: invalid call to '%s' in '%s'" callee caller
  in

  let simplify result =
    let pred = const true in
    ( result,
      result.res_entailment result.res_constraint,
      Logic.simplify
      @@ Logic.quantify ~pred ~quant:Qexists result.res_constraint )
  in
  let simplified_results = SMap.map simplify results in

  let log_solver name (result, _, simplified) =
    Format.printf "@[<v>";
    Format.printf "Flow constraints for %s:@.  @[<v>" name;
    Format.printf "@,@[<hov>Simplified:@ @[<hov>%a@]@]" Pp.prop simplified;
    let raw = result.res_constraint in
    Format.printf "@,@[<hov>Raw:@ @[<hov>%a@]@]" Pp.prop raw;
    Format.printf "@]";
    Format.printf "@]\n\n"
  in
  if should_print ~user_mode:opts.opt_mode ~phase:Msolve then
    SMap.iter log_solver simplified_results;

  (* Checking phase *)
  let check_valid_flow _ (result, implicit, simple) =
    let simple_illegal_flows =
      Logic.entailment_violations opts.opt_security_lattice simple
    in
    let to_err node =
      ( PosSet.elements (pos_set_of_policy node),
        Format.asprintf "%a" Pp.policy node )
    in
    let illegal_information_flow (poss, source, sink) =
      (* Separate error positions that are not in the result and filter out
         unknown positions *)
      let (primary_poss, other_poss) =
        PosSet.filter (fun p -> not @@ Pos.equal Pos.none p) poss
        |> PosSet.elements
        |> List.partition_tf ~f:(Pos.overlaps result.res_span)
      in
      (* Make sure the primary error position is the latest position in the
         callable being analysed *)
      let (primary_pos, other_poss) =
        match List.sort ~compare:Pos.compare primary_poss |> List.rev with
        | [] -> (result.res_span, other_poss)
        | primary :: primary_poss ->
          (primary, List.unordered_append primary_poss other_poss)
      in

      let (source, sink) = (to_err source, to_err sink) in
      Errors.illegal_information_flow primary_pos other_poss source sink
    in

    let context_implicit_policy_leakage (pos, source, sink) =
      (* The latest program point contributing to the violation is the
         primary error *)
      let (primary, secondaries) =
        let poss =
          PosSet.elements pos |> List.sort ~compare:Pos.compare |> List.rev
        in
        match poss with
        | [] -> (result.res_span, [])
        | primary :: secondaries -> (primary, secondaries)
      in
      let (source, sink) = (to_err source, to_err sink) in
      Errors.context_implicit_policy_leakage primary secondaries source sink
    in

    if should_print ~user_mode:opts.opt_mode ~phase:Mcheck then begin
      List.iter ~f:illegal_information_flow simple_illegal_flows;
      List.iter ~f:context_implicit_policy_leakage implicit
    end
  in
  SMap.iter check_valid_flow simplified_results

let do_ opts files_info ctx =
  ( if should_print ~user_mode:opts.opt_mode ~phase:Mlattice then
    let lattice = opts.opt_security_lattice in
    Format.printf "@[Lattice:@. %a@]\n\n" Pp.security_lattice lattice );

  let handle_file path info errors =
    match info.FileInfo.file_mode with
    | Some FileInfo.Mstrict ->
      let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
      let { Tast_provider.Compute_tast.tast; _ } =
        Tast_provider.compute_tast_unquarantined ~ctx ~entry
      in
      let check () = check opts tast in
      let (new_errors, _) = Errors.do_with_context path Errors.Typing check in
      errors @ Errors.get_error_list new_errors
    | _ -> errors
  in

  Relative_path.Map.fold files_info ~init:[] ~f:handle_file

let magic_builtins =
  [|
    ( "ifc_magic.hhi",
      {|<?hh // strict
class InferFlows
  implements
    HH\FunctionAttribute,
    HH\MethodAttribute {}
class Policied
  implements
    HH\InstancePropertyAttribute,
    HH\ClassAttribute,
    HH\ParameterAttribute,
    HH\FunctionAttribute {
  public function __construct(public string $purpose) { }
}
class Governed
 implements
   HH\FunctionAttribute,
   HH\MethodAttribute {
  public function __construct(public string $purpose = "") { }
}
class External
  implements
    HH\ParameterAttribute {}
class CanCall
  implements
    HH\ParameterAttribute {}
|}
    );
  |]
