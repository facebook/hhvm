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
open Utils
open Typing_defs
open Typing_env_types
open Typing_logic_helpers
module Reason = Typing_reason
module Env = Typing_env
module Inter = Typing_intersection
module TUtils = Typing_utils
module SN = Naming_special_names
module Phase = Typing_phase
module TL = Typing_logic
module Cls = Decl_provider.Class
module ITySet = Internal_type_set
module MakeType = Typing_make_type
module Nast = Aast

(* We maintain a "visited" set for subtype goals. We do this only
 * for goals of the form T <: t or t <: T where T is a generic parameter,
 * as this is the more common case.
 * T83096774: work out how to do this *efficiently* for all subtype goals.
 *
 * Here's a non-trivial example (assuming a contravariant type Contra).
 * Under assumption T <: Contra<Contra<T>> show T <: Contra<T>.
 * This leads to cycle of implications
 *    T <: Contra<T> =>
 *    Contra<Contra<T>> <: Contra<T> =>
 *    T <: Contra<T>
 * at which point we are back at the original goal.
 *
 * Note that it's not enough to just keep a set of visited generic parameters,
 * else we would reject good code e.g. consider
 *   class C extends B implements Contra<B>
 * Now under assumption T <: C show T <: Contra<T>
 * This leads to cycle of implications
 *   T <: Contra<T> =>
 *   C <: Contra<T> =>
 *   Contra<B> <: Contra<T> =>
 *   T <: B =>     // DO NOT REJECT here just because we've visited T before!
 *   C <: B => done.
 *
 * We represent the visited set as a map from generic parameters
 * to pairs of sets of types, such that an entry T := ({t1,...,tm},{u1,...,un})
 * represents a set of goals
 *   T <: u1, ..., t <: un , t1 <: T, ..., tn <: T
 *)
module VisitedGoals = struct
  type t = (ITySet.t * ITySet.t) SMap.t

  let empty : t = SMap.empty

  (* Return None if (name <: ty) is already present, otherwise return Some v'
   * where v' has the pair added
   *)
  let try_add_visited_generic_sub v name ty =
    match SMap.find_opt name v with
    | None -> Some (SMap.add name (ITySet.empty, ITySet.singleton ty) v)
    | Some (lower, upper) ->
      if ITySet.mem ty upper then
        None
      else
        Some (SMap.add name (lower, ITySet.add ty upper) v)

  (* Return None if (ty <: name) is already present, otherwise return Some v'
   * where v' has the pair added
   *)
  let try_add_visited_generic_super v ty name =
    match SMap.find_opt name v with
    | None -> Some (SMap.add name (ITySet.singleton ty, ITySet.empty) v)
    | Some (lower, upper) ->
      if ITySet.mem ty lower then
        None
      else
        Some (SMap.add name (ITySet.add ty lower, upper) v)
end

