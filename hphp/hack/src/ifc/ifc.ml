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

let should_print ~user_mode ~phase =
  equal_mode user_mode Mdebug || equal_mode user_mode phase

let fail fmt =
  Format.kasprintf (fun s -> raise (IFCError (FlowInference s))) fmt

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
  | Tnull p
  | Tprim p ->
    (pol p, emp, emp)
  | Tnonnull (pself, plump) -> (pol pself, pol plump, emp)
  | Tgeneric p -> (emp, pol p, emp)
  | Tdynamic p -> (emp, pol p, emp)
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
      ((pol f_self, emp, pol f_pc)
      :: policy_occurrences f_ret
      :: policy_occurrences f_exn
      :: List.map ~f:swap_policy_occurrences f_args)
  | Tcow_array { a_key; a_value; a_length; _ } ->
    on_list
      [
        (pol a_length, emp, emp);
        policy_occurrences a_key;
        policy_occurrences a_value;
      ]
  | Tshape { sh_kind; sh_fields } ->
    let f acc { sft_ty; sft_policy; _ } =
      (pol sft_policy, emp, emp) :: policy_occurrences sft_ty :: acc
    in
    let init =
      match sh_kind with
      | Closed_shape -> []
      | Open_shape ty -> [policy_occurrences ty]
    in
    Typing_defs.TShapeMap.values sh_fields |> List.fold ~f ~init |> on_list

exception SubtypeFailure of string * ptype * ptype

(* A constraint accumulator that registers a subtyping
   requirement t1 <: t2 *)
let rec subtype ~pos t1 t2 acc =
  let subtype = subtype ~pos in
  let err msg = raise (SubtypeFailure (msg, t1, t2)) in
  let rec first_ok ~f msg = function
    | [] -> err msg
    | x :: l -> begin
      try f x with
      | SubtypeFailure (msg, _, _) -> first_ok ~f msg l
    end
  in
  match (t1, t2) with
  | (Tnull p1, Tnull p2)
  | (Tprim p1, Tprim p2) ->
    L.(p1 < p2) ~pos acc
  | (Tnull _, Tnonnull _) -> err "null is not a subtype of nonnull"
  | (_, Tnonnull (pself, plump)) ->
    let (cov, inv, _cnt) = policy_occurrences t1 in
    (* we leave contravariant policies unconstrained;
       that is sound and should not pose precision problems
       since function types are seldom refined from `mixed` *)
    L.(PSet.elements cov <* [pself] && [plump] =* PSet.elements inv) ~pos acc
  | ((Tgeneric p1 | Tdynamic p1), _) ->
    let (cov, inv, cnt) = policy_occurrences t2 in
    L.(
      [p1] <* PSet.elements cov
      && [p1] =* PSet.elements inv
      && PSet.elements cnt <* [p1])
      ~pos
      acc
  | (_, (Tgeneric p2 | Tdynamic p2)) ->
    let (cov, inv, cnt) = policy_occurrences t1 in
    L.(
      PSet.elements cov <* [p2]
      && [p2] =* PSet.elements inv
      && [p2] <* PSet.elements cnt)
      ~pos
      acc
  | (Ttuple tl1, Ttuple tl2) ->
    (match List.zip tl1 tl2 with
    | List.Or_unequal_lengths.Ok zip ->
      List.fold zip ~init:acc ~f:(fun acc (t1, t2) -> subtype t1 t2 acc)
    | List.Or_unequal_lengths.Unequal_lengths -> err "incompatible tuple types")
  | (Tclass cl1, Tclass cl2) ->
    (* We do not attempt to replicate the work Hack did and instead
       only act on policies. A bit of precision could be gained when
       dealing with disjunctive subtyping queries if we realize that,
       for example, cl1 and cl2 are incompatible; but let's keep it
       simple for now. *)
    L.(cl1.c_lump = cl2.c_lump && cl1.c_self < cl2.c_self) ~pos acc
  | (Tfun f1, Tfun f2) ->
    (* TODO(T70139741): Account for variadic argument lists. *)
    (* Truncate argument list on the right, in case the left one is shorter
       due to omitted arguments to functions with default values.
       TODO(T79395145): Default values can be arbitrary expressions and hence
       they need to be conditionally executed and joined with the
       environment. *)
    let truncated_size = List.length f2.f_args in
    let truncated_f1_args = List.take f1.f_args truncated_size in
    let zipped_args =
      match List.zip truncated_f1_args f2.f_args with
      | List.Or_unequal_lengths.Ok zip -> zip
      | List.Or_unequal_lengths.Unequal_lengths ->
        err "functions have different number of arguments"
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
  | (Tshape s1, Tshape s2) ->
    let acc =
      match (s1.sh_kind, s2.sh_kind) with
      | (Open_shape _, Closed_shape) ->
        err "An open shape cannot subtype a closed shape"
      | (Open_shape t1, Open_shape t2) -> subtype t1 t2 acc
      | _ -> acc
    in
    let preprocess kind = function
      (* A missing field of an open shape becomes an optional field of type mixed *)
      | None -> begin
        match kind with
        | Open_shape sft_ty ->
          Some { sft_optional = true; sft_policy = pbot; sft_ty }
        | Closed_shape -> None
      end
      (* If a field is optional with type nothing, consider it missing.
          Since it has type nothing, it can never be assigned and therefore
          we do not need to consider its policy *)
      | Some { sft_ty = Tunion []; sft_optional = true; _ } -> None
      | sft -> sft
    in
    let combine acc _ f1 f2 =
      let f1 = preprocess s1.sh_kind f1 in
      let f2 = preprocess s2.sh_kind f2 in
      match (f1, f2) with
      | (Some { sft_optional = true; _ }, Some { sft_optional = false; _ }) ->
        err "optional field cannot be subtype of required"
      | ( Some { sft_ty = t1; sft_policy = p1; _ },
          Some { sft_ty = t2; sft_policy = p2; _ } ) ->
        (L.(p1 < p2) ~pos acc |> subtype t1 t2, None)
      | (Some _, None) -> err "missing field"
      | (None, Some { sft_optional; _ }) ->
        if sft_optional then
          (acc, None)
        else
          err "missing field"
      | (None, None) -> (acc, None)
    in
    let (acc, _) =
      Typing_defs.TShapeMap.merge_env ~combine acc s1.sh_fields s2.sh_fields
    in
    acc
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

(* Overwrite subtype and equivalent catching the SubtypeFailure
   exception *)
let subtype =
  let wrap f ~pos t1 t2 acc =
    try f ~pos t1 t2 acc with
    | SubtypeFailure (msg, tsub, tsup) ->
      fail "subtype: %s (%a <: %a)" msg Pp.ptype tsub Pp.ptype tsup
  in
  wrap subtype

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
let refine renv tyori pos ltyref =
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
      | Pfree_var (var, s) when Scope.equal s ref_scope -> begin
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
    | Tnull p -> simple_freshen env (fun p -> Tnull p) p
    | Tprim p -> simple_freshen env (fun p -> Tprim p) p
    | Tnonnull (pself, plump) ->
      (* plump is invariant, so we do not adjust it *)
      simple_freshen env (fun pself -> Tnonnull (pself, plump)) pself
    | Tgeneric p -> (env, Tgeneric p)
    | Tdynamic p -> (env, Tdynamic p)
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
    | Tshape { sh_kind; sh_fields } ->
      let f env _ sft =
        let (env, ty) = freshen_cov env sft.sft_ty in
        let (env, p) = freshen_pol_cov env sft.sft_policy in
        (env, { sft_ty = ty; sft_policy = p; sft_optional = sft.sft_optional })
      in
      let (env, sh_fields) = Typing_defs.TShapeMap.map_env f env sh_fields in
      let (env, sh_kind) =
        match sh_kind with
        | Open_shape ty ->
          let (env, ty) = freshen_cov env ty in
          (env, Open_shape ty)
        | Closed_shape -> (env, Closed_shape)
      in
      (env, Tshape { sh_kind; sh_fields })
  in
  freshen adjustment env ty

