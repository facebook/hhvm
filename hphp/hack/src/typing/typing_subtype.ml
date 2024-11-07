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
module Cls = Folded_class
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
module VisitedGoalsFunctor (Tset : Stdlib.Set.S) : sig
  type t

  val empty : t

  val try_add_visited_generic_sub : t -> string -> Tset.elt -> t option

  val try_add_visited_generic_super : t -> Tset.elt -> string -> t option
end = struct
  type t = (Tset.t * Tset.t) SMap.t

  let empty : t = SMap.empty

  (* Return None if (name <: ty) is already present, otherwise return Some v'
   * where v' has the pair added
   *)
  let try_add_visited_generic_sub v name ty =
    match SMap.find_opt name v with
    | None -> Some (SMap.add name (Tset.empty, Tset.singleton ty) v)
    | Some (lower, upper) ->
      if Tset.mem ty upper then
        None
      else
        Some (SMap.add name (lower, Tset.add ty upper) v)

  (* Return None if (ty <: name) is already present, otherwise return Some v'
   * where v' has the pair added
   *)
  let try_add_visited_generic_super v ty name =
    match SMap.find_opt name v with
    | None -> Some (SMap.add name (Tset.singleton ty, Tset.empty) v)
    | Some (lower, upper) ->
      if Tset.mem ty lower then
        None
      else
        Some (SMap.add name (Tset.add ty lower, upper) v)
end

module VisitedGoals = VisitedGoalsFunctor (Typing_set)
module VisitedGoalsInternal = VisitedGoalsFunctor (Internal_type_set)