module Subtype_env = struct
  type t = {
    require_soundness: bool;
        (** If set, requires the simplification of subtype constraints to be sound,
          meaning that the simplified constraint must imply the original one. *)
    require_completeness: bool;
        (** If set, requires the simplification of subtype constraints to be complete,
          meaning that the original constraint must imply the simplified one.
          If set, we also finish as soon as we see a goal of the form T <: t or
          t <: T for generic parameter T *)
    visited: VisitedGoals.t;
        (** If above is not set, maintain a visited goal set *)
    no_top_bottom: bool;
    coerce: TL.coercion_direction option;
        (** Coerce indicates whether subtyping should allow
          coercion to or from dynamic. For coercion to dynamic, types that implement
          dynamic are considered sub-types of dynamic. For coercion from dynamic,
          dynamic is treated as a sub-type of all types. *)
    on_error: Typing_error.Reasons_callback.t option;
    tparam_constraints: (Pos_or_decl.t * Typing_defs.pos_id) list;
        (** This is used for better error reporting to flag violated
          constraints on type parameters, if any. *)
    is_coeffect: bool;
        (** A flag which, if set, indicates that coeffects are being subtyped.
          Note: this is a short-term solution to provide coeffects.pretty-printing of
          `locl_ty`s that represent coeffects, since there is no good way to
          tell apart coeffects from regular types *)
    log_level: int;
        (** Which level the recursive calls to simplify_subtype should be logged at *)
    in_transitive_closure: bool;
        (** This is a subtype check from within transitive closure
          e.g. string <: #1 <: int doing string <: int *)
  }

  let set_on_error t on_error = { t with on_error }

  let set_visited t visited = { t with visited }

  let coercing_from_dynamic se =
    match se.coerce with
    | Some TL.CoerceFromDynamic -> true
    | _ -> false

  let coercing_to_dynamic se =
    match se.coerce with
    | Some TL.CoerceToDynamic -> true
    | _ -> false

  let set_coercing_to_dynamic se = { se with coerce = Some TL.CoerceToDynamic }

  let create
      ?(require_soundness = true)
      ?(require_completeness = false)
      ?(no_top_bottom = false)
      ?(coerce = None)
      ?(is_coeffect = false)
      ?(in_transitive_closure = false)
      ~(log_level : int)
      on_error =
    {
      require_soundness;
      require_completeness;
      visited = VisitedGoals.empty;
      no_top_bottom;
      coerce;
      is_coeffect;
      on_error;
      tparam_constraints = [];
      log_level;
      in_transitive_closure;
    }

  let possibly_add_violated_constraint subtype_env ~r_sub ~r_super =
    {
      subtype_env with
      tparam_constraints =
        (match (r_super, r_sub) with
        | (Reason.Rcstr_on_generics (p, tparam), _)
        | (_, Reason.Rcstr_on_generics (p, tparam)) ->
          (match subtype_env.tparam_constraints with
          | (p_prev, tparam_prev) :: _
            when Pos_or_decl.equal p p_prev
                 && Typing_defs.equal_pos_id tparam tparam_prev ->
            (* since tparam_constraints is used for error reporting, it's
             * unnecessary to add duplicates. *)
            subtype_env.tparam_constraints
          | _ -> (p, tparam) :: subtype_env.tparam_constraints)
        | _ -> subtype_env.tparam_constraints);
    }
end

module Logging = struct
  (* Given a pair of types `ty_sub` and `ty_super` attempt to apply simplifications
   * and add to the accumulated constraints in `constraints` any necessary and
   * sufficient [(t1,ck1,u1);...;(tn,ckn,un)] such that
   *   ty_sub <: ty_super iff t1 ck1 u1, ..., tn ckn un
   * where ck is `as` or `=`. Essentially we are making solution-preserving
   * simplifications to the subtype assertion, for now, also generating equalities
   * as well as subtype assertions, for backwards compatibility with use of
   * unification.
   *
   * If `constraints = []` is returned then the subtype assertion is valid.
   *
   * If the subtype assertion is unsatisfiable then return `failed = Some f`
   * where `f` is a `unit-> unit` function that records an error message.
   * (Sometimes we don't want to call this function e.g. when just checking if
   *  a subtype holds)
   *
   * Elide singleton unions, treat invariant generics as both-ways
   * subtypes, and actually chase hierarchy for extends and implements.
   *
   * Annoyingly, we need to pass env back too, because Typing_phase.localize
   * expands type constants. (TODO: work out a better way of handling this)
   *
   * Special cases:
   *   If assertion is valid (e.g. string <: arraykey) then
   *     result can be the empty list (i.e. nothing is added to the result)
   *   If assertion is unsatisfiable (e.g. arraykey <: string) then
   *     we record this in the failed field of the result.
   *)

  let log_subtype_i ~level ~this_ty ~function_name env ty_sub ty_super =
    Typing_log.(
      log_with_level env "sub" ~level (fun () ->
          let types =
            [Log_type_i ("ty_sub", ty_sub); Log_type_i ("ty_super", ty_super)]
          in
          let types =
            Option.value_map this_ty ~default:types ~f:(fun ty ->
                Log_type ("this_ty", ty) :: types)
          in
          if
            level >= 3
            || not
                 (TUtils.is_capability_i ty_sub
                 || TUtils.is_capability_i ty_super)
          then
            log_types
              (Reason.to_pos (reason ty_sub))
              env
              [Log_head (function_name, types)]
          else
            ()))

  let log_subtype ~this_ty ~function_name env ty_sub ty_super =
    log_subtype_i
      ~this_ty
      ~function_name
      env
      (LoclType ty_sub)
      (LoclType ty_super)
end

module Subtype_negation = struct
  let is_tprim_disjoint tp1 tp2 =
    let one_side tp1 tp2 =
      Aast_defs.(
        match (tp1, tp2) with
        | (Tnum, Tint)
        | (Tnum, Tfloat)
        | (Tarraykey, Tint)
        | (Tarraykey, Tstring)
        | (Tarraykey, Tnum) ->
          false
        | ( _,
            ( Tnum | Tint | Tvoid | Tbool | Tarraykey | Tfloat | Tstring | Tnull
            | Tresource | Tnoreturn ) ) ->
          true)
    in
    (not (Aast_defs.equal_tprim tp1 tp2))
    && one_side tp1 tp2
    && one_side tp2 tp1

  (* Two classes c1 and c2 are disjoint iff there exists no c3 such that
     c3 <: c1 and c3 <: c2. *)
  let is_class_disjoint env c1 c2 =
    let is_interface_or_trait c_def =
      Ast_defs.(
        match Cls.kind c_def with
        | Cinterface
        | Ctrait ->
          true
        | Cclass _
        | Cenum_class _
        | Cenum ->
          false)
    in
    if String.equal c1 c2 then
      false
    else
      match (Env.get_class env c1, Env.get_class env c2) with
      | (Decl_entry.Found c1_def, Decl_entry.Found c2_def) ->
        let is_disjoint =
          if Cls.final c1_def then
            (* if c1 is final, then c3 would have to be equal to c1 *)
            not (Cls.has_ancestor c1_def c2)
          else if Cls.final c2_def then
            (* if c2 is final, then c3 would have to be equal to c2 *)
            not (Cls.has_ancestor c2_def c1)
          else
            (* Given two non-final classes, if either is an interface or trait, then
               there could be a c3, and so we consider the classes to not be disjoint.
               However, if they are both classes, then c3 must be either c1 or c2 since
               we don't have multiple inheritance. *)
            (not (is_interface_or_trait c1_def))
            && (not (is_interface_or_trait c2_def))
            && (not (Cls.has_ancestor c2_def c1))
            && not (Cls.has_ancestor c1_def c2)
        in
        if is_disjoint then (
          (* We've used the facts that 'c1 is not a subtype of c2'
           * and 'c2 is not a subtype of c1' to conclude that a type is nothing
           * and therefore a bunch of things typecheck.
           * If these facts get invalidated by a decl change,
           * e.g. adding c2 as a parent of c1, we'd therefore need
           * to recheck the current def. *)
          Typing_env.add_not_subtype_dep env c1;
          Typing_env.add_not_subtype_dep env c2;
          ()
        );
        is_disjoint
      | _ ->
        (* This is a decl error that should have already been caught *)
        false

  (** [negate_ak_null_type env r ty] performs type negation similar to
  TUtils.negate_type, but restricted to arraykey and null (and their
  negations). *)
  let negate_ak_null_type env r ty =
    let (env, ty) = Env.expand_type env ty in
    let neg_ty =
      match get_node ty with
      | Tprim Aast.Tnull -> Some (MakeType.nonnull r)
      | Tprim Aast.Tarraykey -> Some (MakeType.neg r (Neg_prim Aast.Tarraykey))
      | Tneg (Neg_prim Aast.Tarraykey) ->
        Some (MakeType.prim_type r Aast.Tarraykey)
      | Tnonnull -> Some (MakeType.null r)
      | _ -> None
    in
    (env, neg_ty)

  let find_type_with_exact_negation env tyl =
    let rec find env tyl acc_tyl =
      match tyl with
      | [] -> (env, None, acc_tyl)
      | ty :: tyl' ->
        let (env, neg_ty) = negate_ak_null_type env (get_reason ty) ty in
        (match neg_ty with
        | None -> find env tyl' (ty :: acc_tyl)
        | Some neg_ty -> (env, Some neg_ty, tyl' @ acc_tyl))
    in
    find env tyl []
end

module Pretty : sig
  val describe_ty_default :
    Typing_env_types.env -> Typing_defs.internal_type -> string

  val describe_ty_super :
    is_coeffect:bool ->
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    string

  val strip_existential :
    ity_sub:Typing_defs.internal_type ->
    ity_sup:Typing_defs.internal_type ->
    (Typing_defs.internal_type * Typing_defs.internal_type) option
end = struct
  let strip_existential_help ty =
    let strip ty k =
      match deref ty with
      | (_, Tdependent (_, ty)) -> Some (k ty)
      | (r, Tgeneric (nm, tys)) when DependentKind.is_generic_dep_ty nm ->
        Option.map ~f:(fun nm -> k @@ mk (r, Tgeneric (nm, tys)))
        @@ DependentKind.strip_generic_dep_ty nm
      | _ -> None
    in
    (* We only want to recurse to a fixed depth so have a flag here to control
       recursion into unions and intersections *)
    let rec strip_nested ty ~recurse =
      match deref ty with
      | (r, Taccess (inner_ty, pos)) ->
        strip inner_ty (fun ty -> mk (r, Taccess (ty, pos)))
      | (r, Toption inner_ty) -> strip inner_ty (fun ty -> mk (r, Toption ty))
      | (r, Tunion ts) when recurse ->
        strip_all ts (fun ts -> mk (r, Tunion ts))
      | (r, Tintersection ts) when recurse ->
        strip_all ts (fun ts -> mk (r, Tintersection ts))
      | _ -> strip ty (fun ty -> ty)
    and strip_all tys k =
      let (tys_rev, stripped) =
        List.fold_left tys ~init:([], false) ~f:(fun (tys, stripped) ty ->
            match strip_nested ty ~recurse:false with
            | None -> (ty :: tys, stripped)
            | Some ty -> (ty :: tys, true))
      in
      if stripped then
        Some (k @@ List.rev tys_rev)
      else
        None
    in
    strip_nested ty ~recurse:true

  (* For reporting purposes we remove top-level existential types and
     existentials in type accesses when they don't occur on both subtype and
     supertype since they don't contribute to underlying error *)
  let strip_existential ~ity_sub ~ity_sup =
    match (ity_sub, ity_sup) with
    | (LoclType lty_sub, LoclType lty_sup) ->
      (match
         (strip_existential_help lty_sub, strip_existential_help lty_sup)
       with
      (* We shouldn't remove if both sub and supertype are existentially quantified *)
      | (Some _, Some _)
      (* There is nothing to do if neither side was existentially quantified *)
      | (None, None) ->
        None
      (* If we have an existential one only side we remove it *)
      | (Some lty_sub, _) -> Some (LoclType lty_sub, ity_sup)
      | (_, Some lty_sup) -> Some (ity_sub, LoclType lty_sup))
    (* The only type to appear in 'ConstraintType' is null so we can always remove
       if we have one LoclType and on ConstraintType, for some reason *)
    | (LoclType lty_sub, ConstraintType _) ->
      Option.map ~f:(fun lty_sub -> (LoclType lty_sub, ity_sup))
      @@ strip_existential_help lty_sub
    | (ConstraintType _, LoclType lty_sup) ->
      Option.map ~f:(fun lty_sup -> (ity_sub, LoclType lty_sup))
      @@ strip_existential_help lty_sup
    | (ConstraintType _, ConstraintType _) -> None

  let describe_ty_default env ty =
    Typing_print.with_blank_tyvars (fun () ->
        Typing_print.full_strip_ns_i env ty)

  let describe_ty ~is_coeffect : env -> internal_type -> string =
    (* Optimization: specialize on partial application, i.e.
       *    let describe_ty_sub = describe_ty ~is_coeffect in
       *  will check the flag only once, not every time the function is called *)
    if not is_coeffect then
      describe_ty_default
    else
      fun env -> function
       | LoclType ty -> Lazy.force @@ Typing_coeffects.pretty env ty
       | ty -> describe_ty_default env ty

  let rec describe_ty_super ~is_coeffect env ty =
    let describe_ty_super = describe_ty_super ~is_coeffect in
    let print = (describe_ty ~is_coeffect) env in
    let default () = print ty in
    match ty with
    | LoclType ty ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      | Tvar v ->
        let upper_bounds = ITySet.elements (Env.get_tyvar_upper_bounds env v) in
        (* The constraint graph is transitively closed so we can filter tyvars. *)
        let upper_bounds =
          List.filter upper_bounds ~f:(fun t -> not (is_tyvar_i t))
        in
        (match upper_bounds with
        | [] -> "some type not known yet"
        | tyl ->
          let (locl_tyl, cstr_tyl) = List.partition_tf tyl ~f:is_locl_type in
          let sep =
            match (locl_tyl, cstr_tyl) with
            | (_ :: _, _ :: _) -> " and "
            | _ -> ""
          in
          let locl_descr =
            match locl_tyl with
            | [] -> ""
            | tyl ->
              "of type "
              ^ (String.concat ~sep:" & " (List.map tyl ~f:print)
                |> Markdown_lite.md_codify)
          in
          let cstr_descr =
            String.concat
              ~sep:" and "
              (List.map cstr_tyl ~f:(describe_ty_super env))
          in
          "something " ^ locl_descr ^ sep ^ cstr_descr)
      | Toption ty when is_tyvar ty ->
        "`null` or " ^ describe_ty_super env (LoclType ty)
      | _ -> Markdown_lite.md_codify (default ()))
    | ConstraintType ty ->
      (match deref_constraint_type ty with
      | (_, Thas_member hm) ->
        let {
          hm_name = (_, name);
          hm_type = _;
          hm_class_id = _;
          hm_explicit_targs = targs;
        } =
          hm
        in
        (match targs with
        | None -> Printf.sprintf "an object with property `%s`" name
        | Some _ -> Printf.sprintf "an object with method `%s`" name)
      | (_, Thas_type_member htm) ->
        let { htm_id = id; htm_lower = lo; htm_upper = up } = htm in
        if phys_equal lo up then
          (* We use physical equality as a heuristic to generate
             slightly more readable descriptions. *)
          Printf.sprintf
            "a class with `{type %s = %s}`"
            id
            (describe_ty ~is_coeffect:false env (LoclType lo))
        else
          let bound_desc ~prefix ~is_trivial bnd =
            if is_trivial env bnd then
              ""
            else
              prefix ^ describe_ty ~is_coeffect:false env (LoclType bnd)
          in
          Printf.sprintf
            "a class with `{type %s%s%s}`"
            id
            (bound_desc ~prefix:" super " ~is_trivial:TUtils.is_nothing lo)
            (bound_desc ~prefix:" as " ~is_trivial:TUtils.is_mixed up)
      | (_, Tcan_traverse _) -> "an array that can be traversed with foreach"
      | (_, Tcan_index _) -> "an array that can be indexed"
      | (_, Tdestructure _) ->
        Markdown_lite.md_codify
          (Typing_print.with_blank_tyvars (fun () ->
               Typing_print.full_strip_ns_i env (ConstraintType ty)))
      | (_, TCunion (lty, cty)) ->
        Printf.sprintf
          "%s or %s"
          (describe_ty_super env (LoclType lty))
          (describe_ty_super env (ConstraintType cty))
      | (_, TCintersection (lty, cty)) ->
        Printf.sprintf
          "%s and %s"
          (describe_ty_super env (LoclType lty))
          (describe_ty_super env (ConstraintType cty)))
end

let get_tyvar_opt t =
  match t with
  | LoclType lt -> begin
    match get_node lt with
    | Tvar var -> Some var
    | _ -> None
  end
  | _ -> None

(* build the interface corresponding to the can_traverse constraint *)
let can_traverse_to_iface ct =
  match (ct.ct_key, ct.ct_is_await) with
  | (None, false) -> MakeType.traversable ct.ct_reason ct.ct_val
  | (None, true) -> MakeType.async_iterator ct.ct_reason ct.ct_val
  | (Some ct_key, false) ->
    MakeType.keyed_traversable ct.ct_reason ct_key ct.ct_val
  | (Some ct_key, true) ->
    MakeType.async_keyed_iterator ct.ct_reason ct_key ct.ct_val

module Sd = struct
  let liken ~super_like env ty =
    if super_like then
      TUtils.make_like env ty
    else
      ty

  (* At present, we don't distinguish between coercions (<:D) and subtyping (<:) in the
   * type variable and type parameter environments. When closing the environment we use subtyping (<:).
   * To mitigate against this, when adding a dynamic upper bound wrt coercion,
   * transform it first into supportdyn<mixed>,
   * as t <:D dynamic iff t <: supportdyn<mixed>.
   *)
  let transform_dynamic_upper_bound ~coerce env ty =
    if Tast.is_under_dynamic_assumptions env.checked then
      ty
    else
      match (coerce, get_node ty) with
      | (Some TL.CoerceToDynamic, Tdynamic) ->
        let r = get_reason ty in
        MakeType.supportdyn_mixed ~mixed_reason:r r
      | (Some TL.CoerceToDynamic, _) -> ty
      | _ -> ty
end

let mk_issubtype_prop ~sub_supportdyn ~coerce env ty1 ty2 =
  let (env, ty1) =
    match sub_supportdyn with
    | None -> (env, ty1)
    | Some r ->
      let (env, ty1) = Env.expand_internal_type env ty1 in
      ( env,
        (match ty1 with
        | LoclType ty ->
          if is_tyvar ty then
            ty1
          else
            let ty = MakeType.supportdyn r ty in
            LoclType ty
        | _ -> ty1) )
  in
  ( env,
    match ty2 with
    | LoclType ty2 ->
      let (coerce, ty2) =
        (* If we are in dynamic-aware subtyping mode, that fact will be lost when ty2
           ends up on the upper bound of a type variable. Here we find if ty2 contains
           dynamic and replace it with supportdyn<mixed> which is equivalent, but does not
           require dynamic-aware subtyping mode to be a supertype of types that support dynamic. *)
        match (coerce, TUtils.try_strip_dynamic env ty2) with
        | (Some TL.CoerceToDynamic, Some non_dyn_ty) ->
          let r = get_reason ty2 in
          ( None,
            MakeType.union
              r
              [non_dyn_ty; MakeType.supportdyn_mixed ~mixed_reason:r r] )
        | _ -> (coerce, ty2)
      in
      TL.IsSubtype (coerce, ty1, LoclType ty2)
    | _ -> TL.IsSubtype (coerce, ty1, ty2) )

module rec Subtype : sig
  (** Given types ty_sub and ty_super, attempt to
   reduce the subtyping proposition ty_sub <: ty_super to
   a logical proposition whose primitive assertions are of the form v <: t or t <: v
   where v is a type variable.

   If super_like=true, then we have already reduced ty_sub <: ~ty_super to ty_sub <: ty_super
   with ty_super known to support dynamic (i.e. ty_super <: supportdyn<mixed>). In this case,
   when "going under" a constructor (for example, we had C<t> <: ~C<u>),
   we can apply "like pushing" on the components (in this example, t <: ~u).
   The parameter defaults to false to guard against incorrectly propagating the option. When
   simplifying ty_sub only (e.g. reducing t|u <: v to t<:v && u<:v) it is correct to
   propagate it.
 *)
  val simplify_subtype :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    super_supportdyn:bool ->
    Typing_defs.locl_ty ->
    Typing_defs.locl_ty ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop

  (** Attempt to "solve" a subtype assertion ty_sub <: ty_super.
    Return a proposition that is logically stronger and simpler than
    the original assertion
    The logical relationship between the original and returned proposition
    depends on the flags require_soundness and require_completeness.
    Fail with Unsat error_function if
    the assertion is unsatisfiable. Some examples:
      string <: arraykey  ==>  True    (represented as Conj [])
    (For covariant C and a type variable v)
      C<string> <: C<v>   ==>  string <: v
    (Assuming that C does *not* implement interface J)
      C <: J              ==>  Unsat _
    (Assuming we have T <: D in tpenv, and class D implements I)
      vec<T> <: vec<I>    ==>  True
    This last one would be left as T <: I if subtype_env.require_completeness=true
   *)
  val simplify_subtype_i :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    super_supportdyn:bool ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop

  val default_subtype :
    subtype_env:Subtype_env.t ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    sub_supportdyn:Reason.t option ->
    fail:Typing_error.t option ->
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  (* If it's clear from the syntax of the type that null isn't in ty, return true.
 *)
  let rec null_not_subtype ty =
    match get_node ty with
    | Tprim (Aast_defs.Tnull | Aast_defs.Tvoid)
    | Tgeneric _
    | Tdynamic
    | Tany _
    | Toption _
    | Tvar _
    | Taccess _
    | Tunapplied_alias _
    | Tneg _
    | Tintersection _ ->
      false
    | Tunion tys -> List.for_all tys ~f:null_not_subtype
    | Tclass _
    | Tprim _
    | Tnonnull
    | Tfun _
    | Ttuple _
    | Tshape _
    | Tvec_or_dict _ ->
      true
    | Tdependent (_, bound)
    | Tnewtype (_, _, bound) ->
      null_not_subtype bound

  let is_final_and_invariant env id =
    let class_def = Env.get_class env id in
    match class_def with
    | Decl_entry.Found class_ty -> TUtils.class_is_final_and_invariant class_ty
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      false

  let simplify_subtype_by_physical_equality env ty_sub ty_super simplify_subtype
      =
    match (ty_sub, ty_super) with
    | (LoclType ty1, LoclType ty2) when phys_equal ty1 ty2 -> (env, TL.valid)
    | _ -> simplify_subtype ()

  let rec simplify_subtype
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      ~super_like
      ~super_supportdyn
      ty_sub
      ty_super =
    simplify_subtype_i
      ~subtype_env
      ~this_ty
      ~super_like
      ~sub_supportdyn
      ~super_supportdyn
      (LoclType ty_sub)
      (LoclType ty_super)

  and simplify_dynamic_aware_subtype ~subtype_env =
    let subtype_env = Subtype_env.set_coercing_to_dynamic subtype_env in
    simplify_subtype ~subtype_env

  and default_subtype
      ~subtype_env
      ~(this_ty : locl_ty option)
      ~super_like
      ~sub_supportdyn
      ~fail
      env
      ty_sub
      ty_super =
    let (env, ty_super) = Env.expand_internal_type env ty_super in
    let (env, ty_sub) = Env.expand_internal_type env ty_sub in
    (* We further refine the default subtype case for rules that apply to all
     * LoclTypes but not to ConstraintTypes
     *)
    match ty_super with
    | LoclType lty_super ->
      (match ty_sub with
      | ConstraintType _ ->
        default_subtype_inner
          ~subtype_env
          ~this_ty
          ~super_like
          ~sub_supportdyn
          ~fail
          env
          ty_sub
          ty_super
      | LoclType lty_sub -> begin
        match deref lty_sub with
        | (_, Tvar _) -> begin
          match (subtype_env.Subtype_env.coerce, get_node lty_super) with
          | (Some TL.CoerceToDynamic, Tdynamic) ->
            let r = get_reason lty_super in
            let ty_super = MakeType.supportdyn_mixed ~mixed_reason:r r in
            default_subtype_inner
              ~subtype_env
              ~this_ty
              ~super_like
              ~sub_supportdyn
              ~fail
              env
              ty_sub
              (LoclType ty_super)
          | (Some cd, _) ->
            mk_issubtype_prop
              ~sub_supportdyn
              ~coerce:(Some cd)
              env
              (LoclType lty_sub)
              (LoclType lty_super)
          | (None, _) ->
            default_subtype_inner
              ~subtype_env
              ~this_ty
              ~super_like
              ~sub_supportdyn
              ~fail
              env
              ty_sub
              ty_super
        end
        | (r_sub, Tprim Nast.Tvoid) ->
          let r =
            Reason.Rimplicit_upper_bound (Reason.to_pos r_sub, "?nonnull")
          in
          simplify_subtype
            ~subtype_env
            ~sub_supportdyn:None
            ~this_ty
            ~super_like:false
            ~super_supportdyn:false
            (MakeType.mixed r)
            lty_super
            env
          |> if_unsat (invalid ~fail)
        | (_, Tany _) ->
          if subtype_env.Subtype_env.no_top_bottom then
            mk_issubtype_prop
              ~sub_supportdyn
              ~coerce:subtype_env.Subtype_env.coerce
              env
              ty_sub
              ty_super
          else
            valid env
        | _ ->
          default_subtype_inner
            ~subtype_env
            ~this_ty
            ~super_like
            ~sub_supportdyn
            ~fail
            env
            ty_sub
            ty_super
      end)
    | ConstraintType _ ->
      default_subtype_inner
        ~subtype_env
        ~this_ty
        ~super_like
        ~sub_supportdyn
        ~fail
        env
        ty_sub
        ty_super

  and default_subtype_inner
      ~subtype_env
      ~this_ty
      ~super_like
      ~sub_supportdyn
      ~fail
      env
      ty_sub
      ty_super =
    let ( ||| ) = ( ||| ) ~fail in
    (* This inner function contains typing rules that are based solely on the subtype
     * if you need to pattern match on the super type it should NOT be included
     * here
     *)
    match ty_sub with
    | ConstraintType cty_sub -> begin
      match deref_constraint_type cty_sub with
      | (_, TCunion (lty_sub, cty_sub)) ->
        env
        |> simplify_subtype_i
             ~subtype_env
             ~sub_supportdyn
             ~this_ty:None
             ~super_like:false
             ~super_supportdyn:false
             (LoclType lty_sub)
             ty_super
        &&& simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty:None
              ~super_like:false
              ~super_supportdyn:false
              (ConstraintType cty_sub)
              ty_super
      | (_, TCintersection (lty_sub, cty_sub)) ->
        env
        |> simplify_subtype_i
             ~subtype_env
             ~sub_supportdyn
             ~this_ty:None
             ~super_like:false
             ~super_supportdyn:false
             (LoclType lty_sub)
             ty_super
        ||| simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty:None
              ~super_like:false
              ~super_supportdyn:false
              (ConstraintType cty_sub)
              ty_super
      | _ -> invalid ~fail env
    end
    | LoclType lty_sub -> begin
      match deref lty_sub with
      | (_, Tunion tyl) ->
        (*
         * t1 | ... | tn <: t
         *   if and only if
         * t1 <: t /\ ... /\ tn <: t
         * We want this even if t is a type variable e.g. consider
         *   int | v <: v
         *
         * Prioritize types that aren't dynamic or intersections with dynamic
         * to get better error messages
         *)
        let rec f ty =
          Typing_defs.is_dynamic ty
          ||
          match get_node ty with
          | Tintersection tyl -> List.exists ~f tyl
          | _ -> false
        in
        let (last_tyl, first_tyl) = TUtils.partition_union ~f tyl in
        let subtype_list env prop tyl =
          List.fold_left tyl ~init:(env, prop) ~f:(fun res ty_sub ->
              res
              &&& simplify_subtype_i
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty:None
                    ~super_like
                    ~super_supportdyn:false
                    (LoclType ty_sub)
                    ty_super)
        in
        let (env, prop) = subtype_list env TL.valid first_tyl in
        subtype_list env prop last_tyl
      (*| (_, Terr) ->
        if subtype_env.no_top_bottom then
          default env
        else
          valid env*)
      | (_, Tvar id) ->
        (* For subtyping queries of the form
         *
         *   Tvar #id <: (Tvar #id | ...)
         *
         * `remove_tyvar_from_upper_bound` simplifies the union to
         * `mixed`. This indicates that the query is discharged. If we find
         * any other upper bound, we leave the subtyping query as it is.
         *)
        let (env, simplified_super_ty) =
          Typing_solver_utils.remove_tyvar_from_upper_bound env id ty_super
        in
        (* If the type is already in the upper bounds of the type variable,
         * then we already know that this subtype assertion is valid
         *)
        if ITySet.mem simplified_super_ty (Env.get_tyvar_upper_bounds env id)
        then
          valid env
        else
          let mixed = MakeType.mixed Reason.none in
          (match simplified_super_ty with
          | LoclType simplified_super_ty when ty_equal simplified_super_ty mixed
            ->
            valid env
          | _ ->
            mk_issubtype_prop
              ~sub_supportdyn
              ~coerce:subtype_env.Subtype_env.coerce
              env
              ty_sub
              ty_super)
      | (r_sub, Tintersection tyl) ->
        (* A & B <: C iif A <: C | !B *)
        (match Subtype_negation.find_type_with_exact_negation env tyl with
        | (env, Some non_ty, tyl) ->
          let (env, ty_super) =
            TUtils.union_i env (get_reason non_ty) ty_super non_ty
          in
          let ty_sub = MakeType.intersection r_sub tyl in
          simplify_subtype_i
            ~subtype_env
            ~sub_supportdyn
            ~this_ty:None
            ~super_like:false
            ~super_supportdyn:false
            (LoclType ty_sub)
            ty_super
            env
        | _ ->
          (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
           * not complete.
           * TODO(T120921930): Don't do this if require_completeness is set.
           *)
          List.fold_left
            tyl
            ~init:(env, TL.invalid ~fail)
            ~f:(fun res ty_sub ->
              let ty_sub = LoclType ty_sub in
              res
              ||| simplify_subtype_i
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty
                    ~super_like
                    ~super_supportdyn:false
                    ty_sub
                    ty_super))
      | (_, Tgeneric (name_sub, tyargs)) ->
        (* TODO(T69551141) handle type arguments. right now, just passing
         * tyargs to Env.get_upper_bounds *)
        (if subtype_env.Subtype_env.require_completeness then
          mk_issubtype_prop
            ~sub_supportdyn
            ~coerce:subtype_env.Subtype_env.coerce
            env
            ty_sub
            ty_super
        else
          (* If we've seen this type parameter before then we must have gone
           * round a cycle so we fail
           *)
          match
            VisitedGoals.try_add_visited_generic_sub
              subtype_env.Subtype_env.visited
              name_sub
              ty_super
          with
          | None -> invalid ~fail env
          | Some new_visited ->
            let subtype_env = Subtype_env.set_visited subtype_env new_visited in
            (* If the generic is actually an expression dependent type,
               we need to update this_ty
            *)
            let this_ty =
              if
                DependentKind.is_generic_dep_ty name_sub
                && Option.is_none this_ty
              then
                Some lty_sub
              else
                this_ty
            in
            (* Otherwise, we collect all the upper bounds ("as" constraints) on
               the generic parameter, and check each of these in turn against
               ty_super until one of them succeeds
            *)
            let rec try_bounds tyl env =
              match tyl with
              | [] ->
                (* Try an implicit mixed = ?nonnull bound before giving up.
                   This can be useful when checking T <: t, where type t is
                   equivalent to but syntactically different from ?nonnull.
                   E.g., if t is a generic type parameter T with nonnull as
                   a lower bound.
                *)
                let r =
                  Reason.Rimplicit_upper_bound (get_pos lty_sub, "?nonnull")
                in
                let tmixed = LoclType (MakeType.mixed r) in
                env
                |> simplify_subtype_i
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like:false
                     ~super_supportdyn:false
                     tmixed
                     ty_super
              | [ty] ->
                simplify_subtype_i
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like
                  ~super_supportdyn:false
                  (LoclType ty)
                  ty_super
                  env
              | ty :: tyl ->
                env
                |> try_bounds tyl
                ||| simplify_subtype_i
                      ~subtype_env
                      ~sub_supportdyn
                      ~this_ty
                      ~super_like
                      ~super_supportdyn:false
                      (LoclType ty)
                      ty_super
            in
            let bounds =
              Typing_set.elements (Env.get_upper_bounds env name_sub tyargs)
            in
            env |> try_bounds bounds)
        |> (* Turn error into a generic error about the type parameter *)
        if_unsat (invalid ~fail)
      | (_, Tdynamic) when Subtype_env.coercing_from_dynamic subtype_env ->
        valid env
      | (_, Taccess _) -> invalid ~fail env
      | (r, Tnewtype (n, _, ty)) ->
        let sub_supportdyn =
          match sub_supportdyn with
          | None ->
            if String.equal n SN.Classes.cSupportDyn then
              Some r
            else
              None
          | _ -> sub_supportdyn
        in
        simplify_subtype_i
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~super_supportdyn:false
          (LoclType ty)
          ty_super
          env
      | (_, Tdependent (_, ty)) ->
        let this_ty = Option.first_some this_ty (Some lty_sub) in
        simplify_subtype_i
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~super_supportdyn:false
          (LoclType ty)
          ty_super
          env
      | _ -> invalid ~fail env
    end

  and simplify_subtype_i
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      ~(super_like : bool)
      ~(super_supportdyn : bool)
      (ty_sub : internal_type)
      (ty_super : internal_type)
      env : env * TL.subtype_prop =
    Logging.log_subtype_i
      ~level:subtype_env.Subtype_env.log_level
      ~this_ty
      ~function_name:
        ("simplify_subtype"
        ^ (match subtype_env.Subtype_env.coerce with
          | None -> ""
          | Some TL.CoerceToDynamic -> " <:D"
          | Some TL.CoerceFromDynamic -> " D<:")
        ^
        let flag str = function
          | true -> str
          | false -> ""
        in
        flag " sub_supportdyn" (Option.is_some sub_supportdyn)
        ^ flag " super_supportdyn" super_supportdyn
        ^ flag " super_like" super_like
        ^ flag " require_soundness" subtype_env.Subtype_env.require_soundness
        ^ flag
            " require_completeness"
            subtype_env.Subtype_env.require_completeness
        ^ flag
            " in_transitive_closure"
            subtype_env.Subtype_env.in_transitive_closure)
      env
      ty_sub
      ty_super;
    simplify_subtype_by_physical_equality env ty_sub ty_super @@ fun () ->
    let (env, ety_super) = Env.expand_internal_type env ty_super in
    let (env, ety_sub) = Env.expand_internal_type env ty_sub in
    simplify_subtype_by_physical_equality env ety_sub ety_super @@ fun () ->
    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(reason ety_sub)
        ~r_super:(reason ety_super)
    in
    let fail_snd_err =
      let (ety_sub, ety_super, stripped_existential) =
        match Pretty.strip_existential ~ity_sub:ety_sub ~ity_sup:ety_super with
        | None -> (ety_sub, ety_super, false)
        | Some (ety_sub, ety_super) -> (ety_sub, ety_super, true)
      in
      match subtype_env.Subtype_env.tparam_constraints with
      | [] ->
        Typing_error.Secondary.Subtyping_error
          {
            ty_sub = ety_sub;
            ty_sup = ety_super;
            is_coeffect = subtype_env.Subtype_env.is_coeffect;
            stripped_existential;
          }
      | cstrs ->
        Typing_error.Secondary.Violated_constraint
          {
            cstrs;
            ty_sub = ety_sub;
            ty_sup = ety_super;
            is_coeffect = subtype_env.Subtype_env.is_coeffect;
          }
    in
    let fail_with_suffix snd_err_opt =
      let open Typing_error in
      let maybe_retain_code =
        match subtype_env.Subtype_env.tparam_constraints with
        | [] -> Reasons_callback.retain_code
        | _ -> Fn.id
      in
      match snd_err_opt with
      | Some snd_err ->
        Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
            apply_reasons
              ~on_error:
                Reasons_callback.(
                  prepend_on_apply (maybe_retain_code on_error) fail_snd_err)
              snd_err)
      | _ ->
        Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
            apply_reasons ~on_error:(maybe_retain_code on_error) fail_snd_err)
    in

    let fail = fail_with_suffix None in
    let ( ||| ) = ( ||| ) ~fail in
    (* We *know* that the assertion is unsatisfiable *)
    let invalid_env env = invalid ~fail env in
    let invalid_env_with env f = invalid ~fail:f env in
    (* We don't know whether the assertion is valid or not *)
    let default env =
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        ety_sub
        ety_super
    in
    let default_subtype_help env =
      default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like
        ~fail
        env
        ety_sub
        ety_super
    in
    match ety_super with
    (* First deal with internal constraint types *)
    | ConstraintType cty_super ->
      Subtype_constraint_super.simplify_subtype_cty_super
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like
        ~fail
        ty_sub
        ety_sub
        ty_super
        cty_super
        env
      (* Next deal with all locl types *)
    | LoclType ty_super ->
      (match deref ty_super with
      | (_, Tvar var_super) ->
        (match ety_sub with
        | ConstraintType cty when is_constraint_type_union cty ->
          default_subtype_help env
        | ConstraintType _ -> default env
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (_, Tunion _) -> default_subtype_help env
          | (_, Tdynamic) when Subtype_env.coercing_from_dynamic subtype_env ->
            default_subtype_help env
          (* We want to treat nullable as a union with the same rule as above.
           * This is only needed for Tvar on right; other cases are dealt with specially as
           * derived rules.
           *)
          | (r, Toption t) ->
            let (env, t) = Env.expand_type env t in
            (match get_node t with
            (* We special case on `mixed <: Tvar _`, adding the entire `mixed` type
               as a lower bound. This enables clearer error messages when upper bounds
               are added to the type variable: transitive closure picks up the
               entire `mixed` type, and not separately consider `null` and `nonnull` *)
            | Tnonnull -> default env
            | _ ->
              let ty_null = MakeType.null r in
              env
              |> simplify_subtype
                   ~subtype_env
                   ~sub_supportdyn
                   ~this_ty
                   ~super_like
                   ~super_supportdyn:false
                   t
                   ty_super
              &&& simplify_subtype
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty
                    ~super_like:false
                    ~super_supportdyn:false
                    ty_null
                    ty_super)
          | (_, Tvar var_sub) when Tvid.equal var_sub var_super -> valid env
          | _ -> begin
            match subtype_env.Subtype_env.coerce with
            | Some cd ->
              mk_issubtype_prop
                ~sub_supportdyn
                ~coerce:(Some cd)
                env
                (LoclType ty_sub)
                (LoclType ty_super)
            | None ->
              if super_like then
                let (env, ty_sub) =
                  Typing_dynamic.strip_covariant_like env ty_sub
                in
                env
                |> simplify_subtype_i
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like:false
                     ~super_supportdyn:false
                     (LoclType ty_sub)
                     (LoclType ty_super)
              else
                default env
          end))
      | (_, Tintersection tyl) ->
        (match ety_sub with
        | ConstraintType cty when is_constraint_type_union cty ->
          default_subtype_help env
        | LoclType lty when is_union lty -> default_subtype_help env
        (* t <: (t1 & ... & tn)
         *   if and only if
         * t <: t1 /\  ... /\ t <: tn
         *)
        | _ ->
          List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_super ->
              let ty_super = LoclType ty_super in
              res
              &&& simplify_subtype_i
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty
                    ~super_like:false
                    ~super_supportdyn:false
                    ty_sub
                    ty_super))
      (* Empty union encodes the bottom type nothing *)
      | (_, Tunion []) -> default_subtype_help env
      (* ty_sub <: union{ty_super'} iff ty_sub <: ty_super' *)
      | (_, Tunion [ty_super']) ->
        simplify_subtype_i
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~super_supportdyn:false
          ty_sub
          (LoclType ty_super')
          env
      | (r, Tunion (_ :: _ as tyl_super)) ->
        let simplify_sub_union env ty_sub tyl_super =
          (* Identify cases heuristically where we just want to reduce t <: ~u to
             t <: u with super-like set, and not also try t <: dynamic or run finish *)
          let avoid_disjunctions env ty =
            let (env, ty) = Env.expand_type env ty in
            match (ty_sub, get_node ty) with
            | (LoclType lty, Tnewtype (n2, _, _)) ->
              (match get_node lty with
              | Tnewtype (n1, _, _)
                when String.equal n1 n2
                     && not (String.equal n1 SN.Classes.cSupportDyn) ->
                (env, true)
              | _ -> (env, false))
            | _ -> (env, false)
          in
          let finish env =
            match ty_sub with
            | LoclType lty -> begin
              match get_node lty with
              | Tnewtype _
              | Tdependent _
              | Tgeneric _ ->
                default_subtype_help env
              | _ -> invalid_env env
            end
            | _ -> invalid_env env
          in
          let simplify_subtype_of_dynamic env =
            simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              ty_sub
              (LoclType (MakeType.dynamic r))
              env
          in
          let dyn_finish ty env =
            let (env, avoid) = avoid_disjunctions env ty in
            if avoid then
              invalid_env env
            else
              simplify_subtype_of_dynamic env ||| fun env ->
              if Typing_utils.is_tyvar env ty then
                invalid_env env
              else
                finish env
          in
          let stripped_dynamic =
            if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
              TUtils.try_strip_dynamic_from_union env tyl_super
            else
              None
          in
          match stripped_dynamic with
          | Some (ty_dynamic, tyl) ->
            let ty = MakeType.union r tyl in
            let (env, ty) = Env.expand_type env ty in
            let delay_push =
              Subtype_ask.is_sub_type_for_union_i
                env
                (LoclType ty)
                (LoclType (MakeType.supportdyn_mixed ~mixed_reason:r r))
            in
            (* This is Typing_logic_helpers.( ||| ) except with a bias towards p1 *)
            let ( ||| ) (env, p1) (f : env -> env * TL.subtype_prop) =
              if TL.is_valid p1 then
                (env, p1)
              else
                let (env, p2) = f env in
                if TL.is_unsat p2 then
                  (env, p1)
                else if TL.is_unsat p1 then
                  (env, p2)
                else
                  (env, TL.disj ~fail p1 p2)
            in
            (* Implement the declarative subtyping rule C<~t1,...,~tn> <: ~C<t1,...,tn>
               * for a type C<t1,...,tn> that supports dynamic. Algorithmically,
               *   t <: ~C<t1,...,tn> iff
               *   t <: C<~t1,...,~tn> /\ C<~t1,...,~tn> <:D dynamic.
               * An SDT class C generalizes to other SDT constructors such as tuples and shapes.
            *)
            let try_push env =
              if delay_push then
                dyn_finish ty env
              else
                (* "Solve" type variables that are bounded from above and below by the same type.
                 * Push this through nullables. This addresses common completeness issues that
                 * bedevil like-pushing because of the disjunction that is generated.
                 *)
                let rec solve_eq_tyvar env ty =
                  let (env, ty) = Env.expand_type env ty in
                  match get_node ty with
                  | Tvar v ->
                    let lower_bounds = Env.get_tyvar_lower_bounds env v in
                    let (nulls, nonnulls) =
                      ITySet.partition
                        (fun ty ->
                          match ty with
                          | LoclType t -> is_prim Aast.Tnull t
                          | _ -> false)
                        lower_bounds
                    in
                    (* Make sure that lower bounds [null;t] intersects with ?t upper bound *)
                    let lower_bounds =
                      if ITySet.is_empty nulls then
                        nonnulls
                      else
                        ITySet.map
                          (fun t -> MakeType.nullable_i Reason.Rnone t)
                          nonnulls
                    in
                    let upper_bounds = Env.get_tyvar_upper_bounds env v in
                    let bounds = ITySet.inter lower_bounds upper_bounds in
                    let bounds_list = ITySet.elements bounds in
                    begin
                      match bounds_list with
                      | [LoclType lty] -> (env, lty)
                      | _ -> (env, ty)
                    end
                  | Toption ty1 ->
                    let (env, ty1) = solve_eq_tyvar env ty1 in
                    (env, mk (get_reason ty, Toption ty1))
                  | _ -> (env, ty)
                in
                let (env, ty) = solve_eq_tyvar env ty in
                (* For generic parameters with lower bounds, try like-pushing wrt
                 * these lower bounds. For example, we want
                 * vec<~int> <: ~T if vec<int> <: T
                 *)
                let ty =
                  match get_node ty with
                  | Tgeneric (name, targs) ->
                    let bounds = Env.get_lower_bounds env name targs in
                    MakeType.union (get_reason ty) (Typing_set.elements bounds)
                  | _ -> ty
                in
                let (env, opt_ty) = Typing_dynamic.try_push_like env ty in
                match opt_ty with
                | None ->
                  let istyvar =
                    match get_node ty with
                    | Tvar _ -> true
                    | Toption ty ->
                      let (_, ty) = Env.expand_type env ty in
                      is_tyvar ty
                    | _ -> false
                  in
                  if istyvar then
                    env
                    |> simplify_subtype_i
                         ~subtype_env
                         ~sub_supportdyn
                         ~this_ty
                         ~super_like:true
                         ~super_supportdyn:false
                         ty_sub
                         (LoclType ty)
                    ||| dyn_finish ty
                  else
                    dyn_finish ty env
                | Some ty ->
                  let simplify_pushed_like env =
                    env
                    |> simplify_dynamic_aware_subtype
                         ~subtype_env
                         ~sub_supportdyn:None
                         ~this_ty
                         ~super_like:false
                         ~super_supportdyn:false
                         ty
                         ty_dynamic
                    &&& simplify_subtype_i
                          ~subtype_env
                          ~sub_supportdyn
                          ~this_ty
                          ~super_like:false
                          ~super_supportdyn:false
                          ty_sub
                          (LoclType ty)
                  in
                  env |> simplify_pushed_like ||| dyn_finish ty
            in
            simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:delay_push
              ~super_supportdyn:false
              ty_sub
              (LoclType ty)
              env
            ||| try_push
          | _ ->
            (* It's sound to reduce t <: t1 | t2 to (t <: t1) || (t <: t2). But
             * not complete e.g. consider (t1 | t3) <: (t1 | t2) | (t2 | t3).
             * But we deal with unions on the left first (see case above), so this
             * particular situation won't arise.
             * TODO: identify under what circumstances this reduction is complete.
             * TODO(T120921930): Don't do this if require_completeness is set.
             *)
            let rec try_disjuncts tys env =
              match tys with
              | [] -> invalid_env env
              | ty :: tys ->
                let ty = LoclType ty in
                env
                |> simplify_subtype_i
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like
                     ~super_supportdyn:false
                     ty_sub
                     ty
                ||| try_disjuncts tys
            in
            env |> try_disjuncts tyl_super
        in

        (match ety_sub with
        | ConstraintType cty when is_constraint_type_union cty ->
          default_subtype_help env
        | ConstraintType _ -> simplify_sub_union env ety_sub tyl_super
        | LoclType lty_sub ->
          (match
             simplify_subtype_arraykey_union
               ~sub_supportdyn
               ~this_ty
               ~subtype_env
               env
               lty_sub
               tyl_super
           with
          | (env, Some props) -> (env, props)
          | (env, None) ->
            (match deref lty_sub with
            | (_, (Tunion _ | Tvar _)) -> default_subtype_help env
            | (_, Tgeneric _) when subtype_env.Subtype_env.require_completeness
              ->
              default_subtype_help env
            (* Num is not atomic: it is equivalent to int|float. The rule below relies
             * on ty_sub not being a union e.g. consider num <: arraykey | float, so
             * we break out num first.
             *)
            | (r, Tprim Nast.Tnum) ->
              let ty_float = MakeType.float r and ty_int = MakeType.int r in
              env
              |> simplify_subtype
                   ~subtype_env
                   ~sub_supportdyn:None
                   ~this_ty
                   ~super_like:false
                   ~super_supportdyn:false
                   ty_float
                   ty_super
              &&& simplify_subtype
                    ~subtype_env
                    ~sub_supportdyn:None
                    ~this_ty
                    ~super_like:false
                    ~super_supportdyn:false
                    ty_int
                    ty_super
            (* Likewise, reduce nullable on left to a union *)
            | (r, Toption ty) ->
              let ty_null = MakeType.null r in
              if_unsat
                invalid_env
                (simplify_subtype_i
                   ~subtype_env
                   ~sub_supportdyn
                   ~this_ty
                   ~super_like:false
                   ~super_supportdyn:false
                   (LoclType ty_null)
                   ety_super
                   env)
              &&& simplify_subtype_i
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty
                    ~super_like:false
                    ~super_supportdyn:false
                    (LoclType ty)
                    ety_super
            | (_, Tintersection tyl)
              when let (_, non_ty_opt, _) =
                     Subtype_negation.find_type_with_exact_negation env tyl
                   in
                   Option.is_some non_ty_opt ->
              default_subtype_help env
            | (_, Tintersection tyl_sub) ->
              let simplify_super_intersection env tyl_sub ty_super =
                (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
                 * not complete.
                 * TODO(T120921930): Don't do this if require_completeness is set.
                 *)
                List.fold_left
                  tyl_sub
                  ~init:(env, TL.invalid ~fail)
                  ~f:(fun res ty_sub ->
                    let ty_sub = LoclType ty_sub in
                    res
                    ||| simplify_subtype_i
                          ~subtype_env
                          ~sub_supportdyn
                          ~this_ty
                          ~super_like:false
                          ~super_supportdyn:false
                          ty_sub
                          ty_super)
              in
              (* Heuristicky logic to decide whether to "break" the intersection
                  or the union first, based on observing that the following cases often occur:
                    - A & B <: (A & B) | C
                      In which case we want to "break" the union on the right first
                      in order to have the following recursive calls :
                          A & B <: A & B
                          A & B <: C
                    - A & (B | C) <: B | C
                      In which case we want to "break" the intersection on the left first
                      in order to have the following recursive calls:
                          A <: B | C
                          B | C <: B | C
                 If there is a type variable in the union, then generally it's helpful to
                 break the union apart.
              *)
              if
                List.exists tyl_super ~f:(fun t ->
                    TUtils.is_tintersection env t
                    || TUtils.is_opt_tyvar env t
                    || TUtils.is_tyvar env t)
              then
                simplify_sub_union env ety_sub tyl_super
              else if List.exists tyl_sub ~f:(TUtils.is_tunion env) then
                simplify_super_intersection env tyl_sub (LoclType ty_super)
              else
                simplify_sub_union env ety_sub tyl_super
            | _ -> simplify_sub_union env ety_sub tyl_super)))
      | (r_super, Toption arg_ty_super) ->
        let (env, ety) = Env.expand_type env arg_ty_super in
        (* Toption(Tnonnull) encodes mixed, which is our top type.
         * Everything subtypes mixed *)
        if is_nonnull ety then
          valid env
        else (
          match ety_sub with
          | ConstraintType _ -> default_subtype_help env
          | LoclType lty_sub ->
            (match (deref lty_sub, get_node ety) with
            (* ?supportdyn<t> is equivalent to supportdyn<?t> *)
            | (_, Tnewtype (name, [tyarg], _))
              when String.equal name SN.Classes.cSupportDyn ->
              let tyarg = MakeType.nullable r_super tyarg in
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty:None
                ~super_like
                ~super_supportdyn:false
                lty_sub
                (MakeType.supportdyn r_super tyarg)
                env
            (*   supportdyn<t> <: ?u   iff
             *   nonnull & supportdyn<t> <: u   iff
             *   supportdyn<nonnull & t> <: u
             *)
            | ((r, Tnewtype (name, [tyarg1], _)), _)
              when String.equal name SN.Classes.cSupportDyn ->
              let (env, ty_sub') =
                Inter.intersect env ~r:r_super tyarg1 (MakeType.nonnull r_super)
              in
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn:(Some r)
                ~this_ty:None
                ~super_like
                ~super_supportdyn:false
                ty_sub'
                ety
                env
            (* A <: ?B iff A & nonnull <: B
               Only apply if B is a type variable or an intersection, to avoid oscillating
               forever between this case and the previous one. *)
            | ((_, Tintersection tyl), (Tintersection _ | Tvar _))
              when let (_, non_ty_opt, _) =
                     Subtype_negation.find_type_with_exact_negation env tyl
                   in
                   Option.is_none non_ty_opt ->
              let (env, ty_sub') =
                Inter.intersect_i env r_super ty_sub (MakeType.nonnull r_super)
              in
              simplify_subtype_i
                ~subtype_env
                ~sub_supportdyn
                ~this_ty:None
                ~super_like
                ~super_supportdyn:false
                ty_sub'
                (LoclType arg_ty_super)
                env
            (* null is the type of null and is a subtype of any option type. *)
            | ((_, Tprim Nast.Tnull), _) -> valid env
            (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
             * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_sub' (widening) and transitivity
             * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
             * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
             * Therefore, this step preserves the set of solutions.
             *)
            | ((_, Toption ty_sub'), _) ->
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like
                ~super_supportdyn:false
                ty_sub'
                ty_super
                env
            (* If the type on the left is disjoint from null, then the Toption on the right is not
               doing anything helpful. *)
            | ((_, (Tintersection _ | Tunion _)), _)
              when TUtils.is_type_disjoint
                     env
                     lty_sub
                     (MakeType.null Reason.Rnone) ->
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like
                ~super_supportdyn:false
                lty_sub
                ety
                env
              (* We do not want to decompose Toption for these cases *)
            | ((_, (Tvar _ | Tunion _ | Tintersection _)), _) ->
              default_subtype_help env
            | ((_, Tgeneric _), _)
              when subtype_env.Subtype_env.require_completeness ->
              (* TODO(T69551141) handle type arguments ? *)
              default_subtype_help env
            (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
             * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
             * true as well.  We can fold the case where t1 is unconstrained
             * into the case analysis below.
             *
             * In the case where it's easy to determine that null isn't in t1,
             * we need only check t1 <: t2.
             *)
            | ( (_, (Tnewtype _ | Tdependent _ | Tgeneric _ | Tprim Nast.Tvoid)),
                _ ) ->
              (* TODO(T69551141) handle type arguments? *)
              if null_not_subtype lty_sub then
                env
                |> simplify_subtype
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like
                     ~super_supportdyn:false
                     lty_sub
                     arg_ty_super
              else
                env
                |> simplify_subtype
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like
                     ~super_supportdyn:false
                     lty_sub
                     arg_ty_super
                ||| default_subtype_help
            (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
             * must also have ty_sub <: ty_super'.  The converse follows by
             * widening and transitivity.  Therefore, this step preserves the set
             * of solutions.
             *)
            | ((_, Tunapplied_alias _), _) ->
              Typing_defs.error_Tunapplied_alias_in_illegal_context ()
            | ( ( _,
                  ( Tdynamic | Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _
                  | Tclass _ | Tvec_or_dict _ | Tany _ | Taccess _ ) ),
                _ ) ->
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like
                ~super_supportdyn:false
                lty_sub
                arg_ty_super
                env
            (* This is treating the option as a union, and using the sound, but incomplete,
               t <: t1 | t2 to (t <: t1) || (t <: t2) reduction
               TODO(T120921930): Don't do this if require_completeness is set.
            *)
            | ((_, Tneg _), _) ->
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like
                ~super_supportdyn:false
                lty_sub
                arg_ty_super
                env)
        )
      | (_r_super, Tdependent (d_sup, bound_sup)) ->
        let (env, bound_sup) = Env.expand_type env bound_sup in
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType ty_sub ->
          (match (deref ty_sub, get_node bound_sup) with
          | ((_, Tclass _), Tclass ((_, x), _, _))
            when is_final_and_invariant env x ->
            (* For final class C, there is no difference between `this as X` and `X`,
             * and `expr<#n> as X` and `X`.
             * But we need to take care with variant classes, since we can't
             * statically guarantee their runtime type.
             *)
            simplify_subtype
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              ty_sub
              bound_sup
              env
          | ( (r_sub, Tclass ((_, y), _, _)),
              Tclass (((_, x) as id), _, _tyl_super) ) ->
            let fail =
              if String.equal x y then
                let p = Reason.to_pos r_sub in
                let (pos_super, class_name) = id in
                fail_with_suffix
                  (Some
                     (Typing_error.Secondary.This_final
                        { pos_super; class_name; pos_sub = p }))
              else
                fail
            in
            invalid_env_with env fail
          | ((_, Tdependent (d_sub, bound_sub)), _) ->
            let this_ty = Option.first_some this_ty (Some ty_sub) in
            (* Dependent types are identical but bound might be different *)
            if equal_dependent_type d_sub d_sup then
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like:false
                ~super_supportdyn:false
                bound_sub
                bound_sup
                env
            else
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like:false
                ~super_supportdyn:false
                bound_sub
                ty_super
                env
          | _ -> default_subtype_help env))
      | (_, Taccess _) -> invalid_env env
      | (_, Tgeneric (name_super, tyargs_super)) ->
        (* TODO(T69551141) handle type arguments. Right now, only passing tyargs_super to
           Env.get_lower_bounds *)
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        (* If subtype and supertype are the same generic parameter, we're done *)
        | LoclType ty_sub ->
          let (generic_lower_bounds, other_lower_bounds) =
            let rec fixpoint new_set bounds_set =
              if Typing_set.is_empty new_set then
                bounds_set
              else
                let add_set =
                  Typing_set.fold
                    (fun ty add_set ->
                      match get_node ty with
                      | Tgeneric (name, targs) ->
                        let gen_bounds = Env.get_lower_bounds env name targs in
                        Typing_set.union add_set gen_bounds
                      | _ -> add_set)
                    new_set
                    Typing_set.empty
                in
                let bounds_set = Typing_set.union new_set bounds_set in
                let new_set = Typing_set.diff add_set bounds_set in
                fixpoint new_set bounds_set
            in
            let lower_bounds =
              fixpoint (Typing_set.singleton ty_super) Typing_set.empty
            in
            Typing_set.fold
              (fun bound_ty (g_set, o_set) ->
                match get_node bound_ty with
                | Tgeneric (name, []) -> (SSet.add name g_set, o_set)
                | _ -> (g_set, Typing_set.add bound_ty o_set))
              lower_bounds
              (SSet.empty, Typing_set.empty)
          in
          (match get_node ty_sub with
          | Tgeneric (name_sub, []) when SSet.mem name_sub generic_lower_bounds
            ->
            valid env
          | Tgeneric (name_sub, tyargs_sub)
            when String.equal name_sub name_super ->
            if List.is_empty tyargs_super then
              valid env
            else
              (* TODO(T69931993) Type parameter env must carry variance information *)
              let variance_reifiedl =
                List.map tyargs_sub ~f:(fun _ ->
                    (Ast_defs.Invariant, Aast.Erased))
              in
              simplify_subtype_variance_for_non_injective
                ~subtype_env
                ~sub_supportdyn
                ~super_like
                name_sub
                None
                variance_reifiedl
                tyargs_sub
                tyargs_super
                ety_sub
                ety_super
                env
          (* When decomposing subtypes for the purpose of adding bounds on generic
           * parameters to the context, (so seen_generic_params = None), leave
           * subtype so that the bounds get added *)
          | Tvar _
          | Tunion _ ->
            default_subtype_help env
          | _ ->
            if subtype_env.Subtype_env.require_completeness then
              default env
            else (
              (* If we've seen this type parameter before then we must have gone
               * round a cycle so we fail
               *)
              match
                VisitedGoals.try_add_visited_generic_super
                  subtype_env.Subtype_env.visited
                  ety_sub
                  name_super
              with
              | None -> invalid_env env
              | Some new_visited ->
                let subtype_env =
                  Subtype_env.set_visited subtype_env new_visited
                in
                (* Collect all the lower bounds ("super" constraints) on the
                 * generic parameter, and check ty_sub against each of them in turn
                 * until one of them succeeds *)
                let rec try_bounds tyl env =
                  match tyl with
                  | [] -> default_subtype_help env
                  | ty :: tyl ->
                    env
                    |> simplify_subtype
                         ~subtype_env
                         ~sub_supportdyn
                         ~super_like
                         ~this_ty
                         ~super_supportdyn:false
                         ty_sub
                         ty
                    ||| try_bounds tyl
                in
                (* Turn error into a generic error about the type parameter *)
                let bounds = Typing_set.elements other_lower_bounds in
                env |> try_bounds bounds |> if_unsat invalid_env
            )))
      | (_, Tnonnull) ->
        (match ety_sub with
        | ConstraintType cty -> begin
          match deref_constraint_type cty with
          | (_, (Thas_member _ | Tdestructure _)) -> valid env
          | _ -> default_subtype_help env
        end
        | LoclType lty ->
          (match deref lty with
          | ( _,
              ( Tprim
                  Nast.(
                    ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
                    | Tarraykey | Tnoreturn ))
              | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tclass _
              | Tvec_or_dict _ | Taccess _ ) ) ->
            valid env
          (* supportdyn<t> <: nonnull iff t <: nonnull *)
          | (r, Tnewtype (name, [tyarg], _))
            when String.equal name SN.Classes.cSupportDyn ->
            env
            |> simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn:(Some r)
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 tyarg
                 ty_super
          (* negations always contain null *)
          | (_, Tneg _) -> invalid_env env
          | _ -> default_subtype_help env))
      | (r_dynamic, Tdynamic)
        when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
             && (Subtype_env.coercing_to_dynamic subtype_env
                || Tast.is_under_dynamic_assumptions env.checked) ->
        let open Ast_defs in
        (match ety_sub with
        | ConstraintType _cty ->
          (* TODO *)
          default_subtype_help env
        | LoclType lty_sub ->
          let dyn =
            lazy (Pretty.describe_ty_super ~is_coeffect:false env ety_super)
          in
          let dynamic_part =
            Lazy.map dyn ~f:(fun dyn ->
                Reason.to_string ("Expected " ^ dyn) r_dynamic)
          and ty_name = lazy (Pretty.describe_ty_default env ety_sub)
          and pos = Reason.to_pos (get_reason lty_sub) in
          let postprocess =
            if_unsat
              (invalid
                 ~fail:
                   (Option.map
                      subtype_env.Subtype_env.on_error
                      ~f:
                        Typing_error.(
                          fun on_error ->
                            apply_reasons ~on_error
                            @@ Secondary.Not_sub_dynamic
                                 { pos; ty_name; dynamic_part })))
          in
          postprocess
          @@
          if Option.is_some sub_supportdyn then
            valid env
          else (
            match deref lty_sub with
            | (_, Tany _)
            | ( _,
                Tprim
                  ( Tint | Tbool | Tfloat | Tstring | Tnum | Tarraykey | Tvoid
                  | Tnoreturn | Tresource ) ) ->
              valid env
            | (_, Tnewtype (name_sub, [_tyarg_sub], _))
              when String.equal name_sub SN.Classes.cSupportDyn ->
              valid env
            | (_, Tnewtype (name_sub, _, _))
              when String.equal name_sub SN.Classes.cEnumClassLabel ->
              valid env
            | (_, Toption ty) ->
              (match deref ty with
              (* Special case mixed <: dynamic for better error message *)
              | (_, Tnonnull) -> invalid_env env
              | _ ->
                simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty:None
                  ~super_like:false
                  ~super_supportdyn:false
                  ty
                  ty_super
                  env)
            | (_, (Tdynamic | Tprim Tnull)) -> valid env
            | (_, Tnonnull)
            | (_, Tvar _)
            | (_, Tunapplied_alias _)
            | (_, Tnewtype _)
            | (_, Tdependent _)
            | (_, Taccess _)
            | (_, Tunion _)
            | (_, Tintersection _)
            | (_, Tgeneric _)
            | (_, Tneg _) ->
              default_subtype_help env
            | (_, Tvec_or_dict (_, ty)) ->
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty:None
                ~super_like:false
                ~super_supportdyn:false
                ty
                ty_super
                env
            | (_, Tfun ft_sub) ->
              if get_ft_support_dynamic_type ft_sub then
                valid env
              else
                (* Special case of function type subtype dynamic.
                 *   (function(ty1,...,tyn):ty <: supportdyn<nonnull>)
                 *   iff
                 *   dynamic <D: ty1 & ... & dynamic <D: tyn & ty <D: dynamic
                 *)
                let ty_dyn_enf = ty_super in
                env
                (* Contravariant subtyping on parameters *)
                |> Subtype_fun.simplify_supertype_params_with_variadic
                     ~subtype_env
                     ft_sub.ft_params
                     ty_dyn_enf
                &&& (* Finally do covariant subtryping on return type *)
                simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty:None
                  ~super_like:false
                  ~super_supportdyn:false
                  ft_sub.ft_ret
                  ty_super
            | (_, Ttuple tyl) ->
              List.fold_left
                ~init:(env, TL.valid)
                ~f:(fun res ty_sub ->
                  res
                  &&& simplify_subtype
                        ~subtype_env
                        ~sub_supportdyn
                        ~this_ty:None
                        ~super_like:false
                        ~super_supportdyn:false
                        ty_sub
                        ty_super)
                tyl
            | ( _,
                Tshape
                  {
                    s_origin = _;
                    s_unknown_value = unknown_fields_type;
                    s_fields = sftl;
                  } ) ->
              List.fold_left
                ~init:(env, TL.valid)
                ~f:(fun res sft ->
                  res
                  &&& simplify_subtype
                        ~subtype_env
                        ~sub_supportdyn
                        ~this_ty:None
                        ~super_like:false
                        ~super_supportdyn:false
                        sft.sft_ty
                        ty_super)
                (TShapeMap.values sftl)
              &&& simplify_subtype
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty:None
                    ~super_like:false
                    ~super_supportdyn:false
                    unknown_fields_type
                    ty_super
            | (_, Tclass ((_, class_id), _exact, tyargs)) ->
              let class_def_sub = Env.get_class env class_id in
              (match class_def_sub with
              | Decl_entry.DoesNotExist
              | Decl_entry.NotYetAvailable ->
                (* This should have been caught already in the naming phase *)
                valid env
              | Decl_entry.Found class_sub ->
                if
                  Cls.get_support_dynamic_type class_sub
                  || Env.is_enum env class_id
                then
                  (* If a class has the __SupportDynamicType annotation, then
                     a type formed from it is a dynamic-aware subtype of dynamic if
                     the type arguments are correctly supplied, which depends on the
                     variance of the parameter, and whether the __RequireDynamic
                     is on the parameter.
                  *)
                  let rec subtype_args tparams tyargs env =
                    match (tparams, tyargs) with
                    | ([], _) -> valid env
                    | (_, []) ->
                      (* If there are missing type arguments, we don't know that they are subtypes of dynamic, unless the bounds enforce that *)
                      invalid_env env
                    | (tp :: tparams, tyarg :: tyargs) ->
                      let has_require_dynamic =
                        Attributes.mem
                          SN.UserAttributes.uaRequireDynamic
                          tp.tp_user_attributes
                      in
                      (if
                       has_require_dynamic
                       (* Implicit pessimisation should ignore the RequireDynamic attribute
                          because everything should be pessimised enough that it isn't necessary. *)
                       && not (TypecheckerOptions.everything_sdt env.genv.tcopt)
                      then
                        (* If the class is marked <<__SupportDynamicType>> then for any
                           * type parameters marked <<__RequireDynamic>> then the class does not
                           * unconditionally implement dynamic, but rather we must check that
                           * it is a subtype of the same type whose corresponding type arguments
                           * are replaced by dynamic, intersected with the parameter's upper bounds.
                           *
                           * For example, to check dict<int,float> <: supportdyn<nonnull>
                           * we check dict<int,float> <D: dict<arraykey,dynamic>
                           * which in turn requires int <D: arraykey and float <D: dynamic.
                        *)
                        let upper_bounds =
                          List.filter_map tp.tp_constraints ~f:(fun (c, ty) ->
                              match c with
                              | Ast_defs.Constraint_as ->
                                let (_env, ty) =
                                  Phase.localize_no_subst
                                    env
                                    ~ignore_errors:true
                                    ty
                                in
                                Some ty
                              | _ -> None)
                        in
                        let super =
                          MakeType.intersection
                            r_dynamic
                            (ty_super :: upper_bounds)
                        in
                        match tp.tp_variance with
                        | Ast_defs.Covariant ->
                          simplify_subtype
                            ~subtype_env
                            ~sub_supportdyn:None
                            ~this_ty:None
                            ~super_like:false
                            ~super_supportdyn:false
                            tyarg
                            super
                            env
                        | Ast_defs.Contravariant ->
                          simplify_subtype
                            ~subtype_env
                            ~sub_supportdyn:None
                            ~this_ty:None
                            ~super_like:false
                            ~super_supportdyn:false
                            super
                            tyarg
                            env
                        | Ast_defs.Invariant ->
                          simplify_subtype
                            ~subtype_env
                            ~sub_supportdyn:None
                            ~this_ty:None
                            ~super_like:false
                            ~super_supportdyn:false
                            tyarg
                            super
                            env
                          &&& simplify_subtype
                                ~subtype_env
                                ~sub_supportdyn:None
                                ~this_ty:None
                                ~super_like:false
                                ~super_supportdyn:false
                                super
                                tyarg
                      else
                        (* If the class is marked <<__SupportDynamicType>> then for any
                           * type parameters not marked <<__RequireDynamic>> then the class is a
                           * subtype of dynamic only when the arguments are also subtypes of dynamic.
                        *)
                        match tp.tp_variance with
                        | Ast_defs.Covariant
                        | Ast_defs.Invariant ->
                          simplify_subtype
                            ~subtype_env
                            ~sub_supportdyn:None
                            ~this_ty:None
                            ~super_like:false
                            ~super_supportdyn:false
                            tyarg
                            ty_super
                            env
                        | Ast_defs.Contravariant ->
                          (* If the parameter is contra-variant, then we only need to
                             check that the lower bounds (if present) are subtypes of
                             dynamic. For example, given <<__SDT>> class C<-T> {...},
                             then for any t, C<t> <: C<nothing>, and since
                             `nothing <D: dynamic`, `C<nothing> <D: dynamic` and so
                             `C<t> <D: dynamic`. If there are lower bounds, we can't
                             push the argument below them. It suffices to check only
                             them because if one of them is not <D: dynamic, then
                             none of their supertypes are either.
                          *)
                          let lower_bounds =
                            List.filter_map tp.tp_constraints ~f:(fun (c, ty) ->
                                match c with
                                | Ast_defs.Constraint_super ->
                                  let (_env, ty) =
                                    Phase.localize_no_subst
                                      env
                                      ~ignore_errors:true
                                      ty
                                  in
                                  Some ty
                                | _ -> None)
                          in
                          (match lower_bounds with
                          | [] -> valid env
                          | _ ->
                            let sub = MakeType.union r_dynamic lower_bounds in
                            simplify_subtype
                              ~subtype_env
                              ~sub_supportdyn:None
                              ~this_ty:None
                              ~super_like:false
                              ~super_supportdyn:false
                              sub
                              ty_super
                              env))
                      &&& subtype_args tparams tyargs
                  in
                  subtype_args (Cls.tparams class_sub) tyargs env
                else (
                  match Cls.kind class_sub with
                  | Ast_defs.Cenum_class _ ->
                    (match Cls.enum_type class_sub with
                    | Some enum_type ->
                      let ((env, _ty_err_opt), subtype) =
                        TUtils.localize_no_subst
                          ~ignore_errors:true
                          env
                          enum_type.te_base
                      in
                      simplify_subtype
                        ~subtype_env
                        ~sub_supportdyn:None
                        ~this_ty:None
                        ~super_like:false
                        ~super_supportdyn:false
                        subtype
                        ty_super
                        env
                    | None -> default_subtype_help env)
                  | _ -> default_subtype_help env
                ))
          ))
      | (_, Tdynamic) ->
        (match ety_sub with
        | LoclType lty when is_dynamic lty -> valid env
        | ConstraintType _
        | LoclType _ ->
          default_subtype_help env)
      | (_, Tprim prim_ty) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          (match (deref lty, prim_ty) with
          | ((_, Tprim (Nast.Tint | Nast.Tfloat)), Nast.Tnum) -> valid env
          | ((_, Tprim (Nast.Tint | Nast.Tstring)), Nast.Tarraykey) -> valid env
          | ((_, Tprim prim_sub), _) when Aast.equal_tprim prim_sub prim_ty ->
            valid env
          | ((_, Toption arg_ty_sub), Nast.Tnull) ->
            simplify_subtype
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              arg_ty_sub
              ty_super
              env
          | (_, _) -> default_subtype_help env))
      | (_, Tany _) ->
        (match ety_sub with
        | ConstraintType cty -> begin
          match deref_constraint_type cty with
          | (_, (TCunion _ | TCintersection _)) -> default_subtype_help env
          | _ -> valid env
        end
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (_, Tany _) -> valid env
          | (_, (Tunion _ | Tintersection _ | Tvar _)) ->
            default_subtype_help env
          | _ when subtype_env.Subtype_env.no_top_bottom -> default env
          | _ -> valid env))
      | (r_super, Tfun ft_super) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          (match deref lty with
          | (r_sub, Tfun ft_sub) ->
            Subtype_fun.simplify_subtype_funs
              ~subtype_env
              ~check_return:true
              ~for_override:false
              ~super_like
              r_sub
              ft_sub
              r_super
              ft_super
              env
          | _ -> default_subtype_help env))
      | (_, Ttuple tyl_super) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
        | LoclType lty ->
          (match get_node lty with
          | Ttuple tyl_sub
            when Int.equal (List.length tyl_super) (List.length tyl_sub) ->
            wfold_left2
              (fun res ty_sub ty_super ->
                let ty_super = Sd.liken ~super_like env ty_super in
                res
                &&& simplify_subtype
                      ~subtype_env
                      ~sub_supportdyn
                      ~this_ty:None
                      ~super_like:false
                      ~super_supportdyn:false
                      ty_sub
                      ty_super)
              (env, TL.valid)
              tyl_sub
              tyl_super
          | _ -> default_subtype_help env))
      | ( r_super,
          Tshape
            {
              s_origin = origin_super;
              s_unknown_value = shape_kind_super;
              s_fields = fdm_super;
            } ) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          let (sub_supportdyn', env, lty) = TUtils.strip_supportdyn env lty in
          let sub_supportdyn =
            Option.is_some sub_supportdyn || sub_supportdyn'
          in
          (match deref lty with
          | ( r_sub,
              Tshape
                {
                  s_origin = origin_sub;
                  s_unknown_value = shape_kind_sub;
                  s_fields = fdm_sub;
                } ) ->
            if same_type_origin origin_super origin_sub then
              (* Fast path for shape types: if they have the same origin,
               * they are equal type. *)
              valid env
            else
              Subtype_shape.simplify_subtype_shape
                ~subtype_env
                ~env
                ~this_ty
                ~super_like
                (sub_supportdyn, r_sub, shape_kind_sub, fdm_sub)
                (super_supportdyn, r_super, shape_kind_super, fdm_super)
          | _ -> default_subtype_help env))
      | (_, Tvec_or_dict _) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          (match (get_node lty, get_node ty_super) with
          | (Tvec_or_dict (tk_sub, tv_sub), Tvec_or_dict (tk_super, tv_super))
            ->
            let tv_super = Sd.liken ~super_like env tv_super in
            let tk_super = Sd.liken ~super_like env tk_super in
            env
            |> simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 tk_sub
                 tk_super
            &&& simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  tv_sub
                  tv_super
          | ( Tclass ((_, n), _, [tk_sub; tv_sub]),
              Tvec_or_dict (tk_super, tv_super) )
            when String.equal n SN.Collections.cDict ->
            let tv_super = Sd.liken ~super_like env tv_super in
            let tk_super = Sd.liken ~super_like env tk_super in
            env
            |> simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 tk_sub
                 tk_super
            &&& simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  tv_sub
                  tv_super
          | (Tclass ((_, n), _, [tv_sub]), Tvec_or_dict (tk_super, tv_super))
            when String.equal n SN.Collections.cVec ->
            let pos = get_pos lty in
            let tk_sub = MakeType.int (Reason.Ridx_vector_from_decl pos) in
            let tv_super = Sd.liken ~super_like env tv_super in
            let tk_super = Sd.liken ~super_like env tk_super in
            env
            |> simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 tk_sub
                 tk_super
            &&& simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  tv_sub
                  tv_super
          | _ -> default_subtype_help env))
        (* If t supports dynamic, and t <: u, then t <: supportdyn<u> *)
      | (r_supportdyn, Tnewtype (name_super, [tyarg_super], _))
        when String.equal name_super SN.Classes.cSupportDyn ->
        (match ety_sub with
        | ConstraintType _cty ->
          (* TODO *)
          default_subtype_help env
        | LoclType lty_sub ->
          (match deref lty_sub with
          | (r, Tnewtype (name_sub, [tyarg_sub], _))
            when String.equal name_sub SN.Classes.cSupportDyn ->
            env
            |> simplify_subtype
                 ~subtype_env
                 ~this_ty
                 ~super_like
                 ~super_supportdyn:true
                 ~sub_supportdyn:(Some r)
                 tyarg_sub
                 tyarg_super
          | (_, Tvar _) -> default_subtype_help env
          | _ ->
            let ty_dyn = MakeType.dynamic r_supportdyn in
            env
            |> simplify_subtype
                 ~subtype_env
                 ~this_ty
                 ~sub_supportdyn
                 ~super_like
                 ~super_supportdyn:true
                 lty_sub
                 tyarg_super
            &&& simplify_dynamic_aware_subtype
                  ~subtype_env
                  ~this_ty
                  ~sub_supportdyn
                  ~super_like:false
                  ~super_supportdyn:false
                  lty_sub
                  ty_dyn))
      | (_, Tnewtype (name_super, tyl_super, _)) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          (match deref lty with
          | (_, Tclass ((_, name_sub), _, _))
            when String.equal name_sub name_super && Env.is_enum env name_super
            ->
            valid env
          | (_, Tnewtype (name_sub, tyl_sub, _))
            when String.equal name_sub name_super ->
            if List.is_empty tyl_sub then
              valid env
            else if Env.is_enum env name_super && Env.is_enum env name_sub then
              valid env
            else
              let td = Env.get_typedef env name_super in
              begin
                match td with
                | Decl_entry.Found { td_tparams; _ } ->
                  let variance_reifiedl =
                    List.map td_tparams ~f:(fun t ->
                        (t.tp_variance, t.tp_reified))
                  in
                  simplify_subtype_variance_for_non_injective
                    ~subtype_env
                    ~sub_supportdyn
                    ~super_like
                    name_sub
                    None
                    variance_reifiedl
                    tyl_sub
                    tyl_super
                    ety_sub
                    ety_super
                    env
                | Decl_entry.DoesNotExist
                | Decl_entry.NotYetAvailable ->
                  (* TODO(hverr): decl_entry propagate *)
                  invalid_env env
              end
          | (r, Toption ty_sub) ->
            let ty_null = MakeType.null r in
            (* Errors due to `null` should refer to full option type *)
            if_unsat
              invalid_env
              (simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 ty_null
                 ty_super
                 env)
            &&& simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  ty_sub
                  ty_super
          | (r, Tprim Aast.Tarraykey) ->
            let ty_string = MakeType.string r and ty_int = MakeType.int r in
            (* Use `if_unsat` so we report arraykey in the error *)
            if_unsat
              invalid_env
              begin
                env
                |> simplify_subtype
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like:false
                     ~super_supportdyn:false
                     ty_string
                     ty_super
                &&& simplify_subtype
                      ~subtype_env
                      ~sub_supportdyn
                      ~this_ty
                      ~super_like:false
                      ~super_supportdyn:false
                      ty_int
                      ty_super
              end
          | (_, Tgeneric _) when subtype_env.Subtype_env.require_completeness ->
            default_subtype_help env
          | _ ->
            (match Env.get_typedef env name_super with
            | Decl_entry.Found
                { td_type = lower; td_vis = Aast.CaseType; td_tparams; _ }
            | Decl_entry.Found
                { td_super_constraint = Some lower; td_tparams; _ } ->
              let try_lower_bound env =
                let ((env, cycle), lower_bound) =
                  (* The this_ty cannot does not need to be set because newtypes
                   * & case types cannot appear within classes thus cannot us
                   * the this type. If we ever change that this could needs to
                   * be changed *)
                  Phase.localize
                    ~ety_env:
                      {
                        empty_expand_env with
                        type_expansions =
                          (* Subtyping can be called when localizing
                             a union type, since we attempt to simplify it.
                             Since case types are encoded as union types,
                             a cyclic reference to the same case type will
                             lead to infinite looping. The chain is:
                               localize -> simplify_union -> sub_type -> localize

                             The expand environment is not threaded through the
                             whole way, so we won't be able to tell we entered a cycle.

                             For this reason we want to report cycles on the
                             case type we are currently expanding. If a cycle
                             occurs we say the proposition is invalid, but
                             don't report an error, since that will be done
                             during well-formedness checks on type defs *)
                          Type_expansions.empty_w_cycle_report
                            ~report_cycle:(Some (Pos.none, name_super));
                        substs =
                          (if List.is_empty tyl_super then
                            SMap.empty
                          else
                            Decl_subst.make_locl td_tparams tyl_super);
                      }
                    env
                    lower
                in
                (* If a cycle is detected, consider the case type as
                   uninhabited and thus an alias for the bottom type.
                   Handling of the bottom will be done as part of
                   [default_subtype] so we can consider this as invalid *)
                if Option.is_some cycle then
                  invalid_env env
                else
                  simplify_subtype
                    ~subtype_env
                    ~sub_supportdyn:None
                    ~this_ty:None
                    ~super_like
                    ~super_supportdyn:false
                    lty
                    lower_bound
                    env
              in
              default_subtype_help env ||| try_lower_bound
            | _ -> default_subtype_help env)))
      | (_, Tunapplied_alias n_sup) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType lty ->
          (match deref lty with
          | (_, Tunapplied_alias n_sub) when String.equal n_sub n_sup ->
            valid env
          | _ -> default_subtype_help env))
      | (r_super, Tneg (Neg_prim tprim_super)) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (r_sub, Tneg (Neg_prim tprim_sub)) ->
            simplify_subtype
              ~subtype_env
              ~sub_supportdyn:None
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              (MakeType.prim_type r_super tprim_super)
              (MakeType.prim_type r_sub tprim_sub)
              env
          | (_, Tneg (Neg_class _)) ->
            (* not C contains all primitive types, and so can't be a subtype of
               not p, which doesn't contain primitive type p *)
            invalid_env env
          | (_, Tprim tprim_sub) ->
            if Subtype_negation.is_tprim_disjoint tprim_sub tprim_super then
              valid env
            else
              invalid_env env
          | (_, Tclass ((_, cname), ex, _))
            when String.equal cname SN.Classes.cStringish
                 && is_nonexact ex
                 && Aast.(
                      equal_tprim tprim_super Tstring
                      || equal_tprim tprim_super Tarraykey) ->
            invalid_env env
          (* All of these are definitely disjoint from primitive types *)
          | (_, (Tfun _ | Ttuple _ | Tshape _ | Tclass _)) -> valid env
          | _ -> default_subtype_help env))
      | (_, Tneg (Neg_class (_, c_super))) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (_, Tneg (Neg_class (_, c_sub))) ->
            if TUtils.is_sub_class_refl env c_super c_sub then
              valid env
            else
              invalid_env env
          | (_, Tneg (Neg_prim _)) ->
            (* not p, for any primitive type p contains all class types, and so
               can't be a subtype of not c, which doesn't contain class types c *)
            invalid_env env
          | (_, Tclass ((_, c_sub), _, _)) ->
            if Subtype_negation.is_class_disjoint env c_sub c_super then
              valid env
            else
              invalid_env env
          (* All of these are definitely disjoint from class types *)
          | (_, (Tfun _ | Ttuple _ | Tshape _ | Tprim _)) -> valid env
          | _ -> default_subtype_help env))
      | (r_super, Tclass (x_super, Nonexact cr_super, tyl_super))
        when (not (Class_refinement.is_empty cr_super))
             && (subtype_env.Subtype_env.require_soundness
                || (* To deal with refinements, the code below generates a
                    * constraint type. That is currently not supported when
                    * require_soundness is not set (see below in the function
                    * decompose_subtype_add_prop). Consequently, if soundness
                    * is not required, we treat the refinement information
                    * only if we know for sure that we can discharge it on
                    * the spot; e.g., when ety_sub is a class-ish. This
                    * limits the information lost by skipping refinements. *)
                TUtils.is_class_i ety_sub) ->
        (* We discharge class refinements before anything
         * else ... *)
        Class_refinement.fold_refined_consts
          cr_super
          ~init:(valid env)
          ~f:(fun type_id { rc_bound; _ } (env, prop) ->
            (env, prop)
            &&&
            let (htm_lower, htm_upper) =
              match rc_bound with
              | TRexact ty -> (ty, ty)
              | TRloose { tr_lower; tr_upper } ->
                let loty = MakeType.union r_super tr_lower in
                let upty = MakeType.intersection r_super tr_upper in
                (loty, upty)
            in
            let htm_ty =
              let htm = { htm_id = type_id; htm_lower; htm_upper } in
              mk_constraint_type (r_super, Thas_type_member htm)
            in
            simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              ~super_supportdyn:false
              ety_sub
              (ConstraintType htm_ty))
        &&&
        (* ... then recursively check the class with all the
         * refinements dropped. *)
        let ty_super = mk (r_super, Tclass (x_super, nonexact, tyl_super)) in
        simplify_subtype_i
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~super_supportdyn:false
          ety_sub
          (LoclType ty_super)
      | (_, Tclass ((_, class_name), exact_super, tyl_super)) ->
        (match ety_sub with
        | ConstraintType _ -> default_subtype_help env
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (_, Tnewtype (enum_name, _, _))
            when String.equal enum_name class_name
                 && is_nonexact exact_super
                 && Env.is_enum env enum_name ->
            valid env
          | (_, Tnewtype (cid, _, _))
            when String.equal class_name SN.Classes.cHH_BuiltinEnum
                 && Env.is_enum env cid ->
            (match tyl_super with
            | [ty_super'] ->
              env
              |> simplify_subtype
                   ~subtype_env
                   ~sub_supportdyn
                   ~this_ty
                   ~super_like:false
                   ~super_supportdyn:false
                   ty_sub
                   ty_super'
            | _ -> default_subtype_help env)
          | (_, Tnewtype (enum_name, _, _))
            when String.equal enum_name class_name && Env.is_enum env enum_name
            ->
            valid env
          | (_, Tnewtype (enum_name, _, _))
            when Env.is_enum env enum_name
                 && String.equal class_name SN.Classes.cXHPChild ->
            valid env
          | (_, Tprim Nast.(Tstring | Tarraykey | Tint | Tfloat | Tnum))
            when String.equal class_name SN.Classes.cXHPChild
                 && is_nonexact exact_super ->
            valid env
          | (_, Tprim Nast.Tstring)
            when String.equal class_name SN.Classes.cStringish
                 && is_nonexact exact_super ->
            valid env
          (* Match what's done in unify for non-strict code *)
          | (_, Tclass _) ->
            Subtype_class.simplify_subtype_classes
              ~fail
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              ty_sub
              ty_super
              env
          | (_r_sub, Tvec_or_dict (_, tv)) ->
            (match (exact_super, tyl_super) with
            | (Nonexact _, [tv_super])
              when String.equal class_name SN.Collections.cTraversable
                   || String.equal class_name SN.Collections.cContainer ->
              (* vec<tv> <: Traversable<tv_super>
               * iff tv <: tv_super
               * Likewise for vec<tv> <: Container<tv_super>
               *          and map<_,tv> <: Traversable<tv_super>
               *          and map<_,tv> <: Container<tv_super>
               *)
              simplify_subtype
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like:false
                ~super_supportdyn:false
                tv
                tv_super
                env
            | (Nonexact _, [tk_super; tv_super])
              when String.equal class_name SN.Collections.cKeyedTraversable
                   || String.equal class_name SN.Collections.cKeyedContainer
                   || String.equal class_name SN.Collections.cAnyArray ->
              (match get_node ty_sub with
              | Tvec_or_dict (tk, _) ->
                env
                |> simplify_subtype
                     ~subtype_env
                     ~sub_supportdyn
                     ~this_ty
                     ~super_like:false
                     ~super_supportdyn:false
                     tk
                     tk_super
                &&& simplify_subtype
                      ~subtype_env
                      ~sub_supportdyn
                      ~this_ty
                      ~super_like:false
                      ~super_supportdyn:false
                      tv
                      tv_super
              | _ -> default_subtype_help env)
            | (Nonexact _, [])
              when String.equal class_name SN.Collections.cKeyedTraversable
                   || String.equal class_name SN.Collections.cKeyedContainer
                   || String.equal class_name SN.Collections.cAnyArray ->
              (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
              valid env
            | (_, _) -> default_subtype_help env)
          | _ -> default_subtype_help env)))

  (* Given a type constructor N that may not be injective (e.g., a newtype)
   * t1 <:v1> u1 /\ ... /\ tn <:vn> un
   * implies
   * N<t1, .., tn> <: N<u1, .., un>
   * where vi is the variance of the i'th generic parameter of N,
   * and <:v denotes the appropriate direction of subtyping for variance v.
   * However, the reverse direction does not hold. *)
  and simplify_subtype_variance_for_non_injective
      ~(subtype_env : Subtype_env.t)
      ~sub_supportdyn
      ~super_like
      (cid : string)
      (class_sub : Cls.t option)
      (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
      (children_tyl : locl_ty list)
      (super_tyl : locl_ty list)
      ty_sub
      ty_super
      env =
    let ((env, p) as res) =
      Subtype_injective_ctor.simplify_subtype_variance_for_injective
        ~subtype_env
        ~sub_supportdyn
        ~super_like
        cid
        class_sub
        variance_reifiedl
        children_tyl
        super_tyl
        env
    in
    if subtype_env.Subtype_env.require_completeness && not (TL.is_valid p) then
      (* If we require completeness, then we can still use the incomplete
       * N<t1, .., tn> <: N<u1, .., un> to t1 <:v1> u1 /\ ... /\ tn <:vn> un
       * simplification if all of the latter constraints already hold.
       * If they don't already hold, there is nothing we can (soundly) simplify. *)
      if subtype_env.Subtype_env.require_soundness then
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          ty_sub
          ty_super
      else
        (env, TL.valid)
    else
      res

  (** [simplify_subtype_arraykey_union env ty_sub tyl_super] implements a special purpose typing
  rule for t <: arraykey | tvar by checking t & not arraykey <: tvar. It also works for
  not arraykey | tvar. By only applying if B is a type variable, we avoid oscillating
  forever between this rule and the generic one that moves from t1 & arraykey <: t2.
  to t1 <: t2 | not arraykey. This is similar to our treatment of A <: ?B iff
  A & nonnull <: B. This returns a subtyp_prop if the pattern this rule looks for matched,
  and returns None if it did not, so that this rule does not apply. ) *)
  and simplify_subtype_arraykey_union
      ~this_ty ~sub_supportdyn ~subtype_env env ty_sub tyl_super =
    match tyl_super with
    | [ty_super1; ty_super2] ->
      let (env, ty_super1) = Env.expand_type env ty_super1 in
      let (env, ty_super2) = Env.expand_type env ty_super2 in
      (match (deref ty_super1, deref ty_super2) with
      | ( ((_, Tvar _) as tvar_ty),
          ((_, (Tprim Aast.Tarraykey | Tneg (Neg_prim Aast.Tarraykey))) as
          ak_ty) )
      | ( ((_, (Tprim Aast.Tarraykey | Tneg (Neg_prim Aast.Tarraykey))) as ak_ty),
          ((_, Tvar _) as tvar_ty) ) ->
        let (env, neg_ty) =
          Inter.negate_type
            env
            (get_reason (mk ak_ty))
            ~approx:Inter.Utils.ApproxDown
            (mk ak_ty)
        in
        let (env, inter_ty) =
          Inter.intersect env ~r:(get_reason ty_sub) neg_ty ty_sub
        in
        let (env, props) =
          Subtype.simplify_subtype_i
            ~this_ty
            ~sub_supportdyn
            ~subtype_env
            ~super_like:false
            ~super_supportdyn:false
            (LoclType inter_ty)
            (LoclType (mk tvar_ty))
            env
        in
        (env, Some props)
      | _ -> (env, None))
    | _ -> (env, None)
end

and Can_index : sig
  val simplify_subtype_can_index :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t * Typing_defs.can_index ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_can_index
      ~subtype_env ~sub_supportdyn ~this_ty ~fail ty_sub ty_super (_r, _ci) env
      =
    (* TODO: implement *)
    Subtype.default_subtype
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~super_like:false
      ~fail
      env
      ty_sub
      ty_super
end

and Can_traverse : sig
  val simplify_subtype_can_traverse :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t * Typing_defs.can_traverse ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_can_traverse
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~fail
      ty_sub
      ty_super
      ((_r : Reason.t), ct)
      env =
    Logging.log_subtype_i
      ~level:2
      ~this_ty
      ~function_name:"simplify_subtype_can_traverse"
      env
      ty_sub
      ty_super;
    match ty_sub with
    | ConstraintType _ ->
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like:false
        ~fail
        env
        ty_sub
        ty_super
    | LoclType lty_sub ->
      if TUtils.is_tyvar_error env lty_sub then
        let trav_ty = can_traverse_to_iface ct in
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like:false
          ~super_supportdyn:false
          lty_sub
          trav_ty
          env
      else
        let subtype_with_dynamic () =
          Subtype.simplify_subtype
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like:false
            ~super_supportdyn:false
            lty_sub
            ct.ct_val
            env
          &&&
          match ct.ct_key with
          | None -> valid
          | Some ct_key ->
            Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              lty_sub
              ct_key
        in
        (match get_node lty_sub with
        | Tdynamic -> subtype_with_dynamic ()
        | _
          when Option.is_some sub_supportdyn
               && TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
               && Tast.is_under_dynamic_assumptions env.checked ->
          subtype_with_dynamic ()
        | Tclass _
        | Tvec_or_dict _
        | Tany _ ->
          let trav_ty = can_traverse_to_iface ct in
          Subtype.simplify_subtype
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like:false
            ~super_supportdyn:false
            lty_sub
            trav_ty
            env
        | _ ->
          Subtype.default_subtype
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like:false
            ~fail
            env
            ty_sub
            ty_super)
end

and Has_type_member : sig
  val simplify_subtype_has_type_member :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t * Typing_defs.has_type_member ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_has_type_member
      ~subtype_env ~sub_supportdyn ~this_ty ~fail ty_sub (r, htm) env =
    let { htm_id = memid; htm_lower = memloty; htm_upper = memupty } = htm in
    let htmty = ConstraintType (mk_constraint_type (r, Thas_type_member htm)) in
    Logging.log_subtype_i
      ~level:2
      ~this_ty
      ~function_name:"simplify_subtype_has_type_member"
      env
      ty_sub
      htmty;
    let (env, ety_sub) = Env.expand_internal_type env ty_sub in
    let default_subtype_help env =
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like:false
        ~fail
        env
        ety_sub
        htmty
    in
    let simplify_subtype_bound kind ~bound ty env =
      let on_error =
        Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
            let open Typing_error in
            let pos = Reason.to_pos (get_reason bound) in
            Reasons_callback.prepend_on_apply
              on_error
              (Secondary.Violated_refinement_constraint { cstr = (kind, pos) }))
      in
      let subtype_env = Subtype_env.set_on_error subtype_env on_error in
      let this_ty = None in
      match kind with
      | `As ->
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty
          ~super_like:false
          ~super_supportdyn:false
          ty
          bound
          env
      | `Super ->
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty
          ~super_like:false
          ~super_supportdyn:false
          bound
          ty
          env
    in
    match ety_sub with
    | ConstraintType _ -> invalid ~fail env
    | LoclType ty_sub ->
      let concrete_rigid_tvar_access env ucckind bndtys =
        (* First, we try to discharge the subtype query on the bound; if
         * that fails, we mint a fresh rigid type variable to represent
         * the concrete type constant and try to solve the query using it *)
        let ( ||| ) = ( ||| ) ~fail in
        let bndty = MakeType.intersection (get_reason ty_sub) bndtys in
        Subtype.simplify_subtype_i
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty
          ~super_like:false
          ~super_supportdyn:false
          (LoclType bndty)
          htmty
          env
        ||| fun env ->
        (* TODO(refinements): The treatment of `this_ty` below is
         * no good; see below. *)
        let (env, dtmemty) =
          Typing_type_member.make_type_member
            env
            ~this_ty:(Option.value this_ty ~default:ty_sub)
            ~on_error:subtype_env.Subtype_env.on_error
            ucckind
            bndtys
            (Reason.to_pos r, memid)
        in
        simplify_subtype_bound `As dtmemty ~bound:memupty env
        &&& simplify_subtype_bound `Super ~bound:memloty dtmemty
      in
      (match deref ty_sub with
      | (_r_sub, Tclass (x_sub, exact_sub, _tyl_sub)) ->
        let (env, type_member) =
          (* TODO(refinements): The treatment of `this_ty` below is
           * no good; we should not default to `ty_sub`. `this_ty`
           * will be used when a type constant refers to another
           * constant either in its def or in its bounds.
           * See related FIXME(T59448452) above. *)
          Typing_type_member.lookup_class_type_member
            env
            ~this_ty:(Option.value this_ty ~default:ty_sub)
            ~on_error:subtype_env.Subtype_env.on_error
            (x_sub, exact_sub)
            (Reason.to_pos r, memid)
        in
        (match type_member with
        | Typing_type_member.NotYetAvailable ->
          failwith "TODO(hverr): propagate decl_entry"
        | Typing_type_member.Error err -> invalid ~fail:err env
        | Typing_type_member.Exact ty ->
          simplify_subtype_bound `As ty ~bound:memupty env
          &&& simplify_subtype_bound `Super ~bound:memloty ty
        | Typing_type_member.Abstract { name; lower = loty; upper = upty } ->
          let r_bnd = Reason.Rtconst_no_cstr name in
          let loty = Option.value ~default:(MakeType.nothing r_bnd) loty in
          let upty = Option.value ~default:(MakeType.mixed r_bnd) upty in
          (* In case the refinement is exact we check that upty <: loty;
           * doing the check early gives us a better chance at generating
           * good error messages. The unification errors we get when
           * doing this check are usually unhelpful, so we drop them. *)
          let is_exact = phys_equal memloty memupty in
          (if is_exact then
            let drop_sub_reasons =
              Option.map
                subtype_env.Subtype_env.on_error
                ~f:Typing_error.Reasons_callback.drop_reasons_on_apply
            in
            let subtype_env =
              Subtype_env.set_on_error subtype_env drop_sub_reasons
            in
            Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn:None
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              upty
              loty
              env
          else
            valid env)
          &&& simplify_subtype_bound `As upty ~bound:memupty
          &&& simplify_subtype_bound `Super ~bound:memloty loty)
      | (_r_sub, Tdependent (DTexpr eid, bndty)) ->
        concrete_rigid_tvar_access env (Typing_type_member.EDT eid) [bndty]
      | (_r_sub, Tgeneric (s, ty_args)) when String.equal s SN.Typehints.this ->
        let bnd_tys =
          Typing_set.elements (Env.get_upper_bounds env s ty_args)
        in
        concrete_rigid_tvar_access env Typing_type_member.This bnd_tys
      | (_, (Tvar _ | Tgeneric _ | Tunion _ | Tintersection _)) ->
        default_subtype_help env
      | _ -> invalid ~fail env)
end

and Has_member : sig
  val simplify_subtype_has_member :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    fail:Typing_error.t option ->
    nullsafe:Typing_defs.locl_phase Typing_defs.Reason.t_ option ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t * Typing_defs.has_member ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  (* This is a duplicate of logic in Typing_error_utils, due to conversion of primary errors to secondary errors
     on some code paths for Typing_object_get, which throws out quickfix information (unsafe for secondary errors). *)
  let add_obj_get_quickfixes
      ty_err (on_error : Typing_error.Reasons_callback.t option) :
      Typing_error.Reasons_callback.t option =
    match ty_err with
    | Typing_error.(Error.Primary (Primary.Null_member { pos; obj_pos_opt; _ }))
      ->
      let quickfixes =
        match obj_pos_opt with
        | Some obj_pos ->
          let (obj_pos_start_line, _) = Pos.line_column obj_pos in
          let (rhs_pos_start_line, rhs_pos_start_column) =
            Pos.line_column pos
          in
          (*
        heuristic: if the lhs and rhs of the Objget are on the same line, then we assume they are
        separated by two characters (`->`). So we do not generate a quickfix for chained Objgets:
        ```
        obj
        ->rhs
        ```
      *)
          if obj_pos_start_line = rhs_pos_start_line then
            let width = 2 (* length of "->" *) in
            let quickfix_pos =
              pos
              |> Pos.set_col_start (rhs_pos_start_column - width)
              |> Pos.set_col_end rhs_pos_start_column
            in
            [
              Quickfix.make
                ~title:"Add null-safe get"
                ~new_text:"?->"
                quickfix_pos;
            ]
          else
            []
        | None -> []
      in
      Option.map
        ~f:(fun cb ->
          Typing_error.Reasons_callback.add_quickfixes cb quickfixes)
        on_error
    | _ -> on_error

  let rec simplify_subtype_has_member
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~fail
      ~nullsafe
      ty_sub
      (r, has_member_ty)
      env =
    let using_new_method_call_inference =
      TypecheckerOptions.method_call_inference (Env.get_tcopt env)
    in
    let {
      hm_name = (name_pos, name_) as name;
      hm_type = member_ty;
      hm_class_id = class_id;
      hm_explicit_targs = explicit_targs;
    } =
      has_member_ty
    in
    let is_method = Option.is_some explicit_targs in
    (* If `nullsafe` is `Some _`, we are allowing the object type on LHS to be nullable. *)
    let mk_maybe_nullable env ty =
      match nullsafe with
      | None -> (env, ty)
      | Some r_null ->
        let null_ty = MakeType.null r_null in
        Typing_union.union_i env r_null ty null_ty
    in
    let (env, maybe_nullable_ty_super) =
      let ty_super = mk_constraint_type (r, Thas_member has_member_ty) in
      mk_maybe_nullable env (ConstraintType ty_super)
    in

    Logging.log_subtype_i
      ~level:2
      ~this_ty
      ~function_name:"simplify_subtype_has_member"
      env
      ty_sub
      maybe_nullable_ty_super;
    let (env, ety_sub) = Env.expand_internal_type env ty_sub in
    let default_subtype_help env =
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like:false
        ~fail
        env
        ety_sub
        maybe_nullable_ty_super
    in
    match ety_sub with
    | ConstraintType cty ->
      (match deref_constraint_type cty with
      | ( _,
          Thas_member
            {
              hm_name = name_sub;
              hm_type = ty_sub;
              hm_class_id = cid_sub;
              hm_explicit_targs = explicit_targs_sub;
            } ) ->
        if
          let targ_equal (_, (_, hint1)) (_, (_, hint2)) =
            Aast_defs.equal_hint_ hint1 hint2
          in
          String.equal (snd name_sub) name_
          && class_id_equal cid_sub class_id
          && Option.equal
               (List.equal targ_equal)
               explicit_targs_sub
               explicit_targs
        then
          Subtype.simplify_subtype
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like:false
            ~super_supportdyn:false
            ty_sub
            member_ty
            env
        else
          invalid ~fail env
      | _ -> default_subtype_help env)
    | LoclType ty_sub ->
      (match deref ty_sub with
      | (_, (Tvar _ | Tunion _)) -> default_subtype_help env
      | (r_null, Tprim Aast.Tnull) when using_new_method_call_inference ->
        if Option.is_some nullsafe then
          valid env
        else
          invalid
            env
            ~fail:
              (Some
                 Typing_error.(
                   primary
                   @@ Primary.Null_member
                        {
                          pos = name_pos;
                          obj_pos_opt = None;
                          member_name = name_;
                          reason =
                            lazy (Reason.to_string "This can be null" r_null);
                          kind =
                            (if is_method then
                              `method_
                            else
                              `property);
                          ctxt = `read;
                        }))
      | (r_option, Toption option_ty) when using_new_method_call_inference ->
        if Option.is_some nullsafe then
          simplify_subtype_has_member
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~fail
            ~nullsafe
            (LoclType option_ty)
            (r, has_member_ty)
            env
        else
          let (env, option_ty) = Env.expand_type env option_ty in
          (match get_node option_ty with
          | Tnonnull ->
            invalid
              env
              ~fail:
                (Some
                   Typing_error.(
                     primary
                     @@ Primary.Top_member
                          {
                            pos = name_pos;
                            name = name_;
                            ctxt = `read;
                            kind =
                              (if is_method then
                                `method_
                              else
                                `property);
                            is_nullable = true;
                            decl_pos = Reason.to_pos r_option;
                            ty_name = lazy (Typing_print.error env ty_sub);
                            (* Subtyping already gives these reasons *)
                            ty_reasons = lazy [];
                          }))
          | _ ->
            invalid
              env
              ~fail:
                (Some
                   Typing_error.(
                     primary
                     @@ Primary.Null_member
                          {
                            pos = name_pos;
                            obj_pos_opt = None;
                            member_name = name_;
                            reason =
                              lazy
                                (Reason.to_string "This can be null" r_option);
                            kind =
                              (if is_method then
                                `method_
                              else
                                `property);
                            ctxt = `read;
                          })))
      | (_, Tintersection tyl)
        when let (_, non_ty_opt, _) =
               Subtype_negation.find_type_with_exact_negation env tyl
             in
             Option.is_some non_ty_opt ->
        (* use default_subtype to perform: A & B <: C <=> A <: C | !B *)
        default_subtype_help env
      | (r_inter, Tintersection []) ->
        (* Tintersection [] = mixed *)
        invalid
          env
          ~fail:
            (Some
               Typing_error.(
                 primary
                 @@ Primary.Top_member
                      {
                        pos = name_pos;
                        name = name_;
                        is_nullable = true;
                        kind =
                          (if is_method then
                            `method_
                          else
                            `property);
                        ctxt = `read;
                        decl_pos = Reason.to_pos r_inter;
                        ty_name = lazy (Typing_print.error env ty_sub);
                        (* Subtyping already gives these reasons *)
                        ty_reasons = lazy [];
                      }))
      | (r_inter, Tintersection tyl) when using_new_method_call_inference ->
        let (env, tyl) = List.map_env ~f:Env.expand_type env tyl in
        let subtype_fresh_has_member_ty env ty_sub =
          let (env, fresh_tyvar) = Env.fresh_type env name_pos in
          let env = Env.set_tyvar_variance env fresh_tyvar in
          let fresh_has_member_ty =
            mk_constraint_type
              (r, Thas_member { has_member_ty with hm_type = fresh_tyvar })
          in
          let (env, maybe_nullable_fresh_has_member_ty) =
            mk_maybe_nullable env (ConstraintType fresh_has_member_ty)
          in
          let (env, ty_err_opt) =
            Subtype_tell.sub_type_inner
              env
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              (LoclType ty_sub)
              maybe_nullable_fresh_has_member_ty
          in
          match ty_err_opt with
          | None ->
            let (env, _ty_err_opt) =
              match get_var fresh_tyvar with
              | Some var ->
                (* TODO: can this actually generate an error? *)
                Typing_solver.solve_to_equal_bound_or_wrt_variance
                  env
                  Reason.Rnone
                  var
              | None -> (env, None)
            in
            ((env, None), Some fresh_tyvar)
          | Some _ -> ((env, ty_err_opt), None)
        in
        let ((env, ty_err_opt), fresh_tyvar_opts) =
          TUtils.run_on_intersection_with_ty_err
            env
            tyl
            ~f:subtype_fresh_has_member_ty
        in
        let fresh_tyvars = List.filter_map ~f:Fn.id fresh_tyvar_opts in
        if List.is_empty fresh_tyvars then
          invalid ~fail:ty_err_opt env
        else
          let (env, intersection_ty) =
            Inter.intersect_list env r_inter fresh_tyvars
          in
          Subtype.simplify_subtype
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like:false
            ~super_supportdyn:false
            intersection_ty
            member_ty
            env
      | (r1, Tnewtype (n, _, newtype_ty)) ->
        let sub_supportdyn =
          match sub_supportdyn with
          | None ->
            if String.equal n SN.Classes.cSupportDyn then
              Some r1
            else
              None
          | _ -> sub_supportdyn
        in
        simplify_subtype_has_member
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~fail
          ~nullsafe
          (LoclType newtype_ty)
          (r, has_member_ty)
          env
      (* TODO
         | (_, Tdependent _) ->
         | (_, Tgeneric _) ->
      *)
      | _ ->
        let explicit_targs =
          match explicit_targs with
          | None -> []
          | Some targs -> targs
        in
        let (res, (obj_get_ty, _tal)) =
          Typing_object_get.obj_get
            ~obj_pos:name_pos
              (* `~obj_pos:name_pos` is a lie: `name_pos` is the rhs of `->` or `?->` *)
            ~is_method
            ~inst_meth:false
            ~meth_caller:false
            ~coerce_from_ty:None
            ~nullsafe
            ~explicit_targs
            ~class_id
            ~member_id:name
            ~on_error:Typing_error.Callback.unify_error
            env
            ty_sub
        in
        let prop =
          match res with
          | (env, None) -> valid env
          | (env, Some ty_err) ->
            let on_error =
              add_obj_get_quickfixes ty_err subtype_env.Subtype_env.on_error
            in
            (* TODO - this needs to somehow(?) account for the fact that the old
               code considered FIXMEs in this position *)
            let fail =
              Option.map
                on_error
                ~f:
                  Typing_error.(
                    fun on_error ->
                      apply_reasons ~on_error @@ Secondary.Of_error ty_err)
            in
            invalid env ~fail
        in

        prop
        &&& Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn:None
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              obj_get_ty
              member_ty)
end

and Subtype_class : sig
  (** Suptyping when the two types are classish *)
  val simplify_subtype_classes :
    fail:Typing_error.t option ->
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    Typing_defs.locl_ty ->
    Typing_defs.locl_ty ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let rec simplify_subtype_classes
      ~fail
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      ~(super_like : bool)
      ty_sub
      ty_super
      env : env * TL.subtype_prop =
    let invalid_env = invalid ~fail in
    let ( ||| ) = ( ||| ) ~fail in
    match (deref ty_sub, deref ty_super) with
    | ( (r_sub, Tclass (x_sub, exact_sub, tyl_sub)),
        (_r_super, Tclass (x_super, exact_super, tyl_super)) ) ->
      let (cid_super, cid_sub) = (snd x_super, snd x_sub) in
      let (exact_match, both_exact) =
        match (exact_sub, exact_super) with
        | (Nonexact _, Exact) -> (false, false)
        | (Exact, Exact) -> (true, true)
        | (_, _) -> (true, false)
      in
      if String.equal cid_super cid_sub then
        if List.is_empty tyl_sub && List.is_empty tyl_super && exact_match then
          valid env
        else
          (* This is side-effecting as it registers a dependency *)
          let class_def_sub = Env.get_class env cid_sub in
          (* If class is final then exactness is superfluous *)
          let (has_generics, is_final) =
            match class_def_sub with
            | Decl_entry.Found tc ->
              (not (List.is_empty (Cls.tparams tc)), Cls.final tc)
            | Decl_entry.DoesNotExist
            | Decl_entry.NotYetAvailable ->
              (false, false)
          in
          if not (exact_match || is_final) then
            invalid_env env
          else if has_generics && List.is_empty tyl_super then
            (* C<t> <: C where C represents all possible instantiations of C's generics *)
            valid env
          else if has_generics && List.is_empty tyl_sub then
            (* C </: C<t>, since C's generic can be instantiated to other things than t *)
            invalid_env env
          else
            let variance_reifiedl =
              if List.is_empty tyl_sub then
                []
              else if both_exact then
                (* Subtyping exact class types following variance
                 * annotations is unsound in general (see T142810099).
                 * When the class is exact, we must treat all generic
                 * parameters as invariant.
                 *)
                List.map tyl_sub ~f:(fun _ -> (Ast_defs.Invariant, Aast.Erased))
              else
                match class_def_sub with
                | Decl_entry.DoesNotExist
                | Decl_entry.NotYetAvailable ->
                  List.map tyl_sub ~f:(fun _ ->
                      (Ast_defs.Invariant, Aast.Erased))
                | Decl_entry.Found class_sub ->
                  List.map (Cls.tparams class_sub) ~f:(fun t ->
                      (t.tp_variance, t.tp_reified))
            in
            Subtype_injective_ctor.simplify_subtype_variance_for_injective
              ~subtype_env
              ~sub_supportdyn
              ~super_like
              cid_sub
              (Decl_entry.to_option class_def_sub)
              variance_reifiedl
              tyl_sub
              tyl_super
              env
      else if not exact_match then
        invalid_env env
      else
        let class_def_sub = Env.get_class env cid_sub in
        (match class_def_sub with
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          (* This should have been caught already in the naming phase *)
          valid env
        | Decl_entry.Found class_sub ->
          (* We handle the case where a generic A<T> is used as A for the sub-class.
             This works because there will be no locls to substitute for type parameters
             T in the type build by get_ancestor. If T does show up in that type, then
             the call to simplify subtype will fail. This is what we expect since we
             would need it to be a sub-type of the super-type for all T. If T is not there,
             then simplify_subtype should succeed. *)
          let ety_env =
            {
              empty_expand_env with
              substs =
                TUtils.make_locl_subst_for_class_tparams class_sub tyl_sub;
              (* FIXME(T59448452): Unsound in general *)
              this_ty = Option.value this_ty ~default:ty_sub;
            }
          in
          let up_obj = Cls.get_ancestor class_sub cid_super in
          (match up_obj with
          | Some up_obj ->
            (* Since we have provided no `Typing_error.Reasons_callback.t`
             * in the `expand_env`, this will not generate any errors *)
            let ((env, _ty_err_opt), up_obj) =
              Phase.localize ~ety_env env up_obj
            in
            simplify_subtype_classes
              ~fail
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              up_obj
              ty_super
              env
          | None ->
            if
              Ast_defs.is_c_trait (Cls.kind class_sub)
              || Ast_defs.is_c_interface (Cls.kind class_sub)
            then
              let reqs_class =
                List.map
                  (Cls.all_ancestor_req_class_requirements class_sub)
                  ~f:snd
              in
              let rec try_upper_bounds_on_this up_objs env =
                match up_objs with
                | [] ->
                  (* It's crucial that we don't lose updates to tpenv in
                   * env that were introduced by Phase.localize.
                   * TODO: avoid this requirement *)
                  invalid_env env
                | ub_obj_typ :: up_objs
                  when List.mem reqs_class ub_obj_typ ~equal:equal_decl_ty ->
                  (* `require class` constraints do not induce subtyping,
                   * so skipping them *)
                  try_upper_bounds_on_this up_objs env
                | ub_obj_typ :: up_objs ->
                  (* A trait is never the runtime type, but it can be used
                   * as a constraint if it has requirements or where
                   * constraints for its using classes *)
                  (* Since we have provided no `Typing_error.Reasons_callback.t`
                   * in the `expand_env`, this will not generate any errors *)
                  let ((env, _ty_err_opt), ub_obj_typ) =
                    Phase.localize ~ety_env env ub_obj_typ
                  in
                  env
                  |> Subtype.simplify_subtype
                       ~subtype_env
                       ~sub_supportdyn
                       ~this_ty
                       ~super_like:false
                       ~super_supportdyn:false
                       (mk (r_sub, get_node ub_obj_typ))
                       ty_super
                  ||| try_upper_bounds_on_this up_objs
              in
              try_upper_bounds_on_this (Cls.upper_bounds_on_this class_sub) env
            else
              invalid_env env))
    | (_, _) -> invalid_env env
end

and Subtype_injective_ctor : sig
  (** Given an injective type constructor C (e.g., a class)
    C<t1, .., tn> <: C<u1, .., un> iff
    t1 <:v1> u1 /\ ... /\ tn <:vn> un
    where vi is the variance of the i'th generic parameter of C,
    and <:v denotes the appropriate direction of subtyping for variance v *)
  val simplify_subtype_variance_for_injective :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Typing_defs.locl_phase Typing_defs.Reason.t_ option ->
    super_like:bool ->
    string ->
    Cls.t option ->
    (Ast_defs.variance * Aast.reify_kind) list ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let rec simplify_subtype_variance_for_injective_loop
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~super_like
      (cid : string)
      (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
      (children_tyl : locl_ty list)
      (super_tyl : locl_ty list) : env -> env * TL.subtype_prop =
   fun env ->
    let simplify_subtype_help reify_kind =
      (* When doing coercions from dynamic we treat dynamic as a bottom type. This is generally
         correct, except for the case when the generic isn't erased. When a generic is
         reified it is enforced as if it is it's own separate class in the runtime. i.e.
         In the code:

           class Box<reify T> {}
           function box_int(): Box<int> { return new Box<~int>(); }

         If is enforced like:
           class Box<reify T> {}
           class Box_int extends Box<int> {}
           class Box_like_int extends Box<~int> {}

           function box_int(): Box_int { return new Box_like_int(); }

         Thus we cannot push the like type to the outside of generic like we can
         we erased generics.
      *)
      let subtype_env =
        if
          (not Aast.(equal_reify_kind reify_kind Erased))
          && Subtype_env.coercing_from_dynamic subtype_env
        then
          Subtype_env.{ subtype_env with coerce = None }
        else
          subtype_env
      in
      Subtype.simplify_subtype
        ~subtype_env
        ~this_ty:None
        ~super_like:false
        ~super_supportdyn:false
    in
    let simplify_subtype_variance_for_injective_loop_help =
      simplify_subtype_variance_for_injective_loop
        ~subtype_env
        ~sub_supportdyn
        ~super_like
    in
    match (variance_reifiedl, children_tyl, super_tyl) with
    | ([], _, _)
    | (_, [], _)
    | (_, _, []) ->
      valid env
    | ( (variance, reify_kind) :: variance_reifiedl,
        child :: childrenl,
        super :: superl ) ->
      let simplify_subtype_help = simplify_subtype_help reify_kind in
      begin
        match variance with
        | Ast_defs.Covariant ->
          let super = Sd.liken ~super_like env super in
          simplify_subtype_help ~sub_supportdyn child super env
        | Ast_defs.Contravariant ->
          let super =
            mk
              ( Reason.Rcontravariant_generic (get_reason super, cid),
                get_node super )
          in
          simplify_subtype_help ~sub_supportdyn super child env
        | Ast_defs.Invariant ->
          let super' =
            mk
              (Reason.Rinvariant_generic (get_reason super, cid), get_node super)
          in
          env
          |> simplify_subtype_help
               ~sub_supportdyn
               child
               (Sd.liken ~super_like env super')
          &&& simplify_subtype_help
                ~sub_supportdyn
                super'
                (Sd.liken ~super_like env child)
      end
      &&& simplify_subtype_variance_for_injective_loop_help
            cid
            variance_reifiedl
            childrenl
            superl

  let simplify_subtype_variance_for_injective
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~super_like
      (cid : string)
      (class_sub : Cls.t option) =
    (* Before looping through the generic arguments, check to see if we should push
       supportdyn onto them. This depends on the generic class itself. *)
    let sub_supportdyn =
      match (sub_supportdyn, class_sub) with
      | (None, _)
      | (_, None) ->
        None
      | (Some _, Some class_sub) ->
        if
          String.equal cid SN.Collections.cTraversable
          || String.equal cid SN.Collections.cKeyedTraversable
          || String.equal cid SN.Collections.cContainer
          || Cls.has_ancestor class_sub SN.Collections.cContainer
        then
          sub_supportdyn
        else
          None
    in
    simplify_subtype_variance_for_injective_loop
      ~subtype_env
      ~sub_supportdyn
      ~super_like
      cid
end

and Subtype_shape : sig
  val simplify_subtype_shape :
    subtype_env:Subtype_env.t ->
    env:Typing_env_types.env ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    bool
    * Typing_defs.locl_phase Typing_defs.Reason.t_
    * Typing_defs.locl_phase Typing_defs.ty
    * Typing_defs.locl_phase Typing_defs.shape_field_type
      Typing_defs.TShapeMap.t ->
    bool
    * Typing_defs.locl_phase Typing_defs.Reason.t_
    * Typing_defs.locl_phase Typing_defs.ty
    * Typing_defs.locl_phase Typing_defs.shape_field_type
      Typing_defs.TShapeMap.t ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_shape
      ~(subtype_env : Subtype_env.t)
      ~(env : env)
      ~(this_ty : locl_ty option)
      ~super_like
      (supportdyn_sub, r_sub, shape_kind_sub, fdm_sub)
      (supportdyn_super, r_super, shape_kind_super, fdm_super) =
    (*
    Shape projection for shape type `s` and field `f` (`s |_ f`) is defined as:
      - if `f` appears in `s` as `f => ty` then `s |_ f` = `Required ty`
      - if `f` appears in `s` as `?f => ty` then `s |_ f` = `Optional ty`
      - if `f` does not appear in `s` and `s` is closed, then `s |_ f` = `Absent`
      - if `f` does not appear in `s` and `s` is open, then `s |_ f` = `Optional mixed`

    EXCEPT
      - `?f => nothing` should be ignored, and treated as `Absent`.
        Such a field cannot be given a value, and so is effectively not present.
  *)
    let shape_projection ~supportdyn field_name shape_kind shape_map r =
      let make_supportdyn ty =
        if
          supportdyn
          && not
               (Subtype_ask.is_sub_type_for_union_i
                  env
                  (LoclType ty)
                  (LoclType (MakeType.supportdyn_mixed ~mixed_reason:r r)))
        then
          MakeType.supportdyn r ty
        else
          ty
      in

      match TShapeMap.find_opt field_name shape_map with
      | Some { sft_ty; sft_optional } ->
        (match (deref sft_ty, sft_optional) with
        | ((_, Tunion []), true) -> `Absent
        | (_, true) -> `Optional (make_supportdyn sft_ty)
        | (_, false) -> `Required (make_supportdyn sft_ty))
      | None ->
        if TUtils.is_nothing env shape_kind then
          `Absent
        else
          let printable_name =
            TUtils.get_printable_shape_field_name field_name
          in
          let ty =
            with_reason
              shape_kind
              (Reason.Rmissing_optional_field (Reason.to_pos r, printable_name))
          in
          `Optional (make_supportdyn ty)
    in
    (*
    For two particular projections `p1` and `p2`, `p1` <: `p2` iff:
      - `p1` = `Required ty1`, `p2` = `Required ty2`, and `ty1` <: `ty2`
      - `p1` = `Required ty1`, `p2` = `Optional ty2`, and `ty1` <: `ty2`
      - `p1` = `Optional ty1`, `p2` = `Optional ty2`, and `ty1` <: `ty2`
      - `p1` = `Absent`, `p2` = `Optional ty2`
      - `p1` = `Absent`, `p2` = `Absent`
    We therefore need to handle all other cases appropriately.
  *)
    let simplify_subtype_shape_projection
        (r_sub, proj_sub) (r_super, proj_super) field_name res =
      let field_pos = TShapeField.pos field_name in
      let printable_name = TUtils.get_printable_shape_field_name field_name in
      match (proj_sub, proj_super) with
      (***** "Successful" cases - 5 / 9 total cases *****)
      | (`Required sub_ty, `Required super_ty)
      | (`Required sub_ty, `Optional super_ty)
      | (`Optional sub_ty, `Optional super_ty) ->
        let super_ty = Sd.liken ~super_like env super_ty in

        res
        &&& Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn:
                (if supportdyn_sub then
                  Some r_sub
                else
                  None)
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              sub_ty
              super_ty
      | (`Absent, `Optional _)
      | (`Absent, `Absent) ->
        res
      (***** Error cases - 4 / 9 total cases *****)
      | (`Required _, `Absent)
      | (`Optional _, `Absent) ->
        let ty_err_opt =
          Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Missing_field
                       {
                         pos = Reason.to_pos r_super;
                         decl_pos = field_pos;
                         name = printable_name;
                       })
        in
        with_error ty_err_opt res
      | (`Optional _, `Required super_ty) ->
        let ty_err_opt =
          Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Required_field_is_optional
                       {
                         pos = Reason.to_pos r_sub;
                         decl_pos = Reason.to_pos r_super;
                         name = printable_name;
                         def_pos = get_pos super_ty;
                       })
        in
        with_error ty_err_opt res
      | (`Absent, `Required _) ->
        let quickfixes_opt =
          match r_sub with
          | Reason.Rshape_literal p ->
            let fix_pos =
              Pos.shrink_to_end (Pos.shrink_by_one_char_both_sides p)
            in
            Some
              [
                Quickfix.make
                  ~title:("Add field " ^ Markdown_lite.md_codify printable_name)
                  ~new_text:(Printf.sprintf ", '%s' => TODO" printable_name)
                  fix_pos;
              ]
          | _ -> None
        in

        let ty_err_opt =
          Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  let on_error =
                    Option.value_map
                      ~default:on_error
                      ~f:(Reasons_callback.add_quickfixes on_error)
                      quickfixes_opt
                  in
                  apply_reasons ~on_error
                  @@ Secondary.Missing_field
                       {
                         decl_pos = field_pos;
                         pos = Reason.to_pos r_sub;
                         name = printable_name;
                       })
        in
        with_error ty_err_opt res
    in
    (* Helper function to project out a field and then simplify subtype *)
    let shape_project_and_simplify_subtype
        (supportdyn_sub, r_sub, shape_kind_sub, shape_map_sub)
        (supportdyn_super, r_super, shape_kind_super, shape_map_super)
        field_name
        res =
      let proj_sub =
        shape_projection
          ~supportdyn:supportdyn_sub
          field_name
          shape_kind_sub
          shape_map_sub
          r_sub
      in
      let proj_super =
        shape_projection
          ~supportdyn:supportdyn_super
          field_name
          shape_kind_super
          shape_map_super
          r_super
      in
      simplify_subtype_shape_projection
        (r_sub, proj_sub)
        (r_super, proj_super)
        field_name
        res
    in
    match
      ( TUtils.is_nothing env shape_kind_sub,
        TUtils.is_nothing env shape_kind_super )
    with
    (* An open shape cannot subtype a closed shape *)
    | (false, true) ->
      let fail =
        Option.map
          subtype_env.Subtype_env.on_error
          ~f:
            Typing_error.(
              fun on_error ->
                apply_reasons ~on_error
                @@ Secondary.Shape_fields_unknown
                     {
                       pos = Reason.to_pos r_sub;
                       decl_pos = Reason.to_pos r_super;
                     })
      in
      invalid ~fail env
    (* Otherwise, all projections must subtype *)
    | _ ->
      TShapeSet.fold
        (shape_project_and_simplify_subtype
           (supportdyn_sub, r_sub, shape_kind_sub, fdm_sub)
           (supportdyn_super, r_super, shape_kind_super, fdm_super))
        (TShapeSet.of_list (TShapeMap.keys fdm_sub @ TShapeMap.keys fdm_super))
        (env, TL.valid)
end

and Subtype_fun : sig
  (** This implements basic subtyping on non-generic function types:
      (1) return type behaves covariantly
      (2) parameter types behave contravariantly
      (3) special casing for variadics
   *)
  val simplify_subtype_funs :
    subtype_env:Subtype_env.t ->
    check_return:bool ->
    for_override:bool ->
    super_like:bool ->
    Typing_defs.locl_phase Typing_defs.Reason.t_ ->
    Typing_defs.locl_phase Typing_defs.ty Typing_defs.fun_type ->
    Typing_defs.locl_phase Typing_defs.Reason.t_ ->
    Typing_defs.locl_phase Typing_defs.ty Typing_defs.fun_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop

  val simplify_supertype_params_with_variadic :
    subtype_env:Subtype_env.t ->
    Typing_defs.locl_fun_param list ->
    Typing_defs.locl_ty ->
    Typing_env_types.env ->
    Typing_env_types.env * Typing_logic.subtype_prop
end = struct
  let rec simplify_subtype_params_with_variadic
      ~(subtype_env : Subtype_env.t)
      (subl : locl_fun_param list)
      (variadic_ty : locl_ty)
      env =
    let simplify_subtype_params_with_variadic_help =
      simplify_subtype_params_with_variadic ~subtype_env
    in
    match subl with
    | [] -> valid env
    | { fp_type = sub; _ } :: subl ->
      env
      |> Subtype.simplify_subtype
           ~subtype_env
           ~sub_supportdyn:None
           ~this_ty:None
           ~super_like:false
           ~super_supportdyn:false
           sub
           variadic_ty
      &&& simplify_subtype_params_with_variadic_help subl variadic_ty

  let simplify_subtype_implicit_params
      ~subtype_env { capability = sub_cap } { capability = super_cap } env =
    if TypecheckerOptions.any_coeffects (Env.get_tcopt env) then
      let expected = Typing_coeffects.get_type sub_cap in
      let got = Typing_coeffects.get_type super_cap in
      let reasons =
        Typing_error.Secondary.Coeffect_subtyping
          {
            pos = get_pos got;
            cap = Typing_coeffects.pretty env got;
            pos_expected = get_pos expected;
            cap_expected = Typing_coeffects.pretty env expected;
          }
      in
      let on_error =
        Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
            let err = Typing_error.apply_reasons ~on_error reasons in
            Typing_error.(Reasons_callback.always err))
      in
      let subtype_env = Subtype_env.set_on_error subtype_env on_error in
      match (sub_cap, super_cap) with
      | (CapTy sub, CapTy super) ->
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty:None
          ~super_like:false
          ~super_supportdyn:false
          sub
          super
          env
      | (CapTy sub, CapDefaults _p) ->
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty:None
          ~super_like:false
          ~super_supportdyn:false
          sub
          got
          env
      | (CapDefaults _p, CapTy super) ->
        Subtype.simplify_subtype
          ~subtype_env
          ~sub_supportdyn:None
          ~this_ty:None
          ~super_like:false
          ~super_supportdyn:false
          expected
          super
          env
      | (CapDefaults _p1, CapDefaults _p2) -> valid env
    else
      valid env

  let rec simplify_supertype_params_with_variadic
      ~(subtype_env : Subtype_env.t)
      (superl : locl_fun_param list)
      (variadic_ty : locl_ty)
      env =
    let simplify_supertype_params_with_variadic_help =
      simplify_supertype_params_with_variadic ~subtype_env
    in
    match superl with
    | [] -> valid env
    | { fp_type = super; _ } :: superl ->
      env
      |> Subtype.simplify_subtype
           ~subtype_env
           ~sub_supportdyn:None
           ~this_ty:None
           ~super_like:false
           ~super_supportdyn:false
           variadic_ty
           super
      &&& simplify_supertype_params_with_variadic_help superl variadic_ty

  let simplify_param_modes ~subtype_env param1 param2 env =
    let { fp_pos = pos1; _ } = param1 in
    let { fp_pos = pos2; _ } = param2 in
    match (get_fp_mode param1, get_fp_mode param2) with
    | (FPnormal, FPnormal)
    | (FPinout, FPinout) ->
      valid env
    | (FPnormal, FPinout) ->
      invalid
        ~fail:
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Inoutness_mismatch
                        { pos = pos2; decl_pos = pos1 }))
        env
    | (FPinout, FPnormal) ->
      invalid
        ~fail:
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Inoutness_mismatch
                        { pos = pos1; decl_pos = pos2 }))
        env

  let simplify_param_accept_disposable ~subtype_env param1 param2 env =
    let { fp_pos = pos1; _ } = param1 in
    let { fp_pos = pos2; _ } = param2 in
    match
      (get_fp_accept_disposable param1, get_fp_accept_disposable param2)
    with
    | (true, false) ->
      invalid
        ~fail:
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Accept_disposable_invariant
                        { pos = pos1; decl_pos = pos2 }))
        env
    | (false, true) ->
      invalid
        ~fail:
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Accept_disposable_invariant
                        { pos = pos2; decl_pos = pos1 }))
        env
    | (_, _) -> valid env

  let readonly_subtype (r_sub : bool) (r_super : bool) =
    match (r_sub, r_super) with
    | (true, false) ->
      false (* A readonly value is a supertype of a mutable one *)
    | _ -> true

  let simplify_param_readonly ~subtype_env sub super env =
    (* The sub param here (as with all simplify_param_* functions)
       is actually the parameter on ft_super, since params are contravariant *)
    (* Thus we check readonly subtyping covariantly *)
    let { fp_pos = pos1; _ } = sub in
    let { fp_pos = pos2; _ } = super in
    if not (readonly_subtype (get_fp_readonly sub) (get_fp_readonly super)) then
      invalid
        ~fail:
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Readonly_mismatch
                        {
                          pos = pos1;
                          kind = `param;
                          reason_sub =
                            lazy [(pos2, "This parameter is mutable")];
                          reason_super =
                            lazy [(pos1, "But this parameter is readonly")];
                        }))
        env
    else
      valid env

  let cross_package_subtype (c_sub : string option) (c_super : string option) =
    match (c_sub, c_super) with
    | (Some s, Some t) -> String.equal s t
    | (Some _, None) -> false
    | (None, Some _) -> true
    | (None, None) -> true

  (* Helper function for subtyping on function types: performs all checks that
   * don't involve actual types:
   *   <<__ReturnDisposable>> attribute
   *   variadic arity
   *  <<__Policied>> attribute
   *  Readonlyness
   * <<__CrossPackage>> attribute
   *)
  let simplify_subtype_funs_attributes
      ~subtype_env
      (r_sub : Reason.t)
      (ft_sub : locl_fun_type)
      (r_super : Reason.t)
      (ft_super : locl_fun_type)
      env =
    let p_sub = Reason.to_pos r_sub in
    let p_super = Reason.to_pos r_super in
    let print_cross_pkg_reason (c : string option) (is_sub : bool) =
      match c with
      | Some s when is_sub ->
        Printf.sprintf
          "This function is marked `<<__CrossPackage(%s)>>`, so it's only compatible with other functions marked `<<__CrossPackage(%s)>>`"
          s
          s
      | Some s ->
        Printf.sprintf "This function is marked <<__CrossPackage(%s)>>" s
      | None -> "This function is not cross package"
    in
    (env, TL.valid)
    |> check_with
         (readonly_subtype
            (* Readonly this is contravariant, so check ft_super_ro <: ft_sub_ro *)
            (get_ft_readonly_this ft_super)
            (get_ft_readonly_this ft_sub))
         (Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Readonly_mismatch
                       {
                         pos = p_sub;
                         kind = `fn;
                         reason_sub =
                           lazy
                             [(p_sub, "This function is not marked readonly")];
                         reason_super =
                           lazy [(p_super, "This function is marked readonly")];
                       }))
    |> check_with
         (readonly_subtype
            (* Readonly return is covariant, so check ft_sub <: ft_super *)
            (get_ft_returns_readonly ft_sub)
            (get_ft_returns_readonly ft_super))
         (Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Readonly_mismatch
                       {
                         pos = p_sub;
                         kind = `fn_return;
                         reason_sub =
                           lazy
                             [(p_sub, "This function returns a readonly value")];
                         reason_super =
                           lazy
                             [
                               ( p_super,
                                 "This function does not return a readonly value"
                               );
                             ];
                       }))
    |> check_with
         (cross_package_subtype
            ft_sub.ft_cross_package
            ft_super.ft_cross_package)
         (Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Cross_package_mismatch
                       {
                         pos = p_sub;
                         reason_sub =
                           lazy
                             [
                               ( p_sub,
                                 print_cross_pkg_reason
                                   ft_sub.ft_cross_package
                                   true );
                             ];
                         reason_super =
                           lazy
                             [
                               ( p_super,
                                 print_cross_pkg_reason
                                   ft_super.ft_cross_package
                                   false );
                             ];
                       }))
    |> check_with
         (Bool.equal
            (get_ft_return_disposable ft_sub)
            (get_ft_return_disposable ft_super))
         (Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Return_disposable_mismatch
                       {
                         pos_super = p_super;
                         pos_sub = p_sub;
                         is_marked_return_disposable =
                           get_ft_return_disposable ft_super;
                       }))
    |> check_with
         (arity_min ft_sub <= arity_min ft_super)
         (Option.map
            subtype_env.Subtype_env.on_error
            ~f:
              Typing_error.(
                fun on_error ->
                  apply_reasons ~on_error
                  @@ Secondary.Fun_too_many_args
                       {
                         expected = arity_min ft_super;
                         actual = arity_min ft_sub;
                         pos = p_sub;
                         decl_pos = p_super;
                       }))
    |> fun res ->
    let ft_sub_variadic =
      if get_ft_variadic ft_sub then
        List.last ft_sub.ft_params
      else
        None
    in
    let ft_super_variadic =
      if get_ft_variadic ft_super then
        List.last ft_super.ft_params
      else
        None
    in

    match (ft_sub_variadic, ft_super_variadic) with
    | (Some { fp_name = None; _ }, Some { fp_name = Some _; _ }) ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      with_error
        (Option.map
           subtype_env.Subtype_env.on_error
           ~f:
             Typing_error.(
               fun on_error ->
                 apply_reasons ~on_error
                 @@ Secondary.Fun_variadicity_hh_vs_php56
                      { pos = p_sub; decl_pos = p_super }))
        res
    | (None, None) ->
      let sub_max = List.length ft_sub.ft_params in
      let super_max = List.length ft_super.ft_params in
      if sub_max < super_max then
        with_error
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Fun_too_few_args
                        {
                          pos = p_sub;
                          decl_pos = p_super;
                          expected = super_max;
                          actual = sub_max;
                        }))
          res
      else
        res
    | (None, Some _) ->
      with_error
        (Option.map
           subtype_env.Subtype_env.on_error
           ~f:
             Typing_error.(
               fun on_error ->
                 apply_reasons ~on_error
                 @@ Secondary.Fun_unexpected_nonvariadic
                      { pos = p_sub; decl_pos = p_super }))
        res
    | (_, _) -> res

  let rec simplify_subtype_params
      ~(subtype_env : Subtype_env.t)
      ~for_override
      (subl : locl_fun_param list)
      (superl : locl_fun_param list)
      (variadic_sub_ty : bool)
      (variadic_super_ty : bool)
      env =
    let simplify_subtype_params_help =
      simplify_subtype_params ~subtype_env ~for_override
    in
    let simplify_subtype_params_with_variadic_help =
      simplify_subtype_params_with_variadic ~subtype_env
    in
    let simplify_supertype_params_with_variadic_help =
      simplify_supertype_params_with_variadic ~subtype_env
    in
    match (subl, superl) with
    (* When either list runs out, we still have to typecheck that
       the remaining portion sub/super types with the other's variadic.
       For example, if
       ChildClass {
         public function a(int $x = 0, string ... $args) // superl = [int], super_var = string
       }
       overrides
       ParentClass {
         public function a(string ... $args) // subl = [], sub_var = string
       }
       , there should be an error because the first argument will be checked against
       int, not string that is, ChildClass::a("hello") would crash,
       but ParentClass::a("hello") wouldn't.

       Similarly, if the other list is longer, aka
       ChildClass  extends ParentClass {
         public function a(mixed ... $args) // superl = [], super_var = mixed
       }
       overrides
       ParentClass {
         //subl = [string], sub_var = string
         public function a(string $x = 0, string ... $args)
       }
       It should also check that string is a subtype of mixed.
    *)
    | ([fp], _) when variadic_sub_ty ->
      simplify_supertype_params_with_variadic_help superl fp.fp_type env
    | (_, [fp]) when variadic_super_ty ->
      simplify_subtype_params_with_variadic_help subl fp.fp_type env
    | ([], _) -> valid env
    | (_, []) -> valid env
    | (sub :: subl, super :: superl) ->
      let { fp_type = ty_sub; _ } = sub in
      let { fp_type = ty_super; _ } = super in
      let subtype_env_for_param =
        (* When overriding in Sound Dynamic, we treat any dynamic-aware subtype of dynamic as a
         * subtype of the dynamic type itself
         *)
        match get_node ty_super with
        | Tdynamic
          when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
               && for_override ->
          Subtype_env.set_coercing_to_dynamic subtype_env
        | _ -> subtype_env
      in
      let simplify_subtype_for_param =
        Subtype.simplify_subtype
          ~subtype_env:subtype_env_for_param
          ~sub_supportdyn:None
          ~this_ty:None
          ~super_like:false
          ~super_supportdyn:false
      in
      (* Check that the calling conventions of the params are compatible. *)
      env
      |> simplify_param_modes ~subtype_env sub super
      &&& simplify_param_readonly ~subtype_env sub super
      &&& simplify_param_accept_disposable ~subtype_env sub super
      &&& begin
            fun env ->
              match (get_fp_mode sub, get_fp_mode super) with
              | (FPinout, FPinout) ->
                (* Inout parameters are invariant wrt subtyping for function types. *)
                env
                |> simplify_subtype_for_param ty_super ty_sub
                &&& simplify_subtype_for_param ty_sub ty_super
              | _ -> env |> simplify_subtype_for_param ty_sub ty_super
          end
      &&& simplify_subtype_params_help
            subl
            superl
            variadic_sub_ty
            variadic_super_ty

  let simplify_subtype_funs
      ~(subtype_env : Subtype_env.t)
      ~(check_return : bool)
      ~(for_override : bool)
      ~super_like
      (r_sub : Reason.t)
      (ft_sub : locl_fun_type)
      (r_super : Reason.t)
      (ft_super : locl_fun_type)
      env : env * TL.subtype_prop =
    (* First apply checks on attributes and variadic arity *)
    let simplify_subtype_implicit_params_help =
      simplify_subtype_implicit_params ~subtype_env
    in
    env
    |> simplify_subtype_funs_attributes
         ~subtype_env
         r_sub
         ft_sub
         r_super
         ft_super
    &&& (* Now do contravariant subtyping on parameters *)
    begin
      simplify_subtype_params
        ~subtype_env
        ~for_override
        ft_super.ft_params
        ft_sub.ft_params
        (get_ft_variadic ft_super)
        (get_ft_variadic ft_sub)
    end
    &&& simplify_subtype_implicit_params_help
          ft_super.ft_implicit_params
          ft_sub.ft_implicit_params
    &&&
    (* Finally do covariant subtyping on return type *)
    if check_return then
      let super_ty = Sd.liken ~super_like env ft_super.ft_ret in
      let subtype_env =
        if
          TypecheckerOptions.enable_sound_dynamic env.genv.tcopt && for_override
        then
          (* When overriding in Sound Dynamic, we allow t to override dynamic if
           * t is a dynamic-aware subtype of dynamic. We also allow Awaitable<t>
           * to override Awaitable<dynamic> and and Awaitable<t> to
           * override ~Awaitable<dynamic>.
           *)
          let super_ty = TUtils.strip_dynamic env super_ty in
          match get_node super_ty with
          | Tdynamic -> Subtype_env.set_coercing_to_dynamic subtype_env
          | Tclass ((_, class_name), _, [ty])
            when String.equal class_name SN.Classes.cAwaitable && is_dynamic ty
            ->
            Subtype_env.set_coercing_to_dynamic subtype_env
          | _ -> subtype_env
        else
          subtype_env
      in
      Subtype.simplify_subtype
        ~subtype_env
        ~sub_supportdyn:None
        ~this_ty:None
        ~super_like:false
        ~super_supportdyn:false
        ft_sub.ft_ret
        super_ty
    else
      valid