(* Returns the set of policies, to be understood as a join,
   that governs an object with the argument type *)
let rec object_policy = function
  | Tnull pol
  | Tprim pol
  | Tnonnull (pol, _)
  | Tgeneric pol
  | Tdynamic pol
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
  | Tshape { sh_kind; sh_fields; _ } ->
    let f _ { sft_ty; sft_policy; _ } acc =
      acc |> PSet.add sft_policy |> PSet.union (object_policy sft_ty)
    in
    let pols =
      match sh_kind with
      | Open_shape ty -> object_policy ty
      | Closed_shape -> PSet.empty
    in
    Typing_defs.TShapeMap.fold f sh_fields pols

let add_dependencies pl t = L.(pl <* PSet.elements (object_policy t))

let join_policies ?(prefix = "join") ~pos renv env pl =
  let drop_pbot (Pbot _ :: pl | pl) = pl in
  match drop_pbot (List.dedup_and_sort ~compare:compare_policy pl) with
  | [] -> (env, pbot)
  | [p] -> (env, p)
  | (Ptop _ as ptop) :: _ -> (env, ptop)
  | pl ->
    let pv = Env.new_policy_var renv prefix in
    let env = Env.acc env (L.(pl <* [pv]) ~pos) in
    (env, pv)

let get_local_type ~pos:_ env lid =
  match Env.get_local_type env lid with
  | None -> None
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

let class_ _pos msg ty =
  let rec find_class = function
    | Tclass class_ -> Some class_
    | Tdynamic pol ->
      (* While it is sound to set the class's lump policy to be the dynamic's
         (invariant) policy, we do not know if the property we are looking for
         is policied, therefore we guess that it has the lump policy and emit an
         error in case we are wrong *)
      Some { c_name = "<dynamic>"; c_self = pol; c_lump = pol }
    | Tinter tys -> List.find_map ~f:find_class tys
    | _ -> None
  in
  match find_class ty with
  | Some pty -> pty
  | None -> fail "%s" msg

let receiver_of_obj_get pos obj_ptype property =
  let msg =
    Format.asprintf "couldn't find a class for the property '%s'" property
  in
  class_ pos msg obj_ptype

(* We generate a ptype out of the property type and fill it with either the
 * purpose of property or the lump policy of some object root. *)
let property_ptype pos renv obj_ptype property property_ty =
  let class_ = receiver_of_obj_get pos obj_ptype property in
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