module Pretty : sig
  val describe_ty_default :
    Typing_env_types.env -> Typing_defs.internal_type -> string

  val describe_ty_super :
    is_coeffect:bool ->
    Typing_env_types.env ->
    Typing_defs.internal_type ->
    string

  val _strip_existential :
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
  let _strip_existential ~ity_sub ~ity_sup =
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
    Typing_print.full_strip_ns_i ~hide_internals:true env ty

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
      | (_, Thas_const { name; ty = _ }) ->
        Printf.sprintf "a class with a member `%s`" name
      | (_, Ttype_switch _)
      | (_, Tdestructure _) ->
        Markdown_lite.md_codify
          (Typing_print.full_strip_ns_i
             ~hide_internals:true
             env
             (ConstraintType ty)))
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
    ignore_readonly: bool;
        (** If set, do not apply readonly subtyping through function types i.e. the
         * readonly qualifier on function types and the readonly qualifier on
         * function parameters. This flag is triggered by the <<__IgnoreReadonlyError>>
         * on parameters.
         *)
    visited: VisitedGoals.t;
        (** If require_completeness is not set, maintain a visited goal set *)
    visited_internal: VisitedGoalsInternal.t;
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
    ignore_likes: bool;
        (** We're ignoring likes because we had a goal of the form `t <: ~u`
            for t <:D dynamic *)
    recursion_tracker: Subtype_recursion_tracker.t;
  }

  let set_on_error t on_error = { t with on_error }

  let set_visited t visited = { t with visited }

  let set_visited_internal t visited_internal = { t with visited_internal }

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
      ?(ignore_readonly = false)
      ?(no_top_bottom = false)
      ?(coerce = None)
      ?(is_coeffect = false)
      ?(in_transitive_closure = false)
      ?(ignore_likes = false)
      ~(log_level : int)
      on_error =
    {
      require_soundness;
      require_completeness;
      ignore_readonly;
      visited = VisitedGoals.empty;
      visited_internal = VisitedGoalsInternal.empty;
      no_top_bottom;
      coerce;
      is_coeffect;
      on_error;
      tparam_constraints = [];
      log_level;
      in_transitive_closure;
      ignore_likes;
      recursion_tracker = Subtype_recursion_tracker.empty;
    }

  let possibly_add_violated_constraint subtype_env ~r_sub ~r_super =
    {
      subtype_env with
      tparam_constraints =
        (match
           ( Reason.Predicates.unpack_cstr_on_generics_opt r_super,
             Reason.Predicates.unpack_cstr_on_generics_opt r_sub )
         with
        | (Some (p, tparam), _)
        | (_, Some (p, tparam)) ->
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

  let mk_secondary_error { tparam_constraints; is_coeffect; _ } ty_sub ty_super
      =
    match tparam_constraints with
    | [] ->
      let stripped_existential = false in
      Typing_error.Secondary.Subtyping_error
        { ty_sub; ty_sup = ty_super; is_coeffect; stripped_existential }
    | cstrs ->
      Typing_error.Secondary.Violated_constraint
        { cstrs; ty_sub; ty_sup = ty_super; is_coeffect }

  let fail t ~ty_sub ~ty_super =
    let secondary_error = mk_secondary_error t ty_sub ty_super in
    match t.tparam_constraints with
    | [] ->
      Option.map
        t.on_error
        ~f:
          Typing_error.(
            fun on_error ->
              apply_reasons
                ~on_error:(Reasons_callback.retain_code on_error)
                secondary_error)
    | _ ->
      Option.map
        t.on_error
        ~f:
          Typing_error.(
            (fun on_error -> apply_reasons ~on_error secondary_error))

  let fail_with_suffix t ~ty_sub ~ty_super suffix =
    let secondary_error = mk_secondary_error t ty_sub ty_super in
    match t.tparam_constraints with
    | [] ->
      Option.map
        t.on_error
        ~f:
          Typing_error.(
            fun on_error ->
              apply_reasons
                ~on_error:
                  Reasons_callback.(
                    prepend_on_apply (retain_code on_error) secondary_error)
                suffix)
    | _ ->
      Option.map
        t.on_error
        ~f:
          Typing_error.(
            fun on_error ->
              apply_reasons
                ~on_error:
                  Reasons_callback.(prepend_on_apply on_error secondary_error)
                suffix)

  let check_infinite_recursion (subtype_env : t) op =
    Subtype_recursion_tracker.add_op_and_check_infinite_recursion
      subtype_env.recursion_tracker
      op
    |> Result.map ~f:(fun recursion_tracker ->
           { subtype_env with recursion_tracker })
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

  let log_subtype_i_
      ~(this_ty : locl_ty option) ~function_name ~result env ty_sub ty_super =
    Typing_log.log_function
      (Reason.to_pos (reason ty_sub))
      ~function_name
      ~arguments:
        [
          ("ty_sub", Typing_print.debug_i env ty_sub);
          ("ty_super", Typing_print.debug_i env ty_super);
          ( "this_ty",
            match this_ty with
            | None -> "None"
            | Some ty -> Typing_print.debug env ty );
        ]
      ~result

  let log_subtype_i_prop =
    log_subtype_i_ ~result:(fun (env, prop) ->
        Some (TL.print (Typing_print.debug_i env) prop))

  let log_subtype_prop env ty_sub ty_super =
    log_subtype_i_prop env (LoclType ty_sub) (LoclType ty_super)

  let should_log_subtype_i env ~level ty_sub ty_super =
    let level =
      if
        (TUtils.is_capability_i ty_sub || TUtils.is_capability_i ty_super)
        && level < 3
      then
        3
      else
        level
    in
    Typing_log.should_log env ~category:"sub" ~level

  let log_subtype_i = log_subtype_i_ ~result:(fun _ -> None)

  let should_log_subtype env ~level ty_sub ty_super =
    should_log_subtype_i env ~level (LoclType ty_sub) (LoclType ty_super)

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
      | Tprim Aast.Tarraykey -> Some (MakeType.neg r (r, IsTag ArraykeyTag))
      | Tneg (r, IsTag ArraykeyTag) ->
        Some (MakeType.prim_type r Aast.Tarraykey)
      | Tneg (r, IsTag NullTag) -> Some (MakeType.null r)
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
  match ty2 with
  | LoclType ty2 ->
    (* TODO akenn: somehow we lose this environment *)
    let (env, coerce, ty2) =
      (* If we are in dynamic-aware subtyping mode, that fact will be lost when ty2
         ends up on the upper bound of a type variable. Here we find if ty2 contains
         dynamic and replace it with supportdyn<mixed> which is equivalent, but does not
         require dynamic-aware subtyping mode to be a supertype of types that support dynamic. *)
      match coerce with
      | Some TL.CoerceToDynamic ->
        let (env, ty_opt) =
          Typing_dynamic_utils.try_strip_dynamic
            ~do_not_solve_likes:true
            env
            ty2
        in
        begin
          match ty_opt with
          | Some non_dyn_ty ->
            let r = get_reason ty2 in
            ( env,
              None,
              MakeType.union
                r
                [non_dyn_ty; MakeType.supportdyn_mixed ~mixed_reason:r r] )
          | None -> (env, coerce, ty2)
        end
      | _ -> (env, coerce, ty2)
    in
    (env, TL.IsSubtype (coerce, ty1, LoclType ty2))
  | _ -> (env, TL.IsSubtype (coerce, ty1, ty2))

(* All of our constraints have a type with additional context to support <:D *)
type lhs = {
  sub_supportdyn: Reason.t option;
  ty_sub: locl_ty;
}

module type Constraint_handler = sig
  type rhs

  val simplify :
    subtype_env:Subtype_env.t ->
    this_ty:Typing_defs.locl_ty option ->
    lhs:lhs ->
    rhs:rhs ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end

module rec Subtype : sig
  type rhs = {
    super_supportdyn: bool;
    super_like: bool;
    ty_super: locl_ty;
  }

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
  include Constraint_handler with type rhs := rhs

  val simplify_funs :
    subtype_env:Subtype_env.t ->
    check_return:bool ->
    for_override:bool ->
    super_like:bool ->
    Reason.t * Typing_defs.locl_fun_type ->
    Reason.t * Typing_defs.locl_fun_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  type rhs = {
    super_supportdyn: bool;
    super_like: bool;
    ty_super: locl_ty;
  }

  module Log = struct
    let level subtype_env ~ty_sub ~ty_super =
      let level = subtype_env.Subtype_env.log_level in
      if
        (TUtils.is_capability ty_sub || TUtils.is_capability ty_super)
        && level < 3
      then
        3
      else
        level

    let should_log env subtype_env ~lhs ~rhs =
      Typing_log.should_log
        env
        ~category:"sub"
        ~level:(level subtype_env ~ty_sub:lhs.ty_sub ~ty_super:rhs.ty_super)

    let function_name subtype_env ~sub_supportdyn ~super_like ~super_supportdyn
        =
      "Typing_subtype.Subtype.simplify"
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
      ^ flag " ignore_likes" subtype_env.Subtype_env.ignore_likes
      ^ flag " require_soundness" subtype_env.Subtype_env.require_soundness
      ^ flag
          " require_completeness"
          subtype_env.Subtype_env.require_completeness
      ^ flag " ignore_readonly" subtype_env.Subtype_env.ignore_readonly
      ^ flag
          " in_transitive_closure"
          subtype_env.Subtype_env.in_transitive_closure

    let log env subtype_env ~this_ty ~lhs ~rhs =
      let { sub_supportdyn; ty_sub } = lhs in
      let { super_like; super_supportdyn; ty_super } = rhs in
      Logging.log_subtype_prop
        ~this_ty
        ~function_name:
          (function_name
             subtype_env
             ~sub_supportdyn
             ~super_like
             ~super_supportdyn)
        env
        ty_sub
        ty_super
  end

  let is_final_and_invariant env id =
    let class_def = Env.get_class env id in
    match class_def with
    | Decl_entry.Found class_ty -> TUtils.class_is_final_and_invariant class_ty
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      false

  let expands_to ty ~p ~env =
    let (_, ty) = Env.expand_type env ty in
    p ty

  let expands_to_nonnull ty ~env = expands_to ty ~p:is_nonnull ~env

  let expands_to_supportdyn ty ~env =
    expands_to ty ~env ~p:(fun ty ->
        match get_node ty with
        | Tnewtype (name, [_], _) -> String.equal name SN.Classes.cSupportDyn
        | _ -> false)

  let expands_to_var_or_intersection ty ~env =
    expands_to ty ~env ~p:(fun ty ->
        match get_node ty with
        | Tvar _
        | Tintersection _ ->
          true
        | _ -> false)

  (* Helper function for inspecting types occuring inside a union from a top-level
     pattern match *)
  let var_and_arraykey_opt ty1 ty2 ~env =
    let (env, ty1) = Env.expand_type env ty1 in
    let (env, ty2) = Env.expand_type env ty2 in
    match (deref ty1, deref ty2) with
    | ( ((_, Tvar _) as tv),
        ((_, (Tprim Aast.Tarraykey | Tneg (_, IsTag ArraykeyTag))) as ak) )
    | ( ((_, (Tprim Aast.Tarraykey | Tneg (_, IsTag ArraykeyTag))) as ak),
        ((_, Tvar _) as tv) ) ->
      (env, Some (tv, ak))
    | _ -> (env, None)

  let var_and_arraykey_exn ty1 ty2 ~env =
    match var_and_arraykey_opt ty1 ty2 ~env with
    | (env, Some (tv, ak)) -> (env, (tv, ak))
    | _ ->
      failwith
        "Expected a pair of types, one of which a [Tvar] and the other [Tprim Tarraykey] or [Tneg (IsTag ArraykeyTag)]"

  let expands_to_var_and_arraykey ty1 ty2 ~env =
    match var_and_arraykey_opt ty1 ty2 ~env with
    | (_, Some _) -> true
    | _ -> false

  let expands_to_class ty ~env =
    expands_to ty ~env ~p:(fun ty ->
        match get_node ty with
        | Tclass _ -> true
        | _ -> false)

  let class_id_exn ty ~env =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tclass (id, _, _) -> (env, id)
    | _ -> failwith "This type is not a Tclass"

  let newtype_exn ty ~env =
    let (env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tnewtype (name, tyargs, bound) -> (env, (name, tyargs, bound))
    | _ -> failwith "This type is not a Tnewtype"

  let generic_lower_bounds env ty_super =
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

  (** If it's clear from the syntax of the type that null isn't in ty, return true. *)
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
    | Tlabel _
    | Tvec_or_dict _
    | Tclass_args _ ->
      true
    | Tdependent (_, bound)
    | Tnewtype (_, _, bound) ->
      null_not_subtype bound

  let simplify_subtype_by_physical_equality env ty_sub ty_super simplify =
    if
      (not (TypecheckerOptions.disable_physical_equality env.genv.tcopt))
      && phys_equal ty_sub ty_super
    then
      (env, TL.valid)
    else
      simplify ()

  let rec default_subtype
      ~subtype_env
      ~this_ty
      ~fail
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:{ super_like; ty_super; super_supportdyn }
      env =
    let (env, ty_super) = Env.expand_type env ty_super in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    match deref ty_sub with
    | (_, Tvar _) -> begin
      match (subtype_env.Subtype_env.coerce, get_node ty_super) with
      | (Some TL.CoerceToDynamic, Tdynamic) ->
        let r = get_reason ty_super in
        let ty_super = MakeType.supportdyn_mixed ~mixed_reason:r r in
        default_subtype_inner
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn; ty_super }
          env
      | (Some cd, _) ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:(Some cd)
          env
          (LoclType ty_sub)
          (LoclType ty_super)
      | (None, _) ->
        default_subtype_inner
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn; ty_super }
          env
    end
    | (r_sub, Tprim Nast.Tvoid) ->
      let r = Reason.implicit_upper_bound (Reason.to_pos r_sub, "mixed") in
      let prop =
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn = None; ty_sub = MakeType.mixed r }
          ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
      in
      if_unsat (invalid ~fail) @@ prop env
    | (_, Tany _) ->
      if subtype_env.Subtype_env.no_top_bottom then
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (LoclType ty_super)
      else
        valid env
    | _ ->
      default_subtype_inner
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn; ty_super }
        env

  and default_subtype_inner
      ~subtype_env
      ~this_ty
      ~fail
      ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
      ~rhs:({ super_like; ty_super; _ } as rhs)
      env =
    match deref lty_sub with
    | (r_sub, Tunion tyl) ->
      let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
      in
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        ~update_reason:
          Typing_env.(
            update_reason ~f:(fun r_sub_prj ->
                Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, tyl)
        rhs
        env
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
        Typing_solver_utils.remove_tyvar_from_upper_bound
          env
          id
          (LoclType ty_super)
      in
      (* If the type is already in the upper bounds of the type variable,
       * then we already know that this subtype assertion is valid
       *)
      if ITySet.mem simplified_super_ty (Env.get_tyvar_upper_bounds env id) then
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
            (LoclType lty_sub)
            (LoclType ty_super))
    | (r_sub, Tintersection tyl) ->
      (* A & B <: C iif A <: C | !B *)
      (match Subtype_negation.find_type_with_exact_negation env tyl with
      | (env, Some non_ty, tyl) -> begin
        let (env, ty_super) = TUtils.union env ty_super non_ty in
        let ty_sub = MakeType.intersection r_sub tyl in
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          env
      end
      | _ ->
        let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
          simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
        in
        (* Otherwise use the incomplete common case which doesn't require inspection of the rhs *)
        Common.simplify_intersection_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop
          ~update_reason:
            Typing_env.(
              update_reason ~f:(fun r_sub_prj ->
                  Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, tyl)
          rhs
          env)
    | (_, Tgeneric _) when subtype_env.Subtype_env.require_completeness ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType lty_sub)
        (LoclType ty_super)
    | (r_generic, Tgeneric (name_sub, tyargs)) -> begin
      match
        VisitedGoals.try_add_visited_generic_sub
          subtype_env.Subtype_env.visited
          name_sub
          ty_super
      with
      | None ->
        (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
        *)
        invalid ~fail env
      | Some new_visited -> begin
        let subtype_env = Subtype_env.set_visited subtype_env new_visited in

        let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
          simplify ~subtype_env ~this_ty ~lhs ~rhs env
        in
        Common.simplify_generic_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop
          (sub_supportdyn, r_generic, name_sub, tyargs)
          { super_like; super_supportdyn = false; ty_super }
          { super_like = false; super_supportdyn = false; ty_super }
          env
      end
    end
    | (_, Tdynamic) when Subtype_env.coercing_from_dynamic subtype_env ->
      valid env
    | (_, Taccess _) -> invalid ~fail env
    | (r, Tnewtype (n, _, ty)) ->
      let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty ~lhs ~rhs env
      in
      Common.simplify_newtype_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        (sub_supportdyn, r, n, ty)
        rhs
        env
    | (r, Tdependent (dep_ty, ty)) ->
      let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty ~lhs ~rhs env
      in
      Common.simplify_dependent_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        (sub_supportdyn, r, dep_ty, ty)
        rhs
        env
    | _ -> invalid ~fail env

  and simplify_sub_union
      ~subtype_env
      ~sub_supportdyn
      ~this_ty
      ~super_like
      ~fail
      lty_sub
      (r_super, lty_supers)
      env =
    (* We *know* that the assertion is unsatisfiable *)
    let invalid_env env = invalid ~fail env in
    let default_subtype_help env =
      Subtype.(
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ~rhs:
            {
              super_like;
              super_supportdyn = false;
              ty_super = mk (r_super, Tunion lty_supers);
            }
          env)
    in
    (* Identify cases heuristically where we just want to reduce t <: ~u to
       t <: u with super-like set, and not also try t <: dynamic or run finish *)
    let avoid_disjunctions env ty =
      let (env, ty) = Env.expand_type env ty in
      match (get_node lty_sub, get_node ty) with
      | (Tnewtype (n1, _, _), Tnewtype (n2, _, _))
        when String.equal n1 n2 && not (String.equal n1 SN.Classes.cSupportDyn)
        ->
        (env, true)
      | _ -> (env, false)
    in
    let finish env =
      match get_node lty_sub with
      | Tnewtype _
      | Tdependent _
      | Tgeneric _ ->
        default_subtype_help env
      | _ -> invalid_env env
    in
    let simplify_subtype_of_dynamic env =
      Subtype.(
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ~rhs:
            {
              super_like = false;
              super_supportdyn = false;
              ty_super = MakeType.dynamic r_super;
            }
          env)
    in
    let dyn_finish ty env =
      let (env, avoid) = avoid_disjunctions env ty in
      if avoid then
        invalid_env env
      else
        let ( ||| ) = ( ||| ) ~fail in
        simplify_subtype_of_dynamic env ||| finish
    in
    let stripped_dynamic =
      if TypecheckerOptions.enable_sound_dynamic env.genv.tcopt then
        Typing_dynamic_utils.try_strip_dynamic_from_union env lty_supers
      else
        None
    in
    match stripped_dynamic with
    | Some (ty_dynamic, tyl) ->
      (* If subtype is literally `dynamic` then we're done *)
      if is_dynamic lty_sub then
        valid env
      else
        let ty = MakeType.union r_super tyl in
        let (env, ty) = Env.expand_type env ty in
        let delay_push =
          Subtype_ask.is_sub_type_for_union_i
            env
            (LoclType ty)
            (LoclType (MakeType.supportdyn_mixed ~mixed_reason:r_super r_super))
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
                      (function
                        | LoclType t ->
                          LoclType (MakeType.nullable Reason.none t)
                        | ConstraintType t -> ConstraintType t)
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
                simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
                  ~rhs:
                    {
                      super_like = true;
                      super_supportdyn = false;
                      ty_super = ty;
                    }
                  env
                ||| dyn_finish ty
              else
                dyn_finish ty env
            | Some ty ->
              let simplify_pushed_like env =
                simplify
                  ~subtype_env:(Subtype_env.set_coercing_to_dynamic subtype_env)
                  ~this_ty
                  ~lhs:{ sub_supportdyn = None; ty_sub = ty }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = ty_dynamic;
                    }
                  env
                &&& simplify
                      ~subtype_env:
                        { subtype_env with Subtype_env.ignore_likes = true }
                      ~this_ty
                      ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
                      ~rhs:
                        {
                          super_like = false;
                          super_supportdyn = false;
                          ty_super = ty;
                        }
              in

              env |> simplify_pushed_like ||| dyn_finish ty
        in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ~rhs:
            { super_like = delay_push; super_supportdyn = false; ty_super = ty }
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
      let rec aux ty_supers ~env ~errs ~props =
        match ty_supers with
        | [] ->
          ( env,
            TL.Disj
              ( Typing_error.intersect_opt @@ List.filter_opt errs,
                List.rev props ) )
        | ty_super :: ty_supers ->
          let ty_super =
            Typing_env.(
              update_reason env ty_super ~f:(fun r_super_prj ->
                  Typing_reason.prj_union_super
                    ~super:r_super
                    ~super_prj:r_super_prj))
          in

          let (env, prop) =
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
              env
          in
          if TL.is_valid prop then
            (env, prop)
          else
            let (errs, props) =
              match TL.get_error_if_unsat prop with
              | Some err -> (err :: errs, props)
              | _ -> (errs, prop :: props)
            in
            aux ty_supers ~env ~errs ~props
      in
      aux lty_supers ~env ~errs:[fail] ~props:[]

  (* == Function subtyping ================================================== *)
  and simplify_subtype_params_with_variadic
      ~subtype_env
      (r_super, idx_super, fn_param_supers)
      (r_sub, idx_sub, ty_sub)
      env =
    match fn_param_supers with
    | [] -> valid env
    | { fp_type = ty_super; _ } :: fn_param_supers ->
      let param_prop =
        let ty_super =
          Typing_env.update_reason env ty_super ~f:(fun r_super_prj ->
              Typing_reason.prj_fn_param
                ~super:r_super
                ~super_prj:r_super_prj
                ~sub:r_sub
                ~idx_sub
                ~idx_super)
        in
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn = None; ty_sub = ty_super }
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = ty_sub }
      in
      param_prop env
      &&& simplify_subtype_params_with_variadic
            ~subtype_env
            (r_super, idx_super + 1, fn_param_supers)
            (r_sub, idx_sub, ty_sub)

  and simplify_subtype_implicit_params
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
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn = None; ty_sub = sub }
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = super }
          env
      | (CapTy sub, CapDefaults _p) ->
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn = None; ty_sub = sub }
          ~rhs:{ super_like = false; super_supportdyn = false; ty_super = got }
          env
      | (CapDefaults _p, CapTy super) ->
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn = None; ty_sub = expected }
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = super }
          env
      | (CapDefaults _p1, CapDefaults _p2) -> valid env
    else
      valid env

  and simplify_supertype_params_with_variadic
      ~subtype_env
      (r_sub, idx_sub, fn_param_subs)
      (r_super, idx_super, ty_super)
      env =
    match fn_param_subs with
    | [] -> valid env
    | { fp_type = ty_sub; _ } :: fn_param_subs ->
      let param_prop =
        let ty_super =
          Typing_env.(
            update_reason env ty_super ~f:(fun r_super_prj ->
                Typing_reason.prj_fn_param
                  ~super:r_super
                  ~super_prj:r_super_prj
                  ~sub:r_sub
                  ~idx_sub
                  ~idx_super))
        in
        simplify
          ~subtype_env
          ~this_ty:None
          ~lhs:{ sub_supportdyn = None; ty_sub = ty_super }
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = ty_sub }
      in
      param_prop env
      &&& simplify_supertype_params_with_variadic
            ~subtype_env
            (r_sub, idx_sub + 1, fn_param_subs)
            (r_super, idx_super, ty_super)

  and simplify_param_modes ~subtype_env ~fn_param_sub ~fn_param_super env =
    let { fp_pos = pos1; _ } = fn_param_super in
    let { fp_pos = pos2; _ } = fn_param_sub in
    match (get_fp_mode fn_param_super, get_fp_mode fn_param_sub) with
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

  and simplify_param_accept_disposable
      ~subtype_env ~fn_param_sub ~fn_param_super env =
    let { fp_pos = pos1; _ } = fn_param_super in
    let { fp_pos = pos2; _ } = fn_param_sub in
    match
      ( get_fp_accept_disposable fn_param_super,
        get_fp_accept_disposable fn_param_sub )
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

  and readonly_subtype (r_sub : bool) (r_super : bool) =
    match (r_sub, r_super) with
    | (true, false) ->
      false (* A readonly value is a supertype of a mutable one *)
    | _ -> true

  and simplify_param_readonly ~subtype_env ~fn_param_sub ~fn_param_super env =
    let { fp_pos = pos1; _ } = fn_param_super in
    let { fp_pos = pos2; _ } = fn_param_sub in
    if
      (not subtype_env.Subtype_env.ignore_readonly)
      && not
           (readonly_subtype
              (get_fp_readonly fn_param_super)
              (get_fp_readonly fn_param_sub))
    then
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

  and cross_package_subtype (c_sub : string option) (c_super : string option) =
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
  and simplify_subtype_funs_attributes
      ~subtype_env (r_sub, ft_sub) (r_super, ft_super) =
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
    let splat_super =
      match List.last ft_super.ft_params with
      | Some fp -> get_fp_splat fp
      | None -> false
    in
    (* Readonly this is contravariant, so check ft_super_ro <: ft_sub_ro *)
    let readonly_this_err =
      if
        (not subtype_env.Subtype_env.ignore_readonly)
        && not
             (readonly_subtype
                (get_ft_readonly_this ft_super)
                (get_ft_readonly_this ft_sub))
      then
        Error
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
      else
        Ok ()
      (* Readonly return is covariant, so check ft_sub <: ft_super *)
    and readonly_ret_err =
      if
        (not subtype_env.Subtype_env.ignore_readonly)
        && not
             (readonly_subtype
                (get_ft_returns_readonly ft_sub)
                (get_ft_returns_readonly ft_super))
      then
        Error
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
                              [
                                (p_sub, "This function returns a readonly value");
                              ];
                          reason_super =
                            lazy
                              [
                                ( p_super,
                                  "This function does not return a readonly value"
                                );
                              ];
                        }))
      else
        Ok ()
    and package_err =
      if
        not
          (cross_package_subtype
             ft_sub.ft_cross_package
             ft_super.ft_cross_package)
      then
        Error
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
      else
        Ok ()
    and return_disposable_err =
      if
        not
          (Bool.equal
             (get_ft_return_disposable ft_sub)
             (get_ft_return_disposable ft_super))
      then
        Error
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
      else
        Ok ()
    and arity_min_err =
      if (not splat_super) && not (arity_min ft_sub <= arity_min ft_super) then
        Error
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
      else
        Ok ()
    and variadic_err =
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
        Error
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Fun_variadicity_hh_vs_php56
                        { pos = p_sub; decl_pos = p_super }))
      | (None, None) when not splat_super ->
        let sub_max = List.length ft_sub.ft_params in
        let super_max = List.length ft_super.ft_params in
        if sub_max < super_max then
          Error
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
        else
          Ok ()
      | (None, Some _) ->
        Error
          (Option.map
             subtype_env.Subtype_env.on_error
             ~f:
               Typing_error.(
                 fun on_error ->
                   apply_reasons ~on_error
                   @@ Secondary.Fun_unexpected_nonvariadic
                        { pos = p_sub; decl_pos = p_super }))
      | (_, _) -> Ok ()
    in
    (* Collect attribute errors *)
    let (_, errs) =
      List.partition_result
        [
          readonly_this_err;
          readonly_ret_err;
          package_err;
          return_disposable_err;
          arity_min_err;
          variadic_err;
        ]
    in
    (* If we have no errors, return valid *)
    if List.is_empty errs then
      TL.valid
    else
      (* Otherwise, combine the errors and return invalid *)
      TL.invalid ~fail:(Typing_error.multiple_opt @@ List.filter_opt errs)

  (* Construct a tuple type from the parameters in a function type.
   * This is used for subtyping function types exactly one of which contains
   * a splat parameter e.g. consider
   *
   * (function(t1',_,tm', ...T):r') <:
   * (function(t1, _, tm, u1, _, un, optional v1, _, vk, w...):r <:
   *
   * (Here ...T is a splat parameter and w... is a variadic).
   *
   * We need to subtype t1<:t1', _, tm<:tm' and r'<:r as normal but
   * splat parameter T is compared a tuple of the remaining parameters in the supertype
   * i.e. T <: (u1, _, un, optional v1, _, optional vk, w...)
   *)
  and params_to_tuple fun_type_pos is_variadic fn_params =
    (* We construct a position that spans all the parameters that we are gathering.
     * If there are no parameters, just use the position from the function type *)
    let acc_pos =
      match fn_params with
      | [] -> fun_type_pos
      | { fp_pos; fp_type; _ } :: _ ->
        Pos_or_decl.merge (get_pos fp_type) fp_pos
    in
    let rec aux fn_params acc_pos acc_required acc_optional acc_variadic =
      match fn_params with
      | [] ->
        mk
          ( Reason.tuple_from_splat acc_pos,
            Ttuple
              {
                t_required = List.rev acc_required;
                t_extra =
                  Textra
                    {
                      t_optional = List.rev acc_optional;
                      t_variadic = acc_variadic;
                    };
              } )
      | ({ fp_type; fp_pos; _ } as fn_param) :: fn_params ->
        let acc_pos =
          if List.is_empty fn_params then
            Pos_or_decl.merge acc_pos fp_pos
          else
            acc_pos
        in
        let (acc_required, acc_optional, acc_variadic) =
          if List.is_empty fn_params && is_variadic then
            (acc_required, acc_optional, fp_type)
          else if get_fp_is_optional fn_param then
            (acc_required, fp_type :: acc_optional, acc_variadic)
          else
            (fp_type :: acc_required, acc_optional, acc_variadic)
        in
        aux fn_params acc_pos acc_required acc_optional acc_variadic
    in
    aux
      fn_params
      acc_pos
      []
      []
      (MakeType.nothing (Reason.witness_from_decl fun_type_pos))

  and simplify_subtype_params
      ~(subtype_env : Subtype_env.t)
      ~for_override
      (r_sub, idx_sub, fn_params_sub, variadic_sub_ty)
      (r_super, idx_super, fn_params_super, variadic_super_ty)
      env =
    match (fn_params_sub, fn_params_super) with
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
    | ([{ fp_type = ty_sub; _ }], _) when variadic_sub_ty ->
      simplify_subtype_params_with_variadic
        ~subtype_env
        (r_super, idx_super, fn_params_super)
        (r_sub, idx_sub, ty_sub)
        env
    | (_, [{ fp_type = ty_super; _ }]) when variadic_super_ty ->
      simplify_supertype_params_with_variadic
        ~subtype_env
        (r_sub, idx_sub, fn_params_sub)
        (r_super, idx_super, ty_super)
        env
    (* Two splat parameters are just compared directly *)
    | ( [({ fp_type = ty_sub; _ } as fn_param_sub)],
        [({ fp_type = ty_super; _ } as fn_param_super)] )
      when get_fp_splat fn_param_sub && get_fp_splat fn_param_super ->
      env
      |> simplify
           ~subtype_env
           ~this_ty:None
           ~lhs:{ sub_supportdyn = None; ty_sub = ty_super }
           ~rhs:
             { super_like = false; super_supportdyn = false; ty_super = ty_sub }
    (* If supertype is a splat parameter then package up remaining parameters in subtype as
     * a tuple and compare that
     *)
    | (_, [({ fp_type = ty_super; _ } as fn_param_super)])
      when get_fp_splat fn_param_super ->
      let tuple_ty_sub =
        params_to_tuple (Reason.to_pos r_sub) variadic_sub_ty fn_params_sub
      in
      env
      |> simplify
           ~subtype_env
           ~this_ty:None
           ~lhs:{ sub_supportdyn = None; ty_sub = ty_super }
           ~rhs:
             {
               super_like = false;
               super_supportdyn = false;
               ty_super = tuple_ty_sub;
             }
    (* If subtype is a splat parameter then package up remaining parameters in supertype as
     * a tuple and compare that
     *)
    | ([({ fp_type = ty_sub; _ } as fn_param_sub)], _)
      when get_fp_splat fn_param_sub ->
      let tuple_ty =
        params_to_tuple
          (Reason.to_pos r_super)
          variadic_super_ty
          fn_params_super
      in
      env
      |> simplify
           ~subtype_env
           ~this_ty:None
           ~lhs:{ sub_supportdyn = None; ty_sub = tuple_ty }
           ~rhs:
             { super_like = false; super_supportdyn = false; ty_super = ty_sub }
    | ([], _) -> valid env
    | (_, []) -> valid env
    | (fn_param_sub :: fn_params_sub, fn_param_super :: fn_params_super) ->
      let { fp_type = ty_sub; _ } = fn_param_sub
      and { fp_type = ty_super; _ } = fn_param_super in

      (* Construct the subtype proposition for the two parameters; function
         paramaters are contravariant unless they are marked with `inout`
         in which case they are typed as invariant *)
      let subty_prop =
        let subtype_env_for_param =
          (* When overriding in Sound Dynamic, we treat any dynamic-aware subtype of dynamic as a
           * subtype of the dynamic type itself
           *)
          match get_node ty_sub with
          | Tdynamic
            when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
                 && for_override ->
            Subtype_env.set_coercing_to_dynamic subtype_env
          | _ -> subtype_env
        in
        (* Construct the contravariant subtype proposition *)
        let subty_prop_contra is_inout =
          (* Update the provenance on the super type:
             i) Reverse the direction of flow for the containing function-type reasons
             ii) Record the projection into the ith function arg, treated as contravariant, of that function-type
             iii) Record the flow from the supertype into that arg *)
          let ty_super =
            Typing_env.update_reason env ty_super ~f:(fun r_super_prj ->
                if is_inout then
                  Typing_reason.prj_fn_param_inout_contra
                    ~super:r_super
                    ~super_prj:r_super_prj
                    ~sub:r_sub
                    ~idx_sub
                    ~idx_super
                else
                  Typing_reason.prj_fn_param
                    ~super:r_super
                    ~super_prj:r_super_prj
                    ~sub:r_sub
                    ~idx_sub
                    ~idx_super)
          in
          simplify
            ~subtype_env:subtype_env_for_param
            ~this_ty:None
            ~lhs:{ sub_supportdyn = None; ty_sub = ty_super }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = ty_sub;
              }
        in
        (* For parameters with the `inout` calling convention also construct
           the covariant subtype proposition *)
        match (get_fp_mode fn_param_sub, get_fp_mode fn_param_super) with
        | (FPinout, FPinout) ->
          let subty_prop_co =
            (* Update the provenance for the subtype:
               i) Record the flow from the parent function types
               ii) Record the projection in the ith arg, treated as covariant, of that function
               iii) Record the flow from the subtype into that arg *)
            let ty_sub =
              Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
                  Typing_reason.prj_fn_param_inout_co
                    ~sub:r_sub
                    ~sub_prj:r_sub_prj
                    ~super:r_super
                    ~idx_sub
                    ~idx_super)
            in
            simplify
              ~subtype_env:subtype_env_for_param
              ~this_ty:None
              ~lhs:{ sub_supportdyn = None; ty_sub }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          in
          (fun env -> subty_prop_contra true env &&& subty_prop_co)
        | _ -> (fun env -> subty_prop_contra false env)
      in

      (* Check that the calling conventions of the params are compatible. *)
      env
      |> simplify_param_modes ~subtype_env ~fn_param_sub ~fn_param_super
      &&& simplify_param_readonly ~subtype_env ~fn_param_sub ~fn_param_super
      &&& simplify_param_accept_disposable
            ~subtype_env
            ~fn_param_sub
            ~fn_param_super
      &&& subty_prop
      &&& simplify_subtype_params
            ~subtype_env
            ~for_override
            (r_sub, idx_sub + 1, fn_params_sub, variadic_sub_ty)
            (r_super, idx_super + 1, fn_params_super, variadic_super_ty)

  and simplify_funs
      ~subtype_env
      ~check_return
      ~for_override
      ~super_like
      (r_sub, ft_sub)
      (r_super, ft_super)
      env =
    (* First apply checks on attributes and variadic arity *)
    let simplify_subtype_implicit_params_help =
      simplify_subtype_implicit_params ~subtype_env
    in
    ( env,
      simplify_subtype_funs_attributes
        ~subtype_env
        (r_sub, ft_sub)
        (r_super, ft_super) )
    &&& (* Now do contravariant subtyping on parameters *)
    begin
      simplify_subtype_params
        ~subtype_env
        ~for_override
        (r_sub, 0, ft_sub.ft_params, get_ft_variadic ft_sub)
        (r_super, 0, ft_super.ft_params, get_ft_variadic ft_super)
    end
    &&& simplify_subtype_implicit_params_help
          ft_super.ft_implicit_params
          ft_sub.ft_implicit_params
    &&&
    (* Finally do covariant subtyping on return type *)
    if check_return then
      let ty_super = Sd.liken ~super_like env ft_super.ft_ret in
      let subtype_env =
        if
          TypecheckerOptions.enable_sound_dynamic env.genv.tcopt && for_override
        then
          (* When overriding in Sound Dynamic, we allow t to override dynamic if
           * t is a dynamic-aware subtype of dynamic. We also allow Awaitable<t>
           * to override Awaitable<dynamic> and and Awaitable<t> to
           * override ~Awaitable<dynamic>.
           *)
          let (_, ty_super) = Typing_dynamic_utils.strip_dynamic env ty_super in
          match get_node ty_super with
          | Tdynamic -> Subtype_env.set_coercing_to_dynamic subtype_env
          | Tclass ((_, class_name), _, [ty])
            when String.equal class_name SN.Classes.cAwaitable && is_dynamic ty
            ->
            Subtype_env.set_coercing_to_dynamic subtype_env
          | _ -> subtype_env
        else
          subtype_env
      in
      (* Update the provenance for the subtype:
         i) Record the flow from the parent function types
         ii) Record the projection on the return type of that function
         iii) Record the flow from the subtype into that return type *)
      let ty_sub =
        Typing_env.update_reason env ft_sub.ft_ret ~f:(fun r_sub_prj ->
            Typing_reason.prj_fn_ret
              ~sub:r_sub
              ~sub_prj:r_sub_prj
              ~super:r_super)
      in
      simplify
        ~subtype_env
        ~this_ty:None
        ~lhs:{ sub_supportdyn = None; ty_sub }
        ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
    else
      valid

  (* == Class / newtype subtyping =========================================== *)
  and simplify_subtype_variance_for_injective_loop
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~super_like
      nm
      (ctor_kind : Typing_reason.ctor_kind)
      (idx : int)
      (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
      (r_sub, children_tyl)
      (r_super, super_tyl)
      env =
    let simplify_subtype_help reify_kind ~sub_supportdyn ty_sub ty_super env =
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
      simplify
        ~subtype_env
        ~this_ty:None
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        env
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

      let prop env =
        match variance with
        | Ast_defs.Covariant ->
          let ty_sub =
            Typing_env.update_reason env child ~f:(fun r_sub_prj ->
                Typing_reason.prj_ctor_co
                  ~sub:r_sub
                  ~sub_prj:r_sub_prj
                  ~super:r_super
                  ctor_kind
                  nm
                  idx
                  false)
          and ty_super = Sd.liken ~super_like env super in
          simplify_subtype_help ~sub_supportdyn ty_sub ty_super env
        | Ast_defs.Contravariant ->
          let ty_super =
            map_reason super ~f:(fun r_super_prj ->
                Typing_reason.prj_ctor_contra
                  ~sub:r_sub
                  ~super:r_super
                  ~super_prj:r_super_prj
                  ctor_kind
                  nm
                  idx
                  false)
          and ty_sub = child in
          simplify_subtype_help ~sub_supportdyn ty_super ty_sub env
        | Ast_defs.Invariant ->
          let covariant_prop env =
            let ty_sub =
              Typing_env.update_reason env child ~f:(fun r_sub_prj ->
                  Typing_reason.prj_ctor_co
                    ~sub:r_sub
                    ~sub_prj:r_sub_prj
                    ~super:r_super
                    ctor_kind
                    nm
                    idx
                    true)
            and ty_super = Sd.liken ~super_like env super in
            simplify_subtype_help ~sub_supportdyn ty_sub ty_super env
          and contravariant_prop env =
            let f r_super_prj =
              Typing_reason.prj_ctor_contra
                ~sub:r_sub
                ~super:r_super
                ~super_prj:r_super_prj
                ctor_kind
                nm
                idx
                true
            in
            let ty_sub = Sd.liken ~super_like env child
            and ty_super = map_reason super ~f in
            simplify_subtype_help ~sub_supportdyn ty_super ty_sub env
          in
          covariant_prop env &&& contravariant_prop
      in
      prop env
      &&& simplify_subtype_variance_for_injective_loop
            ~subtype_env
            ~sub_supportdyn
            ~super_like
            nm
            ctor_kind
            (idx + 1)
            variance_reifiedl
            (r_sub, childrenl)
            (r_super, superl)

  and simplify_subtype_variance_for_injective
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~super_like
      cid
      ctor_kind
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
      ctor_kind
      0

  and simplify_subtype_classes
      ~fail
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      ~(super_like : bool)
      (r_sub, (class_id_sub, exact_sub, tyl_sub))
      (r_super, (class_id_super, exact_super, tyl_super))
      env : env * TL.subtype_prop =
    let invalid_env = invalid ~fail in
    let (cid_super, cid_sub) = (snd class_id_super, snd class_id_sub) in
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
                List.map tyl_sub ~f:(fun _ -> (Ast_defs.Invariant, Aast.Erased))
              | Decl_entry.Found class_sub ->
                List.map (Cls.tparams class_sub) ~f:(fun t ->
                    (t.tp_variance, t.tp_reified))
          in
          simplify_subtype_variance_for_injective
            ~subtype_env
            ~sub_supportdyn
            ~super_like
            cid_sub
            Typing_reason.Ctor_class
            (Decl_entry.to_option class_def_sub)
            variance_reifiedl
            (r_sub, tyl_sub)
            (r_super, tyl_super)
            env
    else if not exact_match then
      invalid_env env
    else
      let class_def_sub = Env.get_class env cid_sub in
      match class_def_sub with
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
            substs = TUtils.make_locl_subst_for_class_tparams class_sub tyl_sub;
            (* FIXME(T59448452): Unsound in general *)
            this_ty =
              Option.value
                this_ty
                ~default:(mk (r_sub, Tclass (class_id_sub, exact_sub, tyl_sub)));
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
          let up_obj =
            Typing_env.update_reason env up_obj ~f:(fun ancestor ->
                Typing_reason.axiom_extends ~child:r_sub ~ancestor)
          in

          (match deref up_obj with
          | (r_sub, Tclass (class_id_sub, exact_sub, tyl_sub)) ->
            let subtype_env =
              Subtype_env.possibly_add_violated_constraint
                subtype_env
                ~r_sub
                ~r_super
            in
            let ity_super =
              LoclType
                (mk (r_super, Tclass (class_id_super, exact_super, tyl_super)))
            in
            let ity_sub = LoclType up_obj in
            let fail =
              Subtype_env.fail subtype_env ~ty_sub:ity_sub ~ty_super:ity_super
            in
            simplify_subtype_classes
              ~fail
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              (r_sub, (class_id_sub, exact_sub, tyl_sub))
              (r_super, (class_id_super, exact_super, tyl_super))
              env
          | _ ->
            (* The superclass must be classish *)
            invalid env ~fail)
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
                let prop env =
                  match deref ub_obj_typ with
                  | (_, (Tclass (class_id_sub, exact_sub, tyl_sub) as cls_sub))
                    ->
                    let ity_super =
                      LoclType
                        (mk
                           ( r_super,
                             Tclass (class_id_super, exact_super, tyl_super) ))
                    in
                    (* Retain reason / position for original subclass, not the upper bound *)
                    let ity_sub = LoclType (mk (r_sub, cls_sub)) in
                    let fail =
                      Subtype_env.fail
                        subtype_env
                        ~ty_sub:ity_sub
                        ~ty_super:ity_super
                    in
                    simplify_subtype_classes
                      ~fail
                      ~subtype_env
                      ~sub_supportdyn
                      ~this_ty
                      ~super_like
                      (r_sub, (class_id_sub, exact_sub, tyl_sub))
                      (r_super, (class_id_super, exact_super, tyl_super))
                      env
                  | _ -> invalid env ~fail
                in
                let ( ||| ) = ( ||| ) ~fail in

                env |> prop ||| try_upper_bounds_on_this up_objs
            in
            try_upper_bounds_on_this (Cls.upper_bounds_on_this class_sub) env
          else
            invalid_env env)

  and simplify_subtype_variance_for_non_injective
      ~subtype_env
      ~sub_supportdyn
      ~super_like
      cid
      class_sub
      (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
      (r_sub, children_tyl)
      (r_super, super_tyl)
      lty_sub
      lty_super
      env =
    let ((env, p) as res) =
      simplify_subtype_variance_for_injective
        ~subtype_env
        ~sub_supportdyn
        ~super_like
        cid
        Typing_reason.Ctor_newtype
        class_sub
        variance_reifiedl
        (r_sub, children_tyl)
        (r_super, super_tyl)
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
          (LoclType lty_sub)
          (LoclType lty_super)
      else
        (env, TL.valid)
    else
      res

  (* == Shape subtyping ===================================================== *)

  and simplify_subtype_shape
      ~(subtype_env : Subtype_env.t)
      ~(env : env)
      ~(this_ty : locl_ty option)
      ~super_like
      (supportdyn_sub, r_sub, shape_kind_sub, fdm_sub)
      (supportdyn_super, r_super, shape_kind_super, fdm_super) =
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
      let field_names =
        TShapeSet.of_list (TShapeMap.keys fdm_sub @ TShapeMap.keys fdm_super)
      in
      TShapeSet.fold
        (fun field_name acc ->
          let res =
            shape_project_and_simplify_subtype
              ~subtype_env
              ~this_ty
              ~super_like
              ~env
              (supportdyn_sub, r_sub, shape_kind_sub, fdm_sub)
              (supportdyn_super, r_super, shape_kind_super, fdm_super)
              field_name
          in
          match res with
          | Ok prop -> acc &&& prop
          | Error fail -> acc &&& invalid ~fail)
        field_names
        (valid env)

  (* Helper function to project out a field and then simplify subtype *)
  and shape_project_and_simplify_subtype
      ~subtype_env
      ~this_ty
      ~super_like
      ~env
      (supportdyn_sub, r_sub, shape_kind_sub, shape_map_sub)
      (supportdyn_super, r_super, shape_kind_super, shape_map_super)
      field_name =
    let proj_sub =
      shape_projection
        ~supportdyn:supportdyn_sub
        ~env
        field_name
        shape_kind_sub
        shape_map_sub
        r_sub
    in
    let proj_super =
      shape_projection
        ~supportdyn:supportdyn_super
        ~env
        field_name
        shape_kind_super
        shape_map_super
        r_super
    in
    simplify_subtype_shape_projection
      ~subtype_env
      ~this_ty
      ~super_like
      ~env
      (supportdyn_sub, r_sub, proj_sub)
      (supportdyn_super, r_super, proj_super)
      field_name

  (* For two particular projections `p1` and `p2`, `p1` <: `p2` iff:
      - `p1` = `Required ty1`, `p2` = `Required ty2`, and `ty1` <: `ty2`
      - `p1` = `Required ty1`, `p2` = `Optional ty2`, and `ty1` <: `ty2`
      - `p1` = `Optional ty1`, `p2` = `Optional ty2`, and `ty1` <: `ty2`
      - `p1` = `Absent`, `p2` = `Optional ty2`
      - `p1` = `Absent`, `p2` = `Absent`
     We therefore need to handle all other cases appropriately. *)
  and simplify_subtype_shape_projection
      ~subtype_env
      ~this_ty
      ~super_like
      ~env
      (supportdyn_sub, r_sub, proj_sub)
      (_supportdyn_super, r_super, proj_super)
      field_name =
    let field_pos = TShapeField.pos field_name in
    let printable_name = TUtils.get_printable_shape_field_name field_name in
    let res =
      match (proj_sub, proj_super) with
      (***** "Successful" cases - 5 / 9 total cases *****)
      | (`Required ty_sub, `Required ty_super) ->
        Ok
          (Some
             ( (Typing_reason.Required, ty_sub),
               (Typing_reason.Required, ty_super) ))
      | (`Required ty_sub, `Optional ty_super) ->
        Ok
          (Some
             ( (Typing_reason.Required, ty_sub),
               (Typing_reason.Optional, ty_super) ))
      | (`Optional ty_sub, `Optional ty_super) ->
        Ok
          (Some
             ( (Typing_reason.Optional, ty_sub),
               (Typing_reason.Optional, ty_super) ))
      | (`Absent, `Optional _)
      | (`Absent, `Absent) ->
        Ok None
      (***** Error cases - 4 / 9 total cases *****)
      | (`Required ty_sub, `Absent) ->
        let r_field_sub =
          get_reason
          @@ Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
                 Typing_reason.(
                   prj_shape
                     ~sub:r_sub
                     ~sub_prj:r_sub_prj
                     ~super:r_super
                     printable_name
                     ~kind_sub:Required
                     ~kind_super:Absent))
        and r_field_super = Typing_reason.missing_field in
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
                         reason_sub = r_field_sub;
                         reason_super = r_field_super;
                       })
        in
        Error ty_err_opt
      | (`Optional ty_sub, `Absent) ->
        let r_field_sub =
          get_reason
          @@ Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
                 Typing_reason.(
                   prj_shape
                     ~sub:r_sub
                     ~sub_prj:r_sub_prj
                     ~super:r_super
                     printable_name
                     ~kind_sub:Optional
                     ~kind_super:Absent))
        and r_field_super = Typing_reason.missing_field in
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
                         reason_sub = r_field_sub;
                         reason_super = r_field_super;
                       })
        in

        Error ty_err_opt
      | (`Optional ty_sub, `Required ty_super) ->
        let r_field_sub =
          get_reason
          @@ Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
                 Typing_reason.(
                   prj_shape
                     ~sub:r_sub
                     ~sub_prj:r_sub_prj
                     ~super:r_super
                     printable_name
                     ~kind_sub:Optional
                     ~kind_super:Required))
        and r_field_super = get_reason ty_super in
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
                         def_pos = get_pos ty_super;
                         reason_sub = r_field_sub;
                         reason_super = r_field_super;
                       })
        in

        Error ty_err_opt
      | (`Absent, `Required ty_super) ->
        let quickfixes_opt =
          match Reason.Predicates.unpack_shape_literal_opt r_sub with
          | Some p ->
            let fix_pos =
              Pos.shrink_to_end (Pos.shrink_by_one_char_both_sides p)
            in
            Some
              [
                Quickfix.make_eager_default_hint_style
                  ~title:("Add field " ^ Markdown_lite.md_codify printable_name)
                  ~new_text:(Printf.sprintf ", '%s' => TODO" printable_name)
                  fix_pos;
              ]
          | None -> None
        in
        let r_field_sub =
          let r_sub_prj = Typing_reason.missing_field in
          Typing_reason.prj_shape
            ~sub:r_sub
            ~sub_prj:r_sub_prj
            ~super:r_super
            printable_name
            ~kind_sub:Typing_reason.Absent
            ~kind_super:Typing_reason.Required
        and r_field_super = get_reason ty_super in
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
                         reason_sub = r_field_sub;
                         reason_super = r_field_super;
                       })
        in

        Error ty_err_opt
    in

    match res with
    | Ok None -> Ok valid
    | Ok (Some ((kind_sub, ty_sub), (kind_super, ty_super))) ->
      let ty_sub =
        Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
            Typing_reason.prj_shape
              ~sub:r_sub
              ~sub_prj:r_sub_prj
              ~super:r_super
              printable_name
              ~kind_sub
              ~kind_super)
      and ty_super = Sd.liken ~super_like env ty_super in
      let sub_supportdyn =
        if supportdyn_sub then
          Some r_sub
        else
          None
      in
      let lhs = { sub_supportdyn; ty_sub }
      and rhs = { super_like = false; super_supportdyn = false; ty_super } in
      Ok (simplify ~subtype_env ~this_ty ~lhs ~rhs)
    | Error err -> Error err

  (* Shape projection for shape type `s` and field `f` (`s |_ f`) is defined as:
     - if `f` appears in `s` as `f => ty` then `s |_ f` = `Required ty`
     - if `f` appears in `s` as `?f => ty` then `s |_ f` = `Optional ty`
     - if `f` does not appear in `s` and `s` is closed, then `s |_ f` = `Absent`
     - if `f` does not appear in `s` and `s` is open, then `s |_ f` = `Optional mixed`
     EXCEPT
     - `?f => nothing` should be ignored, and treated as `Absent`.
     Such a field cannot be given a value, and so is effectively not present. *)
  and shape_projection ~supportdyn ~env field_name shape_kind shape_map r =
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
        let printable_name = TUtils.get_printable_shape_field_name field_name in
        let ty =
          with_reason
            shape_kind
            (Reason.missing_optional_field (Reason.to_pos r, printable_name))
        in
        `Optional (make_supportdyn ty)

  (* == Entry point ========================================================= *)
  and simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    if Log.should_log env subtype_env ~lhs ~rhs then
      Log.log env subtype_env ~this_ty ~lhs ~rhs @@ fun () ->
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      (* This path should be fast, so we make sure not to create a closure unlike the other path. *)
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_ ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn; ty_sub } = lhs
    and { super_supportdyn; super_like; ty_super } = rhs in
    simplify_subtype_by_physical_equality env ty_sub ty_super @@ fun () ->
    let (env, ty_super) = Env.expand_type env ty_super in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    simplify_subtype_by_physical_equality env ty_sub ty_super @@ fun () ->
    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(get_reason ty_sub)
        ~r_super:(get_reason ty_super)
    in
    let fail =
      Subtype_env.fail
        subtype_env
        ~ty_sub:(LoclType ty_sub)
        ~ty_super:(LoclType ty_super)
    in
    let (env, ty_sub) =
      if subtype_env.Subtype_env.ignore_likes then
        let (env, ty_opt) =
          Typing_dynamic_utils.try_strip_dynamic
            ~do_not_solve_likes:true
            ~accept_intersections:true
            env
            ty_sub
        in
        (env, Option.value ~default:ty_sub ty_opt)
      else
        (env, ty_sub)
    in
    match (deref ty_sub, deref ty_super) with
    (* First come all the rewrites.
       By "rewrite", we mean stuff like A <: ?B ---> A & nonnull <: B *)
    (* -- Rewrite: x <: Tunion[t] ---> x <: t *)
    | (_, (_, Tunion [ty_super'])) ->
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super = ty_super' }
        env
    (* -- Rewrite:
          t <: #1 | arraykey => t & !arraykey <: #1
          or t <: #1 | !arraykey => t & arraykey <: #1
       TODO(mjt) this is awkward because we need to expand and dereference types
       in a nested position. This would be easier if we didn't have the
       implmentation abstract within constraint solving and we had a function
       (or set of functions) for expansion to a given depth *)
    | (_, (_, Tunion [ty_super1; ty_super2]))
      when expands_to_var_and_arraykey ty_super1 ty_super2 ~env ->
      let (env, (tvar_ty, ak_ty)) =
        var_and_arraykey_exn ty_super1 ty_super2 ~env
      in

      let (env, neg_ty) =
        Inter.negate_type
          env
          (get_reason (mk ak_ty))
          ~approx:Inter.Utils.ApproxDown
          (mk ak_ty)
      in
      let (env, inter_ty) =
        Inter.intersect env ~r:(get_reason lhs.ty_sub) neg_ty lhs.ty_sub
      in
      let lhs = { lhs with ty_sub = inter_ty }
      and rhs =
        { super_like = false; super_supportdyn = false; ty_super = mk tvar_ty }
      in
      simplify ~subtype_env ~this_ty ~lhs ~rhs env
    (* -- Rewrite: x <: ?supportdyn<t> ---> x <: supportdyn<?t> *)
    | ( ( _,
          ( Tany _ | Tnonnull | Toption _ | Tdynamic | Tprim _ | Tfun _
          | Ttuple _ | Tshape _ | Tvec_or_dict _ | Taccess _
          | Tunapplied_alias _ | Tgeneric _ | Tnewtype _ | Tdependent _
          | Tclass _ | Tneg _ | Tunion _ | Tintersection _ | Tvar _
          | Tclass_args _ ) ),
        (r_super, Toption ty_inner) )
      when expands_to_supportdyn ty_inner ~env ->
      let (env, ty_inner) = Env.expand_type env ty_inner in
      (* Since we have guarded on [ty_inner] being a supportdyn new type,
         the following calls will not generate exceptions *)
      let (env, (_, tyargs, _)) = newtype_exn ty_inner ~env in
      let tyarg = List.hd_exn tyargs in
      let tyarg = MakeType.nullable r_super tyarg in
      let ty_super = MakeType.supportdyn r_super tyarg in
      let lhs = { lhs with ty_sub }
      and rhs =
        { super_like = rhs.super_like; super_supportdyn = false; ty_super }
      in
      simplify ~subtype_env ~this_ty ~lhs ~rhs env
    (* -- Rewrite:
          supportdyn<t> <: ?u
          ---> nonnull & supportdyn<t> <: u
          ---> supportdyn<nonnull & t> <: u
    *)
    | ((r, Tnewtype (name, [tyarg1], _)), (r_super, Toption lty_inner))
      when String.equal name SN.Classes.cSupportDyn ->
      (* TODO(mjt) add 'rewrite' reason including both r & r_super? *)
      let (env, ty_sub') =
        Inter.intersect env ~r:r_super tyarg1 (MakeType.nonnull r_super)
      in
      let lty_inner =
        Typing_env.update_reason env lty_inner ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      simplify
        ~subtype_env
        ~this_ty:None
        ~lhs:{ sub_supportdyn = Some r; ty_sub = ty_sub' }
        ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
        env
    (* -- Rewrite: _ & ...  & _ <: ?t ---> _ & .. & _ & nonnull <: t
       but only when the intersection does not contain any type with an exact
       negation (we apply a rewrite in the other direction for those cases) *)
    | ((_, Tintersection ty_subs), (r_super, Toption ty_inner_super))
      when expands_to_var_or_intersection ty_inner_super ~env
           &&
           let (_, non_ty_opt, _) =
             Subtype_negation.find_type_with_exact_negation env ty_subs
           in
           Option.is_none non_ty_opt ->
      let (env, ty_sub) =
        Inter.intersect env ~r:r_super ty_sub (MakeType.nonnull r_super)
      in
      let ty_inner_super =
        Typing_env.update_reason env ty_inner_super ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      let lhs = { lhs with ty_sub }
      and rhs =
        { rhs with super_supportdyn = false; ty_super = ty_inner_super }
      in
      simplify ~subtype_env ~this_ty ~lhs ~rhs env
    (* -- Rewrite: class<T> <: C = XHPChild or Stringish
          ---> classname<T> <: C *)
    | ( (r_sub, Tclass_args ty_sub),
        (_r_super, Tclass ((_, class_nm_super), exact_super, _)) )
      when TypecheckerOptions.class_sub_classname env.genv.tcopt
           && is_nonexact exact_super
           && (String.equal class_nm_super SN.Classes.cXHPChild
              || String.equal class_nm_super SN.Classes.cStringish) ->
      (* TODO(T199610905) replace reason with upcoming rewrite reasons *)
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = MakeType.classname r_sub [ty_sub] }
        ~rhs:{ super_like; super_supportdyn; ty_super }
        env
    (* -- Rewrite: class<T> <: N ---> classname<T> <: N
          instead of e.g. considering class<T> as an element in a case type *)
    | ((r_sub, Tclass_args ty_sub), (_r_super, Tnewtype _))
      when TypecheckerOptions.class_sub_classname env.genv.tcopt ->
      (* TODO(T199610905) replace reason with upcoming rewrite reasons *)
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = MakeType.classname r_sub [ty_sub] }
        ~rhs:{ super_like; super_supportdyn; ty_super }
        env
    (* -- Rewrite: class<T> <: prim ---> classname<T> <: prim *)
    | ((r_sub, Tclass_args ty_sub), (_r_super, Tprim _))
      when TypecheckerOptions.class_sub_classname env.genv.tcopt ->
      (* TODO(T199610905) replace reason with upcoming rewrite reasons *)
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = MakeType.classname r_sub [ty_sub] }
        ~rhs:{ super_like; super_supportdyn; ty_super }
        env
    (* -- C-Var-R ----------------------------------------------------------- *)
    | ((_, Tunion _), (_, Tvar _)) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    | ((_, Tdynamic), (_, Tvar _))
      when Subtype_env.coercing_from_dynamic subtype_env ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* We want to treat nullable as a union with the same rule as above.
     * This is only needed for Tvar on right; other cases are dealt with specially as
     * derived rules.
     *)
    | ((r, Toption t), (_r_super, Tvar _var_super)) ->
      let (env, t) = Env.expand_type env t in
      (match get_node t with
      (* We special case on `mixed <: Tvar _`, adding the entire `mixed` type
         as a lower bound. This enables clearer error messages when upper bounds
         are added to the type variable: transitive closure picks up the
         entire `mixed` type, and not separately consider `null` and `nonnull` *)
      | Tnonnull ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (LoclType ty_super)
      | _ ->
        let ty_null = MakeType.null r in
        let t =
          Typing_env.update_reason env t ~f:(fun r_sub_prj ->
              Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r_sub_prj)
        in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = t }
          ~rhs:{ super_like; super_supportdyn = false; ty_super }
          env
        &&& simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = ty_null }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super })
    | ((_, Tvar var_sub), (_, Tvar var_super)) when Tvid.equal var_sub var_super
      ->
      valid env
    | ( ( _,
          ( Tany _ | Tnonnull | Tdynamic | Tprim _ | Tvar _ | Tfun _ | Ttuple _
          | Tshape _ | Tgeneric _ | Tintersection _ | Tvec_or_dict _ | Taccess _
          | Tnewtype _ | Tunapplied_alias _ | Tdependent _ | Tclass _ | Tneg _
          | Tlabel _ | Tclass_args _ ) ),
        (_r_super, Tvar var_super_id) ) -> begin
      let (env, simplified_sub_ty) =
        Typing_solver_utils.remove_tyvar_from_lower_bound
          env
          var_super_id
          (LoclType ty_sub)
      in
      match simplified_sub_ty with
      | LoclType simplified_sub_ty
      (* Better than checking nothing like this might be to change
         remove_tyvar_from_lower_bound to return an Option to distinguish
         whether a simplification was actually done.
         And then we can recursively do a subtype check to check this.
      *)
        when ty_equal simplified_sub_ty (MakeType.nothing Reason.none) ->
        valid env
      | _ ->
        (match subtype_env.Subtype_env.coerce with
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
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
              env
          else
            mk_issubtype_prop
              ~sub_supportdyn
              ~coerce:subtype_env.Subtype_env.coerce
              env
              (LoclType ty_sub)
              (LoclType ty_super))
    end
    (* -- C-Inter-R --------------------------------------------------------- *)
    | ((r_sub, Tunion ty_subs), (_r_super, Tintersection _)) ->
      let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
      in
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | ( ( _r_sub,
          ( Tany _ | Tnonnull | Toption _ | Tdynamic | Tprim _ | Tvar _ | Tfun _
          | Ttuple _ | Tshape _ | Tgeneric _ | Tintersection _ | Tvec_or_dict _
          | Taccess _ | Tnewtype _ | Tunapplied_alias _ | Tdependent _
          | Tclass _ | Tneg _ | Tlabel _ | Tclass_args _ ) ),
        (r_super, Tintersection tyl) ) ->
      (* t <: (t1 & ... & tn)
       *   if and only if
       * t <: t1 /\  ... /\ t <: tn
       *)
      List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_super ->
          let ty_super =
            Typing_env.update_reason env ty_super ~f:(fun r_super_prj ->
                Typing_reason.prj_inter_super
                  ~super:r_super
                  ~super_prj:r_super_prj)
          in
          res
          &&& simplify
                ~subtype_env
                ~this_ty
                ~lhs:{ sub_supportdyn; ty_sub }
                ~rhs:{ super_like = false; super_supportdyn = false; ty_super })
    (* -- C-Bot-R ----------------------------------------------------------- *)
    (* Empty union encodes the bottom type nothing *)
    | ((_, Tunion []), (_, Tunion [])) -> (env, TL.valid)
    | ((r_sub, Tunion tyl), (_r_super, Tunion [])) ->
      let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
      in
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, tyl)
        rhs
        env
    | ((r_sub, Tintersection tyl), (_r_super, Tunion [])) ->
      let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
      in
      (* Otherwise use the incomplete common case which doesn't require inspection of the rhs *)
      Common.simplify_intersection_l
        ~subtype_env
        ~this_ty
        ~fail
        ~mk_prop
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, tyl)
        rhs
        env
    | ((r, Tnewtype (n, _, ty)), (_, Tunion [])) ->
      let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty ~lhs ~rhs env
      in

      Common.simplify_newtype_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        (sub_supportdyn, r, n, ty)
        rhs
        env
    | ((_, Tgeneric _), (_, Tunion []))
      when subtype_env.Subtype_env.require_completeness ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        (LoclType ty_super)
    | ((r_generic, Tgeneric (name_sub, tyargs)), (_, Tunion [])) -> begin
      match
        VisitedGoals.try_add_visited_generic_sub
          subtype_env.Subtype_env.visited
          name_sub
          ty_super
      with
      | None ->
        (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
        *)
        invalid ~fail env
      | Some new_visited -> begin
        let subtype_env = Subtype_env.set_visited subtype_env new_visited in

        let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
          simplify ~subtype_env ~this_ty ~lhs ~rhs env
        in
        Common.simplify_generic_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop
          (sub_supportdyn, r_generic, name_sub, tyargs)
          { super_like; super_supportdyn = false; ty_super }
          { super_like = false; super_supportdyn = false; ty_super }
          env
      end
    end
    | ((r, Tdependent (dep_ty, ty)), (_, Tunion [])) ->
      let mk_prop ~subtype_env ~this_ty ~lhs ~rhs env =
        simplify ~subtype_env ~this_ty ~lhs ~rhs env
      in
      Common.simplify_dependent_l
        ~subtype_env
        ~this_ty
        ~mk_prop
        (sub_supportdyn, r, dep_ty, ty)
        rhs
        env
    | ((_, Tany _), (_, Tunion [])) when subtype_env.Subtype_env.no_top_bottom
      ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        (LoclType ty_super)
    | ((_, Tany _), (_, Tunion [])) -> valid env
    | ((_, Tdynamic), (_, Tunion []))
      when Subtype_env.coercing_from_dynamic subtype_env ->
      valid env
    | ((_, Tvar _), (_, Tunion [])) ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        (LoclType ty_super)
    | ( ( _,
          ( Tnonnull | Toption _ | Tdynamic | Tprim _ | Tfun _ | Ttuple _
          | Tshape _ | Tvec_or_dict _ | Taccess _ | Tunapplied_alias _
          | Tclass _ | Tneg _ ) ),
        (_, Tunion []) ) ->
      invalid env ~fail
    (* -- C-Union-R --------------------------------------------------------- *)
    | (_, (r_super, Tunion lty_supers)) -> begin
      match deref ty_sub with
      | (r_sub, Tunion ty_subs) ->
        let mk_prop ~subtype_env ~this_ty:_ ~lhs ~rhs env =
          simplify ~subtype_env ~this_ty:None ~lhs ~rhs env
        in
        Common.simplify_union_l
          ~subtype_env
          ~this_ty
          ~mk_prop
          ~update_reason:
            (Typing_env.update_reason ~f:(fun r_sub_prj ->
                 Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, ty_subs)
          rhs
          env
      | (_, Tvar _) when Option.is_some subtype_env.Subtype_env.coerce ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (LoclType ty_super)
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
          Typing_solver_utils.remove_tyvar_from_upper_bound
            env
            id
            (LoclType ty_super)
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
              (LoclType ty_sub)
              (LoclType ty_super))
      | (_, Tgeneric _) when subtype_env.Subtype_env.require_completeness ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (LoclType ty_super)
      (* Num is not atomic: it is equivalent to int|float. The rule below relies
       * on ty_sub not being a union e.g. consider num <: arraykey | float, so
       * we break out num first.
       *)
      | (r, Tprim Nast.Tnum) ->
        let r = Typing_reason.prj_num_sub ~sub:r ~sub_prj:r in
        let ty_float = MakeType.float r and ty_int = MakeType.int r in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn = None; ty_sub = ty_float }
          ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          env
        &&& simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn = None; ty_sub = ty_int }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
      (* Likewise, reduce nullable on left to a union *)
      | (r, Toption ty) ->
        let r = Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r in
        let ty_null = MakeType.null r in
        let prop_null =
          let prop =
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = ty_null }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          in

          (fun env -> if_unsat (invalid ~fail) @@ prop env)
        in
        let ty =
          Typing_env.update_reason env ty ~f:(fun r_sub_prj ->
              Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r_sub_prj)
        in
        prop_null env
        &&& simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = ty }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
      | (r_sub, Tintersection tyl) ->
        (* First try to rewrite
            _ & ... & !t & _ <: _ | ... | _ => _ & ... & _ <: _ | ... | t | _
        *)
        (match Subtype_negation.find_type_with_exact_negation env tyl with
        | (env, Some non_ty, tyl) ->
          let (env, ty_super) = TUtils.union env ty_super non_ty in
          let ty_sub = MakeType.intersection r_sub tyl in
          let lhs = { lhs with ty_sub }
          and rhs =
            { super_like = false; super_supportdyn = false; ty_super }
          in
          simplify ~subtype_env ~this_ty ~lhs ~rhs env
        | _ ->
          let ( ||| ) = ( ||| ) ~fail in
          let simplify_super_intersection env tyl_sub ty_super =
            (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
             * not complete.
             * TODO(T120921930): Don't do this if require_completeness is set.
             *)
            List.fold_left
              tyl_sub
              ~init:(env, TL.invalid ~fail)
              ~f:(fun res ty_sub ->
                res
                ||| simplify
                      ~subtype_env
                      ~this_ty
                      ~lhs:{ sub_supportdyn; ty_sub }
                      ~rhs:
                        {
                          super_like = false;
                          super_supportdyn = false;
                          ty_super;
                        })
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
            List.exists lty_supers ~f:(fun t ->
                TUtils.is_tintersection env t
                || TUtils.is_opt_tyvar env t
                || TUtils.is_tyvar env t)
          then
            simplify_sub_union
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              ~fail
              ty_sub
              (r_super, lty_supers)
              env
          else if List.exists tyl ~f:(TUtils.is_tunion env) then
            simplify_super_intersection
              env
              tyl
              (mk (r_super, Tunion lty_supers))
          else
            simplify_sub_union
              ~subtype_env
              ~sub_supportdyn
              ~this_ty
              ~super_like
              ~fail
              ty_sub
              (r_super, lty_supers)
              env)
      | ( _,
          ( Tany _ | Tprim _ | Tnonnull | Tdynamic | Tfun _ | Ttuple _
          | Tshape _ | Tgeneric _ | Tvec_or_dict _ | Taccess _ | Tnewtype _
          | Tunapplied_alias _ | Tdependent _ | Tclass _ | Tneg _ | Tlabel _
          | Tclass_args _ ) ) ->
        simplify_sub_union
          ~subtype_env
          ~sub_supportdyn
          ~this_ty
          ~super_like
          ~fail
          ty_sub
          (r_super, lty_supers)
          env
    end
    (* -- C-Top-R ----------------------------------------------------------- *)
    (* `Toption(Tnonnull)` encodes mixed, which is our top type *)
    | ( ( _,
          ( Tany _ | Tnonnull | Toption _ | Tdynamic | Tprim _ | Tfun _
          | Ttuple _ | Tshape _ | Tvec_or_dict _ | Taccess _
          | Tunapplied_alias _ | Tgeneric _ | Tnewtype _ | Tdependent _
          | Tclass _ | Tneg _ | Tunion _ | Tintersection _ | Tvar _ ) ),
        (_, Toption ty_inner) )
      when expands_to_nonnull ty_inner ~env ->
      valid env
    (* -- C-Option-R -------------------------------------------------------- *)
    (* null is the type of null and is a subtype of any option type. *)
    | ((_, Tprim Nast.Tnull), (_, Toption _)) -> valid env
    (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
     * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_sub' (widening) and transitivity
     * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
     * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
     * Therefore, this step preserves the set of solutions.
     *)
    | ((r_sub, Toption ty_sub'), (r_super, Toption lty_inner)) ->
      let ty_sub' =
        Typing_env.update_reason env ty_sub' ~f:(fun r_sub_prj ->
            Typing_reason.prj_nullable_sub ~sub:r_sub ~sub_prj:r_sub_prj)
      in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = ty_sub' }
        ~rhs:
          {
            super_like;
            super_supportdyn = false;
            ty_super = mk (r_super, Toption lty_inner);
          }
        env
    (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
     * must also have ty_sub <: ty_super'.  The converse follows by
     * widening and transitivity.  Therefore, this step preserves the set
     * of solutions.
     *)
    | ((_, (Tintersection _ | Tunion _)), (r_super, Toption lty_inner))
      when TUtils.is_type_disjoint env ty_sub (MakeType.null Reason.none) ->
      let lty_inner =
        Typing_env.update_reason env lty_inner ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
        env
    (* We do not want to decompose Toption for these cases *)
    | ((_, (Tvar _ | Tunion _ | Tintersection _)), (r_super, Toption lty_inner))
      ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:
          {
            super_like;
            super_supportdyn = false;
            ty_super = mk (r_super, Toption lty_inner);
          }
        env
    | ((_, Tgeneric _), (r_super, Toption lty_inner))
      when subtype_env.Subtype_env.require_completeness ->
      (* TODO(T69551141) handle type arguments ? *)
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:
          {
            super_like;
            super_supportdyn = false;
            ty_super = mk (r_super, Toption lty_inner);
          }
        env
    (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
     * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
     * true as well.  We can fold the case where t1 is unconstrained
     * into the case analysis below.
     *
     * In the case where it's easy to determine that null isn't in t1,
     * we need only check t1 <: t2.
     *)
    | ( (_r_sub, (Tnewtype _ | Tdependent _ | Tgeneric _ | Tprim Nast.Tvoid)),
        (r_super, Toption lty_inner) ) ->
      let lty_inner =
        Typing_env.update_reason env lty_inner ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      (* TODO(T69551141) handle type arguments? *)
      if null_not_subtype ty_sub then
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
          env
      else
        let ( ||| ) = ( ||| ) ~fail in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
          env
        ||| default_subtype
              ~subtype_env
              ~this_ty
              ~fail
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
    | ((_, Tunapplied_alias _), (_, Toption _)) ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | ( ( _r_sub,
          ( Tdynamic | Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _
          | Tclass _ | Tvec_or_dict _ | Tany _ | Taccess _ | Tlabel _
          | Tclass_args _ ) ),
        (r_super, Toption lty_inner) ) ->
      let lty_inner =
        Typing_env.update_reason env lty_inner ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
        env
    (* This is treating the option as a union, and using the sound, but incomplete,
       t <: t1 | t2 to (t <: t1) || (t <: t2) reduction
       TODO(T120921930): Don't do this if require_completeness is set.
    *)
    | ((_r_sub, Tneg _), (r_super, Toption lty_inner)) ->
      let lty_inner =
        Typing_env.update_reason env lty_inner ~f:(fun r_super_prj ->
            Typing_reason.prj_nullable_super
              ~super:r_super
              ~super_prj:r_super_prj)
      in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super = lty_inner }
        env
    (* -- C-Dep-R ----------------------------------------------------------- *)
    | ((_, Tdependent (d_sub, bound_sub)), (_, Tdependent (d_sup, bound_sup)))
      when equal_dependent_type d_sub d_sup ->
      simplify
        ~subtype_env
        ~this_ty:(Option.first_some this_ty (Some ty_sub))
        ~lhs:{ sub_supportdyn; ty_sub = bound_sub }
        ~rhs:
          { super_like = false; super_supportdyn = false; ty_super = bound_sup }
        env
    | ((_, Tdependent (_, bound_sub)), (_, Tdependent _)) ->
      simplify
        ~subtype_env
        ~this_ty:(Option.first_some this_ty (Some ty_sub))
        ~lhs:{ sub_supportdyn; ty_sub = bound_sub }
        ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        env
    | ((r_sub, Tclass ((_, y), _, _)), (_r_super, Tdependent (_, bound_sup)))
      when expands_to_class bound_sup ~env ->
      let (env, ((_, x) as id)) = class_id_exn bound_sup ~env in

      if is_final_and_invariant env x then
        (* For final class C, there is no difference between `this as X` and `X`,
           and `expr<#n> as X` and `X`. But we need to take care with variant
           classes, since we can't statically guarantee their runtime type. *)
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:
            {
              super_like = false;
              super_supportdyn = false;
              ty_super = bound_sup;
            }
          env
      else
        let fail =
          if String.equal x y then
            let p = Reason.to_pos r_sub in
            let (pos_super, class_name) = id in
            Subtype_env.fail_with_suffix
              subtype_env
              ~ty_sub:(LoclType ty_sub)
              ~ty_super:(LoclType ty_super)
              (Typing_error.Secondary.This_final
                 { pos_super; class_name; pos_sub = p })
          else
            Subtype_env.fail
              subtype_env
              ~ty_sub:(LoclType ty_sub)
              ~ty_super:(LoclType ty_super)
        in
        invalid env ~fail
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tgeneric _
          | Taccess _ | Tnewtype _ | Tprim _ | Tnonnull | Tclass _
          | Tvec_or_dict _ | Ttuple _ | Tshape _ | Tdynamic | Tneg _ | Tfun _
          | Tvar _ | Tunapplied_alias _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tdependent _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* -- C-Access-R -------------------------------------------------------- *)
    | (_, (_, Taccess _)) -> invalid ~fail env
    (* -- C-Generic-R ------------------------------------------------------- *)
    | ( (r_sub, Tgeneric (name_sub, tyargs_sub)),
        (r_super, Tgeneric (name_super, tyargs_super)) )
      when String.equal name_sub name_super ->
      if List.is_empty tyargs_super then
        valid env
      else
        (* TODO(T69931993) Type parameter env must carry variance information *)
        let variance_reifiedl =
          List.map tyargs_sub ~f:(fun _ -> (Ast_defs.Invariant, Aast.Erased))
        in
        (* Unfortunately, we have to expose this function for proto-HKTs *)
        simplify_subtype_variance_for_non_injective
          ~subtype_env
          ~sub_supportdyn
          ~super_like
          name_sub
          None
          variance_reifiedl
          (r_sub, tyargs_sub)
          (r_super, tyargs_super)
          ty_sub
          ty_super
          env
    | ((_r_sub, Tgeneric (name_sub, _)), (_, Tgeneric _))
      when let (generic_lower_bounds, _other_lower_bounds) =
             generic_lower_bounds env ty_super
           in
           SSet.mem name_sub generic_lower_bounds ->
      valid env
      (* When decomposing subtypes for the purpose of adding bounds on generic
       * parameters to the context, (so seen_generic_params = None), leave
       * subtype so that the bounds get added *)
    | ((_, (Tvar _ | Tunion _)), (_, Tgeneric _)) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    | ( ( _,
          ( Tany _ | Toption _ | Tnonnull | Tdynamic | Tprim _ | Tfun _
          | Ttuple _ | Tshape _ | Tintersection _ | Tunapplied_alias _
          | Tvec_or_dict _ | Taccess _ | Tnewtype _ | Tdependent _ | Tclass _
          | Tneg _ | Tgeneric _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tgeneric _) )
      when subtype_env.Subtype_env.require_completeness ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        (LoclType ty_super)
    | ( ( _r_sub,
          ( Tany _ | Toption _ | Tnonnull | Tdynamic | Tprim _ | Tfun _
          | Ttuple _ | Tshape _ | Tintersection _ | Tunapplied_alias _
          | Tvec_or_dict _ | Taccess _ | Tnewtype _ | Tdependent _ | Tclass _
          | Tneg _ | Tgeneric _ | Tlabel _ | Tclass_args _ ) ),
        (r_super, Tgeneric (name_super, _tyargs_super)) ) ->
      (* If we've seen this type parameter before then we must have gone
       * round a cycle so we fail
       *)
      (match
         VisitedGoals.try_add_visited_generic_super
           subtype_env.Subtype_env.visited
           ty_sub
           name_super
       with
      | None -> invalid env ~fail
      | Some new_visited ->
        let (_generic_lower_bounds, other_lower_bounds) =
          generic_lower_bounds env ty_super
        in
        let subtype_env = Subtype_env.set_visited subtype_env new_visited in

        (* Collect all the lower bounds ("super" constraints) on the
         * generic parameter, and check ty_sub against each of them in turn
         * until one of them succeeds *)
        let rec aux lower_bounds ~env ~errs ~props =
          match lower_bounds with
          | [] ->
            let (env, prop) =
              default_subtype
                ~subtype_env
                ~this_ty
                ~fail
                ~lhs:{ sub_supportdyn; ty_sub }
                ~rhs:{ super_like; super_supportdyn = false; ty_super }
                env
            in
            if TL.is_valid prop then
              (env, prop)
            else
              let (errs, props) =
                match TL.get_error_if_unsat prop with
                | Some err -> (err :: errs, props)
                | _ -> (errs, prop :: props)
              in
              let err = Typing_error.intersect_opt @@ List.filter_opt errs in
              (env, TL.Disj (err, props))
          | lower_bound :: lower_bounds ->
            let lower_bound =
              Typing_env.update_reason env lower_bound ~f:(fun bound ->
                  Typing_reason.axiom_lower_bound ~bound ~of_:r_super)
            in
            let (env, prop) =
              simplify
                ~subtype_env
                ~this_ty
                ~lhs:{ sub_supportdyn; ty_sub }
                ~rhs:
                  {
                    super_like;
                    super_supportdyn = false;
                    ty_super = lower_bound;
                  }
                env
            in
            if TL.is_valid prop then
              (env, prop)
            else
              let (errs, props) =
                match TL.get_error_if_unsat prop with
                | Some err -> (err :: errs, props)
                | _ -> (errs, prop :: props)
              in
              aux lower_bounds ~env ~errs ~props
        in

        (* Turn error into a generic error about the type parameter *)
        let lower_bounds = Typing_set.elements other_lower_bounds in
        aux lower_bounds ~env ~errs:[fail] ~props:[])
    | ( ( _,
          ( Tprim
              Ast_defs.(
                ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum | Tarraykey
                | Tnoreturn ))
          | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tclass _ | Tvec_or_dict _
          | Taccess _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tnonnull) ) ->
      valid env
    (* supportdyn<t> <: nonnull iff t <: nonnull *)
    | ((r, Tnewtype (name, [upper_bound], _)), (r_nonnull, Tnonnull))
      when String.equal name SN.Classes.cSupportDyn ->
      (* TODO(mjt) update to be an asymm projection *)
      let ty_sub =
        Typing_env.update_reason env upper_bound ~f:(fun bound ->
            Typing_reason.axiom_upper_bound ~bound ~of_:r)
      in
      env
      |> simplify
           ~subtype_env
           ~this_ty
           ~lhs:{ sub_supportdyn = Some r; ty_sub }
           ~rhs:
             {
               super_like = false;
               super_supportdyn = false;
               ty_super = mk (r_nonnull, Tnonnull);
             }
    (* negations always contain null *)
    | ((_, Tneg _), (_, Tnonnull)) -> invalid ~fail env
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tdynamic
          | Tprim Aast.(Tvoid | Tnull)
          | Tvar _ | Tgeneric _ | Tnewtype _ | Tunapplied_alias _ | Tdependent _
            ) ),
        (_, Tnonnull) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* -- C-Dynamic-R ------------------------------------------------------- *)
    | (_, (r_dynamic, Tdynamic))
      when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
           && (Subtype_env.coercing_to_dynamic subtype_env
              || Tast.is_under_dynamic_assumptions env.checked) -> begin
      let dyn =
        lazy
          (Pretty.describe_ty_super ~is_coeffect:false env (LoclType ty_super))
      in
      let dynamic_part =
        Lazy.map dyn ~f:(fun dyn ->
            Reason.to_string ("Expected " ^ dyn) r_dynamic)
      and ty_name = lazy (Pretty.describe_ty_default env (LoclType ty_sub))
      and pos = Reason.to_pos (get_reason ty_sub) in
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
      else
        match deref ty_sub with
        | (_, Tlabel _)
        | (_, Tany _)
        | ( _,
            Tprim
              Ast_defs.(
                ( Tint | Tbool | Tfloat | Tstring | Tnum | Tarraykey | Tvoid
                | Tnoreturn | Tresource )) ) ->
          valid env
        | (_, Tnewtype (name_sub, [_tyarg_sub], _))
          when String.equal name_sub SN.Classes.cSupportDyn ->
          valid env
        | (_, Tnewtype (name_sub, _, _))
          when String.equal name_sub SN.Classes.cEnumClassLabel ->
          valid env
        | (r_sub, Toption ty) ->
          (match deref ty with
          (* Special case mixed <: dynamic for better error message *)
          | (_, Tnonnull) -> invalid env ~fail
          | _ ->
            let ty =
              Typing_env.update_reason env ty ~f:(fun r_sub_prj ->
                  Typing_reason.prj_nullable_sub ~sub:r_sub ~sub_prj:r_sub_prj)
            in
            simplify
              ~subtype_env
              ~this_ty:None
              ~lhs:{ sub_supportdyn; ty_sub = ty }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
              env)
        | (_, (Tdynamic | Tprim Ast_defs.Tnull)) -> valid env
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
          default_subtype
            ~subtype_env
            ~this_ty
            ~fail
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ super_like; super_supportdyn = false; ty_super }
            env
        | (_, Tvec_or_dict (_, ty)) ->
          simplify
            ~subtype_env
            ~this_ty:None
            ~lhs:{ sub_supportdyn; ty_sub = ty }
            ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
            env
        | (r_sub, Tfun ft_sub) ->
          if get_ft_support_dynamic_type ft_sub then
            valid env
          else
            (* Special case of function type subtype dynamic.
             *   (function(ty1,...,tyn):ty <: supportdyn<nonnull>)
             *   iff
             *   dynamic <D: ty1 & ... & dynamic <D: tyn & ty <D: dynamic
             *)
            (* Contravariant subtyping on parameters *)
            let param_props =
              simplify_supertype_params_with_variadic
                ~subtype_env
                (r_sub, 0, ft_sub.ft_params)
                (r_dynamic, 0, ty_super)
            in
            (* Finally do covariant subtyping on return type *)
            let ret_prop =
              let ty_sub =
                Typing_env.update_reason env ft_sub.ft_ret ~f:(fun r_sub_prj ->
                    Typing_reason.prj_fn_ret
                      ~sub:r_sub
                      ~sub_prj:r_sub_prj
                      ~super:r_dynamic)
              in
              simplify
                ~subtype_env
                ~this_ty:None
                ~lhs:{ sub_supportdyn; ty_sub }
                ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
            in
            param_props env &&& ret_prop
        | (_, Ttuple { t_required; t_extra = Textra { t_optional; t_variadic } })
          ->
          List.fold_left
            ~init:(env, TL.valid)
            ~f:(fun res ty_sub ->
              res
              &&& simplify
                    ~subtype_env
                    ~this_ty:None
                    ~lhs:{ sub_supportdyn; ty_sub }
                    ~rhs:
                      { super_like = false; super_supportdyn = false; ty_super })
            ((t_variadic :: t_optional) @ t_required)
        (* TODO splats in tuples *)
        | (_, Ttuple { t_extra = Tsplat _; _ }) -> invalid env ~fail
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
              &&& simplify
                    ~subtype_env
                    ~this_ty:None
                    ~lhs:{ sub_supportdyn; ty_sub = sft.sft_ty }
                    ~rhs:
                      { super_like = false; super_supportdyn = false; ty_super })
            (TShapeMap.values sftl)
          &&& simplify
                ~subtype_env
                ~this_ty:None
                ~lhs:{ sub_supportdyn; ty_sub = unknown_fields_type }
                ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        | (_, Tclass_args ty) ->
          (* TODO(T199606542) May need to change this for migration, but
           * conceptually a class pointer for class C should only <D: dynamic
           * if C is <<__SDT>> *)
          simplify
            ~subtype_env
            ~this_ty:None
            ~lhs:{ sub_supportdyn; ty_sub = ty }
            ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
            env
        | (_, Tclass ((_, class_id), _exact, tyargs)) ->
          let class_def_sub = Env.get_class env class_id in
          (match class_def_sub with
          | Decl_entry.DoesNotExist
          | Decl_entry.NotYetAvailable ->
            (* This should have been caught already in the naming phase *)
            valid env
          | Decl_entry.Found class_sub ->
            if
              Cls.get_support_dynamic_type class_sub || Env.is_enum env class_id
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
                  invalid env ~fail
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
                              Phase.localize_no_subst env ~ignore_errors:true ty
                            in
                            Some ty
                          | _ -> None)
                    in
                    let super =
                      MakeType.intersection r_dynamic (ty_super :: upper_bounds)
                    in
                    match tp.tp_variance with
                    | Ast_defs.Covariant ->
                      simplify
                        ~subtype_env
                        ~this_ty:None
                        ~lhs:{ sub_supportdyn = None; ty_sub = tyarg }
                        ~rhs:
                          {
                            super_like = false;
                            super_supportdyn = false;
                            ty_super = super;
                          }
                        env
                    | Ast_defs.Contravariant ->
                      simplify
                        ~subtype_env
                        ~this_ty:None
                        ~lhs:{ sub_supportdyn = None; ty_sub = super }
                        ~rhs:
                          {
                            super_like = false;
                            super_supportdyn = false;
                            ty_super = tyarg;
                          }
                        env
                    | Ast_defs.Invariant ->
                      simplify
                        ~subtype_env
                        ~this_ty:None
                        ~lhs:{ sub_supportdyn = None; ty_sub = tyarg }
                        ~rhs:
                          {
                            super_like = false;
                            super_supportdyn = false;
                            ty_super = super;
                          }
                        env
                      &&& simplify
                            ~subtype_env
                            ~this_ty:None
                            ~lhs:{ sub_supportdyn = None; ty_sub = super }
                            ~rhs:
                              {
                                super_like = false;
                                super_supportdyn = false;
                                ty_super = tyarg;
                              }
                  else
                    (* If the class is marked <<__SupportDynamicType>> then for any
                       * type parameters not marked <<__RequireDynamic>> then the class is a
                       * subtype of dynamic only when the arguments are also subtypes of dynamic.
                    *)
                    match tp.tp_variance with
                    | Ast_defs.Covariant
                    | Ast_defs.Invariant ->
                      simplify
                        ~subtype_env
                        ~this_ty:None
                        ~lhs:{ sub_supportdyn = None; ty_sub = tyarg }
                        ~rhs:
                          {
                            super_like = false;
                            super_supportdyn = false;
                            ty_super;
                          }
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
                        simplify
                          ~subtype_env
                          ~this_ty:None
                          ~lhs:{ sub_supportdyn = None; ty_sub = sub }
                          ~rhs:
                            {
                              super_like = false;
                              super_supportdyn = false;
                              ty_super;
                            }
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
                  simplify
                    ~subtype_env
                    ~this_ty:None
                    ~lhs:{ sub_supportdyn = None; ty_sub = subtype }
                    ~rhs:
                      { super_like = false; super_supportdyn = false; ty_super }
                    env
                | None ->
                  default_subtype
                    ~subtype_env
                    ~this_ty
                    ~fail
                    ~lhs:{ sub_supportdyn; ty_sub }
                    ~rhs:{ super_like; super_supportdyn = false; ty_super }
                    env)
              | _ ->
                default_subtype
                  ~subtype_env
                  ~this_ty
                  ~fail
                  ~lhs:{ sub_supportdyn; ty_sub }
                  ~rhs:{ super_like; super_supportdyn = false; ty_super }
                  env
            ))
    end
    | (_, (_, Tdynamic)) when is_dynamic ty_sub -> valid env
    | (_, (_, Tdynamic)) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
    (* -- C-Prim-R ---------------------------------------------------------- *)
    | ((_, Tprim (Nast.Tint | Nast.Tfloat)), (_, Tprim Nast.Tnum)) -> valid env
    | ((_, Tprim (Nast.Tint | Nast.Tstring)), (_, Tprim Nast.Tarraykey)) ->
      valid env
    | ((_, Tprim prim_sub), (_, Tprim prim_sup))
      when Aast.equal_tprim prim_sub prim_sup ->
      valid env
    | ((_, Tprim _), (_, Tprim _)) -> invalid env ~fail
    | ((r_sub, Toption arg_ty_sub), (_r_super, Tprim Nast.Tnull)) ->
      let arg_ty_sub =
        Typing_env.update_reason env arg_ty_sub ~f:(fun r_sub_prj ->
            Typing_reason.prj_nullable_sub ~sub:r_sub ~sub_prj:r_sub_prj)
      in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = arg_ty_sub }
        ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        env
    | ( ( _,
          ( Tany _ | Tdynamic | Tunion _ | Toption _ | Tintersection _
          | Tdependent _ | Taccess _ | Tgeneric _ | Tnonnull | Tfun _ | Ttuple _
          | Tshape _ | Tvec_or_dict _ | Tclass _ | Tnewtype _ | Tneg _
          | Tunapplied_alias _ | Tvar _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tprim _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* -- C-Any-R ----------------------------------------------------------- *)
    | ((_, Tany _), (_, Tany _)) -> valid env
    | ((_, (Tunion _ | Tintersection _ | Tvar _)), (_, Tany _)) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
    | (_, (_, Tany _)) when subtype_env.Subtype_env.no_top_bottom ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        (LoclType ty_super)
    | (_, (_, Tany _)) -> valid env
    (* -- C-Fun-R ----------------------------------------------------------- *)
    | ((r_sub, Tfun ft_sub), (r_super, Tfun ft_super)) ->
      simplify_funs
        ~subtype_env
        ~check_return:true
        ~for_override:false
        ~super_like
        (r_sub, ft_sub)
        (r_super, ft_super)
        env
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tgeneric _
          | Taccess _ | Tnewtype _ | Tprim _ | Tnonnull | Tclass _
          | Tvec_or_dict _ | Ttuple _ | Tshape _ | Tdynamic | Tneg _
          | Tdependent _ | Tvar _ | Tunapplied_alias _ | Tlabel _
          | Tclass_args _ ) ),
        (_, Tfun _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
      (* -- C-Tuple-R --------------------------------------------------------- *)
    | ( ( _,
          Ttuple
            {
              t_required = t_required_sub;
              t_extra =
                Textra
                  { t_optional = t_optional_sub; t_variadic = t_variadic_sub };
            } ),
        ( _,
          Ttuple
            {
              t_required = t_required_super;
              t_extra =
                Textra
                  {
                    t_optional = t_optional_super;
                    t_variadic = t_variadic_super;
                  };
            } ) ) ->
      (* Subtype should have at least as many required elements as supertype *)
      if List.length t_required_sub < List.length t_required_super then
        invalid env ~fail
      else
        let sub_closed = is_nothing t_variadic_sub in
        let super_closed = is_nothing t_variadic_super in
        (* Shortcut: closed tuples with no optional elements should have the same arity *)
        if
          sub_closed
          && super_closed
          && List.is_empty t_optional_sub
          && List.is_empty t_optional_super
          && not
               (Int.equal
                  (List.length t_required_sub)
                  (List.length t_required_super))
        then
          invalid env ~fail
        else
          let ty_subs = t_required_sub @ t_optional_sub in
          let ty_supers = t_required_super @ t_optional_super in
          let rec simplify_elems ty_subs ty_supers env =
            match (ty_subs, ty_supers) with
            (* We've run out of elements in the supertype so use the variadic *)
            | (ty_sub :: ty_subs, []) ->
              (* Shortcut if variadic supertype is nothing (closed) *)
              if super_closed then
                invalid env ~fail
              else
                let ty_super = Sd.liken ~super_like env t_variadic_super in
                env
                |> simplify
                     ~subtype_env
                     ~this_ty:None
                     ~lhs:{ sub_supportdyn; ty_sub }
                     ~rhs:
                       {
                         super_like = false;
                         super_supportdyn = false;
                         ty_super;
                       }
                &&& simplify_elems ty_subs ty_supers
            (* We've run out of elements in both subtype and the supertype so just compare variadics *)
            | ([], []) ->
              let ty_super = Sd.liken ~super_like env t_variadic_super in
              env
              |> simplify
                   ~subtype_env
                   ~this_ty:None
                   ~lhs:{ sub_supportdyn; ty_sub = t_variadic_sub }
                   ~rhs:
                     { super_like = false; super_supportdyn = false; ty_super }
            (* We have a supertype and no more subtypes so compare variadic subtype *)
            | ([], ty_super :: ty_supers) ->
              let ty_super = Sd.liken ~super_like env ty_super in
              env
              |> simplify
                   ~subtype_env
                   ~this_ty:None
                   ~lhs:{ sub_supportdyn; ty_sub = t_variadic_sub }
                   ~rhs:
                     { super_like = false; super_supportdyn = false; ty_super }
              &&& simplify_elems ty_subs ty_supers
            | (ty_sub :: ty_subs, ty_super :: ty_supers) ->
              let ty_super = Sd.liken ~super_like env ty_super in
              env
              |> simplify
                   ~subtype_env
                   ~this_ty:None
                   ~lhs:{ sub_supportdyn; ty_sub }
                   ~rhs:
                     { super_like = false; super_supportdyn = false; ty_super }
              &&& simplify_elems ty_subs ty_supers
          in
          env |> simplify_elems ty_subs ty_supers
    | (_, (_, Ttuple _)) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
    (* -- C-Shape-R --------------------------------------------------------- *)
    | ((r_sub, Tnewtype (name_sub, _, ty_bound_sub)), (_r_super, Tshape _)) ->
      Common.simplify_newtype_l
        ~subtype_env
        ~this_ty
        ~mk_prop:simplify
        (sub_supportdyn, r_sub, name_sub, ty_bound_sub)
        rhs
        env
    | ( ( r_sub,
          Tshape
            {
              s_origin = origin_sub;
              s_unknown_value = shape_kind_sub;
              s_fields = fdm_sub;
            } ),
        ( r_super,
          Tshape
            {
              s_origin = origin_super;
              s_unknown_value = shape_kind_super;
              s_fields = fdm_super;
            } ) ) ->
      if same_type_origin origin_super origin_sub then
        (* Fast path for shape types: if they have the same origin,
         * they are equal type. *)
        valid env
      else
        let sub_supportdyn = Option.is_some sub_supportdyn in
        simplify_subtype_shape
          ~subtype_env
          ~env
          ~this_ty
          ~super_like
          (sub_supportdyn, r_sub, shape_kind_sub, fdm_sub)
          (super_supportdyn, r_super, shape_kind_super, fdm_super)
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tfun _
          | Tgeneric _ | Taccess _ | Tprim _ | Tnonnull | Tclass _
          | Tvec_or_dict _ | Ttuple _ | Tdynamic | Tneg _ | Tdependent _
          | Tvar _ | Tunapplied_alias _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tshape _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
    (* -- C-Vec-or-Dict-R --------------------------------------------------- *)
    | ( (_, Tvec_or_dict (lty_key_sub, lty_val_sub)),
        (_r_super, Tvec_or_dict (lty_key_sup, lty_val_sup)) ) ->
      let lty_val_sup = Sd.liken ~super_like env lty_val_sup in
      let lty_key_sup = Sd.liken ~super_like env lty_key_sup in
      env
      |> simplify
           ~subtype_env
           ~this_ty
           ~lhs:{ sub_supportdyn; ty_sub = lty_key_sub }
           ~rhs:
             {
               super_like = false;
               super_supportdyn = false;
               ty_super = lty_key_sup;
             }
      &&& simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = lty_val_sub }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = lty_val_sup;
              }
    | ( (_, Tclass ((_, n), _, [lty_key_sub; lty_val_sub])),
        (_r_super, Tvec_or_dict (lty_key_sup, lty_val_sup)) )
      when String.equal n SN.Collections.cDict ->
      let lty_val_sup = Sd.liken ~super_like env lty_val_sup in
      let lty_key_sup = Sd.liken ~super_like env lty_key_sup in
      env
      |> simplify
           ~subtype_env
           ~this_ty
           ~lhs:{ sub_supportdyn; ty_sub = lty_key_sub }
           ~rhs:
             {
               super_like = false;
               super_supportdyn = false;
               ty_super = lty_key_sup;
             }
      &&& simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = lty_val_sub }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = lty_val_sup;
              }
    | ( (_, Tclass ((_, n), _, [lty_val_sub])),
        (_r_super, Tvec_or_dict (lty_key_sup, lty_val_sup)) )
      when String.equal n SN.Collections.cVec ->
      let pos = get_pos ty_sub in
      let lty_key_sub = MakeType.int (Reason.idx_vector_from_decl pos) in
      let lty_val_sup = Sd.liken ~super_like env lty_val_sup in
      let lty_key_sup = Sd.liken ~super_like env lty_key_sup in
      env
      |> simplify
           ~subtype_env
           ~this_ty
           ~lhs:{ sub_supportdyn; ty_sub = lty_key_sub }
           ~rhs:
             {
               super_like = false;
               super_supportdyn = false;
               ty_super = lty_key_sup;
             }
      &&& simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = lty_val_sub }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = lty_val_sup;
              }
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tfun _
          | Tgeneric _ | Taccess _ | Tprim _ | Tnonnull | Tclass _ | Ttuple _
          | Tshape _ | Tnewtype _ | Tdynamic | Tneg _ | Tdependent _ | Tvar _
          | Tunapplied_alias _ | Tlabel _ | Tclass_args _ ) ),
        (_, Tvec_or_dict _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* -- C-Newtype-R ------------------------------------------------------- *)
    (* x <: supportdyn<u> *)
    | (_, (r_supportdyn, Tnewtype (name_super, [lty_inner], bound_super)))
      when String.equal name_super SN.Classes.cSupportDyn -> begin
      match deref ty_sub with
      (* supportdyn<t> <: supportdyn<u> *)
      | (r, Tnewtype (name_sub, [tyarg_sub], _))
        when String.equal name_sub SN.Classes.cSupportDyn ->
        let ty_sub =
          Typing_env.update_reason env tyarg_sub ~f:(fun r_sub_prj ->
              Typing_reason.prj_supportdyn
                ~sub:r
                ~sub_prj:r_sub_prj
                ~super:r_supportdyn)
        and ty_super = lty_inner in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn = Some r; ty_sub }
          ~rhs:{ super_like; super_supportdyn = true; ty_super }
          env
      | (_, Tvar _) ->
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:
            {
              super_like;
              super_supportdyn = false;
              ty_super =
                mk
                  ( r_supportdyn,
                    Tnewtype (SN.Classes.cSupportDyn, [lty_inner], bound_super)
                  );
            }
          env
      | _ ->
        let ty_dyn = MakeType.dynamic r_supportdyn in
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn = true; ty_super = lty_inner }
          env
        &&& simplify
              ~subtype_env:(Subtype_env.set_coercing_to_dynamic subtype_env)
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:
                {
                  super_like = false;
                  super_supportdyn = false;
                  ty_super = ty_dyn;
                }
    end
    (* #A <: \\HH\\EnumClass\\Label<u, v> *)
    | ((r_sub, Tlabel name), (r_sup, Tnewtype (label_kind, [ty_from; ty_to], _)))
      when String.equal label_kind SN.Classes.cEnumClassLabel ->
      let ty =
        Sd.liken ~super_like env
        @@ mk (r_sup, Tnewtype (SN.Classes.cMemberOf, [ty_from; ty_to], ty_to))
      in
      Has_const.(
        simplify
          ~subtype_env
          ~this_ty:(Some ty_from)
          ~lhs:{ sub_supportdyn; ty_sub = ty_from }
          ~rhs:{ reason_super = r_sub; name; ty }
          env)
    | (_, (r_super, Tnewtype (name_super, lty_supers, _bound_super))) -> begin
      match deref ty_sub with
      | (_, Tclass ((_, name_sub), _, _))
        when String.equal name_sub name_super && Env.is_enum env name_super ->
        valid env
      | (r_sub, Tnewtype (name_sub, lty_subs, _))
        when String.equal name_sub name_super ->
        if List.is_empty lty_subs then
          valid env
        else if Env.is_enum env name_super && Env.is_enum env name_sub then
          valid env
        else
          let td = Env.get_typedef env name_super in
          begin
            match td with
            | Decl_entry.Found { td_tparams; _ } ->
              let variance_reifiedl =
                List.map td_tparams ~f:(fun t -> (t.tp_variance, t.tp_reified))
              in
              simplify_subtype_variance_for_non_injective
                ~subtype_env
                ~sub_supportdyn
                ~super_like
                name_sub
                None
                variance_reifiedl
                (r_sub, lty_subs)
                (r_super, lty_supers)
                ty_sub
                ty_super
                env
            | Decl_entry.DoesNotExist
            | Decl_entry.NotYetAvailable ->
              (* TODO(hverr): decl_entry propagate *)
              invalid ~fail env
          end
      (* ?t <: A <=> null <: A && t <: A *)
      | (r, Toption ty_sub) ->
        let r = Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r in
        let ty_null = MakeType.null r in
        (* Errors due to `null` should refer to full option type *)
        let prop_null =
          let prop =
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = ty_null }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          in
          (fun env -> if_unsat (invalid ~fail) @@ prop env)
        in
        let ty_sub =
          Typing_env.update_reason env ty_sub ~f:(fun r_sub_prj ->
              Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r_sub_prj)
        in
        prop_null env
        &&& simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
      | (r, Tprim Aast.Tarraykey) ->
        let r = Typing_reason.prj_arraykey_sub ~sub:r ~sub_prj:r in
        let ty_string = MakeType.string r and ty_int = MakeType.int r in
        let prop env =
          env
          |> simplify
               ~subtype_env
               ~this_ty
               ~lhs:{ sub_supportdyn; ty_sub = ty_string }
               ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          &&& simplify
                ~subtype_env
                ~this_ty
                ~lhs:{ sub_supportdyn; ty_sub = ty_int }
                ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        in
        (* Use `if_unsat` so we report arraykey in the error *)
        if_unsat (invalid ~fail) @@ prop env
      | (r, Tprim Aast.Tnum) ->
        let r = Typing_reason.prj_num_sub ~sub:r ~sub_prj:r in
        let ty_float = MakeType.float r and ty_int = MakeType.int r in
        let prop env =
          env
          |> simplify
               ~subtype_env
               ~this_ty
               ~lhs:{ sub_supportdyn; ty_sub = ty_float }
               ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          &&& simplify
                ~subtype_env
                ~this_ty
                ~lhs:{ sub_supportdyn; ty_sub = ty_int }
                ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        in
        (* Use `if_unsat` so we report num in the error *)
        if_unsat (invalid ~fail) @@ prop env
      | (_, Tgeneric _) when subtype_env.Subtype_env.require_completeness ->
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn = false; ty_super }
          env
      | ( _,
          ( Tany _ | Tdynamic | Tunion _ | Tintersection _ | Tdependent _
          | Taccess _ | Tgeneric _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _
          | Tvec_or_dict _ | Tclass _ | Tnewtype _ | Tneg _
          | Tprim
              Aast.(
                ( Tnull | Tvoid | Tint | Tbool | Tfloat | Tstring | Tresource
                | Tnoreturn ))
          | Tunapplied_alias _ | Tvar _ | Tlabel _ | Tclass_args _ ) ) -> begin
        match
          Subtype_env.check_infinite_recursion
            subtype_env
            { Subtype_recursion_tracker.Subtype_op.ty_sub; ty_super }
        with
        | Error _ -> invalid env ~fail
        | Ok subtype_env ->
          let localize td_tparams hint env =
            let ((env, _err), ty) =
              let ety_env =
                (* The this_ty cannot does not need to be set because newtypes
                   * & case types cannot appear within classes thus cannot us
                   * the this type. If we ever change that this could needs to
                   * be changed *)
                {
                  empty_expand_env with
                  substs =
                    (if List.is_empty lty_supers then
                      SMap.empty
                    else
                      Decl_subst.make_locl td_tparams lty_supers);
                }
              in
              Phase.localize ~ety_env env hint
            in
            (env, ty)
          in
          let simplify_ ty_sub ty_super env =
            simplify
              ~subtype_env
              ~this_ty:None
              ~lhs:{ sub_supportdyn = None; ty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
              env
          in
          (* x <: case_type *)
          (match Env.get_typedef env name_super with
          | Decl_entry.Found
              {
                td_type_assignment = CaseType (variant, variants);
                td_tparams;
                _;
              } ->
            let check_where_constraint (left, ck, right) env :
                Typing_env_types.env * TL.subtype_prop =
              let (env, local_left) = localize td_tparams left env in
              let (env, local_right) = localize td_tparams right env in
              match ck with
              | Ast_defs.Constraint_as -> simplify_ local_left local_right env
              | Ast_defs.Constraint_super ->
                simplify_ local_right local_left env
              | Ast_defs.Constraint_eq ->
                simplify_ local_left local_right env
                &&& simplify_ local_right local_left
            in
            (* For
             * CT<T1, ... Tn> =
             * | U1 where [constraints_1] | ... | Um where [constraints_m]
             * Then T < CT<t1, ... tn> iff
             * for any k in [1..m]:
             * T < Vk[t1/T1, ...] &&& constraints_k[t1/T1, ...]
             *)
            let ( ||| ) = ( ||| ) ~fail in
            let try_variant (hint, wcs) env =
              let (env, hint_ty) = localize td_tparams hint env in
              List.fold_left
                wcs
                ~init:(simplify_ ty_sub hint_ty env)
                ~f:(fun acc wc -> acc &&& check_where_constraint wc)
            in
            List.fold_left
              (variant :: variants)
              ~init:
                (default_subtype
                   ~subtype_env
                   ~this_ty
                   ~fail
                   ~lhs:{ sub_supportdyn; ty_sub }
                   ~rhs:{ super_like; super_supportdyn = false; ty_super }
                   env)
              ~f:(fun acc variant -> acc ||| try_variant variant)
          | Decl_entry.Found
              {
                td_super_constraint = Some lower;
                td_tparams;
                td_type_assignment = SimpleTypeDef _;
                _;
              } ->
            let ( ||| ) = ( ||| ) ~fail in
            default_subtype
              ~subtype_env
              ~this_ty
              ~fail
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
              env
            ||| fun env ->
            let (env, lower_ty) = localize td_tparams lower env in
            simplify_ ty_sub lower_ty env
          | _ ->
            default_subtype
              ~subtype_env
              ~this_ty
              ~fail
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
              env)
      end
    end
    (* -- C-unapplied-alias-R --------------------------------------------------- *)
    | (_, (_, Tunapplied_alias n_sup)) ->
      (match deref ty_sub with
      | (_, Tunapplied_alias n_sub) when String.equal n_sub n_sup -> valid env
      | _ ->
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_supportdyn; super_like; ty_super }
          env)
    (* -- C-Neg-R ----------------------------------------------------------- *)
    | (_, (r_super, Tneg (r_pred, IsTag (ClassTag c_super)))) -> begin
      match deref ty_sub with
      | (_, Tneg (_, IsTag (ClassTag c_sub))) ->
        if TUtils.is_sub_class_refl env c_super c_sub then
          valid env
        else
          invalid ~fail env
      | (_, Tclass ((_, c_sub), _, _)) ->
        if Subtype_negation.is_class_disjoint env c_sub c_super then
          valid env
        else
          invalid ~fail env
      (* Functions can be instances of the Closure class *)
      | (_, Tfun _) when String.equal SN.Classes.cClosure c_super ->
        invalid ~fail env
      (* All of these are definitely disjoint from class types *)
      | (_, (Tfun _ | Ttuple _ | Tshape _ | Tprim _)) -> valid env
      | _ ->
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:
            {
              super_like;
              super_supportdyn = false;
              ty_super = mk (r_super, Tneg (r_pred, IsTag (ClassTag c_super)));
            }
          env
    end
    | (_, (reason_super, Tneg predicate)) -> begin
      match deref ty_sub with
      | (_, Tvar _) ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (LoclType ty_super)
      | (r_sub, Tunion ty_subs) ->
        Common.simplify_union_l
          ~subtype_env
          ~this_ty
          ~mk_prop:simplify
          ~update_reason:
            (Typing_env.update_reason ~f:(fun r_sub_prj ->
                 Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, ty_subs)
          rhs
          env
      | _ ->
        (* T < !P iff left < nothing and span & P < nothing
           However, just do span < nothing since we know that
           if span is not nothing, it isn't known to be disjoint from P.
           This also avoids the need to do special handling of dynamic
           in the union case above to avoid dynamic & P *)
        let (env, partition) =
          Typing_refinement.partition_ty env ty_sub predicate
        in
        let simplify_subtype tyl ty_super env =
          Subtype.(
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:
                {
                  sub_supportdyn;
                  ty_sub = MakeType.intersection reason_super tyl;
                }
              ~rhs:{ super_supportdyn = false; super_like; ty_super }
              env)
        in
        let nothing = MakeType.nothing reason_super in
        List.fold_left
          (partition.Typing_refinement.left @ partition.Typing_refinement.span)
          ~init:(env, TL.valid)
          ~f:(fun res tyl -> res &&& simplify_subtype tyl nothing)
    end
    (* -- C-Class-R --------------------------------------------------------- *)
    (* class refinement
       x <: A with type T = y <=> x <: A && x <: has_type_member(T, y) *)
    | (_, (r_super, Tclass (class_id_super, Nonexact cr_super, tyargs_super)))
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
              TUtils.is_class ty_sub) -> begin
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
          let htm = { htm_id = type_id; htm_lower; htm_upper } in
          let lhs = { sub_supportdyn; ty_sub }
          and rhs =
            Has_type_member.{ reason_super = r_super; has_type_member = htm }
          in
          let subtype_env =
            Subtype_env.possibly_add_violated_constraint
              subtype_env
              ~r_sub:(get_reason ty_sub)
              ~r_super
          in
          Has_type_member.simplify ~subtype_env ~this_ty ~lhs ~rhs)
      &&&
      (* then recursively check the class with all the
         refinements dropped. *)
      let ty_super =
        mk (r_super, Tclass (class_id_super, nonexact, tyargs_super))
      in

      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
    end
    | ( _,
        ( _r_super,
          Tclass ((_pos_super, class_nm_super), exact_super, tyargs_super) ) )
      -> begin
      match deref ty_sub with
      (* Enums... *)
      | (_, Tnewtype (enum_name, _, _))
        when String.equal enum_name class_nm_super
             && is_nonexact exact_super
             && Env.is_enum env enum_name ->
        valid env
      (* E <: \HH\BuiltinEnum<t> *)
      | (_, Tnewtype (cid, _, _))
        when String.equal class_nm_super SN.Classes.cHH_BuiltinEnum
             && Env.is_enum env cid ->
        (match tyargs_super with
        | [lty_super'] ->
          env
          |> simplify
               ~subtype_env
               ~this_ty
               ~lhs:{ sub_supportdyn; ty_sub }
               ~rhs:
                 {
                   super_like = false;
                   super_supportdyn = false;
                   ty_super = lty_super';
                 }
        | _ ->
          default_subtype
            ~subtype_env
            ~this_ty
            ~fail
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ super_like; super_supportdyn = false; ty_super }
            env)
      | (_, Tnewtype (enum_name, _, _))
        when String.equal enum_name class_nm_super && Env.is_enum env enum_name
        ->
        valid env
      | (_, Tnewtype (enum_name, _, _))
        when Env.is_enum env enum_name
             && String.equal class_nm_super SN.Classes.cXHPChild ->
        valid env
      (* t_prim <: XHPChild *)
      | (_, Tprim Nast.(Tstring | Tarraykey | Tint | Tfloat | Tnum))
        when String.equal class_nm_super SN.Classes.cXHPChild
             && is_nonexact exact_super ->
        valid env
      (* string <: Stringish *)
      | (_, Tprim Nast.Tstring)
        when String.equal class_nm_super SN.Classes.cStringish
             && is_nonexact exact_super ->
        valid env
      (* Match what's done in unify for non-strict code *)
      | (r_sub, Tclass (class_id_sub, exact_sub, tyl_sub)) ->
        (match deref ty_super with
        | (r_super, Tclass (class_id_super, exact_super, tyl_super)) ->
          simplify_subtype_classes
            ~fail
            ~subtype_env
            ~sub_supportdyn
            ~this_ty
            ~super_like
            (r_sub, (class_id_sub, exact_sub, tyl_sub))
            (r_super, (class_id_super, exact_super, tyl_super))
            env
        | _ -> invalid env ~fail)
      | (_r_sub, Tvec_or_dict (_, tv)) ->
        (match (exact_super, tyargs_super) with
        | (Nonexact _, [tv_super])
          when String.equal class_nm_super SN.Collections.cTraversable
               || String.equal class_nm_super SN.Collections.cContainer ->
          (* vec<tv> <: Traversable<tv_super>
           * iff tv <: tv_super
           * Likewise for vec<tv> <: Container<tv_super>
           *          and map<_,tv> <: Traversable<tv_super>
           *          and map<_,tv> <: Container<tv_super>
           *)
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = tv }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = tv_super;
              }
            env
        | (Nonexact _, [tk_super; tv_super])
          when String.equal class_nm_super SN.Collections.cKeyedTraversable
               || String.equal class_nm_super SN.Collections.cKeyedContainer
               || String.equal class_nm_super SN.Collections.cAnyArray ->
          (match get_node ty_sub with
          | Tvec_or_dict (tk, _) ->
            env
            |> simplify
                 ~subtype_env
                 ~this_ty
                 ~lhs:{ sub_supportdyn; ty_sub = tk }
                 ~rhs:
                   {
                     super_like = false;
                     super_supportdyn = false;
                     ty_super = tk_super;
                   }
            &&& simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub = tv }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = tv_super;
                    }
          | _ ->
            default_subtype
              ~subtype_env
              ~this_ty
              ~fail
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:{ super_like; super_supportdyn = false; ty_super }
              env)
        | (Nonexact _, [])
          when String.equal class_nm_super SN.Collections.cKeyedTraversable
               || String.equal class_nm_super SN.Collections.cKeyedContainer
               || String.equal class_nm_super SN.Collections.cAnyArray ->
          (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
          valid env
        | (_, _) ->
          default_subtype
            ~subtype_env
            ~this_ty
            ~fail
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ super_like; super_supportdyn = false; ty_super }
            env)
      | _ ->
        default_subtype
          ~subtype_env
          ~this_ty
          ~fail
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like; super_supportdyn = false; ty_super }
          env
      (* -- C-Label-R ---------------------------------------------------------- *)
    end
    | ((_, Tlabel sub_name), (_, Tlabel sup_name)) ->
      if String.equal sub_name sup_name then
        valid env
      else
        invalid ~fail env
    | ( ( _,
          ( Tany _ | Tdynamic | Tunion _ | Toption _ | Tintersection _
          | Tdependent _ | Taccess _ | Tgeneric _ | Tnonnull | Tfun _ | Ttuple _
          | Tshape _ | Tvec_or_dict _ | Tclass _ | Tnewtype _ | Tneg _ | Tprim _
          | Tunapplied_alias _ | Tvar _ | Tclass_args _ ) ),
        (_, Tlabel _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn = false; ty_super }
        env
    (* -- C-ClassArgs-R ----------------------------------------------------- *)
    | ((_r_sub, Tclass_args ty_sub), (_r_super, Tclass_args ty_super)) ->
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_like; super_supportdyn; ty_super }
        env
    | ( ( _,
          ( Tany _ | Tunion _ | Toption _ | Tintersection _ | Tgeneric _
          | Taccess _ | Tnewtype _ | Tprim _ | Tnonnull | Tclass _
          | Tvec_or_dict _ | Ttuple _ | Tshape _ | Tdynamic | Tneg _
          | Tdependent _ | Tvar _ | Tunapplied_alias _ | Tlabel _ | Tfun _ ) ),
        (_, Tclass_args _) ) ->
      default_subtype
        ~subtype_env
        ~this_ty
        ~fail
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:{ super_supportdyn; super_like; ty_super }
        env