end

and Subtype_ask : sig
  val is_sub_type_alt_i :
    require_completeness:bool ->
    no_top_bottom:bool ->
    coerce:TL.coercion_direction option ->
    sub_supportdyn:Reason.t option ->
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    bool option

  val is_sub_type_for_union_i :
    Typing_env_types.env ->
    ?coerce:TL.coercion_direction option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    bool

  val is_sub_type_ignore_generic_params_i :
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    bool
end = struct
  let is_sub_type_alt_i
      ~require_completeness ~no_top_bottom ~coerce ~sub_supportdyn env ty1 ty2 =
    let this_ty =
      match ty1 with
      | LoclType ty1 -> Some ty1
      | ConstraintType _ -> None
    in
    let (_env, prop) =
      Subtype.simplify_subtype_i
        ~subtype_env:
          (Subtype_env.create
             ~require_completeness
             ~no_top_bottom
             ~coerce
             ~log_level:3
             None)
        ~sub_supportdyn
        ~this_ty
        ~super_like:false
        ~super_supportdyn:false
        (* It is weird that this can cause errors, but I am wary to discard them.
         * Using the generic unify_error to maintain current behavior. *)
        ty1
        ty2
        env
    in
    if TL.is_valid prop then
      Some true
    else if TL.is_unsat prop then
      Some false
    else
      None

  let is_sub_type_for_union_i env ?(coerce = None) ty1 ty2 =
    let ( = ) = Option.equal Bool.equal in
    is_sub_type_alt_i
      ~require_completeness:false
      ~no_top_bottom:true
      ~coerce
      ~sub_supportdyn:None
      env
      ty1
      ty2
    = Some true

  let is_sub_type_ignore_generic_params_i env ty1 ty2 =
    let ( = ) = Option.equal Bool.equal in
    is_sub_type_alt_i
    (* TODO(T121047839): Should this set a dedicated ignore_generic_param flag instead? *)
      ~require_completeness:true
      ~no_top_bottom:true
      ~coerce:None
      ~sub_supportdyn:None
      env
      ty1
      ty2
    = Some true