let shape_field_name_ this field =
  Aast.(
    match field with
    | (p, Int name) -> Ok (Ast_defs.SFlit_int (p, name))
    | (p, String name) -> Ok (Ast_defs.SFlit_str (p, name))
    | (_, Class_const ((_, _, CI x), y)) -> Ok (Ast_defs.SFclass_const (x, y))
    | (_, Class_const ((_, _, CIself), y)) ->
      (match force this with
      | Some sid -> Ok (Ast_defs.SFclass_const (sid, y))
      | None -> Error `Expected_class)
    | _ -> Error `Invalid_shape_field_name)

let shape_field_name : ptype renv_ -> _ -> Typing_defs.tshape_field_name option
    =
 fun renv ix ->
  let ix =
    (* The utility function does not expect a TAST *)
    let (_, p, e) = ix in
    (p, e)
  in
  (* TODO(T72024862): This does not support late static binding *)
  let this =
    lazy
      (match renv.re_this with
      | Some (Tclass cls) -> Some (fst ix, cls.c_name)
      | _ -> None)
  in
  match shape_field_name_ this ix with
  | Ok fld -> Some (Typing_defs.TShapeField.of_ast Pos_or_decl.of_raw_pos fld)
  | Error _ -> None

let call_special ~pos renv env args ret = function
  | StaticMethod ("\\HH\\Shapes", "idx") ->
    let (ty, key, def) =
      match args with
      | [(ty, _); (_, Some (_, key))] -> (ty, key, None)
      | [(ty, _); (_, Some (_, key)); (def, _)] -> (ty, key, Some def)
      | _ -> fail "incorrect arguments to Shapes::idx"
    in
    let key =
      match shape_field_name renv key with
      | Some k -> k
      | None -> fail "invalid shape key"
    in
    let field =
      {
        sft_policy = Env.new_policy_var renv "shape";
        sft_ty = ret;
        sft_optional = true;
      }
    in
    let tshape =
      let sh_fields = Typing_defs.TShapeMap.singleton key field in
      Tshape { sh_fields; sh_kind = Open_shape (Tinter []) }
    in
    let env =
      Env.acc env
      @@ L.(subtype ty tshape && add_dependencies [field.sft_policy] ret) ~pos
    in
    let env =
      match def with
      | None -> env
      | Some def -> Env.acc env @@ subtype ~pos def ret
    in
    Some (env, ret)
  | _ -> None

let call_regular ~pos renv env call_type name that_pty_opt args_pty ret_pty =
  let callee = Env.new_policy_var renv (name ^ "_self") in
  let env = Env.acc env (add_dependencies ~pos [callee] ret_pty) in
  let (env, callee_exn) =
    let exn = Lift.class_ty ~prefix:(name ^ "_exn") renv Decl.exception_id in
    let env = may_throw ~pos renv env (object_policy exn) exn in
    (env, exn)
  in
  (* The PC of the function being called depends on the join of the current
   * PC dependencies, as well as the function's own self policy *)
  let (env, pc_joined) =
    let pc_list = PSet.elements (Env.get_gpc renv env) in
    join_policies ~pos renv env ~prefix:"pcjoin" pc_list
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
  let result_from_fun_decl fp callable_name = function
    | { fd_kind = FDPolicied policy; fd_args } ->
      let scheme = Decl.make_callable_scheme renv policy fp fd_args in
      let prop =
        (* because cipp_scheme is created after fp they cannot
           mismatch and call_constraint will not fail *)
        Option.value_exn (Solver.call_constraint ~subtype ~pos fp scheme)
      in
      (env, prop)
    | { fd_kind = FDInferFlows; _ } ->
      let env = Env.add_dep env (Decl.callable_name_to_string callable_name) in
      (env, Chole (pos, fp))
  in
  match call_type with
  | Clocal fun_ ->
    let env = Env.acc env @@ subtype ~pos (Tfun fun_) (Tfun hole_ty) in
    (env, ret_pty)
  | Cglobal (callable_name, fty) ->
    let fp = { fp_name = name; fp_this = that_pty_opt; fp_type = hole_ty } in
    let (env, call_constraint) =
      let fun_decl = Decl.convert_fun_type renv.re_ctx fty in
      result_from_fun_decl fp callable_name fun_decl
    in
    let env = Env.acc env (fun acc -> call_constraint :: acc) in
    (env, ret_pty)
  | Cconstructor callable_name ->
    (* We don't have the function type on the TAST with constructors, so grab it from
       decl heap. *)
    let fp = { fp_name = name; fp_this = that_pty_opt; fp_type = hole_ty } in
    let (env, call_constraint) =
      match Decl.get_callable_decl renv.re_ctx callable_name with
      | Some decl -> result_from_fun_decl fp callable_name decl
      | None -> fail "Could not find %s in declarations" name
    in
    let env = Env.acc env (fun acc -> call_constraint :: acc) in
    (env, ret_pty)

let call ~pos renv env call_type that_pty_opt args ret_pty =
  let name =
    match call_type with
    | Cconstructor callable_name
    | Cglobal (callable_name, _) ->
      Decl.callable_name_to_string callable_name
    | Clocal _ -> "anonymous"
  in
  let special =
    match call_type with
    | Cglobal (cn, _) -> call_special ~pos renv env args ret_pty cn
    | _ -> None
  in
  match special with
  | Some res -> res
  | None ->
    let args_pty = List.map ~f:fst args in
    call_regular ~pos renv env call_type name that_pty_opt args_pty ret_pty

let array_like ~cow ~shape ~klass ~tuple ~dynamic ty =
  let rec search ty =
    match ty with
    | Tcow_array _ when cow -> Some ty
    | Tshape _ when shape -> Some ty
    | Tclass _ when klass -> Some ty
    | Ttuple _ when tuple -> Some ty
    | Tdynamic _ when dynamic -> Some ty
    | Tinter tys -> List.find_map tys ~f:search
    | _ -> None
  in
  search ty

let array_like_with_default ~cow ~shape ~klass ~tuple ~dynamic ~pos:_ renv ty =
  match array_like ~cow ~shape ~klass ~tuple ~dynamic ty with
  | Some ty -> ty
  | None ->
    (* The default is completely arbitrary but it should be the least
       precisely handled array structure given the search options. *)
    if dynamic then
      Tdynamic (Env.new_policy_var renv "fake_dynamic")
    else if klass then
      Tclass
        {
          c_name = "fake";
          c_self = Env.new_policy_var renv "fake_self";
          c_lump = Env.new_policy_var renv "fake_lump";
        }
    else if cow then
      Tcow_array
        {
          a_kind = Adict;
          a_key = Tprim (Env.new_policy_var renv "fake_key");
          a_value = Tprim (Env.new_policy_var renv "fake_value");
          a_length = Env.new_policy_var renv "fake_length";
        }
    else if shape then
      Tshape { sh_kind = Closed_shape; sh_fields = Typing_defs.TShapeMap.empty }
    else if tuple then
      Ttuple []
    else
      fail "`array_like_with_default` has no options turned on"

let cow_array ~pos renv ty =
  let cow_array =
    array_like_with_default
      ~cow:true
      ~shape:false
      ~klass:false
      ~tuple:false
      ~dynamic:false
      ~pos
      renv
      ty
  in
  match cow_array with
  | Tcow_array arry -> arry
  | _ -> fail "expected Hack array"

(* Deals with an assignment to a local variable or an object property *)
let assign_helper
    ?(use_pc = true) ~expr ~pos renv env (lhs_ty, _, lhs_exp) rhs_pty =
  match lhs_exp with
  | A.(Lvar (_, lid) | Dollardollar (_, lid)) ->
    let prefix = Local_id.to_string lid in
    let lhs_pty = Lift.ty ~prefix renv lhs_ty in
    let env = Env.set_local_type env lid lhs_pty in
    let env =
      if use_pc then
        let deps = PSet.elements (Env.get_lpc env) in
        Env.acc env (add_dependencies ~pos deps lhs_pty)
      else
        env
    in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | A.Obj_get (obj, (_, _, A.Id (_, property)), _, _) ->
    let (env, obj_pty) = expr env obj in
    let obj_pol = (receiver_of_obj_get pos obj_pty property).c_self in
    let lhs_pty = property_ptype pos renv obj_pty property lhs_ty in
    let pc =
      if use_pc then
        PSet.elements (Env.get_gpc renv env)
      else
        []
    in
    let deps = obj_pol :: pc in
    let env = Env.acc env (add_dependencies ~pos deps lhs_pty) in
    let env = Env.acc env (subtype ~pos rhs_pty lhs_pty) in
    env
  | _ -> env

(* Hack array accesses and mutations may throw when the indexed
   element is not in the array. may_throw_out_of_bounds_exn is
   used to register this fact. *)
let may_throw_out_of_bounds_exn ~pos renv env length_pol ix_pty =
  let exn_ty = Lift.class_ty renv Decl.out_of_bounds_exception_id in
  (* both the indexing expression and the array length influence
     whether an exception is thrown or not; indeed, the check
     performed by the indexing is of the form:
       if ($ix >= $arry->length) { throw ...; } *)
  let pc_deps = PSet.add length_pol (object_policy ix_pty) in
  may_throw ~pos renv env pc_deps exn_ty

let int_of_exp ix_exp =
  match ix_exp with
  | (_, _, A.Int i) -> int_of_string i
  | _ -> fail "expected an integer literal while indexing a tuple"

let nth_tuple_pty ptys ix_exp =
  let ix = int_of_exp ix_exp in
  match List.nth ptys ix with
  | Some pty -> pty
  | None -> fail "tuple arity is too little"

let overwrite_nth_pty tuple_pty ix_exp pty =
  let ix = int_of_exp ix_exp in
  match tuple_pty with
  | Ttuple ptys ->
    let ptys = List.take ptys ix @ (pty :: List.drop ptys (ix + 1)) in
    Ttuple ptys
  | _ -> fail "policy type overwrite expected a tuple"

(* A wrapper around assign_helper that deals with Hack arrays' syntactic
   sugar; this accounts for the CoW semantics of Hack arrays:

     $x->p[4][] = "hi";

   does not mutate an array cell, but the property p of
   the object $x instead. *)
let rec assign
    ?(use_pc = true) ~expr ~pos renv env ((lhs_ty, _, lhs_expr_) as lhs) rhs_pty
    =
  match lhs_expr_ with
  | A.Array_get (((arry_ty, _, _) as arry_exp), ix_opt) ->
    let handle_collection
        env ~should_skip_exn old_array new_array key value length =
      (* TODO(T68269878): track flows due to length changes *)
      let env = Env.acc env (subtype ~pos old_array new_array) in

      (* Evaluate the index *)
      let (env, ix_pty_opt) =
        match ix_opt with
        | Some ix ->
          let (env, ix_pty) = expr env ix in
          (* The index flows to they key of the collection *)
          let env = Env.acc env (subtype ~pos ix_pty key) in
          (env, Some ix_pty)
        | None -> (env, None)
      in

      (* Potentially raise `OutOfBoundsException` *)
      let env =
        match ix_pty_opt with
        (* When there is no index, we add a new element, hence no exception. *)
        | None -> env
        (* Dictionaries don't throw on assignment, they register a new key. *)
        | Some _ when should_skip_exn -> env
        | Some ix_pty -> may_throw_out_of_bounds_exn ~pos renv env length ix_pty
      in

      (* Do the assignment *)
      let env = Env.acc env (subtype ~pos rhs_pty value) in
      (env, true)
    in
    (* Evaluate the array *)
    let (env, old_arry_pty) = expr env arry_exp in
    let new_arry_pty = Lift.ty ~prefix:"arr" renv arry_ty in
    (* If the array is in an intersection, we pull it out and if we cannot
       find find something suitable return a fake type to make the analysis go
       through. *)
    let new_arry_pty =
      array_like_with_default
        ~cow:true
        ~shape:true
        ~klass:true
        ~tuple:true
        ~dynamic:false
        ~pos
        renv
        new_arry_pty
    in

    let (env, use_pc) =
      match new_arry_pty with
      | Tcow_array arry ->
        let should_skip_exn = equal_array_kind arry.a_kind Adict in
        handle_collection
          env
          ~should_skip_exn
          old_arry_pty
          new_arry_pty
          arry.a_key
          arry.a_value
          arry.a_length
      | Tshape { sh_kind; sh_fields } -> begin
        match Option.(ix_opt >>= shape_field_name renv) with
        | Some key ->
          (* The key can only be a literal (int, string) or class const, so
             it is always public *)
          let p = Env.new_policy_var renv "field" in
          let pc = Env.get_lpc env |> PSet.elements in
          let env =
            Env.acc env @@ L.(pc <* [p] && add_dependencies pc rhs_pty) ~pos
          in
          let sh_fields =
            Typing_defs.TShapeMap.add
              key
              { sft_optional = false; sft_policy = p; sft_ty = rhs_pty }
              sh_fields
          in
          let tshape = Tshape { sh_kind; sh_fields } in
          let env = Env.acc env (subtype ~pos tshape new_arry_pty) in
          (env, false)
        | None -> (env, true)
      end
      | Ttuple _ ->
        let ix =
          match ix_opt with
          | Some ix_exp -> ix_exp
          | _ -> fail "indexed tuple assignment requires an index"
        in
        let tuple_pty = overwrite_nth_pty old_arry_pty ix rhs_pty in
        let env = Env.acc env (subtype ~pos tuple_pty new_arry_pty) in
        (env, true)
      | Tclass { c_name; c_lump; c_self } ->
        let should_skip_exn = String.equal c_name "\\HH\\Map" in
        (* The key is an arraykey so it will always be a Tprim *)
        let key_pty = Tprim c_lump in
        let env = Env.acc env (add_dependencies ~pos [c_self] key_pty) in
        (* The value is constructed as if a property access is made *)
        let value_pty = Lift.ty ~lump:c_lump renv lhs_ty in
        let env = Env.acc env (add_dependencies ~pos [c_self] value_pty) in
        handle_collection
          env
          ~should_skip_exn
          old_arry_pty
          new_arry_pty
          key_pty
          value_pty
          c_lump
      | _ -> fail "the default type for array assignment is not handled"
    in

    (* assign the array/shape to itself *)
    assign ~use_pc ~expr ~pos renv env arry_exp new_arry_pty
  | _ -> assign_helper ~use_pc ~expr ~pos renv env lhs rhs_pty

let seq ~run (env, out) x =
  let (out, next_opt) = Env.strip_cont out K.Next in
  match next_opt with
  | None ->
    (* we are looking at dead code *)
    (env, out)
  | Some cont ->
    let env = Env.prep_stmt env cont in
    let (env, out_exceptional) = run env x in
    (* we have to merge the exceptional outcomes from
        before `x` with the ones after `x` *)
    (env, Env.merge_out out_exceptional out)

(* Generate flow constraints for an expression *)
let rec expr ~pos renv (env : Env.expr_env) ((ety, epos, e) : Tast.expr) =
  let expr = expr ~pos renv in
  let expr_with_deps env deps e =
    Env.expr_with_pc_deps env deps (fun env -> expr env e)
  in
  let add_element_dependencies env exprs element_pty =
    let mk_element_subtype env exp =
      let (env, pty) = expr env exp in
      Env.acc env (subtype ~pos pty element_pty)
    in
    List.fold ~f:mk_element_subtype ~init:env exprs
  in
  (* Any copy-on-write collection with only values such as vec, keyset,
     ImmVector. *)
  let cow_array_literal ~prefix exprs =
    let arry_pty = Lift.ty ~prefix renv ety in
    let element_pty = (cow_array ~pos renv arry_pty).a_value in
    (* Each element is a subtype of the collection's value. *)
    let env = add_element_dependencies env exprs element_pty in
    (env, arry_pty)
  in
  (* Anything class that is a Collection, but in particular Vector and Set. *)
  let mut_array_literal ~prefix exprs =
    let class_pty = Lift.ty ~prefix renv ety in
    let class_ = class_ pos "mutable array literal is not a class" class_pty in
    let rec find_element_ty ty =
      match T.get_node ty with
      | T.Tclass (_, _, [element_ty]) -> Some element_ty
      | T.Tintersection tys -> List.find_map tys ~f:find_element_ty
      | _ -> None
    in
    let element_pty =
      match find_element_ty ety with
      | Some element_ty -> Lift.ty ~lump:class_.c_lump renv element_ty
      | None -> Tprim (Env.new_policy_var renv "fake_element")
    in
    let env = Env.acc env (add_dependencies ~pos [class_.c_self] element_pty) in
    (* Each element is a subtype of the collection's value. *)
    let env = add_element_dependencies env exprs element_pty in
    (env, class_pty)
  in
  let add_key_value_dependencies env fields key_pty value_pty =
    let mk_field_subtype env (key, value) =
      let subtype = subtype ~pos in
      let (env, key_pty') = expr env key in
      let (env, value_pty') = expr env value in
      Env.acc env @@ fun acc ->
      acc |> subtype key_pty' key_pty |> subtype value_pty' value_pty
    in
    List.fold ~f:mk_field_subtype ~init:env fields
  in
  (* Any copy-on-write collection with keys and values such as dict, ImmMap,
     ConstMap. *)
  let cow_keyed_array_literal ~prefix fields =
    let dict_pty = Lift.ty ~prefix renv ety in
    let arr = cow_array ~pos renv dict_pty in
    (* Each field is a subtype of collection keys and values. *)
    let env = add_key_value_dependencies env fields arr.a_key arr.a_value in
    (env, dict_pty)
  in
  (* Any class that is a KeyedCollection, but in particular Map. *)
  let mut_keyed_array_literal ~prefix fields =
    (* Each element of the array is a subtype of the array's value parameter. *)
    let class_pty = Lift.ty ~prefix renv ety in
    let class_ = class_ pos "mutable array literal is not a class" class_pty in
    let rec find_key_value_tys ty =
      match T.get_node ty with
      | T.Tclass (_, _, [key_ty; value_ty]) -> Some (key_ty, value_ty)
      | T.Tintersection tys -> List.find_map tys ~f:find_key_value_tys
      | _ -> None
    in
    let (key_pty, value_pty) =
      match find_key_value_tys ety with
      | Some (key_ty, value_ty) ->
        ( Lift.ty ~lump:class_.c_lump renv key_ty,
          Lift.ty ~lump:class_.c_lump renv value_ty )
      | None ->
        ( Tprim (Env.new_policy_var renv "fake_key"),
          Tprim (Env.new_policy_var renv "fake_value") )
    in
    let env = Env.acc env (add_dependencies ~pos [class_.c_self] key_pty) in
    let env = Env.acc env (add_dependencies ~pos [class_.c_self] value_pty) in
    (* Each field is a subtype of collection keys and values. *)
    let env = add_key_value_dependencies env fields key_pty value_pty in
    (env, class_pty)
  in
  let funargs env =
    let f env (pk, e) =
      let (env, ty) = expr env e in
      (env, (ty, Some (pk, e)))
    in
    List.map_env env ~f
  in
  match e with
  | A.Null -> (env, Tnull (Env.new_policy_var renv "null"))
  | A.True
  | A.False
  | A.Int _
  | A.Float _
  | A.String _ ->
    (* literals are public *)
    (env, Tprim (Env.new_policy_var renv "lit"))
  | A.(Binop { bop = Ast_defs.Eq None; lhs; rhs }) ->
    let (env, ty2) = expr env rhs in
    let env = assign ~expr ~pos renv env lhs ty2 in
    (env, ty2)
  | A.(Binop { bop = Ast_defs.Eq (Some op); lhs; rhs }) ->
    (* it is simpler to create a fake expression lhs = lhs op rhs to
       make sure all operations (e.g., ??) are handled correctly *)
    expr
      env
      ( ety,
        epos,
        A.(
          Binop
            {
              bop = Ast_defs.Eq None;
              lhs;
              rhs = (ety, epos, Binop { bop = op; lhs; rhs });
            }) )
  | A.(Binop { bop = Ast_defs.QuestionQuestion; lhs = e1; rhs = e2 })
  | A.Eif (e1, None, e2) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr_with_deps env (object_policy ty1) e2 in
    let ty = Lift.ty ~prefix:"qq" renv ety in
    let null_policy = Env.new_policy_var renv "nullqq" in
    let env =
      Env.acc env
      @@ L.(
           subtype ty1 (Tunion [ty; Tnull null_policy])
           && subtype ty2 ty
           && add_dependencies [null_policy] ty)
           ~pos
    in
    (env, ty)
  | A.Eif (e1, Some e2, e3) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr_with_deps env (object_policy ty1) e2 in
    let (env, ty3) = expr_with_deps env (object_policy ty1) e3 in
    let ty = Lift.ty ~prefix:"eif" renv ety in
    let env = Env.acc env (L.(subtype ty2 ty && subtype ty3 ty) ~pos) in
    (env, ty)
  | A.(
      Binop
        {
          bop =
            Ast_defs.(
              ( Plus | Minus | Star | Slash | Starstar | Percent | Ltlt | Gtgt
              | Xor | Amp | Bar | Dot ));
          lhs = e1;
          rhs = e2;
        }) ->
    (* arithmetic and bitwise operations all take primitive types
       and return a primitive type; string concatenation (Dot) is
       also handled here although it might need special casing
       because of HH\FormatString<T> (TODO) *)
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr env e2 in
    let ty = Tprim (Env.new_policy_var renv "arith") in
    let env = Env.acc env (subtype ~pos ty1 ty) in
    let env = Env.acc env (subtype ~pos ty2 ty) in
    (env, ty)
  | A.(
      Binop
        {
          bop =
            Ast_defs.(
              ( Eqeqeq | Diff2 | Barbar | Ampamp | Eqeq | Diff | Lt | Lte | Gt
              | Gte | Cmp )) as op;
          lhs = e1;
          rhs = e2;
        }) ->
    let (env, ty1) = expr env e1 in
    let (env, ty2) = expr env e2 in
    let deps =
      let gather_policies =
        match op with
        | Ast_defs.(Eqeqeq | Diff2 | Barbar | Ampamp) ->
          (* === and !== check for object identity, the resulting
             boolean is thus governed by the object policies of
             the two operands
             || and && are also safely handled by object_policy
             since they either look at a boolean value or test
             if an object is null *)
          object_policy
        | _ ->
          (* other comparison operators can inspect mutable fields
             when applied to DateTime objects, so we conservatively
             use all policies of both arguments *)
          fun t ->
           let (cov, inv, cnt) = policy_occurrences t in
           PSet.union cov (PSet.union inv cnt)
      in
      PSet.union (gather_policies ty1) (gather_policies ty2)
    in
    let (env, cmp_policy) =
      join_policies ~pos ~prefix:"cmp" renv env (PSet.elements deps)
    in
    (env, Tprim cmp_policy)
  | A.Unop (op, e) -> begin
    match op with
    (* Operators that mutate *)
    | Ast_defs.Uincr
    | Ast_defs.Udecr
    | Ast_defs.Upincr
    | Ast_defs.Updecr ->
      (* register constraints that'd be generated if
         e = e +/- 1 were being analyzed *)
      let (env, tye) = expr env e in
      let env = assign ~expr ~pos renv env e tye in
      (env, tye)
    (* Prim operators that don't mutate *)
    | Ast_defs.Unot ->
      let (env, tye) = expr env e in
      (* use object_policy to account for both booleans
         and null checks *)
      let (env, not_policy) =
        join_policies
          ~prefix:"not"
          ~pos
          renv
          env
          (PSet.elements (object_policy tye))
      in
      (env, Tprim not_policy)
    | Ast_defs.Utild
    | Ast_defs.Uplus
    | Ast_defs.Uminus ->
      expr env e
    | Ast_defs.Usilence -> expr env e
  end
  | A.Lvar (_pos, lid) -> refresh_local_type ~pos renv env lid ety
  | A.Obj_get (obj, (_, _, A.Id (_, property)), _, _) ->
    let (env, obj_ptype) = expr env obj in
    let prop_pty = property_ptype pos renv obj_ptype property ety in
    let prefix = "." ^ property in
    let (env, super_pty) =
      adjust_ptype ~pos ~prefix ~adjustment:Aweaken renv env prop_pty
    in
    let obj_pol = (receiver_of_obj_get pos obj_ptype property).c_self in
    let env = Env.acc env (add_dependencies ~pos [obj_pol] super_pty) in
    (env, super_pty)
  | A.This ->
    (match renv.re_this with
    | Some ptype -> (env, ptype)
    | None -> fail "encountered $this outside of a class context")
  | A.ET_Splice e
  | A.ExpressionTree { A.et_runtime_expr = e; _ } ->
    (* TODO: IFC should consider spliced values too *)
    expr env e
  (* TODO(T68414656): Support calls with type arguments *)
  | A.(Call { func; args; _ }) ->
    let fty = Tast.get_type func in
    let (env, args_pty) = funargs env args in
    let ret_pty = Lift.ty ~prefix:"ret" renv ety in
    let call env call_type this_pty =
      call ~pos renv env call_type this_pty args_pty ret_pty
    in
    begin
      match func with
      (* Generally a function call *)
      | (_, _, A.Id (_, name)) ->
        let call_id = Decl.make_callable_name ~is_static:false None name in
        call env (Cglobal (call_id, fty)) None
      (* Regular method call *)
      | (_, _, A.Obj_get (obj, (_, _, A.Id (_, meth_name)), _, _)) ->
        let (env, obj_pty) = expr env obj in
        begin
          match obj_pty with
          | Tclass { c_name; _ } ->
            let call_id =
              Decl.make_callable_name ~is_static:false (Some c_name) meth_name
            in
            call env (Cglobal (call_id, fty)) (Some obj_pty)
          | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
        end
      (* Static method call*)
      | (_, _, A.Class_const ((ty, _, cid), (_, meth_name))) ->
        let env =
          match cid with
          | A.CIexpr e -> fst @@ expr env e
          | A.CIstatic ->
            (* TODO(T72024862): Handle late static binding *)
            env
          | _ -> env
        in
        let find_class_name ty =
          match T.get_node ty with
          | T.Tclass ((_, class_name), _, _) -> class_name
          | _ -> fail "unhandled method call on a non-class"
        in
        let class_name = find_class_name ty in
        let call_id =
          Decl.make_callable_name ~is_static:true (Some class_name) meth_name
        in
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
        call env (Cglobal (call_id, fty)) this_pty
      | _ ->
        let (env, func_ty) = expr env func in
        let ifc_fty =
          match func_ty with
          | Tfun fty -> fty
          | _ -> failwith "calling something that is not a function"
        in
        call env (Clocal ifc_fty) None
    end
  | A.FunctionPointer (id, _) ->
    let ty = Lift.ty renv ety in
    let fty =
      match ty with
      | Tfun fty -> fty
      | _ -> fail "FunctionPointer does not have a function type"
    in
    let ctype =
      match id with
      | A.FP_id (_, name) ->
        let call_id = Decl.make_callable_name ~is_static:false None name in
        Cglobal (call_id, ety)
      | A.FP_class_const _ ->
        (* TODO(T72024862): Handle late static binding *)
        Clocal fty
    in
    (* Act as though we are defining a lambda that wraps a call to the function
        pointer. *)
    let renv = { renv with re_gpc = fty.f_pc; re_exn = fty.f_exn } in
    let args = List.map ~f:(fun t -> (t, None)) fty.f_args in
    let (env, _ret_ty) = call ~pos renv env ctype None args fty.f_ret in
    (env, ty)
  | A.Varray (_, exprs) -> cow_array_literal ~prefix:"varray" exprs
  | A.ValCollection
      ((_, ((A.Vec | A.Keyset | A.ImmSet | A.ImmVector) as kind)), _, exprs) ->
    let prefix = A.show_vc_kind kind in
    cow_array_literal ~prefix exprs
  | A.ValCollection ((_, ((A.Vector | A.Set) as kind)), _, exprs) ->
    let prefix = A.show_vc_kind kind in
    mut_array_literal ~prefix exprs
  | A.Darray (_, fields) -> cow_keyed_array_literal ~prefix:"darray" fields
  | A.KeyValCollection ((_, ((A.Dict | A.ImmMap) as kind)), _, fields) ->
    let prefix = A.show_kvc_kind kind in
    cow_keyed_array_literal ~prefix fields
  | A.KeyValCollection ((_, (A.Map as kind)), _, fields) ->
    let prefix = A.show_kvc_kind kind in
    mut_keyed_array_literal ~prefix fields
  | A.Array_get (arry, ix_opt) ->
    (* Evaluate the array *)
    let (env, arry_pty) = expr env arry in
    let arry =
      array_like_with_default
        ~cow:true
        ~shape:true
        ~klass:true
        ~tuple:true
        ~dynamic:false
        ~pos
        renv
        arry_pty
    in

    (* Evaluate the index, it might have side-effects! *)
    let (env, ix_exp, ix_pty) =
      match ix_opt with
      | Some ix ->
        let (env, ty) = expr env ix in
        (env, ix, ty)
      | None -> fail "cannot have an empty index when reading"
    in
    begin
      match arry with
      | Tcow_array arry ->
        (* The index flows into the array key which flows into the array value *)
        let env = Env.acc env @@ subtype ~pos ix_pty arry.a_key in

        let env =
          may_throw_out_of_bounds_exn ~pos renv env arry.a_length ix_pty
        in

        (env, arry.a_value)
      | Tshape { sh_fields; _ } ->
        let sft =
          Option.(
            shape_field_name renv ix_exp >>= fun f ->
            Typing_defs.TShapeMap.find_opt f sh_fields)
        in
        begin
          match sft with
          | Some { sft_ty; _ } -> (env, sft_ty)
          | None -> (env, Lift.ty renv ety)
        end
      | Ttuple ptys ->
        let indexed_pty = nth_tuple_pty ptys ix_exp in
        (env, indexed_pty)
      | Tclass { c_self = self; c_lump = lump; _ } ->
        let value_pty = Lift.ty ~lump renv ety in
        let env = Env.acc env (add_dependencies ~pos [self] value_pty) in
        (* Collection keys are arraykey's which are Tprims. *)
        let key_pty = Tprim lump in
        let env = Env.acc env (add_dependencies ~pos [self] key_pty) in
        (* Indexing expression flows into the key policy type of the collection . *)
        let env = Env.acc env @@ subtype ~pos ix_pty key_pty in
        (* Join of lump and self governs the length of the collection as well. *)
        let (env, join_pol) = join_policies ~pos renv env [lump; self] in
        let env = may_throw_out_of_bounds_exn ~pos renv env join_pol ix_pty in
        (env, value_pty)
      | _ -> fail "the default type for array access is not handled"
    end
  | A.New ((lty, _, cid), _targs, args, _extra_args, _) ->
    (* TODO(T70139741): support variadic functions and constructors
     * TODO(T70139893): support classes with type parameters
     *)
    let (env, args_pty) =
      funargs env (List.map ~f:(fun e -> (Ast_defs.Pnormal, e)) args)
    in
    let env =
      match cid with
      | A.CIexpr e -> fst @@ expr env e
      | A.CIstatic ->
        (* TODO(T72024862): Handle late static binding *)
        env
      | _ -> env
    in
    let obj_pty = Lift.ty renv lty in
    begin
      match obj_pty with
      | Tclass { c_name; _ } ->
        let call_id =
          Decl.make_callable_name
            ~is_static:false
            (Some c_name)
            Decl.construct_id
        in
        let lty = T.mk (Typing_reason.Rnone, T.Tprim A.Tvoid) |> Lift.ty renv in
        let (env, _) =
          call ~pos renv env (Cconstructor call_id) (Some obj_pty) args_pty lty
        in
        (env, obj_pty)
      | _ -> fail "unhandled method call on %a" Pp.ptype obj_pty
    end
  | A.Efun { A.ef_fun = fun_; ef_use = captured_ids; _ }
  | A.Lfun (fun_, captured_ids) ->
    (* weaken all the captured local variables and reset the local
       pc to create the initial continuation we'll use to check the
       lambda literal *)
    let (env, start_cont) =
      let (env, k_vars) =
        let lids =
          List.map ~f:(fun (_, (_, id)) -> id) captured_ids |> LSet.of_list
        in
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
  | A.Pair (_, e0, e1) ->
    let (env, ptys) = List.map_env env [e0; e1] ~f:expr in
    (env, Ttuple ptys)
  | A.List es ->
    let (env, ptys) = List.map_env env es ~f:expr in
    (env, Ttuple ptys)
  | A.Tuple es ->
    let (env, ptys) = List.map_env env es ~f:expr in
    (env, Ttuple ptys)
  | A.Pipe ((_, dollardollar), e1, e2) ->
    let (env, t1) = expr env e1 in
    let dd_old = Env.get_local_type env dollardollar in
    let env = Env.set_local_type env dollardollar t1 in
    let (env, t2) = expr env e2 in
    let env = Env.set_local_type_opt env dollardollar dd_old in
    (env, t2)
  | A.Dollardollar (_, lid) -> refresh_local_type ~pos renv env lid ety
  | A.Shape s ->
    let p = Env.new_policy_var renv "field" in
    let pc = Env.get_lpc env |> PSet.elements in
    let env = Env.acc env @@ L.(pc <* [p]) ~pos in
    let f (env, m) (key, e) =
      let (env, t) = expr env e in
      let sft = { sft_ty = t; sft_optional = false; sft_policy = p } in
      ( env,
        Typing_defs.TShapeMap.add
          (Typing_defs.TShapeField.of_ast Pos_or_decl.of_raw_pos key)
          sft
          m )
    in
    let (env, sh_fields) =
      List.fold ~f ~init:(env, Typing_defs.TShapeMap.empty) s
    in
    (env, Tshape { sh_kind = Closed_shape; sh_fields })
  | A.Is (e, _hint) ->
    let (env, ety) = expr env e in
    (* whether an object has one tag or another is governed
       by its object policy *)
    let (env, tag_policy) =
      let tag_policy_list = PSet.elements (object_policy ety) in
      join_policies ~pos ~prefix:"tag" renv env tag_policy_list
    in
    (env, Tprim tag_policy)
  | A.As (e, _hint, is_nullable) ->
    let (env, ty) = expr env e in
    let tag_policies = object_policy ty in
    let (env, tag_test_ty) =
      if is_nullable then
        (* The 'e ?as Thint' construct can be seen as
             ((e is Thint) ? e : null) as ?Thint
           that is, we can see the construct as *first* setting e's
           value to null if it is not a subtype of Thint, *then*
           performing a refinement with ?Thint. Note that the 'as'
           refinement above will always succeed because the ternary
           evaluates either to e that has type Thint (<: ?Thint), or
           null that has type null (<: ?Thint).

           The result of this as refinement is in ety, so here, we
           construct the type of ternary expression. *)
        let (env, tag_policy) =
          join_policies ~pos ~prefix:"tag" renv env (PSet.elements tag_policies)
        in
        (env, Tunion [ty; Tnull tag_policy])
      else
        let exn = Lift.class_ty ~prefix:"as" renv Decl.exception_id in
        let env = may_throw ~pos renv env tag_policies exn in
        (env, ty)
    in
    (env, refine renv tag_test_ty pos ety)
  (* --- A valid AST does not contain these nodes *)
  | A.Import _
  | A.Collection _ ->
    failwith "AST should not contain these nodes"
  (* --- expressions below are not yet supported *)
  | _ -> (env, Lift.ty renv ety)

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
        (fun var (_, hint) env ->
          match get_local_type ~pos env var with
          | None -> env
          | Some pty ->
            let new_pty = refine renv pty pos hint in
            Env.set_local_type env var new_pty)
        tymap
        env
    in
    Env.close_stmt env K.Next
  | A.Awaitall (el, b) ->
    let (env, out) =
      List.fold_left el ~init:(env, KMap.empty) ~f:(fun (env, out) (lid, e) ->
          let expr = expr_ ~pos renv in
          let (env, ety) = expr (Env.prep_expr env) e in
          let env =
            let (pty, p, _) = e in
            assign ~expr ~pos renv env (pty, p, A.Lvar lid) ety
          in
          let (env, ethrow) = Env.close_expr env in
          (env, Env.merge_out out ethrow))
    in
    let (env, out1) = block renv env b in
    let out = Env.merge_out out out1 in
    (env, out)
  | A.Expr e ->
    let (env, _ety, ethrow) = expr renv env e in
    Env.close_stmt ~merge:ethrow env K.Next
  | A.If (cond, b1, b2) ->
    let (_, pos, _) = cond in
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
    let (_, pos, _) = cond in
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
  | A.For (inits, cond_opt, incrs, (_, A.AssertEnv (A.Join, tymap)) :: blk) ->
    (* Helper that evaluates an expression, but discards its type *)
    let discarding_expr env ((_, pos, _) as exp) =
      let (env, _, out_exceptional) = expr ~pos renv env exp in
      Env.close_stmt ~merge:out_exceptional env K.Next
    in

    (* Evaluate the initialisers of the loop *)
    let init = Env.close_stmt env K.Next in
    let (env, out) = List.fold_left ~f:(seq ~run:discarding_expr) ~init inits in
    let env = Env.prep_stmt env (KMap.find K.Next out) in

    (* Use the position of the condition if awailable; entire loop's position
       otherwise. *)
    let pos =
      match cond_opt with
      | Some (_, cond_pos, _) -> cond_pos
      | None -> pos
    in

    let env = refresh_with_tymap ~pos renv env tymap in
    let beg_locals = Env.get_locals env in

    (* TODO: pc_pols should also flow into cty because the condition is evaluated
       unconditionally only the first time around the loop. *)
    let (env, pc_policies, cthrow) =
      match cond_opt with
      | Some cond ->
        let (env, cty, cthrow) = expr ~pos renv env cond in
        (env, object_policy cty, cthrow)
      | None -> (env, PSet.empty, KMap.empty)
    in

    Env.with_pc_deps env pc_policies @@ fun env ->
    let tainted_lpc = Env.get_lpc env in
    let (env, out_blk) = block renv env blk in
    let out_blk = Env.merge_out out_blk cthrow in
    let out_blk = Env.merge_in_next out_blk K.Continue in

    (* Handle loop increments *)
    let init = (env, out_blk) in
    let (env, out) = List.fold_left ~f:(seq ~run:discarding_expr) ~init incrs in

    let env = loop_the_env ~pos env out beg_locals in
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
    let (_, pos, _) = collection in
    let env = refresh_with_tymap ~pos renv env tymap in
    let beg_locals = Env.get_locals env in

    (* TODO: pc should also flow into cty because the condition is evaluated
       unconditionally only the first time around the loop. *)
    let (env, collection_pty, cthrow) = expr ~pos renv env collection in
    let collection_pty =
      array_like_with_default
        ~cow:true
        ~shape:false
        ~klass:true
        ~tuple:false
        ~dynamic:true
        ~pos
        renv
        collection_pty
    in
    let env = Env.prep_expr env in
    let expr = expr_ ~pos renv in
    let (env, pc_policies) =
      match as_exp with
      | Aast.As_v ((value_ty, _, _) as value)
      | Aast.Await_as_v (_, ((value_ty, _, _) as value)) ->
        let (env, value_pty, pc_policies) =
          match collection_pty with
          | Tcow_array arry -> (env, arry.a_value, PSet.singleton arry.a_length)
          | Tclass class_ ->
            (* TODO(T80403715): Ensure during class definition that Iterable
               instances behave sensibly. Otherwise, this treatment is unsound. *)
            (* Value is, in effect, a field access, hence governed by the
               lump policy of the class. *)
            let value_pty = Lift.ty ~lump:class_.c_lump renv value_ty in
            (* Length, key, and value are governed by the self policy if the
               class is in fact a Hack array and by the lump policy otherwise. *)
            let cl_pols = [class_.c_self; class_.c_lump] in
            let env = Env.acc env (add_dependencies ~pos cl_pols value_pty) in
            (env, value_pty, PSet.of_list cl_pols)
          | Tdynamic dyn_pol ->
            (* Deconstruction of dynamic also produces dynamic *)
            (env, collection_pty, PSet.singleton dyn_pol)
          | _ -> fail "Collection is neither a class nor a cow array"
        in
        let env = assign_helper ~expr ~pos renv env value value_pty in
        (env, pc_policies)
      | Aast.As_kv (((key_ty, _, _) as key), ((value_ty, _, _) as value))
      | Aast.Await_as_kv
          (_, ((key_ty, _, _) as key), ((value_ty, _, _) as value)) ->
        let (env, key_pty, value_pty, pc_policies) =
          match collection_pty with
          | Tcow_array arry ->
            (env, arry.a_key, arry.a_value, PSet.singleton arry.a_length)
          | Tclass class_ ->
            (* TODO(T80403715): Ensure during class definition that Iterable
               instances behave sensibly. Otherwise, this treatment is unsound. *)
            (* Key and value are, in effect, a field accesses, hence governed
               by the lump policy of the class. *)
            let key_pty = Lift.ty ~lump:class_.c_lump renv key_ty in
            let value_pty = Lift.ty ~lump:class_.c_lump renv value_ty in
            (* Length, key, and value are governed by the self policy if the
               class is in fact a Hack array and by the lump policy otherwise. *)
            let cl_pols = [class_.c_self; class_.c_lump] in
            let env = Env.acc env (add_dependencies ~pos cl_pols value_pty) in
            let env = Env.acc env (add_dependencies ~pos cl_pols key_pty) in
            (env, key_pty, value_pty, PSet.of_list cl_pols)
          | Tdynamic dyn_pol ->
            (* Deconstruction of dynamic also produces dynamic *)
            (env, collection_pty, collection_pty, PSet.singleton dyn_pol)
          | _ -> fail "Collection is neither a class nor a cow array"
        in
        let env = assign_helper ~expr ~pos renv env key key_pty in
        let env = assign_helper ~expr ~pos renv env value value_pty in
        (env, pc_policies)
    in

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
  | A.Switch (e, cl, dfl) ->
    let (_, pos, _) = e in
    let (env, ety, ethrow) = expr ~pos renv env e in
    let (env, out_cond) = Env.close_stmt ~merge:ethrow env K.Fallthrough in
    let case_gen (env, (out, deps)) c =
      let out = Env.merge_out out_cond out in
      let (out, ft_cont_opt) = Env.strip_cont out K.Fallthrough in
      (* out_cond has a Fallthrough so ft_cont_opt cannot be None *)
      let env = Env.prep_stmt env (Option.value_exn ft_cont_opt) in
      Env.with_pc_deps env deps @@ fun env ->
      let (env, out, new_deps, b) =
        match c with
        | Aast.Default (_, b) -> (env, out, PSet.empty, b)
        | Aast.Case (e, b) ->
          let (_, pos, _) = e in
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
    let case state c = case_gen state (Aast.Case c) in
    let default_case state c = case_gen state (Aast.Default c) in
    let (env, (out, _final_deps)) =
      let initial_deps = object_policy ety in
      let state = (env, (out_cond, initial_deps)) in
      let state = List.fold ~f:case ~init:state cl in
      let state = Option.fold ~f:default_case ~init:state dfl in
      state
    in
    let out = Env.merge_in_next out K.Continue in
    let out = Env.merge_in_next out K.Break in
    let out = Env.merge_in_next out K.Fallthrough in
    let out = clear_pc_deps out in
    (env, out)
  | A.Noop -> Env.close_stmt env K.Next
  (* --- These nodes do not appear after naming *)
  | A.Block _
  | A.Markup _ ->
    failwith
      "Unexpected nodes in AST. These nodes should have been removed in naming."
  (* --- These nodes are not yet supported *)
  | _ -> Env.close_stmt env K.Next

and block renv env (blk : Tast.block) =
  let init = Env.close_stmt env K.Next in
  List.fold_left ~f:(seq ~run:(stmt renv)) ~init blk

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
    ~ctx
    name
    params
    body
    return =
  try
    (* Setup the read-only environment *)
    let scope = Scope.alloc () in
    let renv = Env.new_renv scope decl_env saved_env ctx in

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
    if should_print ~user_mode:opts.opt_mode ~phase:Manalyse then begin
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

    let callable_name = Decl.make_callable_name ~is_static class_name name in
    let callable_name_str = Decl.callable_name_to_string callable_name in
    let f_self = Env.new_policy_var renv callable_name_str in
    let proto =
      {
        fp_name = callable_name_str;
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
      match Decl.get_callable_decl renv.re_ctx callable_name with
      | Some { fd_kind = FDPolicied policy; fd_args } ->
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
  with
  | IFCError (FlowInference s) ->
    if should_print ~user_mode:opts.opt_mode ~phase:Manalyse then
      Format.printf "Analyzing %s:@.  Failure: %s@.@." name s;
    None

let walk_tast opts decl_env ctx =
  let def = function
    | A.Fun fd ->
      let (_, name) = fd.A.fd_name in
      let {
        A.f_annotation = saved_env;
        f_params = params;
        f_body = body;
        f_ret = (return, _);
        f_span = pos;
        _;
      } =
        fd.A.fd_fun
      in
      let is_static = false in
      let callable_res =
        analyse_callable
          ~opts
          ~pos
          ~decl_env
          ~is_static
          ~saved_env
          ~ctx
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
          ~ctx
          name
          params
          body
          return
      in
      Some (List.filter_map ~f:handle_method methods)
    | _ -> None
  in
  (fun tast -> List.concat (List.filter_map ~f:def tast))

let check_valid_flow _opts _ (_result, _implicit, _simple) = ()

let simplify result =
  let pred = const true in
  ( result,
    result.res_entailment result.res_constraint,
    Logic.simplify @@ Logic.quantify ~pred ~quant:Qexists result.res_constraint
  )

let get_solver_result results =
  try Ifc_solver.global_exn ~subtype results with
  | Ifc_solver.Error Ifc_solver.RecursiveCycle ->
    fail "solver error: cyclic call graph"
  | Ifc_solver.Error (Ifc_solver.MissingResults callable) ->
    fail "solver error: missing results for callable '%s'" callable
  | Ifc_solver.Error (Ifc_solver.InvalidCall (caller, callee)) ->
    fail "solver error: invalid call to '%s' in '%s'" callee caller

let check opts tast ctx =
  (* Declaration phase *)
  let decl_env = Decl.collect_sigs tast in
  if should_print ~user_mode:opts.opt_mode ~phase:Mdecl then
    Format.printf "%a@." Pp.decl_env decl_env;

  (* Flow analysis phase *)
  let results = walk_tast opts decl_env ctx tast in

  (* Solver phase *)
  let results = get_solver_result results in

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

  SMap.iter (check_valid_flow opts) simplified_results