end

(* -- Non-subtype constraints ----------------------------------------------- *)
and Destructure : sig
  type rhs = {
    reason_super: Reason.t;
    destructure: destructure;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    destructure: destructure;
  }

  let destructure_array
      ~subtype_env
      ~this_ty
      (sub_supportdyn, ty_sub_inner)
      {
        reason_super = r_super;
        destructure = { d_kind; d_required; d_optional; d_variadic };
      }
      env =
    (* If this is a splat, there must be a variadic box to receive the elements
     * but for list(...) destructuring this is not required. Example:
     *
     * function f(int $i): void {}
     * function g(vec<int> $v): void {
     *   list($a) = $v; // ok (but may throw)
     *   f(...$v); // error
     * } *)
    let fpos =
      match Reason.Predicates.unpack_unpack_param_opt r_super with
      | Some (_, fpos, _) -> fpos
      | None -> Reason.to_pos r_super
    in
    match (d_kind, d_required, d_variadic) with
    | (SplatUnpack, _ :: _, _) ->
      (* return the env so as not to discard the type variable that might
         have been created for the Traversable type created below. *)
      invalid
        env
        ~fail:
          (Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
               Typing_error.(
                 apply_reasons ~on_error
                 @@ Secondary.Unpack_array_required_argument
                      { pos = Reason.to_pos r_super; decl_pos = fpos })))
    | (SplatUnpack, [], None) ->
      invalid
        env
        ~fail:
          (Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
               Typing_error.(
                 apply_reasons ~on_error
                 @@ Secondary.Unpack_array_variadic_argument
                      { pos = Reason.to_pos r_super; decl_pos = fpos })))
    | (SplatUnpack, [], Some _)
    | (ListDestructure, _, _) ->
      List.fold d_required ~init:(env, TL.valid) ~f:(fun res ty_dest ->
          res
          &&& Subtype.(
                simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub = ty_sub_inner }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = ty_dest;
                    }))
      &&& fun env ->
      List.fold d_optional ~init:(env, TL.valid) ~f:(fun res ty_dest ->
          res
          &&& Subtype.(
                simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub = ty_sub_inner }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = ty_dest;
                    }))
      &&& fun env ->
      Option.value_map ~default:(env, TL.valid) d_variadic ~f:(fun vty ->
          Subtype.(
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub = ty_sub_inner }
              ~rhs:
                { super_like = false; super_supportdyn = false; ty_super = vty }
              env))

  let destructure_dynamic
      ~subtype_env
      ~this_ty
      (sub_supportdyn, ty_sub)
      ({ destructure = { d_required; d_optional; d_variadic; _ }; _ } as rhs)
      env =
    if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
      List.fold d_required ~init:(env, TL.valid) ~f:(fun res ty_dest ->
          res
          &&& Subtype.(
                simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = ty_dest;
                    }))
      &&& fun env ->
      List.fold d_optional ~init:(env, TL.valid) ~f:(fun res ty_dest ->
          res
          &&& Subtype.(
                simplify
                  ~subtype_env
                  ~this_ty
                  ~lhs:{ sub_supportdyn; ty_sub }
                  ~rhs:
                    {
                      super_like = false;
                      super_supportdyn = false;
                      ty_super = ty_dest;
                    }))
      &&& fun env ->
      Option.value_map ~default:(env, TL.valid) d_variadic ~f:(fun vty ->
          Subtype.(
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:
                { super_like = false; super_supportdyn = false; ty_super = vty }
              env))
    else
      destructure_array ~subtype_env ~this_ty (sub_supportdyn, ty_sub) rhs env

  let destructure_tuple
      ~subtype_env
      ~this_ty
      (sub_supportdyn, reason_tuple, ty_subs)
      {
        reason_super = r_super;
        destructure = { d_required; d_optional; d_variadic; _ };
      }
      env =
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
    let len_ts = List.length ty_subs in
    let len_required = List.length d_required in
    let arity_error f =
      let (epos, fpos, prefix) =
        match Reason.Predicates.unpack_unpack_param_opt r_super with
        | Some (epos, fpos, c) -> (Pos_or_decl.of_raw_pos epos, fpos, c)
        | None -> (Reason.to_pos r_super, Reason.to_pos reason_tuple, 0)
      in
      invalid
        env
        ~fail:
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
      let (ts_required, remain) = List.split_n ty_subs len_required in
      let (ts_optional, ts_variadic) = List.split_n remain len_optional in
      let (res, n) =
        List.fold2_exn
          ts_required
          d_required
          ~init:((env, TL.valid), 0)
          ~f:(fun (res, n) ty_from ty_into ->
            let ty_from =
              Typing_env.update_reason env ty_from ~f:(fun sub_prj ->
                  Typing_reason.(
                    prj_tuple ~sub:reason_tuple ~sub_prj ~super:r_super n))
            in
            ( (res
              &&& Subtype.(
                    simplify
                      ~subtype_env
                      ~this_ty
                      ~lhs:{ sub_supportdyn; ty_sub = ty_from }
                      ~rhs:
                        {
                          super_like = false;
                          super_supportdyn = false;
                          ty_super = ty_into;
                        })),
              n + 1 ))
      in

      res &&& fun env ->
      let len_ts_opt = List.length ts_optional in
      let d_optional_part =
        if len_ts_opt < len_optional then
          List.take d_optional len_ts_opt
        else
          d_optional
      in
      let (res, n) =
        List.fold2_exn
          ts_optional
          d_optional_part
          ~init:((env, TL.valid), n)
          ~f:(fun (res, n) ty_from ty_into ->
            let ty_from =
              Typing_env.update_reason env ty_from ~f:(fun sub_prj ->
                  Typing_reason.(
                    prj_tuple ~sub:reason_tuple ~sub_prj ~super:r_super n))
            in
            ( (res
              &&& Subtype.(
                    simplify
                      ~subtype_env
                      ~this_ty
                      ~lhs:{ sub_supportdyn; ty_sub = ty_from }
                      ~rhs:
                        {
                          super_like = false;
                          super_supportdyn = false;
                          ty_super = ty_into;
                        })),
              n + 1 ))
      in

      res &&& fun env ->
      match (ts_variadic, d_variadic) with
      | (vars, Some ty_into) ->
        List.fold vars ~init:(env, TL.valid) ~f:(fun res ty_from ->
            res
            &&& Subtype.(
                  let ty_from =
                    Typing_env.update_reason env ty_from ~f:(fun sub_prj ->
                        Typing_reason.(
                          prj_tuple
                            ~sub:reason_tuple
                            ~sub_prj
                            ~super:r_super
                            (* TODO(mjt) allow different indices here *)
                            (n + 1)))
                  in
                  simplify
                    ~subtype_env
                    ~this_ty
                    ~lhs:{ sub_supportdyn; ty_sub = ty_from }
                    ~rhs:
                      {
                        super_like = false;
                        super_supportdyn = false;
                        ty_super = ty_into;
                      }))
      | ([], None) -> valid env
      | (_, None) ->
        (* Elements remain but we have nowhere to put them *)
        arity_error (fun expected actual pos decl_pos on_error_opt ->
            Option.map on_error_opt ~f:(fun on_error ->
                Typing_error.(
                  apply_reasons ~on_error
                  @@ Secondary.Typing_too_many_args
                       { pos; decl_pos; expected; actual })))

  let rec simplify
      ~subtype_env
      ~this_ty
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:({ reason_super = r_super; destructure } as rhs)
      env =
    begin
      let (env, ty_sub) = Env.expand_type env ty_sub in
      let subtype_env =
        Subtype_env.possibly_add_violated_constraint
          subtype_env
          ~r_sub:(get_reason ty_sub)
          ~r_super
      in
      let fail =
        Subtype_env.fail
          subtype_env
          ~ty_sub:(LoclType ty_sub)
          ~ty_super:
            (ConstraintType
               (mk_constraint_type (r_super, Tdestructure destructure)))
      in
      match (deref ty_sub, destructure.d_kind) with
      (* TODO optional and variadic components T201398626 T201398652 *)
      | ((r, Ttuple { t_required; _ }), _) ->
        destructure_tuple
          ~subtype_env
          ~this_ty
          (sub_supportdyn, r, t_required)
          rhs
          env
      | ((r, Tclass ((_, x), _, tyl)), _)
        when String.equal x SN.Collections.cPair ->
        destructure_tuple ~subtype_env ~this_ty (sub_supportdyn, r, tyl) rhs env
      | ((_, Tclass ((_, x), _, [elt_type])), _)
        when String.equal x SN.Collections.cVector
             || String.equal x SN.Collections.cImmVector
             || String.equal x SN.Collections.cVec
             || String.equal x SN.Collections.cConstVector ->
        destructure_array
          ~subtype_env
          ~this_ty
          (sub_supportdyn, elt_type)
          rhs
          env
      | ((_, Tdynamic), _) ->
        destructure_dynamic
          ~subtype_env
          ~this_ty
          (sub_supportdyn, ty_sub)
          rhs
          env
      | ((_, Tvar _), _) ->
        mk_issubtype_prop
          ~sub_supportdyn
          ~coerce:subtype_env.Subtype_env.coerce
          env
          (LoclType ty_sub)
          (ConstraintType
             (mk_constraint_type (r_super, Tdestructure destructure)))
      | ((r_sub, Tunion ty_subs), _) ->
        Common.simplify_union_l
          ~subtype_env
          ~this_ty
          ~mk_prop:simplify
          ~update_reason:
            (Typing_env.update_reason ~f:(fun r_sub_prj ->
                 Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, ty_subs)
          rhs
          env
      | ((r_sub, Tintersection ty_subs), _) ->
        (* A & B <: C iif A <: C | !B *)
        (match Subtype_negation.find_type_with_exact_negation env ty_subs with
        | (env, Some non_ty, tyl) ->
          let ty_sub = MakeType.intersection r_sub tyl in
          let mk_prop = simplify
          and lift_rhs { reason_super; destructure } =
            mk_constraint_type (reason_super, Tdestructure destructure)
          and lhs = (sub_supportdyn, ty_sub)
          and rhs_subtype =
            Subtype.
              {
                super_supportdyn = false;
                super_like = false;
                ty_super = non_ty;
              }
          and rhs_destructure = { reason_super = r_super; destructure } in
          let rhs = (r_super, rhs_subtype, rhs_destructure) in
          Common.simplify_disj_r
            ~subtype_env
            ~this_ty
            ~fail
            ~lift_rhs
            ~mk_prop
            lhs
            rhs
            env
        | _ ->
          Common.simplify_intersection_l
            ~subtype_env
            ~this_ty
            ~fail
            ~mk_prop:simplify
            ~update_reason:
              (Typing_env.update_reason ~f:(fun r_sub_prj ->
                   Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
            (sub_supportdyn, ty_subs)
            rhs
            env)
      | ((r_generic, Tgeneric (generic_nm, generic_ty_args)), _) ->
        Common.simplify_generic_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop:simplify
          (sub_supportdyn, r_generic, generic_nm, generic_ty_args)
          rhs
          rhs
          env
      | (_, SplatUnpack) ->
        (* Allow splatting of arbitrary Traversables *)
        let (env, ty_inner) = Env.fresh_type env Pos.none in
        let traversable = MakeType.traversable r_super ty_inner in
        env
        |> Subtype.(
             simplify
               ~subtype_env
               ~this_ty
               ~lhs:{ sub_supportdyn; ty_sub }
               ~rhs:
                 {
                   super_like = false;
                   super_supportdyn = false;
                   ty_super = traversable;
                 })
        &&& destructure_array ~subtype_env ~this_ty (None, ty_inner) rhs
      | ((r_newtype, Tnewtype (nm, _, ty_newtype)), ListDestructure) ->
        Common.simplify_newtype_l
          ~subtype_env
          ~this_ty
          ~mk_prop:simplify
          (sub_supportdyn, r_newtype, nm, ty_newtype)
          rhs
          env
      | ((r_dep, Tdependent (dep_ty, ty_inner_sub)), ListDestructure) ->
        Common.simplify_dependent_l
          ~subtype_env
          ~this_ty
          ~mk_prop:simplify
          (sub_supportdyn, r_dep, dep_ty, ty_inner_sub)
          rhs
          env
      | ( ( _,
            ( Tany _ | Tnonnull | Toption _ | Tprim _ | Tfun _ | Tshape _
            | Tvec_or_dict _ | Taccess _ | Tclass _ | Tneg _ | Tlabel _
            | Tunapplied_alias _ | Tclass_args _ ) ),
          ListDestructure ) ->
        let ty_sub_descr =
          lazy (Typing_print.full_strip_ns ~hide_internals:true env ty_sub)
        in
        invalid
          env
          ~fail:
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
    end
end

and Can_index : sig
  type rhs = {
    reason_super: Reason.t;
    can_index: can_index;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    can_index: can_index;
  }

  let simplify
      ~subtype_env
      ~this_ty:_
      ~lhs:{ ty_sub; _ }
      ~rhs:{ reason_super; can_index }
      env =
    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(get_reason ty_sub)
        ~r_super:reason_super
    in
    let fail =
      Subtype_env.fail
        subtype_env
        ~ty_sub:(LoclType ty_sub)
        ~ty_super:
          (ConstraintType
             (mk_constraint_type (reason_super, Tcan_index can_index)))
    in
    invalid env ~fail
end

and Can_traverse : sig
  type rhs = {
    reason_super: Reason.t;
    can_traverse: can_traverse;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    can_traverse: can_traverse;
  }

  let subtype_with_dynamic ~subtype_env ~this_ty ~lhs { ct_key; ct_val; _ } env
      =
    let subty_prop_val env =
      Subtype.(
        simplify
          ~subtype_env
          ~this_ty
          ~lhs
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = ct_val }
          env)
    and subty_prop_key env =
      match ct_key with
      | None -> valid env
      | Some ct_key ->
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = ct_key;
              }
            env)
    in
    subty_prop_val env &&& subty_prop_key

  let rec simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn = _; ty_sub } = lhs in
    let { reason_super; can_traverse } = rhs in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let ty_sub = LoclType ty_sub in
    let ty_super =
      ConstraintType
        (mk_constraint_type (reason_super, Tcan_traverse can_traverse))
    in
    if Logging.should_log_subtype_i env ~level:2 ty_sub ty_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:"Typing_subtype.Can_traverse.simplify"
        env
        ty_sub
        ty_super
      @@ fun () -> simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_
      ~subtype_env
      ~this_ty
      ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
      ~rhs:({ reason_super = r; can_traverse = ct } as rhs)
      env =
    let (env, lty_sub) = Env.expand_type env lty_sub in
    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(get_reason lty_sub)
        ~r_super:r
    in
    let fail =
      Subtype_env.fail
        subtype_env
        ~ty_sub:(LoclType lty_sub)
        ~ty_super:(ConstraintType (mk_constraint_type (r, Tcan_traverse ct)))
    in
    if TUtils.is_tyvar_error env lty_sub then
      let trav_ty = can_traverse_to_iface ct in
      Subtype.(
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ~rhs:
            { super_like = false; super_supportdyn = false; ty_super = trav_ty }
          env)
    else
      let mk_prop ~subtype_env ~this_ty ~lhs ~rhs =
        simplify ~subtype_env ~this_ty ~lhs ~rhs
      in

      match deref lty_sub with
      | (_, Tdynamic) when Subtype_env.coercing_from_dynamic subtype_env ->
        valid env
      | (_, Tdynamic) ->
        subtype_with_dynamic
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ct
          env
      | _
        when Option.is_some sub_supportdyn
             && TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
             && Tast.is_under_dynamic_assumptions env.checked ->
        subtype_with_dynamic
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
          ct
          env
      | (_, Tclass _)
      | (_, Tvec_or_dict _)
      | (_, Tany _) ->
        let trav_ty = can_traverse_to_iface ct in
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = lty_sub }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = trav_ty;
              }
            env)
      | (r_sub, Tunion ty_subs) ->
        Common.simplify_union_l
          ~subtype_env
          ~this_ty
          ~mk_prop
          ~update_reason:
            (Typing_env.update_reason ~f:(fun r_sub_prj ->
                 Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, ty_subs)
          rhs
          env
      | (_, Tvar id) ->
        (* If the type is already in the upper bounds of the type variable,
           * then we already know that this subtype assertion is valid
        *)
        let cty = ConstraintType (mk_constraint_type (r, Tcan_traverse ct)) in
        if ITySet.mem cty (Env.get_tyvar_upper_bounds env id) then
          valid env
        else
          mk_issubtype_prop
            ~sub_supportdyn
            ~coerce:subtype_env.Subtype_env.coerce
            env
            (LoclType lty_sub)
            (ConstraintType (mk_constraint_type (r, Tcan_traverse ct)))
      | (r_sub, Tintersection ty_subs) ->
        (* A & B <: C iif A <: C | !B *)
        (match Subtype_negation.find_type_with_exact_negation env ty_subs with
        | (env, Some non_ty, tyl) ->
          let ty_sub = MakeType.intersection r_sub tyl in

          let mk_prop = simplify
          and lift_rhs { reason_super; can_traverse } =
            mk_constraint_type (reason_super, Tcan_traverse can_traverse)
          and lhs = (sub_supportdyn, ty_sub)
          and rhs_subtype =
            Subtype.
              {
                super_supportdyn = false;
                super_like = false;
                ty_super = non_ty;
              }
          and rhs_destructure = { reason_super = r; can_traverse = ct } in
          let rhs = (r, rhs_subtype, rhs_destructure) in
          Common.simplify_disj_r
            ~subtype_env
            ~this_ty
            ~fail
            ~lift_rhs
            ~mk_prop
            lhs
            rhs
            env
        | _ ->
          Common.simplify_intersection_l
            ~subtype_env
            ~this_ty
            ~fail
            ~mk_prop
            ~update_reason:
              (Typing_env.update_reason ~f:(fun r_sub_prj ->
                   Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
            (sub_supportdyn, ty_subs)
            rhs
            env)
      | (r_sub, Tgeneric (generic_nm, generic_ty_args)) ->
        Common.simplify_generic_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop
          (sub_supportdyn, r_sub, generic_nm, generic_ty_args)
          rhs
          rhs
          env
      | (r_sub, Tnewtype (alias_name, _, ty_newtype)) ->
        Common.simplify_newtype_l
          ~subtype_env
          ~this_ty
          ~mk_prop
          (sub_supportdyn, r_sub, alias_name, ty_newtype)
          rhs
          env
      | (r_sub, Tdependent (dep_ty, ty_inner)) ->
        Common.simplify_dependent_l
          ~subtype_env
          ~this_ty
          ~mk_prop
          (sub_supportdyn, r_sub, dep_ty, ty_inner)
          rhs
          env
      | (_, Toption _)
      | (_, Tprim _)
      | (_, Tnonnull)
      | (_, Tneg _)
      | (_, Tfun _)
      | (_, Ttuple _)
      | (_, Tshape _)
      | (_, Taccess _)
      | (_, Tlabel _)
      | (_, Tunapplied_alias _) ->
        invalid ~fail env
      | (_, Tclass_args _) ->
        (* TODO(T199606542) confirm CanTraverse behavior for class pointers *)
        invalid ~fail env
end

and Has_type_member : sig
  type rhs = {
    reason_super: Reason.t;
    has_type_member: has_type_member;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    has_type_member: has_type_member;
  }

  let rec simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn = _; ty_sub } = lhs in
    let { reason_super; has_type_member } = rhs in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let ty_sub = LoclType ty_sub in
    let ty_super =
      ConstraintType
        (mk_constraint_type (reason_super, Thas_type_member has_type_member))
    in
    if Logging.should_log_subtype_i env ~level:2 ty_sub ty_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:"Typing_subtype.Has_type_member.simplify"
        env
        ty_sub
        ty_super
      @@ fun () -> simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_
      ~subtype_env
      ~this_ty
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:({ reason_super = r; has_type_member = htm } as rhs)
      env =
    let { htm_id = memid; htm_lower = memloty; htm_upper = memupty } = htm in
    let htmty = ConstraintType (mk_constraint_type (r, Thas_type_member htm)) in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(get_reason ty_sub)
        ~r_super:r
    in
    let fail =
      Subtype_env.fail subtype_env ~ty_sub:(LoclType ty_sub) ~ty_super:htmty
    in

    let ity_super =
      ConstraintType (mk_constraint_type (r, Thas_type_member htm))
    in
    let ity_sub = LoclType ty_sub in
    let secondary_error =
      Subtype_env.mk_secondary_error subtype_env ity_sub ity_super
    in
    (* Contextualize errors that may be generated when
                         * checking refinement bounds. *)
    let on_error =
      Option.map subtype_env.Subtype_env.on_error ~f:(fun on_error ->
          let open Typing_error.Reasons_callback in
          prepend_on_apply on_error secondary_error)
    in
    let subtype_env = Subtype_env.{ subtype_env with on_error } in

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
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn = None; ty_sub = ty }
            ~rhs:
              { super_like = false; super_supportdyn = false; ty_super = bound }
            env)
      | `Super ->
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn = None; ty_sub = bound }
            ~rhs:{ super_like = false; super_supportdyn = false; ty_super = ty }
            env)
    in

    let concrete_rigid_tvar_access env ucckind bndtys =
      (* First, we try to discharge the subtype query on the bound; if
       * that fails, we mint a fresh rigid type variable to represent
       * the concrete type constant and try to solve the query using it *)
      let bndty = MakeType.intersection (get_reason ty_sub) bndtys in
      let ( ||| ) = ( ||| ) ~fail in
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn = None; ty_sub = bndty }
        ~rhs
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

    match deref ty_sub with
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
        let r_bnd = Reason.tconst_no_cstr name in
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
          Subtype.(
            simplify
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn = None; ty_sub = upty }
              ~rhs:
                {
                  super_like = false;
                  super_supportdyn = false;
                  ty_super = loty;
                }
              env)
        else
          valid env)
        &&& simplify_subtype_bound `As upty ~bound:memupty
        &&& simplify_subtype_bound `Super ~bound:memloty loty)
    | (_r_sub, Tdependent (DTexpr eid, bndty)) ->
      concrete_rigid_tvar_access env (Typing_type_member.EDT eid) [bndty]
    | (_r_sub, Tgeneric (s, ty_args)) when String.equal s SN.Typehints.this ->
      let bnd_tys = Typing_set.elements (Env.get_upper_bounds env s ty_args) in
      concrete_rigid_tvar_access env Typing_type_member.This bnd_tys
    | (_, Tvar _) ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        htmty
    | (r_sub, Tunion ty_subs) ->
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop:simplify
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | (r_sub, Tintersection ty_subs) ->
      Common.simplify_intersection_l
        ~subtype_env
        ~this_ty
        ~fail
        ~mk_prop:simplify
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | (r_generic, Tgeneric (generic_nm, generic_ty_args)) ->
      (match
         VisitedGoalsInternal.try_add_visited_generic_sub
           subtype_env.Subtype_env.visited_internal
           generic_nm
           htmty
       with
      | None -> invalid ~fail env
      | Some new_visited ->
        let subtype_env =
          Subtype_env.set_visited_internal subtype_env new_visited
        in
        Common.simplify_generic_l
          ~subtype_env
          ~this_ty
          ~fail
          ~mk_prop:simplify
          (sub_supportdyn, r_generic, generic_nm, generic_ty_args)
          rhs
          rhs
          env)
    | ( _,
        ( Tany _ | Tdynamic | Tnonnull | Toption _ | Tprim _ | Tneg _ | Tfun _
        | Ttuple _ | Tshape _ | Tvec_or_dict _ | Taccess _ | Tnewtype _
        | Tlabel _ | Tunapplied_alias _ ) ) ->
      invalid ~fail env
    | (_, Tclass_args _) ->
      (* TODO(T199606542) Confirm if class pointers appear in HasTypeMember positions *)
      invalid ~fail env
end

and Has_member : sig
  type rhs = {
    reason_super: Reason.t;
    has_member: has_member;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    has_member: has_member;
  }

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
              Quickfix.make_eager_default_hint_style
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

  let typing_obj_get
      ~subtype_env
      ~this_ty
      ~class_id
      ~member_id
      ~explicit_targs
      ~member_ty
      ty_sub
      env =
    let (explicit_targs, is_method) =
      match explicit_targs with
      | None -> ([], false)
      | Some targs -> (targs, true)
    in
    let (res, (obj_get_ty, _tal)) =
      Typing_object_get.obj_get
        ~obj_pos:(fst member_id)
          (* `~obj_pos:name_pos` is a lie: `name_pos` is the rhs of `->` or `?->` *)
        ~is_method
        ~meth_caller:false
        ~coerce_from_ty:None
        ~nullsafe:None
        ~explicit_targs
        ~class_id
        ~member_id
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
    &&& Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn = None; ty_sub = obj_get_ty }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super = member_ty;
              })

  let rec simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn = _; ty_sub } = lhs in
    let { reason_super = r; has_member = has_member_ty } = rhs in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let ty_sub = LoclType ty_sub in
    let ty_super =
      ConstraintType (mk_constraint_type (r, Thas_member has_member_ty))
    in
    if Logging.should_log_subtype_i env ~level:2 ty_sub ty_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:"Typing_subtype.Has_member.simplify"
        env
        ty_sub
        ty_super
      @@ fun () -> simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_
      ~subtype_env
      ~this_ty
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:({ reason_super = r; has_member = has_member_ty } as rhs)
      env =
    let {
      hm_name = (name_pos, name_) as member_id;
      hm_type = member_ty;
      hm_class_id = class_id;
      hm_explicit_targs = explicit_targs;
    } =
      has_member_ty
    in
    let is_method = Option.is_some explicit_targs in
    let cty_super = mk_constraint_type (r, Thas_member has_member_ty) in
    let ity_super = ConstraintType cty_super in
    let (env, ty_sub) = Env.expand_type env ty_sub in

    let subtype_env =
      Subtype_env.possibly_add_violated_constraint
        subtype_env
        ~r_sub:(get_reason ty_sub)
        ~r_super:r
    in
    let fail =
      Subtype_env.fail subtype_env ~ty_sub:(LoclType ty_sub) ~ty_super:ity_super
    in

    match deref ty_sub with
    | (_, Tvar _) ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        ity_super
    | (r_sub, Tunion ty_subs) ->
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop:simplify
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
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
    | (r_sub, Tintersection ty_subs) ->
      (* A & B <: C iif A <: C | !B *)
      (match Subtype_negation.find_type_with_exact_negation env ty_subs with
      | (env, Some non_ty, tyl) ->
        let ty_sub = MakeType.intersection r_sub tyl in
        let mk_prop = simplify
        and lift_rhs { reason_super; has_member } =
          mk_constraint_type (reason_super, Thas_member has_member)
        and lhs = (sub_supportdyn, ty_sub)
        and rhs_subtype =
          Subtype.
            { super_supportdyn = false; super_like = false; ty_super = non_ty }
        and rhs_destructure =
          { reason_super = r; has_member = has_member_ty }
        in
        let rhs = (r, rhs_subtype, rhs_destructure) in
        Common.simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          lhs
          rhs
          env
      | _ ->
        typing_obj_get
          ~subtype_env
          ~this_ty
          ~class_id
          ~member_id
          ~explicit_targs
          ~member_ty
          ty_sub
          env)
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
      simplify
        ~subtype_env
        ~this_ty
        ~lhs:{ sub_supportdyn; ty_sub = newtype_ty }
        ~rhs:{ reason_super = r; has_member = has_member_ty }
        env
    | ( _,
        ( Toption _ | Tdynamic | Tnonnull | Tany _ | Tprim _ | Tfun _ | Ttuple _
        | Tshape _ | Tgeneric _ | Tdependent _ | Tvec_or_dict _ | Taccess _
        | Tunapplied_alias _ | Tclass _ | Tneg _ | Tlabel _ | Tclass_args _ ) )
      ->
      typing_obj_get
        ~subtype_env
        ~this_ty
        ~class_id
        ~member_id
        ~explicit_targs
        ~member_ty
        ty_sub
        env
end

and Has_const : sig
  type rhs = {
    reason_super: Reason.t;
    name: string;
    ty: locl_ty;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    reason_super: Reason.t;
    name: string;
    ty: locl_ty;
  }

  let rec simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn = _; ty_sub } = lhs in
    let { reason_super = r; name; ty = member_ty } = rhs in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let ty_sub = LoclType ty_sub in
    let ty_super =
      ConstraintType
        (mk_constraint_type (r, Thas_const { name; ty = member_ty }))
    in
    if Logging.should_log_subtype_i env ~level:2 ty_sub ty_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:"Typing_subtype.Has_const.simplify"
        env
        ty_sub
        ty_super
      @@ fun () -> simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_
      ~subtype_env
      ~(this_ty : locl_ty option)
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:({ reason_super = r; name; ty = member_ty } as rhs)
      env =
    let cty_super =
      mk_constraint_type (r, Thas_const { name; ty = member_ty })
    in
    let ity_super = ConstraintType cty_super in
    let fail =
      Subtype_env.fail subtype_env ~ty_sub:(LoclType ty_sub) ~ty_super:ity_super
    in
    simplify_has_const
      ~subtype_env
      ~this_ty:(Option.value this_ty ~default:ty_sub)
      ~fail
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs
      env

  and simplify_has_const
      ~subtype_env
      ~(this_ty : locl_ty)
      ~fail
      ~lhs:{ sub_supportdyn; ty_sub }
      ~rhs:({ reason_super = r; name; ty = member_ty } as rhs)
      env =
    let (env, ety_sub) = Env.expand_type env ty_sub in
    match deref ety_sub with
    | (_, Tclass ((_, enum_name), _, _)) ->
      let cls = Env.get_class env enum_name in
      (match cls with
      | Decl_entry.Found cls ->
        let maybe_const =
          match Env.get_typeconst env cls name with
          | Some _ -> None
          | None -> Env.get_const env cls name
        in
        (match maybe_const with
        | Some const_def ->
          let dty = const_def.cc_type in
          let ety_env = { empty_expand_env with this_ty } in
          let ((env, _ty_err_opt), lty) =
            if
              TypecheckerOptions.experimental_feature_enabled
                (Env.get_tcopt env)
                TypecheckerOptions.experimental_sound_enum_class_type_const
            then
              Phase.localize env ~ety_env dty
            else
              Phase.localize_no_subst env ~ignore_errors:true dty
          in
          Subtype.(
            simplify
              ~subtype_env
              ~this_ty:None
              ~lhs:{ sub_supportdyn; ty_sub = lty }
              ~rhs:
                {
                  super_like = false;
                  super_supportdyn = false;
                  ty_super = member_ty;
                }
              env)
        | None ->
          let consts =
            Cls.consts cls
            |> List.filter ~f:(fun (name, _) ->
                   not
                     (String.equal name SN.Members.mClass
                     || Cls.has_typeconst cls name))
          in
          let fail =
            let open Option.Let_syntax in
            let open Typing_error in
            let pos_opt =
              Pos_or_decl.fill_in_filename_if_in_current_decl
                ~current_decl_and_file:(Env.get_current_decl_and_file env)
              @@ Reason.to_pos r
            in
            let (most_similar, quickfixes_opt) =
              match Env.most_similar name consts fst with
              | Some (name, const) ->
                let new_text = name in
                ( Some (new_text, const.cc_pos),
                  Option.map pos_opt ~f:(fun pos ->
                      [
                        (let (_, start_column) = Pos.line_column pos in
                         Quickfix.make_eager_default_hint_style
                           ~title:("Change to " ^ Markdown_lite.md_codify name)
                           ~new_text
                         @@ Pos.set_col_start (start_column + 1) pos);
                      ]) )
              | None -> (None, None)
            in
            let subtype_env =
              match quickfixes_opt with
              | None -> subtype_env
              | Some quickfixes ->
                Subtype_env.set_on_error subtype_env
                @@ let* on_error = subtype_env.Subtype_env.on_error in
                   return
                   @@ Typing_error.Reasons_callback.add_quickfixes
                        on_error
                        quickfixes
            in
            Subtype_env.fail_with_suffix
              subtype_env
              ~ty_sub:(LoclType ty_sub)
              ~ty_super:
                (ConstraintType
                   (mk_constraint_type (r, Thas_const { name; ty = member_ty })))
            @@ Secondary.Unknown_label
                 { enum_name; decl_pos = Cls.pos cls; most_similar }
          in
          invalid env ~fail)
      | Decl_entry.NotYetAvailable
      | Decl_entry.DoesNotExist ->
        invalid env ~fail)
    | (_, Tvar _) ->
      let cty_super =
        mk_constraint_type (r, Thas_const { name; ty = member_ty })
      in
      let ity_super = ConstraintType cty_super in
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        ity_super
    | (r_sub, Tunion ty_subs) ->
      Common.simplify_union_l
        ~subtype_env
        ~this_ty:None
        ~mk_prop:simplify
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | (r_sub, Tintersection ty_subs) ->
      (* A & B <: C iif A <: C | !B *)
      (match Subtype_negation.find_type_with_exact_negation env ty_subs with
      | (env, Some non_ty, tyl) ->
        let ty_sub = MakeType.intersection r_sub tyl in
        let mk_prop = simplify
        and lift_rhs { reason_super; name; ty } =
          mk_constraint_type (reason_super, Thas_const { name; ty })
        and lhs = (sub_supportdyn, ty_sub)
        and rhs_subtype =
          Subtype.
            { super_supportdyn = false; super_like = false; ty_super = non_ty }
        and rhs_destructure = { reason_super = r; name; ty = member_ty } in
        let rhs = (r, rhs_subtype, rhs_destructure) in
        Common.simplify_disj_r
          ~subtype_env
          ~this_ty:None
          ~fail
          ~lift_rhs
          ~mk_prop
          lhs
          rhs
          env
      | _ ->
        Common.simplify_intersection_l
          ~subtype_env
          ~this_ty:None
          ~fail
          ~mk_prop:simplify
          ~update_reason:
            (Typing_env.update_reason ~f:(fun r_sub_prj ->
                 Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
          (sub_supportdyn, ty_subs)
          rhs
          env)
    | (r_generic, Tgeneric (generic_nm, generic_ty_args)) ->
      Common.simplify_generic_l
        ~subtype_env
        ~this_ty:(Some this_ty)
        ~fail
        ~mk_prop:simplify
        (sub_supportdyn, r_generic, generic_nm, generic_ty_args)
        rhs
        rhs
        env
    | (r_newtype, Tnewtype (nm, _, ty_newtype)) ->
      Common.simplify_newtype_l
        ~subtype_env
        ~this_ty:(Some this_ty)
        ~mk_prop:simplify
        (sub_supportdyn, r_newtype, nm, ty_newtype)
        rhs
        env
    | (r_dep, Tdependent (dep_ty, ty_inner_sub)) ->
      Common.simplify_dependent_l
        ~subtype_env
        ~this_ty:(Some this_ty)
        ~mk_prop:simplify
        (sub_supportdyn, r_dep, dep_ty, ty_inner_sub)
        rhs
        env
    | ( _,
        ( Toption _ | Tdynamic | Tnonnull | Tany _ | Tprim _ | Tfun _ | Ttuple _
        | Tshape _ | Tvec_or_dict _ | Taccess _ | Tunapplied_alias _ | Tneg _
        | Tlabel _ ) ) ->
      invalid ~fail env
    | (_, Tclass_args _) ->
      (* TODO(T199606542) Check if this can be used to improve inference for $c::FOO *)
      invalid ~fail env
end

and Type_switch : sig
  type rhs = {
    super_like: bool;
    reason_super: Reason.t;
    predicate: Typing_defs.type_predicate;
    ty_true: locl_ty;
    ty_false: locl_ty;
  }

  include Constraint_handler with type rhs := rhs
end = struct
  type rhs = {
    super_like: bool;
    reason_super: Reason.t;
    predicate: Typing_defs.type_predicate;
    ty_true: locl_ty;
    ty_false: locl_ty;
  }

  let rec simplify ~subtype_env ~this_ty ~lhs ~rhs env =
    let { sub_supportdyn = _; ty_sub } = lhs in
    let { super_like = _; reason_super; predicate; ty_true; ty_false } = rhs in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    let ty_sub = LoclType ty_sub in
    let ty_super =
      ConstraintType
        (mk_constraint_type
           (reason_super, Ttype_switch { predicate; ty_true; ty_false }))
    in
    if Logging.should_log_subtype_i env ~level:2 ty_sub ty_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:"Typing_subtype.Type_switch.simplify"
        env
        ty_sub
        ty_super
      @@ fun () -> simplify_ ~subtype_env ~this_ty ~lhs ~rhs env
    else
      simplify_ ~subtype_env ~this_ty ~lhs ~rhs env

  and simplify_
      ~subtype_env
      ~this_ty
      ~lhs:{ ty_sub; sub_supportdyn }
      ~rhs:({ super_like; reason_super; predicate; ty_true; ty_false } as rhs)
      env =
    let cty =
      mk_constraint_type
        (reason_super, Ttype_switch { predicate; ty_true; ty_false })
    in
    let ty_super = ConstraintType cty in
    let (env, ty_sub) = Env.expand_type env ty_sub in
    match get_node ty_sub with
    | Tvar _ ->
      mk_issubtype_prop
        ~sub_supportdyn
        ~coerce:subtype_env.Subtype_env.coerce
        env
        (LoclType ty_sub)
        ty_super
    | Tunion ty_subs ->
      let r_sub = get_reason ty_sub in
      Common.simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop:simplify
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | _ ->
      let (env, partition) =
        Typing_refinement.partition_ty env ty_sub predicate
      in
      let intersect tyl = MakeType.intersection reason_super tyl in
      let simplify_subtype ~f tyl ty_super env =
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub = f tyl }
            ~rhs:{ super_supportdyn = false; super_like; ty_super }
            env)
      in
      (* When we split a type we have some component that is a subset and
         some component that is a span. For the component that is a subset
         we need to ensure it is a subtype of the given super type, but
         for the span we need to refine the type down to a type we know
         would pass the given predicate. *)
      let simplify_split
          ~init
          ~refine
          (subset : Typing_refinement.dnf_ty)
          (span : Typing_refinement.dnf_ty)
          ty_sup =
        let init =
          List.fold_left subset ~init ~f:(fun res tyl ->
              res &&& simplify_subtype ~f:intersect tyl ty_sup)
        in
        List.fold_left span ~init ~f:(fun res tyl ->
            res &&& simplify_subtype ~f:refine tyl ty_sup)
      in

      let refine_true tyl =
        intersect (Typing_refinement.TyPredicate.to_ty predicate :: tyl)
      in
      let refine_false tyl =
        let neg = MakeType.neg reason_super predicate in
        intersect (neg :: tyl)
      in
      let left = partition.Typing_refinement.left in
      let span = partition.Typing_refinement.span in
      let right = partition.Typing_refinement.right in
      (* Handle dynamic a bit differently to avoid intersecting with dynamic on
         the false side. This is less precise, but plays better with dynamic
         stripping logic elsewhere in the typechecker.
         dynamic will always span (except cases like IsMixed, but this actually
         isn't a problem if dynamic is in a different part since we only
         intersect the span) so this picks out the dynamic and, for the false
         side, moves it to the right.
         This makes it such that (e.g. in the case when there is no other part
         of the span) instead of `(dynamic & !P) | right` you get
         `dynamic | right`
      *)
      let (span_for_false, dyn) =
        List.fold_left
          ~init:([], None)
          ~f:(fun (tyll, dyn_opt) tyl ->
            match tyl with
            | [ty] when Typing_defs.is_dynamic ty -> (tyll, Some ty)
            | _ -> (tyl :: tyll, dyn_opt))
          span
      in
      let right =
        match dyn with
        | Some dyn -> [dyn] :: right
        | _ -> right
      in
      let (env, props) =
        simplify_split
          ~refine:refine_true
          ~init:(env, TL.valid)
          left
          span
          ty_true
      in
      simplify_split
        ~refine:refine_false
        ~init:(env, props)
        right
        span_for_false
        ty_false
end

and Common : sig
  val simplify_union_l :
    subtype_env:Subtype_env.t ->
    this_ty:locl_ty option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * TL.subtype_prop) ->
    update_reason:(env -> locl_ty -> locl_ty) ->
    Reason.t option * locl_phase ty list ->
    'rhs ->
    env ->
    env * TL.subtype_prop

  val simplify_intersection_l :
    subtype_env:Subtype_env.t ->
    this_ty:locl_ty option ->
    fail:Typing_error.t option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * TL.subtype_prop) ->
    update_reason:(env -> locl_ty -> locl_ty) ->
    Reason.t option * locl_phase ty list ->
    'rhs ->
    env ->
    env * TL.subtype_prop

  val simplify_generic_l :
    subtype_env:Subtype_env.t ->
    this_ty:locl_phase ty option ->
    fail:Typing_error.t option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_phase ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * TL.subtype_prop) ->
    Reason.t option * locl_phase Reason.t_ * string * locl_ty list ->
    'rhs ->
    'rhs ->
    env ->
    env * TL.subtype_prop

  val simplify_newtype_l :
    subtype_env:Subtype_env.t ->
    this_ty:locl_ty option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * TL.subtype_prop) ->
    Reason.t option * Reason.t * string * locl_phase ty ->
    'rhs ->
    env ->
    env * TL.subtype_prop

  val simplify_dependent_l :
    subtype_env:Subtype_env.t ->
    this_ty:locl_ty option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * TL.subtype_prop) ->
    Reason.t option * Reason.t * dependent_type * locl_phase ty ->
    'rhs ->
    env ->
    env * TL.subtype_prop

  val simplify_disj_r :
    subtype_env:Subtype_env.t ->
    this_ty:locl_ty option ->
    fail:Typing_error.t option ->
    mk_prop:
      (subtype_env:Subtype_env.t ->
      this_ty:locl_ty option ->
      lhs:lhs ->
      rhs:'rhs ->
      env ->
      env * Typing_logic.subtype_prop) ->
    lift_rhs:('rhs -> constraint_type) ->
    Reason.t option * locl_ty ->
    Reason.t * Subtype.rhs * 'rhs ->
    env ->
    env * Typing_logic.subtype_prop

  val dispatch_constraint :
    subtype_env:Subtype_env.t ->
    this_ty:Typing_defs.locl_ty option ->
    sub_supportdyn:Reason.t option ->
    Typing_defs.internal_type ->
    Typing_defs.internal_type ->
    Typing_env_types.env ->
    Typing_env_types.env * TL.subtype_prop
end = struct
  (* Helper function which returns true if a type is dynamic or a (nested)
     intersection of types where any type in the intersection is dynamic. Used
     to delay generation disjunctions in the c-union-l case. *)
  let rec contains_dynamic_through_intersection ty =
    Typing_defs.is_dynamic ty
    ||
    match get_node ty with
    | Tintersection tyl ->
      List.exists ~f:contains_dynamic_through_intersection tyl
    | _ -> false

  let simplify_union_l
      ~subtype_env
      ~this_ty
      ~mk_prop
      ~update_reason
      (sub_supportdyn, ty_subs)
      rhs
      env =
    let f res ty_sub =
      let ty_sub = update_reason env ty_sub in
      res &&& mk_prop ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs
    in
    (* Prioritize types that aren't dynamic or intersections with dynamic
       to get better error messages *)
    let (last_tyl, first_tyl) =
      Typing_dynamic_utils.partition_union
        ~f:contains_dynamic_through_intersection
        env
        ty_subs
    in
    let init = List.fold_left first_tyl ~init:(env, TL.valid) ~f in
    List.fold_left last_tyl ~init ~f

  (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
     not complete.
     TODO(T120921930): Don't do this if require_completeness is set. *)

  let simplify_intersection_l
      ~subtype_env
      ~this_ty
      ~fail
      ~mk_prop
      ~update_reason
      (sub_supportdyn, tys_sub)
      rhs
      env =
    let rec aux ty_subs ~env ~errs ~props =
      match ty_subs with
      | [] ->
        ( env,
          TL.Disj (Typing_error.intersect_opt @@ List.filter_opt errs, props) )
      | ty_sub :: ty_subs ->
        let ty_sub = update_reason env ty_sub in
        let (env, prop) =
          mk_prop ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env
        in
        if TL.is_valid prop then
          (env, prop)
        else (
          match TL.get_error_if_unsat prop with
          | Some err -> aux ty_subs ~env ~errs:(err :: errs) ~props
          | _ -> aux ty_subs ~env ~errs ~props:(prop :: props)
        )
    in
    aux tys_sub ~env ~errs:[fail] ~props:[]

  let simplify_generic_l
      ~subtype_env
      ~this_ty
      ~fail
      ~mk_prop
      (sub_supportdyn, reason_generic, generic_nm, generic_ty_args)
      rhs
      rhs_for_mixed
      env =
    begin
      let lty_sub =
        mk (reason_generic, Tgeneric (generic_nm, generic_ty_args))
      in
      let (env, prop) =
        (* If the generic is actually an expression dependent type,
           we need to update this_ty
        *)
        let this_ty =
          if
            DependentKind.is_generic_dep_ty generic_nm && Option.is_none this_ty
          then
            Some lty_sub
          else
            this_ty
        in

        (* Otherwise, we collect all the upper bounds ("as" constraints) on
           the generic parameter, and check each of these in turn against
           ty_super until one of them succeeds
        *)
        let accumulate prop ~errs ~props =
          if TL.is_valid prop then
            Ok prop
          else
            match TL.get_error_if_unsat prop with
            | Some err -> Error (err :: errs, props)
            | None -> Error (errs, prop :: props)
        in
        let rec aux upper_bounds ~env ~errs ~props =
          match upper_bounds with
          | [] ->
            let err =
              Typing_error.intersect_opt @@ List.filter_opt @@ List.rev errs
            in
            (env, TL.Disj (err, List.rev props))
          | upper_bound :: upper_bounds ->
            let ty_sub =
              Typing_env.update_reason env upper_bound ~f:(fun bound ->
                  Typing_reason.axiom_upper_bound ~bound ~of_:reason_generic)
            in
            let (env, prop) =
              mk_prop
                ~subtype_env
                ~this_ty
                ~lhs:{ sub_supportdyn; ty_sub }
                ~rhs
                env
            in
            (match accumulate prop ~errs ~props with
            | Ok prop -> (env, prop)
            | Error (errs, props) -> aux upper_bounds ~env ~errs ~props)
        in

        let upper_bounds =
          Typing_set.elements
            (Env.get_upper_bounds env generic_nm generic_ty_args)
        in
        (* TODO(mjt) We reverse bounds here to match the evaluation order
           of older code; not doing so triggers errors due to incompleteness *)
        match List.rev upper_bounds with
        | [] ->
          (* Try an implicit mixed = ?nonnull bound before giving up.
             This can be useful when checking T <: t, where type t is
             equivalent to but syntactically different from ?nonnull.
             E.g., if t is a generic type parameter T with nonnull as
             a lower bound.
          *)
          let r = Reason.implicit_upper_bound (get_pos lty_sub, "mixed") in
          let tmixed = MakeType.mixed r in
          let ty_sub =
            Typing_env.update_reason env tmixed ~f:(fun bound ->
                Typing_reason.axiom_upper_bound ~bound ~of_:reason_generic)
          in
          let (env, prop) =
            mk_prop
              ~subtype_env
              ~this_ty
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:rhs_for_mixed
              env
          in
          if TL.is_valid prop then
            (env, prop)
          else
            let (err, props) =
              match TL.get_error_if_unsat prop with
              | Some err ->
                ( Typing_error.intersect_opt @@ List.filter_opt @@ [fail; err],
                  [] )
              | _ -> (fail, [prop])
            in
            (env, TL.Disj (err, props))
        | upper_bounds -> aux upper_bounds ~env ~errs:[fail] ~props:[]
      in
      (env, prop)
    end

  let simplify_newtype_l
      ~subtype_env
      ~this_ty
      ~mk_prop
      (sub_supportdyn, reason_newtype, newtype_nm, newtype_ty)
      rhs
      env =
    let sub_supportdyn =
      match sub_supportdyn with
      | None ->
        if String.equal newtype_nm SN.Classes.cSupportDyn then
          Some reason_newtype
        else
          None
      | _ -> sub_supportdyn
    in
    let ty_sub =
      Typing_env.update_reason env newtype_ty ~f:(fun bound ->
          Typing_reason.axiom_upper_bound ~bound ~of_:reason_newtype)
    in

    mk_prop ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env

  let simplify_dependent_l
      ~subtype_env
      ~this_ty
      ~mk_prop
      (sub_supportdyn, reason_dep, dep_ty, ty_inner_sub)
      rhs
      env =
    let this_ty =
      Option.first_some
        this_ty
        (Some (mk (reason_dep, Tdependent (dep_ty, ty_inner_sub))))
    in
    let lhs = { sub_supportdyn; ty_sub = ty_inner_sub } in

    mk_prop ~subtype_env ~this_ty ~lhs ~rhs env

  let rec simplify_disj_r
      ~subtype_env
      ~this_ty
      ~fail
      ~mk_prop
      ~lift_rhs
      (sub_supportdyn, ty_sub)
      ((reason_super, rhs_subtype, rhs_other) as rhs)
      env =
    let (env, ty_sub) = Env.expand_type env ty_sub in

    match deref ty_sub with
    | (r, Toption ty) ->
      let ty_null = MakeType.null r in
      let ty =
        Typing_env.update_reason env ty ~f:(fun r_sub_prj ->
            Typing_reason.prj_nullable_sub ~sub:r ~sub_prj:r_sub_prj)
      in
      let prop env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_null)
          rhs
          env
        &&& simplify_disj_r
              ~subtype_env
              ~this_ty
              ~fail
              ~lift_rhs
              ~mk_prop
              (sub_supportdyn, ty)
              rhs
      in

      if_unsat (invalid ~fail) @@ prop env
    | (r_sub, Tintersection ty_subs) ->
      let mk_prop_intersection
          ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_sub)
          rhs
          env
      in
      simplify_intersection_l
        ~subtype_env
        ~this_ty
        ~fail
        ~mk_prop:mk_prop_intersection
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_inter_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | (r_sub, Tunion ty_subs) ->
      let mk_prop_union
          ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_sub)
          rhs
          env
      in
      simplify_union_l
        ~subtype_env
        ~this_ty
        ~mk_prop:mk_prop_union
        ~update_reason:
          (Typing_env.update_reason ~f:(fun r_sub_prj ->
               Typing_reason.prj_union_sub ~sub:r_sub ~sub_prj:r_sub_prj))
        (sub_supportdyn, ty_subs)
        rhs
        env
    | (_, Tvar _) ->
      let (env, ty_fresh) = Env.fresh_type env Pos.none in
      let mk_cstr_prop env =
        mk_prop
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn = None; ty_sub = ty_fresh }
          ~rhs:rhs_other
          env
      in
      let mk_subty_prop env =
        Subtype.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:
              {
                super_like = false;
                super_supportdyn = false;
                ty_super =
                  Typing_make_type.union
                    reason_super
                    [rhs_subtype.Subtype.ty_super; ty_fresh];
              }
            env)
      in
      mk_subty_prop env &&& mk_cstr_prop
    | (r_generic, Tgeneric (nm, tyargs)) ->
      let mk_prop_generic
          ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_sub)
          rhs
          env
      in
      simplify_generic_l
        ~subtype_env
        ~this_ty
        ~fail
        ~mk_prop:mk_prop_generic
        (sub_supportdyn, r_generic, nm, tyargs)
        rhs
        rhs
        env
    | (r_dep, Tdependent (dep_ty, ty_sub_inner)) ->
      let mk_prop_dependent
          ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_sub)
          rhs
          env
      in
      simplify_dependent_l
        ~subtype_env
        ~this_ty
        ~mk_prop:mk_prop_dependent
        (sub_supportdyn, r_dep, dep_ty, ty_sub_inner)
        rhs
        env
    | (r_newtype, Tnewtype (nm, _, ty_newtype)) ->
      let mk_prop_newtype
          ~subtype_env ~this_ty ~lhs:{ sub_supportdyn; ty_sub } ~rhs env =
        simplify_disj_r
          ~subtype_env
          ~this_ty
          ~fail
          ~lift_rhs
          ~mk_prop
          (sub_supportdyn, ty_sub)
          rhs
          env
      in
      let ( ||| ) = ( ||| ) ~fail in
      mk_prop
        ~subtype_env
        ~this_ty:None
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:rhs_other
        env
      ||| Subtype.(
            simplify
              ~subtype_env
              ~this_ty:None
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:rhs_subtype)
      ||| simplify_newtype_l
            ~subtype_env
            ~this_ty
            ~mk_prop:mk_prop_newtype
            (sub_supportdyn, r_newtype, nm, ty_newtype)
            rhs
    | (_, Tdynamic) when Subtype_env.coercing_from_dynamic subtype_env ->
      valid env
    | ( _,
        ( Tany _ | Tdynamic | Tprim _ | Tneg _ | Tnonnull | Tunapplied_alias _
        | Tfun _ | Ttuple _ | Tshape _ | Tvec_or_dict _ | Taccess _ | Tclass _
        | Tlabel _ | Tclass_args _ ) ) ->
      let ( ||| ) = ( ||| ) ~fail in
      mk_prop
        ~subtype_env
        ~this_ty:None
        ~lhs:{ sub_supportdyn; ty_sub }
        ~rhs:rhs_other
        env
      ||| Subtype.(
            simplify
              ~subtype_env
              ~this_ty:None
              ~lhs:{ sub_supportdyn; ty_sub }
              ~rhs:rhs_subtype)

  let dispatch_constraint
      ~subtype_env ~this_ty ~sub_supportdyn ity_sub ity_super env =
    match (ity_sub, ity_super) with
    | (LoclType ty_sub, LoclType ty_super) ->
      Subtype.(
        simplify
          ~subtype_env
          ~this_ty
          ~lhs:{ sub_supportdyn; ty_sub }
          ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
          env)
    | (LoclType ty_sub, ConstraintType cstr) ->
      (match deref_constraint_type cstr with
      | (r_super, Tdestructure destructure) ->
        Destructure.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super = r_super; destructure }
            env)
      | (r, Tcan_index can_index) ->
        Can_index.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super = r; can_index }
            env)
      | (r, Tcan_traverse can_traverse) ->
        Can_traverse.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super = r; can_traverse }
            env)
      | (r, Thas_member has_member) ->
        Has_member.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super = r; has_member }
            env)
      | (r, Thas_type_member has_type_member) ->
        Has_type_member.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super = r; has_type_member }
            env)
      | (reason_super, Thas_const { name; ty }) ->
        Has_const.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:{ reason_super; name; ty }
            env)
      | (reason_super, Ttype_switch { predicate; ty_true; ty_false }) ->
        Type_switch.(
          simplify
            ~subtype_env
            ~this_ty
            ~lhs:{ sub_supportdyn; ty_sub }
            ~rhs:
              { reason_super; predicate; ty_true; ty_false; super_like = false }
            env))
    | (ConstraintType _, (LoclType _ | ConstraintType _)) ->
      let subtype_env =
        Subtype_env.possibly_add_violated_constraint
          subtype_env
          ~r_sub:(reason ity_sub)
          ~r_super:(reason ity_super)
      in
      let fail =
        Subtype_env.fail subtype_env ~ty_sub:ity_sub ~ty_super:ity_super
      in
      invalid env ~fail
end

(* -- API drivers ----------------------------------------------------------- *)
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
      ~require_completeness
      ~no_top_bottom
      ~coerce
      ~sub_supportdyn
      env
      ity_sub
      ity_super =
    let this_ty =
      match ity_sub with
      | LoclType ty1 -> Some ty1
      | ConstraintType _ -> None
    in
    let subtype_env =
      Subtype_env.create
        ~require_completeness
        ~no_top_bottom
        ~coerce
        ~log_level:3
        None
    in
    (* It is weird that this can cause errors, but I am wary to discard them.
     * Using the generic unify_error to maintain current behavior. *)
    let (_env, prop) =
      Common.dispatch_constraint
        ~subtype_env
        ~this_ty
        ~sub_supportdyn
        ity_sub
        ity_super
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

module Subtype_simplify : sig
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

module Subtype_trans : sig
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
  let add_non_subtype_constraint
      ~coerce
      (env, prop)
      (r_sub, var_sub)
      cty_super
      (on_error : Typing_error.Reasons_callback.t option) =
    let ty_super = ConstraintType cty_super in
    let upper_bounds_before = Env.get_tyvar_upper_bounds env var_sub in
    let env =
      Env.add_tyvar_upper_bound_and_update_variances
        ~intersect:(Subtype_simplify.try_intersect_i ~ignore_tyvars:true env)
        env
        var_sub
        ty_super
    in
    let upper_bounds_after = Env.get_tyvar_upper_bounds env var_sub in
    (* Because of [try_intersect_i], the changed upper bounds may include
       [locl_ty]s *)
    let added_upper_bounds =
      ITySet.diff upper_bounds_after upper_bounds_before
    in
    (* We have the invariant that the lower bound must always be a [locl_ty] *)
    let lower_bounds =
      List.filter_map ~f:(function
          | LoclType ty -> Some ty
          | _ -> failwith "constraint_type in lowerbounds")
      @@ ITySet.elements
      @@ Env.get_tyvar_lower_bounds env var_sub
    in
    let subtype_env =
      Subtype_env.create
        ~coerce
        ~log_level:2
        ~in_transitive_closure:true
        on_error
    in
    let (env, prop) =
      ITySet.fold
        (fun upper_bound (env, prop) ->
          let (env, ty_err_opt) =
            Typing_subtype_tconst.make_all_type_consts_equal
              env
              var_sub
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
          List.fold_left
            ~f:(fun (env, prop1) lower_bound ->
              let lower_bound =
                Typing_env.update_reason env lower_bound ~f:(fun bound ->
                    Typing_reason.trans_lower_bound ~bound ~of_:r_sub)
              in
              (* Since we can have either the rhs of a subtype constraint or
                   the rhs of any other constraint in the upper bounds we
                   have to inspect the upper bound and dispatch to the
                   appropriate top-level handler *)
              let (env, prop2) =
                Common.dispatch_constraint
                  ~subtype_env
                  ~this_ty:None
                  ~sub_supportdyn:None
                  (LoclType lower_bound)
                  upper_bound
                  env
              in
              (env, TL.conj prop1 prop2))
            lower_bounds
            ~init:(env, prop))
        added_upper_bounds
        (env, prop)
    in
    (env, prop)

  (* Add a new upper bound ty on var.  Apply transitivity of sutyping,
     * so if we already have tyl <: var, then check that for each ty_sub
     * in tyl we have ty_sub <: ty.
  *)
  let add_tyvar_upper_bound_and_close
      ~coerce
      (env, prop)
      (r_sub, var)
      ty_super
      (on_error : Typing_error.Reasons_callback.t option) =
    let ty_super = Sd.transform_dynamic_upper_bound ~coerce env ty_super in
    let upper_bounds_before = Env.get_tyvar_upper_bounds env var in
    let env =
      Env.add_tyvar_upper_bound_and_update_variances
        ~intersect:(Subtype_simplify.try_intersect_i ~ignore_tyvars:true env)
        env
        var
        (LoclType ty_super)
    in
    let upper_bounds_after = Env.get_tyvar_upper_bounds env var in
    let added_upper_bounds =
      List.filter_map ~f:(function
          | LoclType ty -> Some ty
          | _ -> failwith "constraint_type in added upperbounds")
      @@ ITySet.elements
      @@ ITySet.diff upper_bounds_after upper_bounds_before
    in
    let lower_bounds =
      List.filter_map ~f:(function
          | LoclType ty -> Some ty
          | _ -> failwith "constraint_type in lowerbound")
      @@ ITySet.elements
      @@ Env.get_tyvar_lower_bounds env var
    in
    let subtype_env =
      Subtype_env.create
        ~coerce
        ~log_level:2
        ~in_transitive_closure:true
        on_error
    in
    let (env, prop) =
      List.fold
        ~f:(fun (env, prop) upper_bound ->
          let (env, ty_err_opt) =
            Typing_subtype_tconst.make_all_type_consts_equal
              env
              var
              (LoclType upper_bound)
              ~on_error
              ~as_tyvar_with_cnstr:true
          in
          let (env, prop) =
            Option.value_map
              ~default:(env, prop)
              ~f:(fun ty_err -> invalid ~fail:(Some ty_err) env)
              ty_err_opt
          in
          List.fold_left
            ~f:(fun (env, prop1) lower_bound ->
              let ty_sub =
                Typing_env.update_reason env lower_bound ~f:(fun bound ->
                    Typing_reason.trans_lower_bound ~bound ~of_:r_sub)
              in
              let (env, prop2) =
                Subtype.(
                  simplify
                    ~subtype_env
                    ~this_ty:None
                    ~lhs:{ sub_supportdyn = None; ty_sub }
                    ~rhs:
                      {
                        super_like = false;
                        super_supportdyn = false;
                        ty_super = upper_bound;
                      }
                    env)
              in
              (env, TL.conj prop1 prop2))
            lower_bounds
            ~init:(env, prop))
        added_upper_bounds
        ~init:(env, prop)
    in
    (env, prop)

  (* Add a new lower bound ty on var.  Apply transitivity of subtyping
   * (so if var <: ty1,...,tyn then assert ty <: tyi for each tyi), using
   * simplify_subtype to produce a subtype proposition.
   *)
  let add_tyvar_lower_bound_and_close
      ~coerce
      (env, prop)
      (r_sup, var)
      ty_sub
      (on_error : Typing_error.Reasons_callback.t option) =
    let ty_sub =
      Typing_env.update_reason env ty_sub ~f:(fun bound ->
          Typing_reason.trans_lower_bound ~bound ~of_:r_sup)
    in
    let lower_bounds_before = Env.get_tyvar_lower_bounds env var in
    let env =
      Env.add_tyvar_lower_bound_and_update_variances
        ~union:(Subtype_simplify.try_union_i env)
        env
        var
        (LoclType ty_sub)
    in
    let lower_bounds_after = Env.get_tyvar_lower_bounds env var in
    (* We have the invariant that lower bounds _must_ be the lhs of a constraint
       and this is always a [LoclType] *)
    let added_lower_bounds =
      List.filter_map ~f:(function
          | LoclType ty -> Some ty
          | _ -> failwith "constraint_type in added lowerbounds")
      @@ ITySet.elements
      @@ ITySet.diff lower_bounds_after lower_bounds_before
    in
    let upper_bounds = Env.get_tyvar_upper_bounds env var in
    let subtype_env =
      Subtype_env.create
        ~coerce
        ~log_level:2
        ~in_transitive_closure:true
        on_error
    in
    let (env, prop) =
      List.fold
        ~f:(fun (env, prop) lower_bound ->
          let (env, ty_err_opt) =
            Typing_subtype_tconst.make_all_type_consts_equal
              env
              var
              (LoclType lower_bound)
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
              (* Since we can have either the rhs of a subtype constraint or
                 the rhs of any other constraint in the upper bounds we
                 have to inspect the upper bound and dispatch to the
                 appropriate top-level handler *)
              let (env, prop2) =
                Common.dispatch_constraint
                  ~subtype_env
                  ~this_ty:None
                  ~sub_supportdyn:None
                  (LoclType lower_bound)
                  upper_bound
                  env
              in
              (env, TL.conj prop1 prop2))
            upper_bounds
            (env, prop))
        added_lower_bounds
        ~init:(env, prop)
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

  let log_non_singleton_disj ty_sub ty_super env msg disj_prop props =
    let rec aux props =
      match props with
      | [] -> ()
      | [TL.Disj (_, props)] -> aux props
      | [_] -> ()
      | _ ->
        Typing_log.log_prop
          1
          (Reason.to_pos (get_reason_i ty_sub))
          ("non-singleton disjunction "
          ^ msg
          ^ " of "
          ^ Typing_print.debug_i env ty_sub
          ^ " <: "
          ^ Typing_print.debug_i env ty_super)
          env
          disj_prop
    in
    aux props

  let rec tell ty_sub ty_super env prop on_error =
    match prop with
    | TL.Conj props ->
      tell_all ty_sub ty_super env ~ty_errs:[] ~remain:[] props on_error
    (* Encodes the invalid proposition with an accompanying error *)
    | TL.Disj (inf_err_opt, []) -> (env, inf_err_opt, [])
    | TL.Disj (inf_err_opt, props) ->
      log_non_singleton_disj
        ty_sub
        ty_super
        env
        "before simplification"
        prop
        props;
      let props = simplify_disj env props in
      log_non_singleton_disj
        ty_sub
        ty_super
        env
        "after simplification"
        prop
        props;
      let ty_errs = Option.to_list inf_err_opt in
      tell_exists ty_sub ty_super env ~ty_errs ~remain:[] props on_error
    | TL.IsSubtype (coerce, ty_sub, ty_super) ->
      tell_cstr env (coerce, ty_sub, ty_super) on_error

  and tell_cstr env (coerce, ty_sub, ty_super) on_error =
    let (env, ty_sub) = Env.expand_internal_type env ty_sub in
    let (env, ty_super) = Env.expand_internal_type env ty_super in

    match (ty_sub, ty_super) with
    | (LoclType lty_sub, LoclType lty_super) -> begin
      match (get_node lty_sub, get_node lty_super) with
      (* var-l-r *)
      | (Tvar var_sub, Tvar var_super) ->
        let (env, prop1) =
          add_tyvar_upper_bound_and_close
            ~coerce
            (valid env)
            (get_reason lty_sub, var_sub)
            lty_super
            on_error
        in
        let (env, prop2) =
          add_tyvar_lower_bound_and_close
            ~coerce
            (valid env)
            (get_reason lty_super, var_super)
            lty_sub
            on_error
        in
        tell_all
          ty_sub
          ty_super
          env
          ~ty_errs:[]
          ~remain:[]
          [prop1; prop2]
          on_error
      (* var-l *)
      | (Tvar var, _) ->
        let (env, prop) =
          add_tyvar_upper_bound_and_close
            ~coerce
            (valid env)
            (get_reason lty_sub, var)
            lty_super
            on_error
        in
        tell ty_sub ty_super env prop on_error
      | (_, Tvar var) ->
        let (env, prop) =
          add_tyvar_lower_bound_and_close
            ~coerce
            (valid env)
            (get_reason lty_super, var)
            lty_sub
            on_error
        in
        tell ty_sub ty_super env prop on_error
      | _ -> (env, None, [TL.IsSubtype (coerce, ty_sub, ty_super)])
    end
    (* [constraint_type]s are only valid on the rhs *)
    | (LoclType lty_sub, ConstraintType cstr) -> begin
      match get_node lty_sub with
      | Tvar var ->
        let (env, prop) =
          add_non_subtype_constraint
            ~coerce
            (valid env)
            (get_reason lty_sub, var)
            cstr
            on_error
        in
        tell ty_sub ty_super env prop on_error
      | _ -> (env, None, [TL.IsSubtype (coerce, ty_sub, ty_super)])
    end
    | (ConstraintType _, (LoclType _ | ConstraintType _)) ->
      (env, None, [TL.IsSubtype (coerce, ty_sub, ty_super)])

  and tell_all ty_sub ty_super env ~ty_errs ~remain props on_error =
    match props with
    | [] ->
      let ty_err_opt = Typing_error.multiple_opt @@ List.rev ty_errs in
      (env, ty_err_opt, List.rev remain)
    | prop :: props ->
      let (env, inf_err_opt, prop_remain) =
        tell ty_sub ty_super env prop on_error
      in
      let remain = prop_remain @ remain in
      let ty_errs =
        Option.value_map
          ~default:ty_errs
          ~f:(fun ty_err -> ty_err :: ty_errs)
          inf_err_opt
      in
      tell_all ty_sub ty_super env ~ty_errs ~remain props on_error

  and tell_exists ty_sub ty_super env ~ty_errs ~remain props on_error =
    (* For now, just find the first prop in the disjunction that works *)
    match props with
    | [] ->
      let inf_err_opt = Typing_error.intersect_opt ty_errs in
      (env, inf_err_opt, List.rev remain)
    | prop :: props ->
      let (prop_env, prop_inf_err, prop_remain) =
        tell ty_sub ty_super env prop on_error
      in
      (match prop_inf_err with
      | Some ty_err ->
        let ty_errs = ty_err :: ty_errs and remain = prop_remain @ remain in
        tell_exists ty_sub ty_super env ~ty_errs ~remain props on_error
      | _ -> (prop_env, None, List.rev remain))

  let prop_to_env ty_sub ty_super env prop on_error =
    let (env, ty_err_opt, _props') = tell ty_sub ty_super env prop on_error in
    (env, ty_err_opt)
end

module Subtype_tell : sig
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
      (ity_sub : internal_type)
      (ity_super : internal_type) : env * Typing_error.t option =
    let (env, prop) =
      Common.dispatch_constraint
        ~subtype_env
        ~this_ty
        ~sub_supportdyn
        ity_sub
        ity_super
        env
    in
    if not (TL.is_valid prop) then
      Typing_log.log_prop
        1
        (Reason.to_pos (reason ity_sub))
        "sub_type_inner"
        env
        prop;
    Subtype_trans.prop_to_env
      ity_sub
      ity_super
      env
      prop
      subtype_env.Subtype_env.on_error

  let sub_type_inner
      (env : env)
      ~(subtype_env : Subtype_env.t)
      ~(sub_supportdyn : Reason.t option)
      ~(this_ty : locl_ty option)
      (ity_sub : internal_type)
      (ity_super : internal_type) : env * Typing_error.t option =
    if Logging.should_log_subtype_i env ~level:1 ity_sub ity_super then
      Logging.log_subtype_i
        ~this_ty
        ~function_name:
          ("Typing_subtype.Subtype_tell.sub_type_inner"
          ^
          match subtype_env.Subtype_env.coerce with
          | Some TL.CoerceToDynamic -> " (dynamic aware)"
          | Some TL.CoerceFromDynamic -> " (treat dynamic as bottom)"
          | None -> "")
        env
        ity_sub
        ity_super
      @@ fun () ->
      sub_type_inner env ~subtype_env ~sub_supportdyn ~this_ty ity_sub ity_super
    else
      sub_type_inner env ~subtype_env ~sub_supportdyn ~this_ty ity_sub ity_super
end

(* == API =================================================================== *)

(* == Tell API ============================================================== *)

(* -- sub_type_i entry point ------------------------------------------------ *)

let sub_type_i env ?(is_coeffect = false) ty_sub ty_super on_error =
  let subtype_env =
    Subtype_env.create ~log_level:2 ~is_coeffect ~coerce:None on_error
  in
  let old_env = env in
  let (env, ty_err_opt) =
    Subtype_tell.sub_type_inner
      ~subtype_env
      ~sub_supportdyn:None
      ~this_ty:None
      env
      ty_sub
      ty_super
  in
  let env =
    Env.log_env_change "sub_type" old_env
    @@
    if Option.is_none ty_err_opt then
      env
    else
      old_env
  in
  (env, ty_err_opt)

(* -- sub_type entry point -------------------------------------------------- *)

let sub_type
    env
    ?(coerce = None)
    ?(is_coeffect = false)
    ?(ignore_readonly = false)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    on_error =
  let subtype_env =
    Subtype_env.create ~log_level:2 ~is_coeffect ~coerce on_error
  in
  let subtype_env = Subtype_env.{ subtype_env with ignore_readonly } in
  let old_env = env in
  let (env, ty_err_opt) =
    Subtype_tell.sub_type_inner
      ~subtype_env
      ~sub_supportdyn:None
      env
      ~this_ty:None
      (LoclType ty_sub)
      (LoclType ty_super)
  in
  let env =
    Env.log_env_change "sub_type" old_env
    @@
    if Option.is_none ty_err_opt then
      env
    else
      old_env
  in
  (env, ty_err_opt)

(* Entry point *)
let sub_type_or_fail env ty1 ty2 err_opt =
  sub_type env ty1 ty2
  @@ Option.map ~f:Typing_error.Reasons_callback.always err_opt

(* -- add_constraint(s) entry point ----------------------------------------- *)
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

let decompose_subtype_add_bound ~coerce env ty_sub ty_super =
  let (env, ty_sub) = Env.expand_type env ty_sub in
  let (env, ty_super) = Env.expand_type env ty_super in
  if Logging.should_log_subtype env ~level:2 ty_sub ty_super then
    Logging.log_subtype
      ~this_ty:None
      ~function_name:"Typing_subtype.decompose_subtype_add_bound"
      env
      ty_sub
      ty_super
    @@ fun () -> decompose_subtype_add_bound ~coerce env ty_sub ty_super
  else
    decompose_subtype_add_bound ~coerce env ty_sub ty_super

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
  let (env, prop) =
    Subtype.(
      simplify
        ~subtype_env:
          (Subtype_env.create
             ~require_soundness:false
             ~require_completeness:true
             ~log_level:2
             on_error)
        ~this_ty:None
        ~lhs:{ sub_supportdyn = None; ty_sub }
        ~rhs:{ super_like = false; super_supportdyn = false; ty_super }
        env)
  in
  decompose_subtype_add_prop env prop

let decompose_subtype
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Typing_error.Reasons_callback.t option) : env =
  let (env, ty_sub) = Env.expand_type env ty_sub in
  let (env, ty_super) = Env.expand_type env ty_super in
  if Logging.should_log_subtype env ~level:2 ty_sub ty_super then
    Logging.log_subtype
      ~this_ty:None
      ~function_name:"Typing_subtype.decompose_subtype"
      env
      ty_sub
      ty_super
    @@ fun () -> decompose_subtype env ty_sub ty_super on_error
  else
    decompose_subtype env ty_sub ty_super on_error

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

let add_constraint
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    on_error : env =
  let (env, ty_sub) = Env.expand_type env ty_sub in
  let (env, ty_super) = Env.expand_type env ty_super in
  if Logging.should_log_subtype env ~level:1 ty_sub ty_super then
    Logging.log_subtype
      ~this_ty:None
      ~function_name:"Typing_subtype.add_constraint"
      env
      ty_sub
      ty_super
    @@ fun () -> add_constraint env ck ty_sub ty_super on_error
  else
    add_constraint env ck ty_sub ty_super on_error

let add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) =
    add_constraint env ck ty1 ty2
    @@ Some (Typing_error.Reasons_callback.unify_error_at p)
  in
  List.fold_left constraints ~f:add_constraint ~init:env

(* -- sub_type_with_dynamic_as_bottom entry point --------------------------- *)
let sub_type_with_dynamic_as_bottom env ty_sub ty_super on_error =
  let old_env = env in
  let subtype_env =
    Subtype_env.create ~coerce:(Some TL.CoerceFromDynamic) ~log_level:2 on_error
  in
  let (env, ty_err) =
    Subtype_tell.sub_type_inner
      env
      ~subtype_env
      ~sub_supportdyn:None
      ~this_ty:None
      (LoclType ty_sub)
      (LoclType ty_super)
  in

  ( (if Option.is_some ty_err then
      old_env
    else
      env),
    ty_err )

let sub_type_with_dynamic_as_bottom env ty_sub ty_super on_error =
  let (env, ty_sub) = Env.expand_type env ty_sub in
  let (env, ty_super) = Env.expand_type env ty_super in
  if Logging.should_log_subtype env ~level:1 ty_sub ty_super then
    Logging.log_subtype
      ~this_ty:None
      ~function_name:"Typing_subtype.sub_type_with_dynamic_as_bottom"
      env
      ty_sub
      ty_super
    @@ fun () -> sub_type_with_dynamic_as_bottom env ty_sub ty_super on_error
  else
    sub_type_with_dynamic_as_bottom env ty_sub ty_super on_error

(* -- subtype_funs entry point ---------------------------------------------- *)
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
    Subtype.simplify_funs
      ~subtype_env:(Subtype_env.create ~log_level:2 ~coerce:None on_error)
      ~check_return
      ~for_override
      ~super_like:false
      (r_sub, ft_sub)
      (r_super, ft_super)
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

(* == Ask API =============================================================== *)

(* -- is_sub_type entry point ----------------------------------------------- *)

let is_sub_type env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  Subtype_ask.is_sub_type_alt_i
    ~require_completeness:false
    ~no_top_bottom:false
    ~coerce:None
    ~sub_supportdyn:None
    env
    (LoclType ty1)
    (LoclType ty2)
  = Some true

(* -- is_dynamic_aware_sub_type entry point --------------------------------- *)
let is_dynamic_aware_sub_type env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  Subtype_ask.is_sub_type_alt_i
    ~require_completeness:false
    ~no_top_bottom:false
    ~coerce:(Some TL.CoerceToDynamic)
    ~sub_supportdyn:None
    env
    (LoclType ty1)
    (LoclType ty2)
  = Some true

(* -- is_sub_type_for_union entry point ------------------------------------- *)
let is_sub_type_for_union_help env ?(coerce = None) ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  Subtype_ask.is_sub_type_alt_i
    ~require_completeness:false
    ~no_top_bottom:true
    ~coerce
    ~sub_supportdyn:None
    env
    (LoclType ty1)
    (LoclType ty2)
  = Some true

let is_sub_type_for_union env ty1 ty2 =
  is_sub_type_for_union_help ~coerce:None env ty1 ty2

(* Entry point *)
let is_sub_type_for_union_i env ty1 ty2 =
  Subtype_ask.is_sub_type_for_union_i ~coerce:None env ty1 ty2

(* -- is_sub_type_ignore_generic_params entry point ------------------------- *)
let is_sub_type_ignore_generic_params env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  Subtype_ask.is_sub_type_alt_i
  (* TODO(T121047839): Should this set a dedicated ignore_generic_param flag instead? *)
    ~require_completeness:true
    ~no_top_bottom:true
    ~coerce:None
    ~sub_supportdyn:None
    env
    (LoclType ty1)
    (LoclType ty2)
  = Some true

(* -- can_sub_type entry point ---------------------------------------------- *)
let can_sub_type env ty1 ty2 =
  let ( <> ) a b = not (Option.equal Bool.equal a b) in
  Subtype_ask.is_sub_type_alt_i
    ~require_completeness:false
    ~no_top_bottom:true
    ~coerce:None
    ~sub_supportdyn:None
    env
    (LoclType ty1)
    (LoclType ty2)
  <> Some false

(* -- is_type_disjoint entry point ------------------------------------------ *)

(* visited record which type variables & generics we've seen, to cut off cycles. *)
let rec is_type_disjoint_help visited env ty1 ty2 =
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
    is_type_disjoint_help
      visited
      env
      MakeType.(dict r (arraykey r) (mixed r))
      ty2
  | (_, Tshape _) ->
    let r = get_reason ty2 in
    is_type_disjoint_help
      visited
      env
      ty1
      MakeType.(dict r (arraykey r) (mixed r))
  | (Ttuple { t_required = tyl1; _ }, Ttuple { t_required = tyl2; _ }) ->
    (match List.exists2 ~f:(is_type_disjoint_help visited env) tyl1 tyl2 with
    | List.Or_unequal_lengths.Ok res -> res
    | List.Or_unequal_lengths.Unequal_lengths -> true)
  (* TODO optional and variadic components T201398626 T201398652 *)
  | (Ttuple _, _) ->
    (* Treat tuples as vec<mixed> because that implementation detail
       leaks through when doing is vec<_> on them, and they are also
       Traversable, Container, etc. along with varrays.
       We could translate varray to a more precise vec type with the same
       type argument, but it doesn't matter since disjointness doesn't ever
       look at it. *)
    let r = get_reason ty1 in
    is_type_disjoint_help visited env MakeType.(vec r (mixed r)) ty2
  | (_, Ttuple _) ->
    let r = get_reason ty2 in
    is_type_disjoint_help visited env ty1 MakeType.(vec r (mixed r))
  | (Tvec_or_dict (tyk, tyv), _) ->
    let r = get_reason ty1 in
    is_type_disjoint_help
      visited
      env
      MakeType.(union r [vec r tyv; dict r tyk tyv])
      ty2
  | (_, Tvec_or_dict (tyk, tyv)) ->
    let r = get_reason ty2 in
    is_type_disjoint_help
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
  | (Tunion tyl, _) ->
    List.for_all ~f:(is_type_disjoint_help visited env ty2) tyl
  | (_, Tunion tyl) ->
    List.for_all ~f:(is_type_disjoint_help visited env ty1) tyl
  | (Toption ty1, _) ->
    is_type_disjoint_help visited env ty1 ty2
    && is_type_disjoint_help visited env (MakeType.null Reason.none) ty2
  | (_, Toption ty2) ->
    is_type_disjoint_help visited env ty1 ty2
    && is_type_disjoint_help visited env ty1 (MakeType.null Reason.none)
  | (Tnonnull, _) ->
    is_sub_type_for_union_help env ty2 (MakeType.null Reason.none)
  | (_, Tnonnull) ->
    is_sub_type_for_union_help env ty1 (MakeType.null Reason.none)
  | (Tneg (_, IsTag (ClassTag c1)), Tclass ((_, c2), _, _tyl))
  | (Tclass ((_, c2), _, _tyl), Tneg (_, IsTag (ClassTag c1))) ->
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
  | (Tneg pred1, _) ->
    is_sub_type_for_union_help
      env
      ty2
      (Typing_refinement.TyPredicate.to_ty pred1)
  | (_, Tneg pred2) ->
    is_sub_type_for_union_help
      env
      ty1
      (Typing_refinement.TyPredicate.to_ty pred2)
  | (Tprim tp1, Tprim tp2) -> Subtype_negation.is_tprim_disjoint tp1 tp2
  | (Tclass ((_, cname), ex, _), Tprim (Aast.Tarraykey | Aast.Tstring))
  | (Tprim (Aast.Tarraykey | Aast.Tstring), Tclass ((_, cname), ex, _))
    when String.equal cname SN.Classes.cStringish && is_nonexact ex ->
    false
  (* A function type can be an instance of a Closure *)
  | (Tfun _, Tclass ((_, closure), _, _))
  | (Tclass ((_, closure), _, _), Tfun _)
    when String.equal SN.Classes.cClosure closure ->
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
  | (Tprim _, Tlabel _)
  | (Tlabel _, Tprim _) ->
    true
  | (Tfun _, Tlabel _)
  | (Tlabel _, Tfun _) ->
    true
  | (Tclass _, Tlabel _)
  | (Tlabel _, Tclass _) ->
    true
  | (Tlabel name1, Tlabel name2) -> not @@ String.equal name1 name2
  | (Tclass_args _ty, _)
  | (_, Tclass_args _ty) ->
    (* TODO(T199606542) Improve disjointnesss check, should be
     * disjoint iff objects of _ty would be disjoint *)
    false

(* incomplete, e.g., is_intersection_type_disjoint (?int & ?float) num *)
and is_intersection_type_disjoint visited_tvyars env inter_tyl ty =
  List.exists ~f:(is_type_disjoint_help visited_tvyars env ty) inter_tyl

and is_intersection_itype_set_disjoint visited_tvyars env inter_ty_set ty =
  ITySet.exists (is_itype_disjoint visited_tvyars env ty) inter_ty_set

and is_itype_disjoint visited_tvyars env (lty1 : locl_ty) (ity : internal_type)
    =
  match ity with
  | LoclType lty2 -> is_type_disjoint_help visited_tvyars env lty1 lty2
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

let is_type_disjoint env ty1 ty2 =
  is_type_disjoint_help (Tvid.Set.empty, SSet.empty) env ty1 ty2

(* -- Set function references ----------------------------------------------- *)
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