end

and Subtype_simplify : sig
  (** Attempt to compute the intersection of a type with an existing list intersection.
    If try_intersect env t [t1;...;tn] = [u1; ...; um]
    then u1&...&um must be the greatest lower bound of t and t1&...&tn wrt subtyping.
    For example:
      try_intersect nonnull [?C] = [C]
      try_intersect t1 [t2] = [t1]  if t1 <: t2
    Note: it's acceptable to return [t;t1;...;tn] but the intention is that
    we simplify (as above) wherever practical.
    It can be assumed that the original list contains no redundancy.
   *)
  val try_intersect_i :
    ?ignore_tyvars:bool ->
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type list ->
    Typing_defs.internal_type list

  val try_intersect :
    ?ignore_tyvars:bool ->
    Typing_env_types.env ->
    Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list

  (** Attempt to compute the union of a type with an existing list union.
    If try_union env t [t1;...;tn] = [u1;...;um]
    then u1|...|um must be the least upper bound of t and t1|...|tn wrt subtyping.
    For example:
      try_union int [float] = [num]
      try_union t1 [t2] = [t1] if t2 <: t1

    Notes:
    1. It's acceptable to return [t;t1;...;tn] but the intention is that
       we simplify (as above) wherever practical.
    2. Do not use Tunion for a syntactic union - the caller can do that.
    3. It can be assumed that the original list contains no redundancy.
    TODO: there are many more unions to implement yet.
   *)
  val try_union_i :
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type list ->
    Typing_defs.internal_type list

  val try_union :
    Typing_env_types.env ->
    Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list
end = struct
  let rec try_intersect_i ?(ignore_tyvars = false) env ty tyl =
    match tyl with
    | [] -> [ty]
    | ty' :: tyl' ->
      let (env, ty) = Env.expand_internal_type env ty in
      let (env, ty') = Env.expand_internal_type env ty' in
      let default env = ty' :: try_intersect_i env ~ignore_tyvars ty tyl' in
      (* Do not attempt to simplify intersection of type variables, as we use
       * intersection simplification when transitively closing through type variable
       * upper bounds and this would result in a type failing to be added.
       *)
      if ignore_tyvars && (is_tyvar_i ty || is_tyvar_i ty') then
        default env
      else if Subtype_ask.is_sub_type_ignore_generic_params_i env ty ty' then
        try_intersect_i ~ignore_tyvars env ty tyl'
      else if Subtype_ask.is_sub_type_ignore_generic_params_i env ty' ty then
        tyl
      else
        let nonnull_ty = LoclType (MakeType.nonnull (reason ty)) in
        (match (ty, ty') with
        | (LoclType lty, _)
          when Subtype_ask.is_sub_type_ignore_generic_params_i
                 env
                 ty'
                 nonnull_ty -> begin
          match get_node lty with
          | Toption t ->
            try_intersect_i ~ignore_tyvars env (LoclType t) (ty' :: tyl')
          | _ -> default env
        end
        | (_, LoclType lty)
          when Subtype_ask.is_sub_type_ignore_generic_params_i env ty nonnull_ty
          -> begin
          match get_node lty with
          | Toption t ->
            try_intersect_i ~ignore_tyvars env (LoclType t) (ty :: tyl')
          | _ -> default env
        end
        | (_, _) -> default env)

  let try_intersect ?(ignore_tyvars = false) env ty tyl =
    List.map
      (try_intersect_i
         ~ignore_tyvars
         env
         (LoclType ty)
         (List.map tyl ~f:(fun ty -> LoclType ty)))
      ~f:(function
        | LoclType ty -> ty
        | _ ->
          failwith
            "The intersection of two locl type should always be a locl type.")

  let rec try_union_i env ty tyl =
    match tyl with
    | [] -> [ty]
    | ty' :: tyl' ->
      if Subtype_ask.is_sub_type_for_union_i env ty ty' then
        tyl
      else if Subtype_ask.is_sub_type_for_union_i env ty' ty then
        try_union_i env ty tyl'
      else
        let (env, ty) = Env.expand_internal_type env ty in
        let (env, ty') = Env.expand_internal_type env ty' in
        (match (ty, ty') with
        | (LoclType t1, LoclType t2)
          when (is_prim Nast.Tfloat t1 && is_prim Nast.Tint t2)
               || (is_prim Nast.Tint t1 && is_prim Nast.Tfloat t2) ->
          let num = LoclType (MakeType.num (reason ty)) in
          try_union_i env num tyl'
        | (_, _) -> ty' :: try_union_i env ty tyl')

  let try_union env ty tyl =
    List.map
      (try_union_i env (LoclType ty) (List.map tyl ~f:(fun ty -> LoclType ty)))
      ~f:(function
        | LoclType ty -> ty
        | _ ->
          failwith "The union of two locl type should always be a locl type.")
end

and Subtype_trans : sig
  (** Given a subtype proposition, resolve conjunctions of subtype assertions
    of the form #v <: t or t <: #v by adding bounds to #v in env. Close env
    wrt transitivity i.e. if t <: #v and #v <: u then resolve t <: u which
    may in turn produce more bounds in env.
    For disjunctions, arbitrarily pick the first disjunct that is not
    unsatisfiable. If any unsatisfiable disjunct remains, return it.
   *)
  val prop_to_env :
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_env_types.env ->
    TL.subtype_prop ->
    Typing_error.Reasons_callback.t option ->
    Typing_env_types.env * Typing_error.t option
end = struct
  (* Add a new upper bound ty on var.  Apply transitivity of sutyping,
     * so if we already have tyl <: var, then check that for each ty_sub
     * in tyl we have ty_sub <: ty.
  *)
  let add_tyvar_upper_bound_and_close
      ~coerce
      (env, prop)
      var
      ty
      (on_error : Typing_error.Reasons_callback.t option) =
    let ty =
      match ty with
      | LoclType ty ->
        LoclType (Sd.transform_dynamic_upper_bound ~coerce env ty)
      | cty -> cty
    in
    let upper_bounds_before = Env.get_tyvar_upper_bounds env var in
    let env =
      Env.add_tyvar_upper_bound_and_update_variances
        ~intersect:(Subtype_simplify.try_intersect_i ~ignore_tyvars:true env)
        env
        var
        ty
    in
    let upper_bounds_after = Env.get_tyvar_upper_bounds env var in
    let added_upper_bounds =
      ITySet.diff upper_bounds_after upper_bounds_before
    in
    let lower_bounds = Env.get_tyvar_lower_bounds env var in
    let (env, prop) =
      ITySet.fold
        (fun upper_bound (env, prop) ->
          let (env, ty_err_opt) =
            Typing_subtype_tconst.make_all_type_consts_equal
              env
              var
              upper_bound
              ~on_error
              ~as_tyvar_with_cnstr:true
          in
          let (env, prop) =
            Option.value_map
              ~default:(env, prop)
              ~f:(fun ty_err -> invalid ~fail:(Some ty_err) env)
              ty_err_opt
          in
          ITySet.fold
            (fun lower_bound (env, prop1) ->
              let (env, prop2) =
                Subtype.simplify_subtype_i
                  ~subtype_env:
                    (Subtype_env.create
                       ~coerce
                       ~log_level:2
                       ~in_transitive_closure:true
                       on_error)
                  ~sub_supportdyn:None
                  ~this_ty:None
                  ~super_like:false
                  ~super_supportdyn:false
                  lower_bound
                  upper_bound
                  env
              in
              (env, TL.conj prop1 prop2))
            lower_bounds
            (env, prop))
        added_upper_bounds
        (env, prop)
    in
    (env, prop)

  (* Add a new lower bound ty on var.  Apply transitivity of subtyping
   * (so if var <: ty1,...,tyn then assert ty <: tyi for each tyi), using
   * simplify_subtype to produce a subtype proposition.
   *)
  let add_tyvar_lower_bound_and_close
      ~coerce
      (env, prop)
      var
      ty
      (on_error : Typing_error.Reasons_callback.t option) =
    let lower_bounds_before = Env.get_tyvar_lower_bounds env var in
    let env =
      Env.add_tyvar_lower_bound_and_update_variances
        ~union:(Subtype_simplify.try_union_i env)
        env
        var
        ty
    in
    let lower_bounds_after = Env.get_tyvar_lower_bounds env var in
    let added_lower_bounds =
      ITySet.diff lower_bounds_after lower_bounds_before
    in
    let upper_bounds = Env.get_tyvar_upper_bounds env var in
    let (env, prop) =
      ITySet.fold
        (fun lower_bound (env, prop) ->
          let (env, ty_err_opt) =
            Typing_subtype_tconst.make_all_type_consts_equal
              env
              var
              lower_bound
              ~on_error
              ~as_tyvar_with_cnstr:false
          in
          let (env, prop) =
            Option.value_map
              ~default:(env, prop)
              ~f:(fun err -> invalid ~fail:(Some err) env)
              ty_err_opt
          in
          ITySet.fold
            (fun upper_bound (env, prop1) ->
              let (env, prop2) =
                Subtype.simplify_subtype_i
                  ~subtype_env:
                    (Subtype_env.create
                       ~coerce
                       ~log_level:2
                       ~in_transitive_closure:true
                       on_error)
                  ~sub_supportdyn:None
                  ~this_ty:None
                  ~super_like:false
                  ~super_supportdyn:false
                  lower_bound
                  upper_bound
                  env
              in
              (env, TL.conj prop1 prop2))
            upper_bounds
            (env, prop))
        added_lower_bounds
        (env, prop)
    in
    (env, prop)

  (* Traverse a list of disjuncts and remove obviously redundant ones.
       t1 <: #1 is considered redundant if t2 <: #1 is also a disjunct and t2 <: t1.
     Dually,
       #1 <: t1 is considered redundant if #1 <: t2 is also a disjunct and t1 <: t2.
     It does not preserve the ordering.
  *)
  let simplify_disj env disj =
    (* even if sub_ty is not a supertype of super_ty, still consider super_ty redunant *)
    let additional_heuristic ~coerce env _sub_ty super_ty =
      let nonnull =
        if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
          MakeType.supportdyn_nonnull Reason.none
        else
          MakeType.nonnull Reason.none
      in
      Subtype_ask.is_sub_type_for_union_i
        ~coerce
        env
        (LoclType nonnull)
        super_ty
    in
    let rec add_new_bound ~is_lower ~coerce ~constr ty bounds =
      match bounds with
      | [] -> [(is_lower, ty, constr)]
      | ((is_lower', bound_ty, _) as b) :: bounds ->
        if is_lower && is_lower' then
          if Subtype_ask.is_sub_type_for_union_i ~coerce env bound_ty ty then
            b :: bounds
          else if Subtype_ask.is_sub_type_for_union_i ~coerce env ty bound_ty
          then
            add_new_bound ~is_lower ~coerce ~constr ty bounds
          else if additional_heuristic ~coerce env bound_ty ty then
            b :: bounds
          else if additional_heuristic ~coerce env ty bound_ty then
            add_new_bound ~is_lower ~coerce ~constr ty bounds
          else
            b :: add_new_bound ~is_lower ~coerce ~constr ty bounds
        else if
          (not is_lower)
          && (not is_lower')
          && Subtype_ask.is_sub_type_for_union_i ~coerce env ty bound_ty
        then
          b :: bounds
        else if
          (not is_lower)
          && (not is_lower')
          && Subtype_ask.is_sub_type_for_union_i ~coerce env bound_ty ty
        then
          add_new_bound ~is_lower ~coerce ~constr ty bounds
        else
          b :: add_new_bound ~is_lower ~coerce ~constr ty bounds
    in
    (* Map a type variable to a list of lower and upper bound types. For any two types
       t1 and t2 both lower or upper in the list, it is not the case that t1 <: t2 or t2 <: t1.
    *)
    let bound_map = ref Tvid.Map.empty in
    let process_bound ~is_lower ~coerce ~constr ty var =
      let ty =
        match ty with
        | LoclType ty when not is_lower ->
          LoclType (Sd.transform_dynamic_upper_bound ~coerce env ty)
        | _ -> ty
      in
      match Tvid.Map.find_opt var !bound_map with
      | None ->
        bound_map := Tvid.Map.add var [(is_lower, ty, constr)] !bound_map
      | Some bounds ->
        let new_bounds = add_new_bound ~is_lower ~coerce ~constr ty bounds in
        bound_map := Tvid.Map.add var new_bounds !bound_map
    in
    let rec fill_bound_map disj =
      match disj with
      | [] -> []
      | d :: disj ->
        (match d with
        | TL.Conj _ -> d :: fill_bound_map disj
        | TL.Disj (_, props) -> fill_bound_map (props @ disj)
        | TL.IsSubtype (coerce, ty_sub, ty_super) ->
          (match get_tyvar_opt ty_super with
          | Some var_super ->
            process_bound ~is_lower:true ~coerce ~constr:d ty_sub var_super;
            fill_bound_map disj
          | None ->
            (match get_tyvar_opt ty_sub with
            | Some var_sub ->
              process_bound ~is_lower:false ~coerce ~constr:d ty_super var_sub;
              fill_bound_map disj
            | None -> d :: fill_bound_map disj)))
    in
    (* Get the constraints from the table that were not removed, and add them to
       the remaining constraints that were not of the form we were looking for. *)
    let rec rebuild_disj remaining to_process =
      match to_process with
      | [] -> remaining
      | (_, bounds) :: to_process ->
        List.map ~f:(fun (_, _, c) -> c) bounds
        @ rebuild_disj remaining to_process
    in
    let remaining = fill_bound_map disj in
    let bounds = Tvid.Map.elements !bound_map in
    rebuild_disj remaining bounds

  let rec props_to_env
      ty_sub
      ty_super
      env
      ty_errs
      remain
      props
      (on_error : Typing_error.Reasons_callback.t option) =
    let props_to_env_help = props_to_env ty_sub ty_super in
    match props with
    | [] -> (env, List.rev ty_errs, List.rev remain)
    | prop :: props ->
      (match prop with
      | TL.Conj props' ->
        props_to_env_help env ty_errs remain (props' @ props) on_error
      | TL.Disj (ty_err_opt, disj_props) ->
        (* For now, just find the first prop in the disjunction that works *)
        let rec try_disj disj_props =
          match disj_props with
          | [] ->
            (* For now let it fail later when calling
               process_simplify_subtype_result on the remaining constraints. *)
            props_to_env_help env (ty_err_opt :: ty_errs) remain props on_error
          | prop :: disj_props' ->
            let (env', ty_errs', other) =
              props_to_env_help env [] remain [prop] on_error
            in
            if List.is_empty ty_errs' || List.for_all ty_errs' ~f:Option.is_none
            then
              props_to_env_help env' ty_errs (remain @ other) props on_error
            else
              try_disj disj_props'
        in

        let rec log_non_singleton_disj msg props =
          match props with
          | [] -> ()
          | [TL.Disj (_, props)] -> log_non_singleton_disj msg props
          | [_] -> ()
          | _ ->
            Typing_log.log_prop
              1
              (Reason.to_pos (get_reason_i ty_sub))
              ("non-singleton disjunction "
              ^ msg
              ^ " of "
              ^ Typing_print.full_i env ty_sub
              ^ " <: "
              ^ Typing_print.full_i env ty_super)
              env
              prop
        in
        let simplified_disj_props = simplify_disj env disj_props in
        log_non_singleton_disj "before simplification" disj_props;
        log_non_singleton_disj "after simplification" simplified_disj_props;
        try_disj simplified_disj_props
      | TL.IsSubtype (coerce, ty_sub, ty_super) ->
        let (env, ty_sub) = Env.expand_internal_type env ty_sub in
        let (env, ty_super) = Env.expand_internal_type env ty_super in
        (match (get_tyvar_opt ty_sub, get_tyvar_opt ty_super) with
        | (Some var_sub, Some var_super) ->
          let (env, prop1) =
            add_tyvar_upper_bound_and_close
              ~coerce
              (valid env)
              var_sub
              ty_super
              on_error
          in
          let (env, prop2) =
            add_tyvar_lower_bound_and_close
              ~coerce
              (valid env)
              var_super
              ty_sub
              on_error
          in
          props_to_env_help
            env
            ty_errs
            remain
            (prop1 :: prop2 :: props)
            on_error
        | (Some var, _) ->
          let (env, prop) =
            add_tyvar_upper_bound_and_close
              ~coerce
              (valid env)
              var
              ty_super
              on_error
          in
          props_to_env_help env ty_errs remain (prop :: props) on_error
        | (_, Some var) ->
          let (env, prop) =
            add_tyvar_lower_bound_and_close
              ~coerce
              (valid env)
              var
              ty_sub
              on_error
          in
          props_to_env_help env ty_errs remain (prop :: props) on_error
        | _ -> props_to_env_help env ty_errs (prop :: remain) props on_error))

  let prop_to_env ty_sub ty_super env prop on_error =
    let (env, ty_errs, props') =
      props_to_env ty_sub ty_super env [] [] [prop] on_error
    in
    let ty_err_opt =
      Typing_error.union_opt @@ List.filter_map ~f:Fn.id ty_errs
    in
    let env = Env.add_subtype_prop env (TL.conj_list props') in
    (env, ty_err_opt)
end

and Subtype_tell : sig
  (** Entry point asserting top-level subtype constraints and all implied constraints *)
  val sub_type_inner :
    Typing_env_types.env ->
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_env_types.env * Typing_error.t option
end = struct
  let sub_type_inner
      (env : env)
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      (ty_sub : internal_type)
      (ty_super : internal_type) : env * Typing_error.t option =
    Logging.log_subtype_i
      ~level:1
      ~this_ty
      ~function_name:
        ("sub_type_inner"
        ^
        match subtype_env.Subtype_env.coerce with
        | Some TL.CoerceToDynamic -> " (dynamic aware)"
        | Some TL.CoerceFromDynamic -> " (treat dynamic as bottom)"
        | None -> "")
      env
      ty_sub
      ty_super;
    let (env, prop) =
      Subtype.simplify_subtype_i
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like:false
        ~super_supportdyn:false
        ty_sub
        ty_super
        env
    in
    if not (TL.is_valid prop) then
      Typing_log.log_prop
        1
        (Reason.to_pos (reason ty_sub))
        "sub_type_inner"
        env
        prop;
    Subtype_trans.prop_to_env
      ty_sub
      ty_super
      env
      prop
      subtype_env.Subtype_env.on_error
end

and Subtype_constraint_super : sig
  val simplify_subtype_cty_super :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.constraint_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_cty_super
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      ~(super_like : bool)
      ~fail
      (ty_sub : internal_type)
      (ety_sub : internal_type)
      (ty_super : internal_type)
      (cty_super : constraint_type)
      env =
    let ety_super = ConstraintType cty_super in
    let fail_snd_err =
      let (ety_sub, ety_super, stripped_existential) =
        match Pretty.strip_existential ~ity_sub:ety_sub ~ity_sup:ety_super with
        | None -> (ety_sub, ety_super, false)
        | Some (ety_sub, ety_super) -> (ety_sub, ety_super, true)
      in
      match subtype_env.Subtype_env.tparam_constraints with
      | [] ->
        Typing_error.Secondary.Subtyping_error
          {
            ty_sub = ety_sub;
            ty_sup = ety_super;
            is_coeffect = subtype_env.Subtype_env.is_coeffect;
            stripped_existential;
          }
      | cstrs ->
        Typing_error.Secondary.Violated_constraint
          {
            cstrs;
            ty_sub = ety_sub;
            ty_sup = ety_super;
            is_coeffect = subtype_env.Subtype_env.is_coeffect;
          }
    in
    begin
      match deref_constraint_type cty_super with
      | (r_sup, TCintersection (lty_sup, cty_sup)) ->
        TCintersection.simplify_subtype_tcintersection_super
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~fail
          ty_sub
          ety_sub
          r_sup
          (lty_sup, cty_sup)
          env
      | (r_sup, TCunion (lty_sup, cty_sup)) ->
        TCunion.simplify_subtype_tcunion_super
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~fail
          ty_sub
          ety_sub
          ty_super
          r_sup
          (lty_sup, cty_sup)
          env
      | (r_super, Tdestructure { d_required; d_optional; d_variadic; d_kind })
        ->
        Tdestructure.simplify_subtype_tdestructure_super
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~fail
          ety_sub
          r_super
          (d_required, d_optional, d_variadic, d_kind)
          env
      | (r, Tcan_index ci) ->
        Can_index.simplify_subtype_can_index
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~fail
          ety_sub
          ety_super
          (r, ci)
          env
      | (r, Tcan_traverse ct) ->
        Can_traverse.simplify_subtype_can_traverse
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~fail
          ety_sub
          ety_super
          (r, ct)
          env
      | (r, Thas_member has_member_ty) ->
        Has_member.simplify_subtype_has_member
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~fail
          ~nullsafe:None
          ety_sub
          (r, has_member_ty)
          env
      | (r, Thas_type_member htm) ->
        (* Contextualize errors that may be generated when
         * checking refinement bounds. *)
        let on_error =
          Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
              let open Typing_error.Reasons_callback in
              prepend_on_apply on_error fail_snd_err)
        in
        let subtype_env = Subtype_env.set_on_error subtype_env on_error in
        Has_type_member.simplify_subtype_has_type_member
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~fail
          ety_sub
          (r, htm)
          env
    end
end

and TCintersection : sig
  val simplify_subtype_tcintersection_super :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t ->
    Typing_defs.locl_ty * Typing_defs.constraint_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_tcintersection_super
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~super_like
      ~fail
      ty_sub
      ety_sub
      r_sup
      (lty_sup, cty_sup)
      env =
    let cty_super =
      mk_constraint_type (r_sup, TCintersection (lty_sup, cty_sup))
    in
    let ety_super = ConstraintType cty_super in
    let default_subtype_help env =
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like
        ~fail
        env
        ety_sub
        ety_super
    in
    match ety_sub with
    | LoclType t when is_union t -> default_subtype_help env
    | ConstraintType t when is_constraint_type_union t ->
      default_subtype_help env
    | _ ->
      env
      |> Subtype.simplify_subtype_i
           ~subtype_env
           ~sub_supportdyn
           ~this_ty:None
           ~super_like:false
           ~super_supportdyn:false
           ty_sub
           (LoclType lty_sup)
      &&& Subtype.simplify_subtype_i
            ~subtype_env
            ~sub_supportdyn
            ~this_ty:None
            ~super_like:false
            ~super_supportdyn:false
            ty_sub
            (ConstraintType cty_sup)
end

and TCunion : sig
  val simplify_subtype_tcunion_super :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t ->
    Typing_defs.locl_ty * Typing_defs.constraint_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_tcunion_super
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~super_like
      ~fail
      ty_sub
      ety_sub
      ty_super
      r_sup
      (lty_sup, cty_sup)
      env =
    let using_new_method_call_inference =
      TypecheckerOptions.method_call_inference (Env.get_tcopt env)
    in
    let cty_super = mk_constraint_type (r_sup, TCunion (lty_sup, cty_sup)) in
    let ety_super = ConstraintType cty_super in
    let default_subtype_help env =
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like
        ~fail
        env
        ety_sub
        ety_super
    in
    let invalid_env env = invalid ~fail env in
    let ( ||| ) = ( ||| ) ~fail in

    match (lty_sup, cty_sup) with
    | (maybe_null, maybe_has_member)
      when using_new_method_call_inference
           && is_has_member maybe_has_member
           &&
           let (_, maybe_null) = Env.expand_type env maybe_null in
           is_prim Aast.Tnull maybe_null ->
      (* `LHS <: Thas_member(...) | null` is morally a null-safe object access *)
      let (env, null_ty) = Env.expand_type env maybe_null in
      let r_null = get_reason null_ty in
      let (r, has_member_ty) = deref_constraint_type maybe_has_member in
      (match has_member_ty with
      | Thas_member has_member_ty ->
        Has_member.simplify_subtype_has_member
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~nullsafe:(Some r_null)
          ~fail
          ety_sub
          (r, has_member_ty)
          env
      | _ -> invalid_env env (* Not possible due to guard in parent match *))
    | (lty_super, cty_super) ->
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype_help env
      | ConstraintType _ ->
        env
        |> Subtype.simplify_subtype_i
             ~subtype_env
             ~sub_supportdyn
             ~this_ty:None
             ~super_like:false
             ~super_supportdyn:false
             ty_sub
             (ConstraintType cty_super)
        ||| Subtype.simplify_subtype_i
              ~subtype_env
              ~sub_supportdyn
              ~this_ty:None
              ~super_like:false
              ~super_supportdyn:false
              ty_sub
              (LoclType lty_super)
        ||| default_subtype_help
      | LoclType lty ->
        (match deref lty with
        | (r, Toption ty) ->
          let ty_null = MakeType.null r in
          if_unsat
            invalid_env
            (Subtype.simplify_subtype_i
               ~subtype_env
               ~sub_supportdyn
               ~this_ty
               ~super_like:false
               ~super_supportdyn:false
               (LoclType ty_null)
               ty_super
               env)
          &&& Subtype.simplify_subtype_i
                ~subtype_env
                ~sub_supportdyn
                ~this_ty
                ~super_like:false
                ~super_supportdyn:false
                (LoclType ty)
                ty_super
        | (_, (Tintersection _ | Tunion _ | Tvar _)) -> default_subtype_help env
        | _ ->
          env
          |> Subtype.simplify_subtype_i
               ~subtype_env
               ~sub_supportdyn
               ~this_ty:None
               ~super_like:false
               ~super_supportdyn:false
               ty_sub
               (ConstraintType cty_super)
          ||| Subtype.simplify_subtype_i
                ~subtype_env
                ~sub_supportdyn
                ~this_ty:None
                ~super_like:false
                ~super_supportdyn:false
                ty_sub
                (LoclType lty_super)
          ||| default_subtype_help))
end

and Tdestructure : sig
  val simplify_subtype_tdestructure_super :
    subtype_env:Subtype_env.t ->
    sub_supportdyn:Reason.t option ->
    this_ty:Typing_defs.locl_ty option ->
    super_like:bool ->
    fail:Typing_error.t option ->
    Typing_defs.internal_type ->
    Typing_defs.Reason.t ->
    Typing_defs.locl_ty list
    * Typing_defs.locl_ty list
    * Typing_defs.locl_ty option
    * Typing_defs.destructure_kind ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  let simplify_subtype_tdestructure_super
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~super_like
      ~fail
      ety_sub
      r_super
      (d_required, d_optional, d_variadic, d_kind)
      env =
    let cty_super =
      mk_constraint_type
        (r_super, Tdestructure { d_required; d_optional; d_variadic; d_kind })
    in
    let ety_super = ConstraintType cty_super in
    let default_subtype_help env =
      Subtype.default_subtype
        ~subtype_env
        ~sub_supportdyn
        ~this_ty
        ~super_like
        ~fail
        env
        ety_sub
        ety_super
    in
    let invalid_env_with env f = invalid ~fail:f env in
    (* List destructuring *)
    let destructure_array ~sub_supportdyn t env =
      (* If this is a splat, there must be a variadic box to receive the elements
       * but for list(...) destructuring this is not required. Example:
       *
       * function f(int $i): void {}
       * function g(vec<int> $v): void {
       *   list($a) = $v; // ok (but may throw)
       *   f(...$v); // error
       * } *)
      let fpos =
        match r_super with
        | Reason.Runpack_param (_, fpos, _) -> fpos
        | _ -> Reason.to_pos r_super
      in
      match (d_kind, d_required, d_variadic) with
      | (SplatUnpack, _ :: _, _) ->
        (* return the env so as not to discard the type variable that might
           have been created for the Traversable type created below. *)
        invalid_env_with
          env
          (Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
               Typing_error.(
                 apply_reasons ~on_error
                 @@ Secondary.Unpack_array_required_argument
                      { pos = Reason.to_pos r_super; decl_pos = fpos })))
      | (SplatUnpack, [], None) ->
        invalid_env_with
          env
          (Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
               Typing_error.(
                 apply_reasons ~on_error
                 @@ Secondary.Unpack_array_variadic_argument
                      { pos = Reason.to_pos r_super; decl_pos = fpos })))
      | (SplatUnpack, [], Some _)
      | (ListDestructure, _, _) ->
        List.fold d_required ~init:(env, TL.valid) ~f:(fun res ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  t
                  ty_dest)
        &&& fun env ->
        List.fold d_optional ~init:(env, TL.valid) ~f:(fun res ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  t
                  ty_dest)
        &&& fun env ->
        Option.value_map ~default:(env, TL.valid) d_variadic ~f:(fun vty ->
            Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              t
              vty
              env)
    in

    let destructure_tuple r ts env =
      (* First fill the required elements. If there are insufficient elements, an error is reported.
       * Fill as many of the optional elements as possible, and the remainder are unioned into the
       * variadic element. Example:
       *
       * (float, bool, string, int) <: Tdestructure(#1, opt#2, ...#3) =>
       * float <: #1 /\ bool <: #2 /\ string <: #3 /\ int <: #3
       *
       * (float, bool) <: Tdestructure(#1, #2, opt#3) =>
       * float <: #1 /\ bool <: #2
       *)
      let len_ts = List.length ts in
      let len_required = List.length d_required in
      let arity_error f =
        let (epos, fpos, prefix) =
          match r_super with
          | Reason.Runpack_param (epos, fpos, c) ->
            (Pos_or_decl.of_raw_pos epos, fpos, c)
          | _ -> (Reason.to_pos r_super, Reason.to_pos r, 0)
        in
        invalid_env_with
          env
          (f
             (prefix + len_required)
             (prefix + len_ts)
             epos
             fpos
             subtype_env.Subtype_env.on_error)
      in
      if len_ts < len_required then
        arity_error (fun expected actual pos decl_pos on_error_opt ->
            Option.map on_error_opt ~f:(fun on_error ->
                let base_err =
                  Typing_error.Secondary.Typing_too_few_args
                    { pos; decl_pos; expected; actual }
                in
                Typing_error.(apply_reasons ~on_error base_err)))
      else
        let len_optional = List.length d_optional in
        let (ts_required, remain) = List.split_n ts len_required in
        let (ts_optional, ts_variadic) = List.split_n remain len_optional in
        List.fold2_exn
          ts_required
          d_required
          ~init:(env, TL.valid)
          ~f:(fun res ty ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  ty
                  ty_dest)
        &&& fun env ->
        let len_ts_opt = List.length ts_optional in
        let d_optional_part =
          if len_ts_opt < len_optional then
            List.take d_optional len_ts_opt
          else
            d_optional
        in
        List.fold2_exn
          ts_optional
          d_optional_part
          ~init:(env, TL.valid)
          ~f:(fun res ty ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  ty
                  ty_dest)
        &&& fun env ->
        match (ts_variadic, d_variadic) with
        | (vars, Some vty) ->
          List.fold vars ~init:(env, TL.valid) ~f:(fun res ty ->
              res
              &&& Subtype.simplify_subtype
                    ~subtype_env
                    ~sub_supportdyn
                    ~this_ty
                    ~super_like:false
                    ~super_supportdyn:false
                    ty
                    vty)
        | ([], None) -> valid env
        | (_, None) ->
          (* Elements remain but we have nowhere to put them *)
          arity_error (fun expected actual pos decl_pos on_error_opt ->
              Option.map on_error_opt ~f:(fun on_error ->
                  Typing_error.(
                    apply_reasons ~on_error
                    @@ Secondary.Typing_too_many_args
                         { pos; decl_pos; expected; actual })))
    in

    let destructure_dynamic t env =
      if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
        List.fold d_required ~init:(env, TL.valid) ~f:(fun res ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  t
                  ty_dest)
        &&& fun env ->
        List.fold d_optional ~init:(env, TL.valid) ~f:(fun res ty_dest ->
            res
            &&& Subtype.simplify_subtype
                  ~subtype_env
                  ~sub_supportdyn
                  ~this_ty
                  ~super_like:false
                  ~super_supportdyn:false
                  t
                  ty_dest)
        &&& fun env ->
        Option.value_map ~default:(env, TL.valid) d_variadic ~f:(fun vty ->
            Subtype.simplify_subtype
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like:false
              ~super_supportdyn:false
              t
              vty
              env)
      else
        env |> destructure_array ~sub_supportdyn t
    in
    begin
      match ety_sub with
      | ConstraintType _ -> default_subtype_help env
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (r, Ttuple tyl) -> env |> destructure_tuple r tyl
        | (r, Tclass ((_, x), _, tyl)) when String.equal x SN.Collections.cPair
          ->
          env |> destructure_tuple r tyl
        | (_, Tclass ((_, x), _, [elt_type]))
          when String.equal x SN.Collections.cVector
               || String.equal x SN.Collections.cImmVector
               || String.equal x SN.Collections.cVec
               || String.equal x SN.Collections.cConstVector ->
          env |> destructure_array ~sub_supportdyn elt_type
        | (_, Tdynamic) -> env |> destructure_dynamic ty_sub
        | (_, (Tunion _ | Tintersection _ | Tgeneric _ | Tvar _)) ->
          (* TODO(T69551141) handle type arguments of Tgeneric? *)
          default_subtype_help env
        | _ -> begin
          match d_kind with
          | SplatUnpack ->
            (* Allow splatting of arbitrary Traversables *)
            let (env, ty_inner) = Env.fresh_type env Pos.none in
            let traversable = MakeType.traversable r_super ty_inner in
            env
            |> Subtype.simplify_subtype
                 ~subtype_env
                 ~sub_supportdyn
                 ~this_ty
                 ~super_like:false
                 ~super_supportdyn:false
                 ty_sub
                 traversable
            &&& destructure_array ~sub_supportdyn:None ty_inner
          | ListDestructure ->
            let ty_sub_descr =
              lazy
                (Typing_print.with_blank_tyvars (fun () ->
                     Typing_print.full_strip_ns env ty_sub))
            in
            default_subtype_help env
            |> if_unsat @@ fun env ->
               invalid_env_with
                 env
                 (Option.map
                    subtype_env.Subtype_env.on_error
                    ~f:
                      Typing_error.(
                        fun on_error ->
                          apply_reasons ~on_error
                          @@ Secondary.Invalid_destructure
                               {
                                 pos = Reason.to_pos r_super;
                                 decl_pos = get_pos ty_sub;
                                 ty_name = ty_sub_descr;
                               }))
        end)
    end
end
(* == API =================================================================== *)

(* Determines whether the types are definitely disjoint, or whether they might
    overlap (i.e., both contain some particular value). *)
(* One of the main entry points to this module *)
let sub_type_i ~subtype_env env ty_sub ty_super =
  let old_env = env in
  match
    Subtype_tell.sub_type_inner
      ~subtype_env
      ~sub_supportdyn:None
      env
      ~this_ty:None
      ty_sub
      ty_super
  with
  | (env, None) -> (Env.log_env_change "sub_type" old_env env, None)
  | (_, ty_err_opt) ->
    (Env.log_env_change "sub_type" old_env old_env, ty_err_opt)

let sub_type
    env
    ?(coerce = None)
    ?(is_coeffect = false)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    on_error =
  sub_type_i
    ~subtype_env:(Subtype_env.create ~is_coeffect ~coerce ~log_level:2 on_error)
    env
    (LoclType ty_sub)
    (LoclType ty_super)

let is_sub_type_alt ~require_completeness ~no_top_bottom env ty1 ty2 =
  Subtype_ask.is_sub_type_alt_i
    ~require_completeness
    ~no_top_bottom
    env
    (LoclType ty1)
    (LoclType ty2)

let is_sub_type env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~require_completeness:false
    ~no_top_bottom:false
    ~coerce:None
    ~sub_supportdyn:None
    env
    ty1
    ty2
  = Some true

let is_dynamic_aware_sub_type env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~require_completeness:false
    ~no_top_bottom:false
    ~coerce:(Some TL.CoerceToDynamic)
    ~sub_supportdyn:None
    env
    ty1
    ty2
  = Some true

let is_sub_type_for_union env ?(coerce = None) ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~require_completeness:false
    ~no_top_bottom:true
    ~coerce
    ~sub_supportdyn:None
    env
    ty1
    ty2
  = Some true

let is_sub_type_ignore_generic_params env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
  (* TODO(T121047839): Should this set a dedicated ignore_generic_param flag instead? *)
    ~require_completeness:true
    ~no_top_bottom:true
    ~coerce:None
    ~sub_supportdyn:None
    env
    ty1
    ty2
  = Some true

let can_sub_type env ty1 ty2 =
  let ( <> ) a b = not (Option.equal Bool.equal a b) in
  is_sub_type_alt
    ~require_completeness:false
    ~no_top_bottom:true
    ~coerce:None
    ~sub_supportdyn:None
    env
    ty1
    ty2
  <> Some false

let is_type_disjoint env ty1 ty2 =
  (* visited record which type variables & generics we've seen, to cut off cycles. *)
  let rec is_type_disjoint visited env ty1 ty2 =
    let (env, ty1) = Env.expand_type env ty1 in
    let (env, ty2) = Env.expand_type env ty2 in
    match (get_node ty1, get_node ty2) with
    | (_, (Tany _ | Tdynamic | Taccess _ | Tunapplied_alias _))
    | ((Tany _ | Tdynamic | Taccess _ | Tunapplied_alias _), _) ->
      false
    | (Tshape _, Tshape _) ->
      (* This could be more precise, e.g., if we have two closed shapes with different fields.
         However, intersection already detects this and simplifies to nothing, so it's not
         so important here. *)
      false
    | (Tshape _, _) ->
      (* Treat shapes as dict<arraykey, mixed> because that implementation detail
         leaks through when doing is dict<_, _> on them, and they are also
         Traversable, KeyedContainer, etc. (along with darrays).
         We could translate darray to a more precise dict type with the same
         type arguments, but it doesn't matter since disjointness doesn't ever
         look at them. *)
      let r = get_reason ty1 in
      is_type_disjoint visited env MakeType.(dict r (arraykey r) (mixed r)) ty2
    | (_, Tshape _) ->
      let r = get_reason ty2 in
      is_type_disjoint visited env ty1 MakeType.(dict r (arraykey r) (mixed r))
    | (Ttuple tyl1, Ttuple tyl2) ->
      (match List.exists2 ~f:(is_type_disjoint visited env) tyl1 tyl2 with
      | List.Or_unequal_lengths.Ok res -> res
      | List.Or_unequal_lengths.Unequal_lengths -> true)
    | (Ttuple _, _) ->
      (* Treat tuples as vec<mixed> because that implementation detail
         leaks through when doing is vec<_> on them, and they are also
         Traversable, Container, etc. along with varrays.
         We could translate varray to a more precise vec type with the same
         type argument, but it doesn't matter since disjointness doesn't ever
         look at it. *)
      let r = get_reason ty1 in
      is_type_disjoint visited env MakeType.(vec r (mixed r)) ty2
    | (_, Ttuple _) ->
      let r = get_reason ty2 in
      is_type_disjoint visited env ty1 MakeType.(vec r (mixed r))
    | (Tvec_or_dict (tyk, tyv), _) ->
      let r = get_reason ty1 in
      is_type_disjoint
        visited
        env
        MakeType.(union r [vec r tyv; dict r tyk tyv])
        ty2
    | (_, Tvec_or_dict (tyk, tyv)) ->
      let r = get_reason ty2 in
      is_type_disjoint
        visited
        env
        ty1
        MakeType.(union r [vec r tyv; dict r tyk tyv])
    | (Tgeneric (name, []), _) -> is_generic_disjoint visited env name ty1 ty2
    | (_, Tgeneric (name, [])) -> is_generic_disjoint visited env name ty2 ty1
    | ((Tgeneric _ | Tnewtype _ | Tdependent _ | Tintersection _), _) ->
      let (env, bounds) =
        TUtils.get_concrete_supertypes ~abstract_enum:false env ty1
      in
      is_intersection_type_disjoint visited env bounds ty2
    | (_, (Tgeneric _ | Tnewtype _ | Tdependent _ | Tintersection _)) ->
      let (env, bounds) =
        TUtils.get_concrete_supertypes ~abstract_enum:false env ty2
      in
      is_intersection_type_disjoint visited env bounds ty1
    | (Tvar tv, _) -> is_tyvar_disjoint visited env tv ty2
    | (_, Tvar tv) -> is_tyvar_disjoint visited env tv ty1
    | (Tunion tyl, _) -> List.for_all ~f:(is_type_disjoint visited env ty2) tyl
    | (_, Tunion tyl) -> List.for_all ~f:(is_type_disjoint visited env ty1) tyl
    | (Toption ty1, _) ->
      is_type_disjoint visited env ty1 ty2
      && is_type_disjoint visited env (MakeType.null Reason.Rnone) ty2
    | (_, Toption ty2) ->
      is_type_disjoint visited env ty1 ty2
      && is_type_disjoint visited env ty1 (MakeType.null Reason.Rnone)
    | (Tnonnull, _) ->
      is_sub_type_for_union env ty2 (MakeType.null Reason.Rnone)
    | (_, Tnonnull) ->
      is_sub_type_for_union env ty1 (MakeType.null Reason.Rnone)
    | (Tneg (Neg_prim tp1), _) ->
      is_sub_type_for_union env ty2 (MakeType.prim_type Reason.Rnone tp1)
    | (_, Tneg (Neg_prim tp2)) ->
      is_sub_type_for_union env ty1 (MakeType.prim_type Reason.Rnone tp2)
    | (Tneg (Neg_class (_, c1)), Tclass ((_, c2), _, _tyl))
    | (Tclass ((_, c2), _, _tyl), Tneg (Neg_class (_, c1))) ->
      (* These are disjoint iff for all objects o, o in c2<_tyl> implies that
         o notin (complement (Union tyl'. c1<tyl'>)), which is just that
         c2<_tyl> subset Union tyl'. c1<tyl'>. If c2 is a subclass of c1, then
         whatever _tyl is, we can chase up the hierarchy to find an instantiation
         for tyl'. If c2 is not a subclass of c1, then no matter what the tyl' are
         the subset realtionship cannot hold, since either c1 and c2 are disjoint tags,
         or c1 is a non-equal subclass of c2, and so objects that are exact c2,
         can't inhabit c1. NB, we aren't allowing abstractness of a class to cause
         types to be considered disjoint.
         e.g., in abstract class C {}; class D extends C {}, we wouldn't consider
         neg D and C to be disjoint.
      *)
      TUtils.is_sub_class_refl env c2 c1
    | (Tneg _, _)
    | (_, Tneg _) ->
      false
    | (Tprim tp1, Tprim tp2) -> Subtype_negation.is_tprim_disjoint tp1 tp2
    | (Tclass ((_, cname), ex, _), Tprim (Aast.Tarraykey | Aast.Tstring))
    | (Tprim (Aast.Tarraykey | Aast.Tstring), Tclass ((_, cname), ex, _))
      when String.equal cname SN.Classes.cStringish && is_nonexact ex ->
      false
    | (Tprim _, (Tfun _ | Tclass _))
    | ((Tfun _ | Tclass _), Tprim _) ->
      true
    | (Tfun _, Tfun _) -> false
    | (Tfun _, Tclass _)
    | (Tclass _, Tfun _) ->
      true
    | (Tclass ((_, c1), _, _), Tclass ((_, c2), _, _)) ->
      Subtype_negation.is_class_disjoint env c1 c2
  (* incomplete, e.g., is_intersection_type_disjoint (?int & ?float) num *)
  and is_intersection_type_disjoint visited_tvyars env inter_tyl ty =
    List.exists ~f:(is_type_disjoint visited_tvyars env ty) inter_tyl
  and is_intersection_itype_set_disjoint visited_tvyars env inter_ty_set ty =
    ITySet.exists (is_itype_disjoint visited_tvyars env ty) inter_ty_set
  and is_itype_disjoint
      visited_tvyars env (lty1 : locl_ty) (ity : internal_type) =
    match ity with
    | LoclType lty2 -> is_type_disjoint visited_tvyars env lty1 lty2
    | ConstraintType _ -> false
  and is_tyvar_disjoint visited env tyvar ty =
    let (visited_tyvars, visited_generics) = visited in
    if Tvid.Set.mem tyvar visited_tyvars then
      (* There is a cyclic type variable bound, this will lead to a type error *)
      false
    else
      let bounds = Env.get_tyvar_upper_bounds env tyvar in
      is_intersection_itype_set_disjoint
        (Tvid.Set.add tyvar visited_tyvars, visited_generics)
        env
        bounds
        ty
  and is_generic_disjoint visited env (name : string) gen_ty ty =
    let (visited_tyvars, visited_generics) = visited in
    if SSet.mem name visited_generics then
      false
    else
      let (env, bounds) =
        TUtils.get_concrete_supertypes ~abstract_enum:false env gen_ty
      in
      is_intersection_type_disjoint
        (visited_tyvars, SSet.add name visited_generics)
        env
        bounds
        ty
  in
  is_type_disjoint (Tvid.Set.empty, SSet.empty) env ty1 ty2

let decompose_subtype_add_bound
    ~coerce (env : env) (ty_sub : locl_ty) (ty_super : locl_ty) : env =
  let (env, ty_super) = Env.expand_type env ty_super in
  let (env, ty_sub) = Env.expand_type env ty_sub in
  match (get_node ty_sub, get_node ty_super) with
  | (_, Tany _) -> env
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (Tgeneric (name_sub, targs), _) when not (phys_equal ty_sub ty_super) ->
    let ty_super = Sd.transform_dynamic_upper_bound ~coerce env ty_super in
    (* TODO(T69551141) handle type arguments. Passing targs to get_lower_bounds,
       but the add_upper_bound call must be adapted *)
    Logging.log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_upper_bounds env name_sub targs in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then
      env
    else
      Env.add_upper_bound
        ~intersect:(Subtype_simplify.try_intersect env)
        env
        name_sub
        ty_super
  (* ty_sub <: name_super so add a lower bound on name_super *)
  | (_, Tgeneric (name_super, targs)) when not (phys_equal ty_sub ty_super) ->
    (* TODO(T69551141) handle type arguments. Passing targs to get_lower_bounds,
       but the add_lower_bound call must be adapted *)
    Logging.log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_lower_bounds env name_super targs in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_sub tys then
      env
    else
      Env.add_lower_bound
        ~union:(Subtype_simplify.try_union env)
        env
        name_super
        ty_sub
  | (_, _) -> env

let rec decompose_subtype_add_prop env prop =
  match prop with
  | TL.Conj props ->
    List.fold_left ~f:decompose_subtype_add_prop ~init:env props
  | TL.Disj (_, []) -> Env.mark_inconsistent env
  | TL.Disj (_, [prop']) -> decompose_subtype_add_prop env prop'
  | TL.Disj _ ->
    let callable_pos = env.genv.callable_pos in
    Typing_log.log_prop
      2
      (Pos_or_decl.of_raw_pos callable_pos)
      "decompose_subtype_add_prop"
      env
      prop;
    env
  | TL.IsSubtype (coerce, LoclType ty1, LoclType ty2) ->
    decompose_subtype_add_bound ~coerce env ty1 ty2
  | TL.IsSubtype _ ->
    (* Subtyping queries between locl types are not creating
       constraint types only if require_soundness is unset.
       Otherwise type refinement subtyping queries may create
       Thas_type_member() constraint types. *)
    failwith
      ("Subtyping locl types in completeness mode should yield "
      ^ "propositions involving locl types only.")

(* Given two types that we know are in a subtype relationship
 *   ty_sub <: ty_super
 * add to env.tpenv any bounds on generic type parameters that must
 * hold for ty_sub <: ty_super to be valid.
 *
 * For example, suppose we know Cov<T> <: Cov<D> for a covariant class Cov.
 * Then it must be the case that T <: D so we add an upper bound D to the
 * bounds for T.
 *
 * Although some of this code is similar to that for sub_type_inner, its
 * purpose is different. sub_type_inner takes two types t and u and makes
 * updates to the substitution of type variables (through unification) to
 * make t <: u true.
 *
 * decompose_subtype takes two types t and u for which t <: u is *assumed* to
 * hold, and makes updates to bounds on generic parameters that *necessarily*
 * hold in order for t <: u.
 *)
let decompose_subtype
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Typing_error.Reasons_callback.t option) : env =
  Logging.log_subtype
    ~level:2
    ~this_ty:None
    ~function_name:"decompose_subtype"
    env
    ty_sub
    ty_super;
  let (env, prop) =
    Subtype.simplify_subtype
      ~subtype_env:
        (Subtype_env.create
           ~require_soundness:false
           ~require_completeness:true
           ~log_level:2
           on_error)
      ~sub_supportdyn:None
      ~this_ty:None
      ~super_like:false
      ~super_supportdyn:false
      ty_sub
      ty_super
      env
  in
  decompose_subtype_add_prop env prop

(* Decompose a general constraint *)
let decompose_constraint
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    on_error : env =
  (* constraints are caught based on reason, not error callback. Using unify_error *)
  match ck with
  | Ast_defs.Constraint_as -> decompose_subtype env ty_sub ty_super on_error
  | Ast_defs.Constraint_super -> decompose_subtype env ty_super ty_sub on_error
  | Ast_defs.Constraint_eq ->
    let env = decompose_subtype env ty_sub ty_super on_error in
    decompose_subtype env ty_super ty_sub on_error

(* Given a constraint ty1 ck ty2 where ck is AS, SUPER or =,
 * add bounds to type parameters in the environment that necessarily
 * must hold in order for ty1 ck ty2.
 *
 * First, we invoke decompose_constraint to add initial bounds to
 * the environment. Then we iterate, decomposing constraints that
 * arise through transitivity across bounds.
 *
 * For example, suppose that env already contains
 *   C<T1> <: T2
 * for some covariant class C. Now suppose we add the
 * constraint "T2 as C<T3>" i.e. we end up with
 *   C<T1> <: T2 <: C<T3>
 * Then by transitivity we know that T1 <: T3 so we add this to the
 * environment too.
 *
 * We repeat this process until no further bounds are added to the
 * environment, or some limit is reached. (It's possible to construct
 * types that expand forever under inheritance.)
 *)
let constraint_iteration_limit = 20

let add_constraint
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    on_error : env =
  Logging.log_subtype
    ~level:1
    ~this_ty:None
    ~function_name:"add_constraint"
    env
    ty_sub
    ty_super;
  let oldsize = Env.get_tpenv_size env in
  let env = decompose_constraint env ck ty_sub ty_super on_error in
  let ( = ) = Int.equal in
  if Env.get_tpenv_size env = oldsize then
    env
  else
    let rec iter n env =
      if n > constraint_iteration_limit then
        env
      else
        let oldsize = Env.get_tpenv_size env in
        let env =
          List.fold_left
            (Env.get_generic_parameters env)
            ~init:env
            ~f:(fun env x ->
              List.fold_left
                (* TODO(T70068435) always using [] as args for now *)
                (Typing_set.elements (Env.get_lower_bounds env x []))
                ~init:env
                ~f:(fun env ty_sub' ->
                  List.fold_left
                    (* TODO(T70068435) always using [] as args for now *)
                    (Typing_set.elements (Env.get_upper_bounds env x []))
                    ~init:env
                    ~f:(fun env ty_super' ->
                      decompose_subtype env ty_sub' ty_super' on_error)))
        in
        if Int.equal (Env.get_tpenv_size env) oldsize then
          env
        else
          iter (n + 1) env
    in
    iter 0 env

let add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) =
    add_constraint env ck ty1 ty2
    @@ Some (Typing_error.Reasons_callback.unify_error_at p)
  in
  List.fold_left constraints ~f:add_constraint ~init:env

let sub_type_with_dynamic_as_bottom env ty_sub ty_super on_error =
  Logging.log_subtype
    ~level:1
    ~this_ty:None
    ~function_name:"coercion"
    env
    ty_sub
    ty_super;
  let old_env = env in
  let (env, prop) =
    Subtype.simplify_subtype
      ~subtype_env:
        (Subtype_env.create
           ~coerce:(Some TL.CoerceFromDynamic)
           ~log_level:2
           on_error)
      ~sub_supportdyn:None
      ~this_ty:None
      ~super_like:false
      ~super_supportdyn:false
      ty_sub
      ty_super
      env
  in
  let (env, ty_err) =
    Subtype_trans.prop_to_env
      (LoclType ty_sub)
      (LoclType ty_super)
      env
      prop
      on_error
  in
  ( (if Option.is_some ty_err then
      old_env
    else
      env),
    ty_err )

let simplify_subtype_i ?(is_coeffect = false) env ty_sub ty_super ~on_error =
  Subtype.simplify_subtype_i
    ~subtype_env:
      (Subtype_env.create
         ~is_coeffect
         ~no_top_bottom:true
         ~log_level:2
         on_error)
    ~sub_supportdyn:None
    ~this_ty:None
    ~super_like:false
    ~super_supportdyn:false
    ty_sub
    ty_super
    env

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let sub_type_i env ?(is_coeffect = false) ty1 ty2 on_error =
  sub_type_i
    ~subtype_env:
      (Subtype_env.create ~log_level:2 ~is_coeffect ~coerce:None on_error)
    env
    ty1
    ty2

let subtype_funs
    ~(check_return : bool)
    ~for_override
    ~on_error
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env =
  (* This is used for checking subtyping of function types for method override
   * (see Typing_subtype_method) so types are fully-explicit and therefore we
   * permit subtyping to dynamic when --enable-sound-dynamic-type is true
   *)
  let old_env = env in
  let (env, prop) =
    Subtype_fun.simplify_subtype_funs
      ~subtype_env:(Subtype_env.create ~log_level:2 ~coerce:None on_error)
      ~check_return
      ~for_override
      ~super_like:false
      r_sub
      ft_sub
      r_super
      ft_super
      env
  in
  let (env, ty_err) =
    Subtype_trans.prop_to_env
      (LoclType (mk (r_sub, Tfun ft_sub)))
      (LoclType (mk (r_super, Tfun ft_super)))
      env
      prop
      on_error
  in
  ( (if Option.is_some ty_err then
      old_env
    else
      env),
    ty_err )

let sub_type_or_fail env ty1 ty2 err_opt =
  sub_type env ty1 ty2
  @@ Option.map ~f:Typing_error.Reasons_callback.always err_opt

let is_sub_type_for_union = is_sub_type_for_union ~coerce:None

let is_sub_type_for_union_i = Subtype_ask.is_sub_type_for_union_i ~coerce:None

let set_fun_refs () =
  TUtils.sub_type_ref := sub_type;
  TUtils.sub_type_i_ref := sub_type_i;
  TUtils.sub_type_with_dynamic_as_bottom_ref := sub_type_with_dynamic_as_bottom;
  TUtils.add_constraint_ref := add_constraint;
  TUtils.is_sub_type_ref := is_sub_type;
  TUtils.is_sub_type_for_union_ref := is_sub_type_for_union;
  TUtils.is_sub_type_for_union_i_ref := is_sub_type_for_union_i;
  TUtils.is_sub_type_ignore_generic_params_ref :=
    is_sub_type_ignore_generic_params;
  TUtils.is_type_disjoint_ref := is_type_disjoint;
  TUtils.can_sub_type_ref := can_sub_type

let () = set_fun_refs ()
