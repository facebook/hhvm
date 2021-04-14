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
module Partial = Partial_provider
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

type subtype_env = {
  (* If set, finish as soon as we see a goal of the form T <: t or t <: T for generic parameter T *)
  ignore_generic_params: bool;
  (* If above is not set, maintain a visited goal set *)
  visited: VisitedGoals.t;
  no_top_bottom: bool;
  (* Coerce indicates whether subtyping should allow
   * coercion to or from dynamic. For coercion to dynamic, types that implement
   * dynamic are considered sub-types of dynamic. For coercion from dynamic,
   * dynamic is treated as a sub-type of all types.
   *)
  coerce: TL.coercion_direction option;
  on_error: Errors.error_from_reasons_callback;
}

let coercing_from_dynamic se =
  match se.coerce with
  | Some TL.CoerceFromDynamic -> true
  | _ -> false

let coercing_to_dynamic se =
  match se.coerce with
  | Some TL.CoerceToDynamic -> true
  | _ -> false

let make_subtype_env
    ?(ignore_generic_params = false)
    ?(no_top_bottom = false)
    ?(coerce = None)
    on_error =
  {
    ignore_generic_params;
    visited = VisitedGoals.empty;
    no_top_bottom;
    coerce;
    on_error;
  }

(* In typing_coercion.ml we sometimes check t1 <: t2 by adding dynamic
   to check t1 < t|dynamic. In that case, we use the Rdynamic_coercion
   reason so that we can detect it here and not print the dynamic if there
   is a type error. *)
let detect_attempting_dynamic_coercion_reason r ty =
  match r with
  | Reason.Rdynamic_coercion r ->
    (match ty with
    | LoclType lty ->
      (match get_node lty with
      | Tunion [t1; t2] ->
        (match (get_node t1, get_node t2) with
        | (Tdynamic, _) -> (r, LoclType t2)
        | (_, Tdynamic) -> (r, LoclType t1)
        | _ -> (r, ty))
      | _ -> (r, ty))
    | _ -> (r, ty))
  | _ -> (r, ty)

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
    log_with_level env "sub" level (fun () ->
        let types =
          [Log_type_i ("ty_sub", ty_sub); Log_type_i ("ty_super", ty_super)]
        in
        let types =
          Option.value_map this_ty ~default:types ~f:(fun ty ->
              Log_type ("this_ty", ty) :: types)
        in
        log_types
          (Reason.to_pos (reason ty_sub))
          env
          [Log_head (function_name, types)]))

let log_subtype ~this_ty ~function_name env ty_sub ty_super =
  log_subtype_i
    ~this_ty
    ~function_name
    env
    (LoclType ty_sub)
    (LoclType ty_super)

let is_final_and_not_contravariant env id =
  let class_def = Env.get_class env id in
  match class_def with
  | Some class_ty -> TUtils.class_is_final_and_not_contravariant class_ty
  | None -> false

(** Make all types appearing in the given type a Tany, e.g.
    - for A<B> return A<_>
    - for function(A): B return function (_): _
    *)
let anyfy env r ty =
  let anyfyer =
    object
      inherit Type_mapper.deep_type_mapper as super

      method! on_type env _ty = (env, mk (r, Typing_defs.make_tany ()))

      method go ty =
        let (_, ty) = super#on_type env ty in
        ty
    end
  in
  anyfyer#go ty

let find_type_with_exact_negation env tyl =
  let rec find env tyl acc_tyl =
    match tyl with
    | [] -> (env, None, acc_tyl)
    | ty :: tyl' ->
      let (env, non_ty) = TUtils.non env (get_reason ty) ty TUtils.ApproxDown in
      let nothing = MakeType.nothing Reason.none in
      if ty_equal non_ty nothing then
        find env tyl' (ty :: acc_tyl)
      else
        (env, Some non_ty, tyl' @ acc_tyl)
  in
  find env tyl []

let rec describe_ty_super env ty =
  let print ty =
    Typing_print.with_blank_tyvars (fun () ->
        Typing_print.full_strip_ns_i env ty)
  in
  let default () = print ty in
  match ty with
  | LoclType ty ->
    let (env, ty) = Env.expand_type env ty in
    (match get_node ty with
    | Tvar v ->
      let upper_bounds = ITySet.elements (Env.get_tyvar_upper_bounds env v) in
      (* The constraint graph is transitively closed so we can filter tyvars. *)
      let is_not_tyvar = function
        | LoclType t -> not (is_tyvar t)
        | _ -> true
      in
      let upper_bounds = List.filter upper_bounds ~f:is_not_tyvar in
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
            ^ ( String.concat ~sep:" & " (List.map tyl ~f:print)
              |> Markdown_lite.md_codify )
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

(** Process the constraint proposition. There should only be errors left now,
    i.e. empty disjunction with error functions we call here. *)
let rec process_simplify_subtype_result prop =
  match prop with
  | TL.IsSubtype (_ty1, _ty2) ->
    (* All subtypes should have been resolved *)
    failwith "unexpected subtype assertion"
  | TL.Coerce _ ->
    (* All coercions should have been resolved *)
    failwith "unexpected coercions assertion"
  | TL.Conj props ->
    (* Evaluates list from left-to-right so preserves order of conjuncts *)
    List.for_all ~f:process_simplify_subtype_result props
  | TL.Disj (f, []) ->
    f ();
    false
  | TL.Disj _ -> failwith "non-empty disjunction"

and simplify_subtype
    ~(subtype_env : subtype_env)
    ?(this_ty : locl_ty option = None)
    ty_sub
    ty_super =
  simplify_subtype_i ~subtype_env ~this_ty (LoclType ty_sub) (LoclType ty_super)

and default_subtype
    ~subtype_env ~(this_ty : locl_ty option) ~fail env ty_sub ty_super =
  let default env = (env, TL.IsSubtype (ty_sub, ty_super)) in
  let ( ||| ) = ( ||| ) ~fail in
  let (env, ty_super) = Env.expand_internal_type env ty_super in
  let (env, ty_sub) = Env.expand_internal_type env ty_sub in
  let default_subtype_inner env ty_sub ty_super =
    (* This inner function contains typing rules that are based solely on the subtype
     * if you need to pattern match on the super type it should NOT be included
     * here
     *)
    match ty_sub with
    | ConstraintType cty_sub ->
      begin
        match deref_constraint_type cty_sub with
        | (_, TCunion (lty_sub, cty_sub)) ->
          env
          |> simplify_subtype_i ~subtype_env (LoclType lty_sub) ty_super
          &&& simplify_subtype_i ~subtype_env (ConstraintType cty_sub) ty_super
        | (_, TCintersection (lty_sub, cty_sub)) ->
          env
          |> simplify_subtype_i ~subtype_env (LoclType lty_sub) ty_super
          ||| simplify_subtype_i ~subtype_env (ConstraintType cty_sub) ty_super
        | _ -> invalid ~fail env
      end
    | LoclType lty_sub ->
      (*
       * t1 | ... | tn <: t
       *   if and only if
       * t1 <: t /\ ... /\ tn <: t
       * We want this even if t is a type variable e.g. consider
       *   int | v <: v
       *)
      begin
        match deref lty_sub with
        | (_, Tunion tyl) ->
          List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_sub ->
              res &&& simplify_subtype_i ~subtype_env (LoclType ty_sub) ty_super)
        | (_, Terr) ->
          if subtype_env.no_top_bottom then
            default env
          else
            valid env
        | (_, Tvar _) -> default env
        | (r_sub, Tintersection tyl) ->
          (* A & B <: C iif A <: C | !B *)
          (match find_type_with_exact_negation env tyl with
          | (env, Some non_ty, tyl) ->
            let (env, ty_super) =
              TUtils.union_i env (get_reason non_ty) ty_super non_ty
            in
            let ty_sub = MakeType.intersection r_sub tyl in
            simplify_subtype_i ~subtype_env (LoclType ty_sub) ty_super env
          | _ ->
            (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
             * not complete.
             *)
            List.fold_left
              tyl
              ~init:(env, TL.invalid ~fail)
              ~f:(fun res ty_sub ->
                let ty_sub = LoclType ty_sub in
                res ||| simplify_subtype_i ~subtype_env ~this_ty ty_sub ty_super))
        | (_, Tgeneric (name_sub, tyargs)) ->
          (* TODO(T69551141) handle type arguments. right now, just passin tyargs to
             Env.get_upper_bounds *)
          ( if subtype_env.ignore_generic_params then
            default env
          else
            (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
             *)
            match
              VisitedGoals.try_add_visited_generic_sub
                subtype_env.visited
                name_sub
                ty_super
            with
            | None -> invalid ~fail env
            | Some new_visited ->
              let subtype_env = { subtype_env with visited = new_visited } in
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
                  |> simplify_subtype_i ~subtype_env ~this_ty tmixed ty_super
                | [ty] ->
                  simplify_subtype_i
                    ~subtype_env
                    ~this_ty
                    (LoclType ty)
                    ty_super
                    env
                | ty :: tyl ->
                  env
                  |> try_bounds tyl
                  ||| simplify_subtype_i
                        ~subtype_env
                        ~this_ty
                        (LoclType ty)
                        ty_super
              in
              env
              |> try_bounds
                   (Typing_set.elements
                      (Env.get_upper_bounds env name_sub tyargs)) )
          |> (* Turn error into a generic error about the type parameter *)
          if_unsat (invalid ~fail)
        | (_, Tdynamic) when coercing_from_dynamic subtype_env -> valid env
        | (_, Taccess _) -> invalid ~fail env
        | _ -> invalid ~fail env
      end
  in
  (* We further refine the default subtype case for rules that apply to all
   * LoclTypes but not to ConstraintTypes
   *)
  match ty_super with
  | LoclType lty_super ->
    (match ty_sub with
    | ConstraintType _ -> default_subtype_inner env ty_sub ty_super
    | LoclType lty_sub ->
      begin
        match deref lty_sub with
        | (_, Tvar _) ->
          begin
            match subtype_env.coerce with
            | Some cd -> (env, TL.Coerce (cd, lty_sub, lty_super))
            | None -> default_subtype_inner env ty_sub ty_super
          end
        | (_, Tnewtype (_, _, ty)) ->
          simplify_subtype ~subtype_env ~this_ty ty lty_super env
        | (_, Tdependent (_, ty)) ->
          let this_ty = Option.first_some this_ty (Some lty_sub) in
          simplify_subtype ~subtype_env ~this_ty ty lty_super env
        | (r_sub, Tany _) ->
          if subtype_env.no_top_bottom then
            default env
          else
            let ty_sub = anyfy env r_sub lty_super in
            simplify_subtype ~subtype_env ~this_ty ty_sub lty_super env
        | (r_sub, Tprim Nast.Tvoid) ->
          let r =
            Reason.Rimplicit_upper_bound (Reason.to_pos r_sub, "?nonnull")
          in
          simplify_subtype
            ~subtype_env
            ~this_ty
            (MakeType.mixed r)
            lty_super
            env
          |> if_unsat (invalid ~fail)
        | _ -> default_subtype_inner env ty_sub ty_super
      end)
  | ConstraintType _ -> default_subtype_inner env ty_sub ty_super

(* Attempt to "solve" a subtype assertion ty_sub <: ty_super.
 * Return a proposition that is equivalent, but simpler, than
 * the original assertion. Fail with Unsat error_function if
 * the assertion is unsatisfiable. Some examples:
 *   string <: arraykey  ==>  True    (represented as Conj [])
 * (For covariant C and a type variable v)
 *   C<string> <: C<v>   ==>  string <: v
 * (Assuming that C does *not* implement interface J)
 *   C <: J              ==>  Unsat _
 * (Assuming we have T <: D in tpenv, and class D implements I)
 *   vec<T> <: vec<I>    ==>  True
 * This last one would be left as T <: I if subtype_env.ignore_generic_params=true
 *)
and simplify_subtype_i
    ~(subtype_env : subtype_env)
    ?(this_ty : locl_ty option = None)
    (ty_sub : internal_type)
    (ty_super : internal_type)
    env : env * TL.subtype_prop =
  log_subtype_i
    ~level:2
    ~this_ty
    ~function_name:"simplify_subtype"
    env
    ty_sub
    ty_super;
  let (env, ety_super) = Env.expand_internal_type env ty_super in
  let (env, ety_sub) = Env.expand_internal_type env ty_sub in
  let fail_with_suffix suffix =
    let r_super = reason ety_super in
    let r_sub = reason ety_sub in
    let (r_super, ety_super) =
      detect_attempting_dynamic_coercion_reason r_super ety_super
    in
    let ty_super_descr = describe_ty_super env ety_super in
    let ty_sub_descr =
      Markdown_lite.md_codify
        (Typing_print.with_blank_tyvars (fun () ->
             Typing_print.full_strip_ns_i env ety_sub))
    in
    let (ty_super_descr, ty_sub_descr) =
      if String.equal ty_super_descr ty_sub_descr then
        ( "exactly the type " ^ ty_super_descr,
          "the nonexact type " ^ ty_sub_descr )
      else
        (ty_super_descr, ty_sub_descr)
    in
    let left = Reason.to_string ("Expected " ^ ty_super_descr) r_super in
    let right = Reason.to_string ("But got " ^ ty_sub_descr) r_sub @ suffix in
    match (r_super, r_sub) with
    | (Reason.Rcstr_on_generics (p, tparam), _)
    | (_, Reason.Rcstr_on_generics (p, tparam)) ->
      Errors.violated_constraint p tparam left right subtype_env.on_error
    | _ -> subtype_env.on_error (left @ right)
  in
  let fail () = fail_with_suffix [] in
  let ( ||| ) = ( ||| ) ~fail in
  (* We *know* that the assertion is unsatisfiable *)
  let invalid_env env = invalid ~fail env in
  let invalid_env_with env f = invalid ~fail:f env in
  (* We don't know whether the assertion is valid or not *)
  let default env = (env, TL.IsSubtype (ety_sub, ety_super)) in
  let default_subtype env =
    default_subtype ~subtype_env ~this_ty ~fail env ety_sub ety_super
  in
  match ety_super with
  (* First deal with internal constraint types *)
  | ConstraintType cty_super ->
    let using_new_method_call_inference =
      TypecheckerOptions.method_call_inference (Env.get_tcopt env)
    in
    begin
      match deref_constraint_type cty_super with
      | (_, TCintersection (lty, cty)) ->
        (match ety_sub with
        | LoclType t when is_union t -> default_subtype env
        | ConstraintType t when is_constraint_type_union t ->
          default_subtype env
        | _ ->
          env
          |> simplify_subtype_i ~subtype_env ty_sub (LoclType lty)
          &&& simplify_subtype_i ~subtype_env ty_sub (ConstraintType cty))
      | (_, TCunion (maybe_null, maybe_has_member))
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
          simplify_subtype_has_member
            ~subtype_env
            ~this_ty
            ~nullsafe:r_null
            ~fail
            ety_sub
            (r, has_member_ty)
            env
        | _ -> invalid_env env (* Not possible due to guard in parent match *))
      | (_, TCunion (lty_super, cty_super)) ->
        (match ety_sub with
        | ConstraintType cty when is_constraint_type_union cty ->
          default_subtype env
        | ConstraintType _ ->
          env
          |> simplify_subtype_i ~subtype_env ty_sub (LoclType lty_super)
          ||| simplify_subtype_i ~subtype_env ty_sub (ConstraintType cty_super)
          ||| default_subtype
        | LoclType lty ->
          (match deref lty with
          | (r, Toption ty) ->
            let ty_null = MakeType.null r in
            if_unsat
              invalid_env
              (simplify_subtype_i
                 ~subtype_env
                 ~this_ty
                 (LoclType ty_null)
                 ty_super
                 env)
            &&& simplify_subtype_i ~subtype_env ~this_ty (LoclType ty) ty_super
          | (_, (Tintersection _ | Tunion _ | Terr | Tvar _)) ->
            default_subtype env
          | _ ->
            env
            |> simplify_subtype_i ~subtype_env ty_sub (LoclType lty_super)
            ||| simplify_subtype_i
                  ~subtype_env
                  ty_sub
                  (ConstraintType cty_super)
            ||| default_subtype))
      | (r_super, Tdestructure { d_required; d_optional; d_variadic; d_kind })
        ->
        (* List destructuring *)
        let destructure_array t env =
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
            invalid_env_with env (fun () ->
                Errors.unpack_array_required_argument
                  (Reason.to_pos r_super)
                  fpos
                  subtype_env.on_error)
          | (SplatUnpack, [], None) ->
            invalid_env_with env (fun () ->
                Errors.unpack_array_variadic_argument
                  (Reason.to_pos r_super)
                  fpos
                  subtype_env.on_error)
          | (SplatUnpack, [], Some _)
          | (ListDestructure, _, _) ->
            List.fold d_required ~init:(env, TL.valid) ~f:(fun res ty_dest ->
                res &&& simplify_subtype ~subtype_env ~this_ty t ty_dest)
            &&& fun env ->
            List.fold d_optional ~init:(env, TL.valid) ~f:(fun res ty_dest ->
                res &&& simplify_subtype ~subtype_env ~this_ty t ty_dest)
            &&& fun env ->
            Option.value_map ~default:(env, TL.valid) d_variadic ~f:(fun vty ->
                simplify_subtype ~subtype_env ~this_ty t vty env)
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
            invalid_env_with env (fun () ->
                f
                  (prefix + len_required)
                  (prefix + len_ts)
                  epos
                  fpos
                  subtype_env.on_error)
          in
          if len_ts < len_required then
            arity_error Errors.typing_too_few_args_w_callback
          else
            let len_optional = List.length d_optional in
            let (ts_required, remain) = List.split_n ts len_required in
            let (ts_optional, ts_variadic) = List.split_n remain len_optional in
            List.fold2_exn
              ts_required
              d_required
              ~init:(env, TL.valid)
              ~f:(fun res ty ty_dest ->
                res &&& simplify_subtype ~subtype_env ~this_ty ty ty_dest)
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
                res &&& simplify_subtype ~subtype_env ~this_ty ty ty_dest)
            &&& fun env ->
            match (ts_variadic, d_variadic) with
            | (vars, Some vty) ->
              List.fold vars ~init:(env, TL.valid) ~f:(fun res ty ->
                  res &&& simplify_subtype ~subtype_env ~this_ty ty vty)
            | ([], None) -> valid env
            | (_, None) ->
              (* Elements remain but we have nowhere to put them *)
              arity_error Errors.typing_too_many_args_w_callback
        in

        begin
          match ety_sub with
          | ConstraintType _ -> default_subtype env
          | LoclType ty_sub ->
            (match deref ty_sub with
            | (r, Ttuple tyl) -> env |> destructure_tuple r tyl
            | (r, Tclass ((_, x), _, tyl))
              when String.equal x SN.Collections.cPair ->
              env |> destructure_tuple r tyl
            | (_, Tclass ((_, x), _, [elt_type]))
              when String.equal x SN.Collections.cVector
                   || String.equal x SN.Collections.cImmVector
                   || String.equal x SN.Collections.cVec
                   || String.equal x SN.Collections.cConstVector ->
              env |> destructure_array elt_type
            | (_, Tvarray elt_type) -> env |> destructure_array elt_type
            | (_, Tdynamic) -> env |> destructure_array ty_sub
            (* TODO: should remove these any cases *)
            | (r, Tany _) ->
              let any = mk (r, Typing_defs.make_tany ()) in
              env |> destructure_array any
            | (_, (Tunion _ | Tintersection _ | Tgeneric _ | Tvar _)) ->
              (* TODO(T69551141) handle type arguments of Tgeneric? *)
              default_subtype env
            | _ ->
              begin
                match d_kind with
                | SplatUnpack ->
                  (* Allow splatting of arbitrary Traversables *)
                  let (env, ty_inner) = Env.fresh_type env Pos.none in
                  let traversable = MakeType.traversable r_super ty_inner in
                  env
                  |> simplify_subtype ~subtype_env ~this_ty ty_sub traversable
                  &&& destructure_array ty_inner
                | ListDestructure ->
                  let ty_sub_descr =
                    Typing_print.with_blank_tyvars (fun () ->
                        Typing_print.full_strip_ns env ty_sub)
                  in
                  default_subtype env
                  |> if_unsat @@ fun env ->
                     invalid_env_with env (fun () ->
                         Errors.invalid_destructure
                           (Reason.to_pos r_super)
                           (get_pos ty_sub)
                           ty_sub_descr
                           subtype_env.on_error)
              end)
        end
      | (r, Thas_member has_member_ty) ->
        simplify_subtype_has_member
          ~subtype_env
          ~this_ty
          ~fail
          ety_sub
          (r, has_member_ty)
          env
    end
  (* Next deal with all locl types *)
  | LoclType ty_super ->
    (match deref ty_super with
    | (_, Terr) ->
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype env
      | ConstraintType _ ->
        if subtype_env.no_top_bottom then
          default env
        else
          valid env
      | LoclType lty ->
        (match deref lty with
        | (_, Tunion _) -> default_subtype env
        | (_, Terr) -> valid env
        | _ ->
          if subtype_env.no_top_bottom then
            default env
          else
            valid env))
    | (_, Tvar var_super) ->
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype env
      | ConstraintType _ -> default env
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (_, (Tunion _ | Terr)) -> default_subtype env
        | (_, Tdynamic) when coercing_from_dynamic subtype_env ->
          default_subtype env
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
            |> simplify_subtype ~subtype_env ~this_ty t ty_super
            &&& simplify_subtype ~subtype_env ~this_ty ty_null ty_super)
        | (_, Tvar var_sub) when Ident.equal var_sub var_super -> valid env
        | _ ->
          begin
            match subtype_env.coerce with
            | Some cd -> (env, TL.Coerce (cd, ty_sub, ty_super))
            | None -> default env
          end))
    | (_, Tintersection tyl) ->
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype env
      | LoclType lty when is_union lty -> default_subtype env
      (* t <: (t1 & ... & tn)
       *   if and only if
       * t <: t1 /\  ... /\ t <: tn
       *)
      | _ ->
        List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_super ->
            let ty_super = LoclType ty_super in
            res &&& simplify_subtype_i ~subtype_env ~this_ty ty_sub ty_super))
    (* Empty union encodes the bottom type nothing *)
    | (_, Tunion []) -> default_subtype env
    (* ty_sub <: union{ty_super'} iff ty_sub <: ty_super' *)
    | (_, Tunion [ty_super']) ->
      simplify_subtype_i ~subtype_env ~this_ty ty_sub (LoclType ty_super') env
    | (_, Tunion (_ :: _ as tyl_super)) ->
      let simplify_sub_union env ty_sub tyl_super =
        (* It's sound to reduce t <: t1 | t2 to (t <: t1) || (t <: t2). But
         * not complete e.g. consider (t1 | t3) <: (t1 | t2) | (t2 | t3).
         * But we deal with unions on the left first (see case above), so this
         * particular situation won't arise.
         * TODO: identify under what circumstances this reduction is complete.
         *)
        let rec try_each tys env =
          match tys with
          | [] ->
            (match ty_sub with
            | LoclType lty ->
              begin
                match get_node lty with
                | Tnewtype _
                | Tdependent _
                | Tgeneric _ ->
                  default_subtype env
                | _ -> invalid_env env
              end
            | _ -> invalid_env env)
          | ty :: tys ->
            let ty = LoclType ty in
            env
            |> simplify_subtype_i ~subtype_env ~this_ty ty_sub ty
            ||| try_each tys
        in
        try_each tyl_super env
      in
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype env
      | ConstraintType _ -> simplify_sub_union env ty_sub tyl_super
      | LoclType lty_sub ->
        (match deref lty_sub with
        | (_, (Tunion _ | Terr | Tvar _)) -> default_subtype env
        | (_, Tgeneric _) when subtype_env.ignore_generic_params ->
          default_subtype env
        (* Num is not atomic: it is equivalent to int|float. The rule below relies
         * on ty_sub not being a union e.g. consider num <: arraykey | float, so
         * we break out num first.
         *)
        | (r, Tprim Nast.Tnum) ->
          let ty_float = MakeType.float r and ty_int = MakeType.int r in
          env
          |> simplify_subtype ~subtype_env ~this_ty ty_float ty_super
          &&& simplify_subtype ~subtype_env ~this_ty ty_int ty_super
        (* Likewise, reduce nullable on left to a union *)
        | (r, Toption ty) ->
          let ty_null = MakeType.null r in
          if_unsat
            invalid_env
            (simplify_subtype_i
               ~subtype_env
               ~this_ty
               (LoclType ty_null)
               ety_super
               env)
          &&& simplify_subtype_i ~subtype_env ~this_ty (LoclType ty) ety_super
        | (_, Tintersection tyl)
          when let (_, non_ty_opt, _) = find_type_with_exact_negation env tyl in
               Option.is_some non_ty_opt ->
          default_subtype env
        | (_, Tintersection tyl_sub) ->
          let simplify_super_intersection env tyl_sub ty_super =
            (* It's sound to reduce t1 & t2 <: t to (t1 <: t) || (t2 <: t), but
             * not complete.
             *)
            List.fold_left
              tyl_sub
              ~init:(env, TL.invalid ~fail)
              ~f:(fun res ty_sub ->
                let ty_sub = LoclType ty_sub in
                res ||| simplify_subtype_i ~subtype_env ~this_ty ty_sub ty_super)
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
          *)
          if List.exists tyl_super ~f:(Typing_utils.is_tintersection env) then
            simplify_sub_union env ty_sub tyl_super
          else if List.exists tyl_sub ~f:(Typing_utils.is_tunion env) then
            simplify_super_intersection env tyl_sub (LoclType ty_super)
          else
            simplify_sub_union env ty_sub tyl_super
        | _ -> simplify_sub_union env ty_sub tyl_super))
    | (r_super, Toption arg_ty_super) ->
      let (env, ety) = Env.expand_type env arg_ty_super in
      (* Toption(Tnonnull) encodes mixed, which is our top type.
       * Everything subtypes mixed *)
      if is_nonnull ety then
        valid env
      else (
        match ety_sub with
        | ConstraintType _ -> default_subtype env
        | LoclType lty_sub ->
          (match (deref lty_sub, get_node ety) with
          | ((_, Tnewtype (name_sub, _, _)), Tnewtype (name_sup, _, _))
            when String.equal name_sup name_sub ->
            simplify_subtype ~subtype_env ~this_ty lty_sub arg_ty_super env
          (* A <: ?B iif A & nonnull <: B
      Only apply if B is a type variable or an intersection, to avoid oscillating
      forever between this case and the previous one. *)
          | ((_, Tintersection tyl), (Tintersection _ | Tvar _))
            when let (_, non_ty_opt, _) =
                   find_type_with_exact_negation env tyl
                 in
                 Option.is_none non_ty_opt ->
            let (env, ty_sub') =
              Inter.intersect_i env r_super ty_sub (MakeType.nonnull r_super)
            in
            simplify_subtype_i ~subtype_env ty_sub' (LoclType arg_ty_super) env
          (* null is the type of null and is a subtype of any option type. *)
          | ((_, Tprim Nast.Tnull), _) -> valid env
          (* ?ty_sub' <: ?ty_super' iff ty_sub' <: ?ty_super'. Reasoning:
           * If ?ty_sub' <: ?ty_super', then from ty_sub' <: ?ty_sub' (widening) and transitivity
           * of <: it follows that ty_sub' <: ?ty_super'.  Conversely, if ty_sub' <: ?ty_super', then
           * by covariance and idempotence of ?, we have ?ty_sub' <: ??ty_sub' <: ?ty_super'.
           * Therefore, this step preserves the set of solutions.
           *)
          | ((_, Toption ty_sub'), _) ->
            simplify_subtype ~subtype_env ~this_ty ty_sub' ty_super env
          (* We do not want to decompose Toption for these cases *)
          | ((_, (Tvar _ | Tunion _ | Tintersection _)), _) ->
            default_subtype env
          | ((_, Tgeneric _), _) when subtype_env.ignore_generic_params ->
            (* TODO(T69551141) handle type arguments ? *)
            default_subtype env
          (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
           * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
           * true as well.  We can fold the case where t1 is unconstrained
           * into the case analysis below.
           *)
          | ((_, (Tnewtype _ | Tdependent _ | Tgeneric _ | Tprim Nast.Tvoid)), _)
            ->
            (* TODO(T69551141) handle type arguments? *)
            env
            |> simplify_subtype ~subtype_env ~this_ty lty_sub arg_ty_super
            ||| default_subtype
          (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
           * must also have ty_sub <: ty_super'.  The converse follows by
           * widening and transitivity.  Therefore, this step preserves the set
           * of solutions.
           *)
          | ((_, Tunapplied_alias _), _) ->
            Typing_defs.error_Tunapplied_alias_in_illegal_context ()
          | ( ( _,
                ( Tdynamic | Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _
                | Tobject | Tclass _ | Tvarray _ | Tdarray _
                | Tvarray_or_darray _ | Tvec_or_dict _ | Tany _ | Terr
                | Taccess _ ) ),
              _ ) ->
            simplify_subtype ~subtype_env ~this_ty lty_sub arg_ty_super env)
      )
    | (r_super, Tdependent (d_sup, bound_sup)) ->
      let (env, bound_sup) = Env.expand_type env bound_sup in
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType ty_sub ->
        (match (deref ty_sub, get_node bound_sup) with
        | ((_, Tclass _), Tclass ((_, x), _, _))
          when is_final_and_not_contravariant env x ->
          (* For final class C, there is no difference between `this as X` and `X`,
           * and `expr<#n> as X` and `X`.
           * But we need to take care with contravariant classes, since we can't
           * statically guarantee their runtime type.
           *)
          simplify_subtype ~subtype_env ~this_ty ty_sub bound_sup env
        | ((r_sub, Tclass ((_, y), _, _)), Tclass (((_, x) as id), _, tyl_super))
          ->
          let fail =
            if String.equal x y then
              fun () ->
            let p = Reason.to_pos r_sub in
            fail_with_suffix (Errors.this_final id p)
            else
              fail
          in

          let class_def = Env.get_class env x in
          (match (d_sup, class_def) with
          | (DTthis, Some class_ty) ->
            let tolerate_wrong_arity =
              not (Partial.should_check_error (Env.get_mode env) 4029)
            in
            let tyl_super =
              if List.is_empty tyl_super && tolerate_wrong_arity then
                List.map (Cls.tparams class_ty) (fun _ ->
                    mk (r_super, Typing_defs.make_tany ()))
              else
                tyl_super
            in
            let ety_env =
              {
                type_expansions = Typing_defs.Type_expansions.empty;
                substs =
                  TUtils.make_locl_subst_for_class_tparams class_ty tyl_super;
                this_ty = Option.value this_ty ~default:ty_super;
                on_error = Errors.ignore_error;
              }
            in
            let lower_bounds_super = Cls.lower_bounds_on_this class_ty in
            let rec try_constraints lower_bounds_super env =
              match lower_bounds_super with
              | [] -> invalid_env_with env fail
              | ty_super :: lower_bounds_super ->
                let (env, ty_super) = Phase.localize ~ety_env env ty_super in
                env
                |> simplify_subtype ~subtype_env ~this_ty ty_sub ty_super
                ||| try_constraints lower_bounds_super
            in
            try_constraints lower_bounds_super env
          | _ -> invalid_env_with env fail)
        | ((_, Tdependent (d_sub, bound_sub)), _) ->
          let this_ty = Option.first_some this_ty (Some ty_sub) in
          (* Dependent types are identical but bound might be different *)
          if equal_dependent_type d_sub d_sup then
            simplify_subtype ~subtype_env ~this_ty bound_sub bound_sup env
          else
            simplify_subtype ~subtype_env ~this_ty bound_sub ty_super env
        | _ -> default_subtype env))
    | (_, Taccess _) -> invalid_env env
    | (_, Tgeneric (name_super, tyargs_super)) ->
      (* TODO(T69551141) handle type arguments. Right now, only passing tyargs_super to
         Env.get_lower_bounds *)
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      (* If subtype and supertype are the same generic parameter, we're done *)
      | LoclType ty_sub ->
        (match get_node ty_sub with
        | Tgeneric (name_sub, tyargs_sub) when String.equal name_sub name_super
          ->
          if List.is_empty tyargs_super then
            valid env
          else
            (* TODO(T69931993) Type parameter env must carry variance information *)
            let variance_reifiedl =
              List.map tyargs_sub (fun _ -> (Ast_defs.Invariant, Aast.Erased))
            in
            simplify_subtype_variance
              ~subtype_env
              name_sub
              variance_reifiedl
              tyargs_sub
              tyargs_super
              env
        (* When decomposing subtypes for the purpose of adding bounds on generic
         * parameters to the context, (so seen_generic_params = None), leave
         * subtype so that the bounds get added *)
        | Tvar _
        | Tunion _
        | Terr ->
          default_subtype env
        | _ ->
          if subtype_env.ignore_generic_params then
            default env
          else (
            (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
             *)
            match
              VisitedGoals.try_add_visited_generic_super
                subtype_env.visited
                ety_sub
                name_super
            with
            | None -> invalid_env env
            | Some new_visited ->
              let subtype_env = { subtype_env with visited = new_visited } in
              (* Collect all the lower bounds ("super" constraints) on the
               * generic parameter, and check ty_sub against each of them in turn
               * until one of them succeeds *)
              let rec try_bounds tyl env =
                match tyl with
                | [] -> default_subtype env
                | ty :: tyl ->
                  env
                  |> simplify_subtype ~subtype_env ~this_ty ty_sub ty
                  ||| try_bounds tyl
              in
              (* Turn error into a generic error about the type parameter *)
              env
              |> try_bounds
                   (Typing_set.elements
                      (Env.get_lower_bounds env name_super tyargs_super))
              |> if_unsat invalid_env
          )))
    | (_, Tnonnull) ->
      (match ety_sub with
      | ConstraintType cty ->
        begin
          match deref_constraint_type cty with
          | (_, (Thas_member _ | Tdestructure _)) -> valid env
          | _ -> default_subtype env
        end
      | LoclType lty ->
        (match deref lty with
        | ( _,
            ( Tprim
                Nast.(
                  ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
                  | Tarraykey | Tnoreturn ))
            | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tobject | Tclass _
            | Tvarray _ | Tdarray _ | Tvarray_or_darray _ | Tvec_or_dict _
            | Taccess _ ) ) ->
          valid env
        | _ -> default_subtype env))
    | (_, Tdynamic)
      when TypecheckerOptions.enable_sound_dynamic env.genv.tcopt
           && coercing_to_dynamic subtype_env ->
      (match ety_sub with
      | ConstraintType _cty ->
        (* TODO *)
        default_subtype env
      | LoclType lty_sub ->
        (match deref lty_sub with
        | (_, Tdynamic)
        | (_, Tany _)
        | (_, Terr)
        | ( _,
            Tprim
              Aast_defs.(
                ( Tnull | Tint | Tbool | Tfloat | Tstring | Tnum | Tarraykey
                | Tvoid )) ) ->
          valid env
        | (_, Tprim Aast_defs.(Tresource | Tnoreturn))
        | (_, Tnonnull)
        | (_, Tshape (Open_shape, _))
        | (_, Tvar _)
        | (_, Tunapplied_alias _)
        | (_, Tnewtype _)
        | (_, Tdependent _)
        | (_, Tobject)
        | (_, Taccess _)
        | (_, Tunion _)
        | (_, Tintersection _)
        | (_, Tgeneric _) ->
          default_subtype env
        | (_, Tdarray (_, ty))
        | (_, Tvarray ty)
        | (_, Tvec_or_dict (_, ty))
        | (_, Tvarray_or_darray (_, ty)) ->
          simplify_subtype ~subtype_env ty ty_super env
        (* Implement (function(dynamic, ..., dynamic): dynamic <: dynamic *)
        | (r, Tfun ft) ->
          simplify_subtype_fun_dynamic ~subtype_env r ft ty_super env
        | (_, Toption ty) ->
          (match deref ty with
          (* Special case mixed <: dynamic for better error message *)
          | (_, Tnonnull) -> invalid_env env
          | _ -> simplify_subtype ~subtype_env ty ty_super env)
        | (_, Ttuple tyl) ->
          List.fold_left
            ~init:(env, TL.valid)
            ~f:(fun res ty_sub ->
              res &&& simplify_subtype ~subtype_env ty_sub ty_super)
            tyl
        | (_, Tshape (Closed_shape, sftl)) ->
          List.fold_left
            ~init:(env, TL.valid)
            ~f:(fun res sft ->
              res &&& simplify_subtype ~subtype_env sft.sft_ty ty_super)
            (TShapeMap.values sftl)
        | (_, Tclass ((_, class_id), _, tyargs)) ->
          let class_def_sub = Typing_env.get_class env class_id in
          (match class_def_sub with
          | None ->
            (* This should have been caught already in the naming phase *)
            valid env
          | Some class_sub ->
            let class_name = Cls.name class_sub in
            if Cls.get_implements_dynamic class_sub then
              valid env
            else if String.equal class_name SN.Collections.cKeyset then
              (* No need to check the argument since it's an arraykey *)
              valid env
            else if String.equal class_name SN.Collections.cVec then
              match tyargs with
              | [tyarg] -> simplify_subtype ~subtype_env tyarg ty_super env
              | _ ->
                (* This ill-formed type should have been caught earlier *)
                valid env
            else if String.equal class_name SN.Collections.cDict then
              match tyargs with
              | [_tykey; tyval] ->
                (* No need to check the key argument since it's an arraykey *)
                simplify_subtype ~subtype_env tyval ty_super env
              | _ ->
                (* This ill-formed type should have been caught earlier *)
                valid env
            else if String.equal class_name SN.Classes.cAwaitable then
              match tyargs with
              | [tyarg] -> simplify_subtype ~subtype_env tyarg ty_super env
              | _ ->
                (* This ill-formed type should have been caught earlier *)
                valid env
            else
              default_subtype env)))
    | (_, Tdynamic) ->
      (match ety_sub with
      | LoclType lty when is_dynamic lty -> valid env
      | ConstraintType _
      | LoclType _ ->
        default_subtype env)
    | (_, Tprim prim_ty) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match (deref lty, prim_ty) with
        | ((_, Tprim (Nast.Tint | Nast.Tfloat)), Nast.Tnum) -> valid env
        | ((_, Tprim (Nast.Tint | Nast.Tstring)), Nast.Tarraykey) -> valid env
        | ((_, Tprim prim_sub), _) when Aast.equal_tprim prim_sub prim_ty ->
          valid env
        | ((_, Toption arg_ty_sub), Nast.Tnull) ->
          simplify_subtype ~subtype_env ~this_ty arg_ty_sub ty_super env
        | (_, _) -> default_subtype env))
    | (_, Tobject) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      (* Any class type is a subtype of object *)
      | LoclType lty ->
        (match get_node lty with
        | Tobject
        | Tclass _ ->
          valid env
        | _ -> default_subtype env))
    | (r_super, Tany _) ->
      (match ety_sub with
      | ConstraintType cty ->
        begin
          match deref_constraint_type cty with
          | (_, (TCunion _ | TCintersection _)) -> default_subtype env
          | _ -> valid env
        end
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (_, Tany _) -> valid env
        | (_, (Tunion _ | Tintersection _ | Tvar _)) -> default_subtype env
        | _ when subtype_env.no_top_bottom -> default env
        (* If ty_sub contains other types, e.g. C<T>, make this a subtype assertion on
            those inner types and `any`. For example transform the assertion
              C<D> <: Tany
            into
              C<D> <: C<Tany>
            which might become
              D <: Tany
            if say C is covariant.
            *)
        | _ ->
          let ty_super = anyfy env r_super ty_sub in
          simplify_subtype ~subtype_env ~this_ty ty_sub ty_super env))
    | (r_super, Tfun ft_super) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (r_sub, Tfun ft_sub) ->
          simplify_subtype_funs
            ~subtype_env
            ~check_return:true
            r_sub
            ft_sub
            r_super
            ft_super
            env
        | _ -> default_subtype env))
    | (_, Ttuple tyl_super) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      (* (t1,...,tn) <: (u1,...,un) iff t1<:u1, ... , tn <: un *)
      | LoclType lty ->
        (match get_node lty with
        | Ttuple tyl_sub
          when Int.equal (List.length tyl_super) (List.length tyl_sub) ->
          wfold_left2
            (fun res ty_sub ty_super ->
              res &&& simplify_subtype ~subtype_env ty_sub ty_super)
            (env, TL.valid)
            tyl_sub
            tyl_super
        | _ -> default_subtype env))
    | (r_super, Tshape (shape_kind_super, fdm_super)) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (r_sub, Tshape (shape_kind_sub, fdm_sub)) ->
          simplify_subtype_shape
            ~subtype_env
            ~env
            ~this_ty
            (r_sub, shape_kind_sub, fdm_sub)
            (r_super, shape_kind_super, fdm_super)
        | _ -> default_subtype env))
    | (_, (Tvarray _ | Tdarray _ | Tvarray_or_darray _ | Tvec_or_dict _)) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match (get_node lty, get_node ty_super) with
        | (Tvarray ty_sub, Tvarray ty_super) ->
          simplify_subtype ~subtype_env ~this_ty ty_sub ty_super env
        | ( Tvarray_or_darray (tk_sub, tv_sub),
            Tvarray_or_darray (tk_super, tv_super) )
        | (Tvec_or_dict (tk_sub, tv_sub), Tvec_or_dict (tk_super, tv_super))
        | (Tdarray (tk_sub, tv_sub), Tdarray (tk_super, tv_super))
        | (Tdarray (tk_sub, tv_sub), Tvarray_or_darray (tk_super, tv_super)) ->
          env
          |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
          &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
        | ( Tclass ((_, n), _, [tk_sub; tv_sub]),
            Tvec_or_dict (tk_super, tv_super) )
          when String.equal n SN.Collections.cDict ->
          env
          |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
          &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
        | (Tvarray tv_sub, Tvarray_or_darray (tk_super, tv_super)) ->
          let pos = get_pos lty in
          let tk_sub = MakeType.int (Reason.Ridx_vector_from_decl pos) in
          env
          |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
          &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
        | (Tclass ((_, n), _, [tv_sub]), Tvec_or_dict (tk_super, tv_super))
          when String.equal n SN.Collections.cVec ->
          let pos = get_pos lty in
          let tk_sub = MakeType.int (Reason.Ridx_vector_from_decl pos) in
          env
          |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
          &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
        | (Tvarray _, Tdarray _)
        | (Tdarray _, Tvarray _)
        | (Tvarray_or_darray _, Tdarray _)
        | (Tvarray_or_darray _, Tvarray _) ->
          invalid_env env
        | _ -> default_subtype env))
    | (_, Tnewtype (name_super, tyl_super, _)) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (_, Tclass ((_, name_sub), _, _)) ->
          if String.equal name_sub name_super && Env.is_enum env name_super then
            valid env
          else
            default_subtype env
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
              | Some { td_tparams; _ } ->
                let variance_reifiedl =
                  List.map td_tparams (fun t -> (t.tp_variance, t.tp_reified))
                in
                simplify_subtype_variance
                  ~subtype_env
                  name_sub
                  variance_reifiedl
                  tyl_sub
                  tyl_super
                  env
              | None -> invalid_env env
            end
        | _ -> default_subtype env))
    | (_, Tunapplied_alias n_sup) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (_, Tunapplied_alias n_sub) when String.equal n_sub n_sup -> valid env
        | _ -> default_subtype env))
    | (r_super, Tclass (((_, class_name) as x_super), exact_super, tyl_super))
      ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (_, Tnewtype (enum_name, _, _))
          when String.equal enum_name class_name
               && equal_exact exact_super Nonexact
               && Env.is_enum env enum_name ->
          valid env
        | (_, Tnewtype (cid, _, _))
          when String.equal class_name SN.Classes.cHH_BuiltinEnum
               && Env.is_enum env cid ->
          (match tyl_super with
          | [ty_super'] ->
            env |> simplify_subtype ~subtype_env ~this_ty ty_sub ty_super'
          | _ -> default_subtype env)
        | (_, Tnewtype (enum_name, _, _))
          when (String.equal enum_name class_name && Env.is_enum env enum_name)
               || String.equal class_name SN.Classes.cXHPChild ->
          valid env
        | ( _,
            ( Tvarray _ | Tdarray _ | Tvarray_or_darray _
            | Tprim Nast.(Tstring | Tarraykey | Tint | Tfloat | Tnum) ) )
          when String.equal class_name SN.Classes.cXHPChild
               && equal_exact exact_super Nonexact ->
          valid env
        | (_, Tprim Nast.Tstring)
          when String.equal class_name SN.Classes.cStringish
               && equal_exact exact_super Nonexact ->
          valid env
        (* Match what's done in unify for non-strict code *)
        | (_, Tobject)
          when not @@ Partial.should_check_error (Env.get_mode env) 4110 ->
          valid env
        | (r_sub, Tclass (x_sub, exact_sub, tyl_sub)) ->
          let (cid_super, cid_sub) = (snd x_super, snd x_sub) in
          let exact_match =
            match (exact_sub, exact_super) with
            | (Nonexact, Exact) -> false
            | (_, _) -> true
          in
          if String.equal cid_super cid_sub then
            if List.is_empty tyl_sub && List.is_empty tyl_super && exact_match
            then
              valid env
            else
              (* This is side-effecting as it registers a dependency *)
              let class_def_sub = Env.get_class env cid_sub in
              (* If class is final then exactness is superfluous *)
              let is_final =
                match class_def_sub with
                | Some tc -> Cls.final tc
                | None -> false
              in
              if not (exact_match || is_final) then
                invalid_env env
              else
                (* We handle the case where a generic A<T> is used as A *)
                let tyl_super =
                  if
                    List.is_empty tyl_super
                    && not (Partial.should_check_error (Env.get_mode env) 4101)
                  then
                    List.map tyl_sub (fun _ ->
                        mk (r_super, Typing_defs.make_tany ()))
                  else
                    tyl_super
                in
                let tyl_sub =
                  if
                    List.is_empty tyl_sub
                    && not (Partial.should_check_error (Env.get_mode env) 4101)
                  then
                    List.map tyl_super (fun _ ->
                        mk (r_super, Typing_defs.make_tany ()))
                  else
                    tyl_sub
                in
                if Int.( <> ) (List.length tyl_sub) (List.length tyl_super) then
                  let n_sub = String_utils.soi (List.length tyl_sub) in
                  let n_super = String_utils.soi (List.length tyl_super) in
                  invalid_env_with env (fun () ->
                      Errors.type_arity_mismatch
                        (fst x_super)
                        n_super
                        (fst x_sub)
                        n_sub
                        subtype_env.on_error)
                else
                  let variance_reifiedl =
                    if List.is_empty tyl_sub then
                      []
                    else
                      match class_def_sub with
                      | None ->
                        List.map tyl_sub (fun _ ->
                            (Ast_defs.Invariant, Aast.Erased))
                      | Some class_sub ->
                        List.map (Cls.tparams class_sub) (fun t ->
                            (t.tp_variance, t.tp_reified))
                  in
                  (* C<t1, .., tn> <: C<u1, .., un> iff
                   *   t1 <:v1> u1 /\ ... /\ tn <:vn> un
                   * where vi is the variance of the i'th generic parameter of C,
                   * and <:v denotes the appropriate direction of subtyping for variance v
                   *)
                  simplify_subtype_variance
                    ~subtype_env
                    cid_sub
                    variance_reifiedl
                    tyl_sub
                    tyl_super
                    env
          else if not exact_match then
            invalid_env env
          else
            let class_def_sub = Env.get_class env cid_sub in
            (match class_def_sub with
            | None ->
              (* This should have been caught already in the naming phase *)
              valid env
            | Some class_sub ->
              (* We handle the case where a generic A<T> is used as A *)
              let tyl_sub =
                if
                  List.is_empty tyl_sub
                  && not (Partial.should_check_error (Env.get_mode env) 4029)
                then
                  List.map (Cls.tparams class_sub) (fun _ ->
                      mk (r_sub, Typing_defs.make_tany ()))
                else
                  tyl_sub
              in
              let ety_env =
                {
                  type_expansions = Typing_defs.Type_expansions.empty;
                  substs =
                    TUtils.make_locl_subst_for_class_tparams class_sub tyl_sub;
                  (* TODO: do we need this? *)
                  this_ty = Option.value this_ty ~default:ty_sub;
                  on_error = Errors.ignore_error;
                }
              in
              let up_obj = Cls.get_ancestor class_sub cid_super in
              (match up_obj with
              | Some up_obj ->
                let (env, up_obj) = Phase.localize ~ety_env env up_obj in
                simplify_subtype ~subtype_env ~this_ty up_obj ty_super env
              | None ->
                if
                  Ast_defs.(equal_class_kind (Cls.kind class_sub) Ctrait)
                  || Ast_defs.(equal_class_kind (Cls.kind class_sub) Cinterface)
                then
                  let rec try_upper_bounds_on_this up_objs env =
                    match up_objs with
                    | [] ->
                      (* It's crucial that we don't lose updates to global_tpenv in env that were
                       * introduced by PHase.localize. TODO: avoid this requirement *)
                      invalid_env env
                    | ub_obj_typ :: up_objs ->
                      (* a trait is never the runtime type, but it can be used
                       * as a constraint if it has requirements or where constraints
                       * for its using classes *)
                      let (env, ub_obj_typ) =
                        Phase.localize ~ety_env env ub_obj_typ
                      in
                      env
                      |> simplify_subtype
                           ~subtype_env
                           ~this_ty
                           (mk (r_sub, get_node ub_obj_typ))
                           ty_super
                      ||| try_upper_bounds_on_this up_objs
                  in
                  try_upper_bounds_on_this
                    (Cls.upper_bounds_on_this class_sub)
                    env
                else
                  invalid_env env))
        | ( r_sub,
            ( Tvarray tv
            | Tdarray (_, tv)
            | Tvarray_or_darray (_, tv)
            | Tvec_or_dict (_, tv) ) ) ->
          (match (exact_super, tyl_super) with
          | (Nonexact, [tv_super])
            when String.equal class_name SN.Collections.cTraversable
                 || String.equal class_name SN.Collections.cContainer ->
            (* vec<tv> <: Traversable<tv_super>
             * iff tv <: tv_super
             * Likewise for vec<tv> <: Container<tv_super>
             *          and map<_,tv> <: Traversable<tv_super>
             *          and map<_,tv> <: Container<tv_super>
             *)
            simplify_subtype ~subtype_env ~this_ty tv tv_super env
          | (Nonexact, [tk_super; tv_super])
            when String.equal class_name SN.Collections.cKeyedTraversable
                 || String.equal class_name SN.Collections.cKeyedContainer
                 || String.equal class_name SN.Collections.cAnyArray ->
            (match get_node ty_sub with
            | Tvarray _ ->
              env
              |> simplify_subtype
                   ~subtype_env
                   ~this_ty
                   (MakeType.int r_sub)
                   tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv tv_super
            | Tvarray_or_darray (tk, _)
            | Tvec_or_dict (tk, _)
            | Tdarray (tk, _) ->
              env
              |> simplify_subtype ~subtype_env ~this_ty tk tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv tv_super
            | _ -> default_subtype env)
          | (Nonexact, [])
            when String.equal class_name SN.Collections.cKeyedTraversable
                 || String.equal class_name SN.Collections.cKeyedContainer
                 || String.equal class_name SN.Collections.cAnyArray ->
            (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
            valid env
          | (_, _) -> default_subtype env)
        | _ -> default_subtype env)))

and simplify_subtype_shape
    ~(subtype_env : subtype_env)
    ~(env : env)
    ~(this_ty : locl_ty option)
    (r_sub, shape_kind_sub, fdm_sub)
    (r_super, shape_kind_super, fdm_super) =
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
  let shape_projection field_name shape_kind shape_map r =
    match TShapeMap.find_opt field_name shape_map with
    | Some { sft_ty; sft_optional } ->
      begin
        match (deref sft_ty, sft_optional) with
        | ((_, Tunion []), true) -> `Absent
        | (_, true) -> `Optional sft_ty
        | (_, false) -> `Required sft_ty
      end
    | None ->
      begin
        match shape_kind with
        | Open_shape ->
          let printable_name =
            TUtils.get_printable_shape_field_name field_name
          in
          let mixed_ty =
            MakeType.mixed
              (Reason.Rmissing_optional_field (Reason.to_pos r, printable_name))
          in
          `Optional mixed_ty
        | Closed_shape -> `Absent
      end
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
    let printable_name = TUtils.get_printable_shape_field_name field_name in
    match (proj_sub, proj_super) with
    (***** "Successful" cases - 5 / 9 total cases *****)
    | (`Required sub_ty, `Required super_ty)
    | (`Required sub_ty, `Optional super_ty)
    | (`Optional sub_ty, `Optional super_ty) ->
      res &&& simplify_subtype ~subtype_env ~this_ty sub_ty super_ty
    | (`Absent, `Optional _)
    | (`Absent, `Absent) ->
      res
    (***** Error cases - 4 / 9 total cases *****)
    | (`Required _, `Absent)
    | (`Optional _, `Absent) ->
      res
      |> with_error (fun () ->
             Errors.missing_field
               (Reason.to_pos r_super)
               (Reason.to_pos r_sub)
               printable_name
               subtype_env.on_error)
    | (`Optional _, `Required _) ->
      res
      |> with_error (fun () ->
             Errors.required_field_is_optional
               (Reason.to_pos r_sub)
               (Reason.to_pos r_super)
               printable_name
               subtype_env.on_error)
    | (`Absent, `Required _) ->
      res
      |> with_error (fun () ->
             Errors.missing_field
               (Reason.to_pos r_sub)
               (Reason.to_pos r_super)
               printable_name
               subtype_env.on_error)
  in
  (* Helper function to project out a field and then simplify subtype *)
  let shape_project_and_simplify_subtype
      (r_sub, shape_kind_sub, shape_map_sub)
      (r_super, shape_kind_super, shape_map_super)
      field_name
      res =
    let proj_sub =
      shape_projection field_name shape_kind_sub shape_map_sub r_sub
    in
    let proj_super =
      shape_projection field_name shape_kind_super shape_map_super r_super
    in
    simplify_subtype_shape_projection
      (r_sub, proj_sub)
      (r_super, proj_super)
      field_name
      res
  in
  match (shape_kind_sub, shape_kind_super) with
  (* An open shape cannot subtype a closed shape *)
  | (Open_shape, Closed_shape) ->
    invalid
      ~fail:(fun () ->
        Errors.shape_fields_unknown
          (Reason.to_pos r_sub)
          (Reason.to_pos r_super)
          subtype_env.on_error)
      env
  (* Otherwise, all projections must subtype *)
  | _ ->
    TShapeSet.fold
      (shape_project_and_simplify_subtype
         (r_sub, shape_kind_sub, fdm_sub)
         (r_super, shape_kind_super, fdm_super))
      (TShapeSet.of_list (TShapeMap.keys fdm_sub @ TShapeMap.keys fdm_super))
      (env, TL.valid)

and simplify_subtype_has_member
    ~subtype_env
    ~this_ty
    ~fail
    ?(nullsafe : Reason.t option)
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

  log_subtype_i
    ~level:2
    ~this_ty
    ~function_name:"simplify_subtype_has_member"
    env
    ty_sub
    maybe_nullable_ty_super;
  let (env, ety_sub) = Env.expand_internal_type env ty_sub in
  let default_subtype env =
    default_subtype
      ~subtype_env
      ~this_ty
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
        simplify_subtype ~subtype_env ~this_ty ty_sub member_ty env
      else
        invalid ~fail env
    | _ -> default_subtype env)
  | LoclType ty_sub ->
    (match deref ty_sub with
    | (_, (Tvar _ | Tunion _ | Terr)) -> default_subtype env
    | (r_null, Tprim Aast.Tnull) when using_new_method_call_inference ->
      if Option.is_some nullsafe then
        valid env
      else
        invalid env ~fail:(fun () ->
            Errors.null_member_read
              ~is_method
              name_
              name_pos
              (Reason.to_string "This can be null" r_null))
    | (r_option, Toption option_ty) when using_new_method_call_inference ->
      if Option.is_some nullsafe then
        simplify_subtype_has_member
          ~subtype_env
          ~this_ty
          ~fail
          ?nullsafe
          (LoclType option_ty)
          (r, has_member_ty)
          env
      else
        let (env, option_ty) = Env.expand_type env option_ty in
        (match get_node option_ty with
        | Tnonnull ->
          invalid env ~fail:(fun () ->
              Errors.top_member_read
                ~is_method
                ~is_nullable:true
                name_
                name_pos
                (Typing_print.error env ty_sub)
                (Reason.to_pos r_option))
        | _ ->
          invalid env ~fail:(fun () ->
              Errors.null_member_read
                ~is_method
                name_
                name_pos
                (Reason.to_string "This can be null" r_option)))
    | (_, Tintersection tyl)
      when let (_, non_ty_opt, _) = find_type_with_exact_negation env tyl in
           Option.is_some non_ty_opt ->
      (* use default_subtype to perform: A & B <: C <=> A <: C | !B *)
      default_subtype env
    | (r_inter, Tintersection []) ->
      (* Tintersection [] = mixed *)
      invalid env ~fail:(fun () ->
          Errors.top_member_read
            ~is_method
            ~is_nullable:true
            name_
            name_pos
            (Typing_print.error env ty_sub)
            (Reason.to_pos r_inter))
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
        let (env, succeeded) =
          sub_type_inner
            env
            ~subtype_env
            ~this_ty
            (LoclType ty_sub)
            maybe_nullable_fresh_has_member_ty
        in
        if succeeded then
          let env =
            match get_var fresh_tyvar with
            | Some var ->
              Typing_solver.solve_to_equal_bound_or_wrt_variance
                env
                Reason.Rnone
                var
            | None -> env
          in
          (env, Some fresh_tyvar)
        else
          (env, None)
      in
      let (env, fresh_tyvar_opts) =
        TUtils.run_on_intersection env tyl ~f:subtype_fresh_has_member_ty
      in
      let fresh_tyvars = List.filter_map ~f:Fn.id fresh_tyvar_opts in
      if List.is_empty fresh_tyvars then
        (* TUtils.run_on_intersection has already added errors - no need to add more *)
        invalid ~fail:(fun () -> ()) env
      else
        let (env, intersection_ty) =
          Inter.intersect_list env r_inter fresh_tyvars
        in
        simplify_subtype ~subtype_env ~this_ty intersection_ty member_ty env
    | (_, Tnewtype (_, _, newtype_ty)) ->
      simplify_subtype_has_member
        ~subtype_env
        ~this_ty
        ~fail
        ?nullsafe
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
      let (errors, (env, (obj_get_ty, _tal))) =
        Errors.do_ (fun () ->
            Typing_object_get.obj_get
              ~obj_pos:name_pos
              ~is_method
              ~inst_meth:false
              ~coerce_from_ty:None
              ~nullsafe
              ~explicit_targs
              ~class_id
              ~member_id:name
              ~on_error:Errors.unify_error
              env
              ty_sub)
      in
      let prop =
        if Errors.is_empty errors then
          valid env
        else
          invalid env ~fail:(fun () ->
              Errors.apply_callback_to_errors errors subtype_env.on_error)
      in
      prop &&& simplify_subtype ~subtype_env ~this_ty obj_get_ty member_ty)

and simplify_subtype_variance
    ~(subtype_env : subtype_env)
    (cid : string)
    (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
    (children_tyl : locl_ty list)
    (super_tyl : locl_ty list) : env -> env * TL.subtype_prop =
 fun env ->
  let simplify_subtype reify_kind =
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
        && coercing_from_dynamic subtype_env
      then
        { subtype_env with coerce = None }
      else
        subtype_env
    in
    simplify_subtype ~subtype_env ~this_ty:None
  in
  let simplify_subtype_variance = simplify_subtype_variance ~subtype_env in
  match (variance_reifiedl, children_tyl, super_tyl) with
  | ([], _, _)
  | (_, [], _)
  | (_, _, []) ->
    valid env
  | ( (variance, reify_kind) :: variance_reifiedl,
      child :: childrenl,
      super :: superl ) ->
    let simplify_subtype = simplify_subtype reify_kind in
    begin
      match variance with
      | Ast_defs.Covariant -> simplify_subtype child super env
      | Ast_defs.Contravariant ->
        let super =
          mk
            ( Reason.Rcontravariant_generic (get_reason super, cid),
              get_node super )
        in
        simplify_subtype super child env
      | Ast_defs.Invariant ->
        let super' =
          mk (Reason.Rinvariant_generic (get_reason super, cid), get_node super)
        in
        env |> simplify_subtype child super' &&& simplify_subtype super' child
    end
    &&& simplify_subtype_variance cid variance_reifiedl childrenl superl

and simplify_subtype_params
    ~(subtype_env : subtype_env)
    ?(check_params_ifc = false)
    (subl : locl_fun_param list)
    (superl : locl_fun_param list)
    (variadic_sub_ty : locl_possibly_enforced_ty option)
    (variadic_super_ty : locl_possibly_enforced_ty option)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced ~subtype_env
  in
  let simplify_subtype_params = simplify_subtype_params ~subtype_env in
  let simplify_subtype_params_with_variadic =
    simplify_subtype_params_with_variadic ~subtype_env
  in
  let simplify_supertype_params_with_variadic =
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
  | ([], _) ->
    (match variadic_super_ty with
    | None -> valid env
    | Some ty -> simplify_supertype_params_with_variadic superl ty env)
  | (_, []) ->
    (match variadic_sub_ty with
    | None -> valid env
    | Some ty -> simplify_subtype_params_with_variadic subl ty env)
  | (sub :: subl, super :: superl) ->
    let { fp_type = ty_sub; _ } = sub in
    let { fp_type = ty_super; _ } = super in
    (* Check that the calling conventions of the params are compatible. *)
    env
    |> simplify_param_modes ~subtype_env sub super
    &&& simplify_param_readonly ~subtype_env sub super
    &&& simplify_param_accept_disposable ~subtype_env sub super
    &&& begin
          if check_params_ifc then
            simplify_param_ifc ~subtype_env sub super
          else
            valid
        end
    &&& begin
          fun env ->
          match (get_fp_mode sub, get_fp_mode super) with
          | (FPinout, FPinout) ->
            (* Inout parameters are invariant wrt subtyping for function types. *)
            env
            |> simplify_subtype_possibly_enforced ty_super ty_sub
            &&& simplify_subtype_possibly_enforced ty_sub ty_super
          | _ -> env |> simplify_subtype_possibly_enforced ty_sub ty_super
        end
    &&& simplify_subtype_params subl superl variadic_sub_ty variadic_super_ty

and simplify_subtype_params_with_variadic
    ~(subtype_env : subtype_env)
    (subl : locl_fun_param list)
    (variadic_ty : locl_possibly_enforced_ty)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced ~subtype_env
  in
  let simplify_subtype_params_with_variadic =
    simplify_subtype_params_with_variadic ~subtype_env
  in
  match subl with
  | [] -> valid env
  | { fp_type = sub; _ } :: subl ->
    env
    |> simplify_subtype_possibly_enforced sub variadic_ty
    &&& simplify_subtype_params_with_variadic subl variadic_ty

and simplify_subtype_implicit_params
    ~subtype_env { capability = sub_cap } { capability = super_cap } env =
  if TypecheckerOptions.any_coeffects (Env.get_tcopt env) then
    let subtype_env =
      {
        subtype_env with
        on_error =
          begin
            fun ?code:_ _ ->
            let expected = Typing_coeffects.get_type sub_cap in
            let got = Typing_coeffects.get_type super_cap in
            Errors.coeffect_subtyping_error
              (get_pos expected)
              (Typing_print.coeffects env expected)
              (get_pos got)
              (Typing_print.coeffects env got)
              subtype_env.on_error
          end;
      }
    in
    match (sub_cap, super_cap) with
    | (CapTy sub, CapTy super) -> simplify_subtype ~subtype_env sub super env
    | (CapTy sub, CapDefaults _p) ->
      let super = Typing_coeffects.get_type super_cap in
      simplify_subtype ~subtype_env sub super env
    | (CapDefaults _p, CapTy super) ->
      let sub = Typing_coeffects.get_type sub_cap in
      simplify_subtype ~subtype_env sub super env
    | (CapDefaults _p1, CapDefaults _p2) -> valid env
  else
    valid env

and simplify_supertype_params_with_variadic
    ~(subtype_env : subtype_env)
    (superl : locl_fun_param list)
    (variadic_ty : locl_possibly_enforced_ty)
    env =
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced ~subtype_env
  in
  let simplify_supertype_params_with_variadic =
    simplify_supertype_params_with_variadic ~subtype_env
  in
  match superl with
  | [] -> valid env
  | { fp_type = super; _ } :: superl ->
    env
    |> simplify_subtype_possibly_enforced variadic_ty super
    &&& simplify_supertype_params_with_variadic superl variadic_ty

and simplify_param_modes ~subtype_env param1 param2 env =
  let { fp_pos = pos1; _ } = param1 in
  let { fp_pos = pos2; _ } = param2 in
  match (get_fp_mode param1, get_fp_mode param2) with
  | (FPnormal, FPnormal)
  | (FPinout, FPinout) ->
    valid env
  | (FPnormal, FPinout) ->
    invalid
      ~fail:(fun () -> Errors.inoutness_mismatch pos2 pos1 subtype_env.on_error)
      env
  | (FPinout, FPnormal) ->
    invalid
      ~fail:(fun () -> Errors.inoutness_mismatch pos1 pos2 subtype_env.on_error)
      env

and simplify_param_accept_disposable ~subtype_env param1 param2 env =
  let { fp_pos = pos1; _ } = param1 in
  let { fp_pos = pos2; _ } = param2 in
  match (get_fp_accept_disposable param1, get_fp_accept_disposable param2) with
  | (true, false) ->
    invalid
      ~fail:(fun () ->
        Errors.accept_disposable_invariant pos1 pos2 subtype_env.on_error)
      env
  | (false, true) ->
    invalid
      ~fail:(fun () ->
        Errors.accept_disposable_invariant pos2 pos1 subtype_env.on_error)
      env
  | (_, _) -> valid env

and simplify_param_ifc ~subtype_env sub super env =
  let { fp_pos = pos1; _ } = sub in
  let { fp_pos = pos2; _ } = super in
  (* TODO: also handle <<CanCall>> *)
  match (get_fp_ifc_external sub, get_fp_ifc_external super) with
  | (true, false) ->
    invalid
      ~fail:(fun () ->
        Errors.ifc_external_contravariant pos2 pos1 subtype_env.on_error)
      env
  | _ -> valid env

and simplify_param_readonly ~subtype_env sub super env =
  (* The sub param here (as with all simplify_param_* functions)
  is actually the parameter on ft_super, since params are contravariant *)
  (* Thus we check readonly subtyping covariantly *)
  let { fp_pos = pos1; _ } = sub in
  let { fp_pos = pos2; _ } = super in
  if not (readonly_subtype (get_fp_readonly sub) (get_fp_readonly super)) then
    invalid
      ~fail:(fun () ->
        Errors.readonly_mismatch_on_error
          "Mismatched parameter readonlyness"
          pos1
          ~reason_sub:[(pos1, "This parameter is mutable")]
          ~reason_super:[(pos2, "But this parameter is readonly")]
          subtype_env.on_error)
      env
  else
    valid env

and ifc_policy_matches (ifc1 : ifc_fun_decl) (ifc2 : ifc_fun_decl) =
  match (ifc1, ifc2) with
  | (FDPolicied (Some s1), FDPolicied (Some s2)) when String.equal s1 s2 -> true
  | (FDPolicied None, FDPolicied None) -> true
  (* TODO(T79510128): IFC needs to check that the constraints inferred by the parent entail those by the subtype *)
  | (FDInferFlows, FDInferFlows) -> true
  | _ -> false

and readonly_subtype (r_sub : bool) (r_super : bool) =
  match (r_sub, r_super) with
  | (true, false) ->
    false (* A readonly value is a supertype of a mutable one *)
  | _ -> true

(* Helper function for subtyping on function types: performs all checks that
 * don't involve actual types:
 *   <<__ReturnDisposable>> attribute
 *   variadic arity
 *  <<__Policied>> attribute
 *  Readonlyness
 *)
and simplify_subtype_funs_attributes
    ~subtype_env
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let ifc_policy_err_str = function
    | FDPolicied (Some s) -> s
    | FDPolicied None -> "the existential policy"
    | FDInferFlows -> "an inferred policy"
  in
  (env, TL.valid)
  |> check_with
       (ifc_policy_matches ft_sub.ft_ifc_decl ft_super.ft_ifc_decl)
       (fun () ->
         Errors.ifc_policy_mismatch
           p_sub
           p_super
           (ifc_policy_err_str ft_sub.ft_ifc_decl)
           (ifc_policy_err_str ft_super.ft_ifc_decl)
           subtype_env.on_error)
  |> check_with
       (readonly_subtype
          (* Readonly this is contravariant, so check ft_super_ro <: ft_sub_ro *)
          (get_ft_readonly_this ft_super)
          (get_ft_readonly_this ft_sub))
       (fun () ->
         Errors.readonly_mismatch_on_error
           "Function readonly mismatch"
           p_sub
           [(p_sub, "This function is not marked readonly")]
           [(p_super, "This function is marked readonly")]
           subtype_env.on_error)
  |> check_with
       (readonly_subtype
          (* Readonly return is covariant, so check ft_sub <: ft_super *)
          (get_ft_returns_readonly ft_sub)
          (get_ft_returns_readonly ft_super))
       (fun () ->
         Errors.readonly_mismatch_on_error
           "Function readonly return mismatch"
           p_sub
           [(p_sub, "This function returns a readonly value")]
           [(p_super, "This function does not return a readonly value")]
           subtype_env.on_error)
  |> check_with
       (Bool.equal
          (get_ft_return_disposable ft_sub)
          (get_ft_return_disposable ft_super))
       (fun () ->
         Errors.return_disposable_mismatch
           (get_ft_return_disposable ft_super)
           p_super
           p_sub
           subtype_env.on_error)
  |> check_with
       (arity_min ft_sub <= arity_min ft_super)
       (fun () ->
         Errors.fun_too_many_args
           (arity_min ft_super)
           (arity_min ft_sub)
           p_sub
           p_super
           subtype_env.on_error)
  |> fun res ->
  match (ft_sub.ft_arity, ft_super.ft_arity) with
  | (Fvariadic { fp_name = None; _ }, Fvariadic { fp_name = Some _; _ }) ->
    (* The HHVM runtime ignores "..." entirely, but knows about
     * "...$args"; for contexts for which the runtime enforces method
     * compatibility (currently, inheritance from abstract/interface
     * methods), letting "..." override "...$args" would result in method
     * compatibility errors at runtime. *)
    with_error
      (fun () ->
        Errors.fun_variadicity_hh_vs_php56 p_sub p_super subtype_env.on_error)
      res
  | (Fstandard, Fstandard) ->
    let sub_max = List.length ft_sub.ft_params in
    let super_max = List.length ft_super.ft_params in
    if sub_max < super_max then
      with_error
        (fun () ->
          Errors.fun_too_few_args
            super_max
            sub_max
            p_sub
            p_super
            subtype_env.on_error)
        res
    else
      res
  | (Fstandard, _) ->
    with_error
      (fun () ->
        Errors.fun_unexpected_nonvariadic p_sub p_super subtype_env.on_error)
      res
  | (_, _) -> res

and simplify_subtype_possibly_enforced
    ~(subtype_env : subtype_env) et_sub et_super =
  simplify_subtype ~subtype_env et_sub.et_type et_super.et_type

(* Special case of function type subtype dynamic.
 *   (function(ty1,...,tyn):ty <: dynamic)
 *   iff
 *   dynamic <: ty1 & ... & dynamic <: tyn & ty <: dynamic
 *)
and simplify_subtype_fun_dynamic
    ~(subtype_env : subtype_env)
    (_r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    ty_dyn
    env : env * TL.subtype_prop =
  let ty_dyn_enf = { et_enforced = Unenforced; et_type = ty_dyn } in
  env
  (* Contravariant subtyping on parameters *)
  |> begin
       match ft_sub.ft_arity with
       | Fvariadic { fp_type; _ } ->
         simplify_subtype ~subtype_env ty_dyn fp_type.et_type
       | _ -> valid
     end
  &&& simplify_supertype_params_with_variadic
        ~subtype_env
        ft_sub.ft_params
        ty_dyn_enf
  &&& (* Finally do covariant subtryping on return type *)
  simplify_subtype ~subtype_env ft_sub.ft_ret.et_type ty_dyn

(* This implements basic subtyping on non-generic function types:
 *   (1) return type behaves covariantly
 *   (2) parameter types behave contravariantly
 *   (3) special casing for variadics
 *)
and simplify_subtype_funs
    ~(subtype_env : subtype_env)
    ~(check_return : bool)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env : env * TL.subtype_prop =
  let variadic_subtype =
    match ft_sub.ft_arity with
    | Fvariadic { fp_type = var_sub; _ } -> Some var_sub
    | _ -> None
  in
  let variadic_supertype =
    match ft_super.ft_arity with
    | Fvariadic { fp_type = var_super; _ } -> Some var_super
    | _ -> None
  in
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced ~subtype_env
  in
  let simplify_subtype_params = simplify_subtype_params ~subtype_env in
  (* First apply checks on attributes and variadic arity *)
  let simplify_subtype_implicit_params =
    simplify_subtype_implicit_params ~subtype_env
  in
  env
  |> simplify_subtype_funs_attributes ~subtype_env r_sub ft_sub r_super ft_super
  &&& (* Now do contravariant subtyping on parameters *)
  begin
    match (variadic_subtype, variadic_supertype) with
    | (Some var_sub, Some var_super) ->
      simplify_subtype_possibly_enforced var_super var_sub
    | _ -> valid
  end
  &&& begin
        (* If both fun policies are IFC public, there's no need to check for inheritance issues *)
        (* There is the chance that the super function has an <<__External>> argument and the sub function does not,
          but <<__External>> on a public policied function literally just means the argument must be governed by the public policy,
          so should be an error in any case.
          *)
        let check_params_ifc =
          non_public_ifc ft_super.ft_ifc_decl
          || non_public_ifc ft_sub.ft_ifc_decl
        in
        simplify_subtype_params
          ~check_params_ifc
          ft_super.ft_params
          ft_sub.ft_params
          variadic_subtype
          variadic_supertype
      end
  &&& simplify_subtype_implicit_params
        ft_super.ft_implicit_params
        ft_sub.ft_implicit_params
  &&&
  (* Finally do covariant subtryping on return type *)
  if check_return then
    simplify_subtype_possibly_enforced ft_sub.ft_ret ft_super.ft_ret
  else
    valid

(* One of the main entry points to this module *)
and sub_type_i
    ~subtype_env (env : env) (ty_sub : internal_type) (ty_super : internal_type)
    : (env, env) result =
  let old_env = env in
  let (env, success) =
    sub_type_inner ~subtype_env env ~this_ty:None ty_sub ty_super
  in
  if success then
    Ok (Env.log_env_change "sub_type" old_env env)
  else
    Error (Env.log_env_change "sub_type" old_env old_env)

and sub_type
    env ?(coerce = None) (ty_sub : locl_ty) (ty_super : locl_ty) on_error =
  sub_type_i
    ~subtype_env:(make_subtype_env ~coerce on_error)
    env
    (LoclType ty_sub)
    (LoclType ty_super)
  |> function
  | Ok env -> env
  | Error env -> env

and sub_type_res
    env ?(coerce = None) (ty_sub : locl_ty) (ty_super : locl_ty) on_error =
  sub_type_i
    ~subtype_env:(make_subtype_env ~coerce on_error)
    env
    (LoclType ty_sub)
    (LoclType ty_super)

(* Add a new upper bound ty on var.  Apply transitivity of sutyping,
 * so if we already have tyl <: var, then check that for each ty_sub
 * in tyl we have ty_sub <: ty.
 *)
and add_tyvar_upper_bound_and_close ~coerce (env, prop) var ty on_error =
  let upper_bounds_before = Env.get_tyvar_upper_bounds env var in
  let env =
    Env.add_tyvar_upper_bound_and_update_variances
      ~intersect:(try_intersect_i env)
      env
      var
      ty
  in
  let upper_bounds_after = Env.get_tyvar_upper_bounds env var in
  let added_upper_bounds = ITySet.diff upper_bounds_after upper_bounds_before in
  let lower_bounds = Env.get_tyvar_lower_bounds env var in
  let (env, prop) =
    ITySet.fold
      (fun upper_bound (env, prop) ->
        let env =
          Typing_subtype_tconst.make_all_type_consts_equal
            env
            var
            upper_bound
            ~on_error
            ~as_tyvar_with_cnstr:true
        in
        ITySet.fold
          (fun lower_bound (env, prop1) ->
            let (env, prop2) =
              simplify_subtype_i
                ~subtype_env:(make_subtype_env ~coerce on_error)
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
and add_tyvar_lower_bound_and_close ~coerce (env, prop) var ty on_error =
  let lower_bounds_before = Env.get_tyvar_lower_bounds env var in
  let env =
    Env.add_tyvar_lower_bound_and_update_variances
      ~union:(try_union_i env)
      env
      var
      ty
  in
  let lower_bounds_after = Env.get_tyvar_lower_bounds env var in
  let added_lower_bounds = ITySet.diff lower_bounds_after lower_bounds_before in
  let upper_bounds = Env.get_tyvar_upper_bounds env var in
  let (env, prop) =
    ITySet.fold
      (fun lower_bound (env, prop) ->
        let env =
          Typing_subtype_tconst.make_all_type_consts_equal
            env
            var
            lower_bound
            ~on_error
            ~as_tyvar_with_cnstr:false
        in
        ITySet.fold
          (fun upper_bound (env, prop1) ->
            let (env, prop2) =
              simplify_subtype_i
                ~subtype_env:(make_subtype_env ~coerce on_error)
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

and get_tyvar_opt t =
  match t with
  | LoclType lt ->
    begin
      match get_node lt with
      | Tvar var -> Some var
      | _ -> None
    end
  | _ -> None

and props_to_env env remain props on_error =
  match props with
  | [] -> (env, List.rev remain)
  | prop :: props ->
    (match prop with
    | TL.Conj props' -> props_to_env env remain (props' @ props) on_error
    | TL.Disj (f, disj_props) ->
      (* For now, just find the first prop in the disjunction that works *)
      let rec try_disj disj_props =
        match disj_props with
        | [] ->
          (* For now let it fail later when calling
             process_simplify_subtype_result on the remaining constraints. *)
          props_to_env env (TL.invalid ~fail:f :: remain) props on_error
        | prop :: disj_props' ->
          let (env', other) = props_to_env env remain [prop] on_error in
          if TL.is_unsat (TL.conj_list other) then
            try_disj disj_props'
          else
            props_to_env env' (remain @ other) props on_error
      in
      try_disj disj_props
    | TL.IsSubtype (ty_sub, ty_super) ->
      begin
        match (get_tyvar_opt ty_sub, get_tyvar_opt ty_super) with
        | (Some var_sub, Some var_super) ->
          let (env, prop1) =
            add_tyvar_upper_bound_and_close
              ~coerce:None
              (valid env)
              var_sub
              ty_super
              on_error
          in
          let (env, prop2) =
            add_tyvar_lower_bound_and_close
              ~coerce:None
              (valid env)
              var_super
              ty_sub
              on_error
          in
          props_to_env env remain (prop1 :: prop2 :: props) on_error
        | (Some var, _) ->
          let (env, prop) =
            add_tyvar_upper_bound_and_close
              ~coerce:None
              (valid env)
              var
              ty_super
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | (_, Some var) ->
          let (env, prop) =
            add_tyvar_lower_bound_and_close
              ~coerce:None
              (valid env)
              var
              ty_sub
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | _ -> props_to_env env (prop :: remain) props on_error
      end
    | TL.Coerce (cd, ty_sub, ty_super) ->
      let coerce = Some cd in
      begin
        match (get_node ty_sub, get_node ty_super) with
        | (Tvar var_sub, Tvar var_super) ->
          let (env, prop1) =
            add_tyvar_upper_bound_and_close
              ~coerce
              (valid env)
              var_sub
              (LoclType ty_super)
              on_error
          in
          let (env, prop2) =
            add_tyvar_lower_bound_and_close
              ~coerce
              (valid env)
              var_super
              (LoclType ty_sub)
              on_error
          in
          props_to_env env remain (prop1 :: prop2 :: props) on_error
        | (Tvar var, _) ->
          let (env, prop) =
            add_tyvar_upper_bound_and_close
              ~coerce
              (valid env)
              var
              (LoclType ty_super)
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | (_, Tvar var) ->
          let (env, prop) =
            add_tyvar_lower_bound_and_close
              ~coerce
              (valid env)
              var
              (LoclType ty_sub)
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | _ -> failwith "Coercion not expected"
      end)

(* Move any top-level conjuncts of the form Tvar v <: t or t <: Tvar v to
 * the type variable environment. To do: use intersection and union to
 * simplify bounds.
 *)
and prop_to_env env prop on_error =
  let (env, props') = props_to_env env [] [prop] on_error in
  (env, TL.conj_list props')

and sub_type_inner
    (env : env)
    ~(subtype_env : subtype_env)
    ~(this_ty : locl_ty option)
    (ty_sub : internal_type)
    (ty_super : internal_type) : env * bool =
  log_subtype_i
    ~level:1
    ~this_ty
    ~function_name:
      ( "sub_type_inner"
      ^
      match subtype_env.coerce with
      | Some TL.CoerceToDynamic -> " (including <: dynamic)"
      | _ -> "" )
    env
    ty_sub
    ty_super;
  let (env, prop) =
    simplify_subtype_i ~subtype_env ~this_ty ty_sub ty_super env
  in
  let (env, prop) = prop_to_env env prop subtype_env.on_error in
  let env = Env.add_subtype_prop env prop in
  let succeeded = process_simplify_subtype_result prop in
  (env, succeeded)

and is_sub_type_alt_i ~ignore_generic_params ~no_top_bottom ~coerce env ty1 ty2
    =
  let this_ty =
    match ty1 with
    | LoclType ty1 -> Some ty1
    | ConstraintType _ -> None
  in
  let (_env, prop) =
    simplify_subtype_i
      ~subtype_env:
        (make_subtype_env
           ~ignore_generic_params
           ~no_top_bottom
           ~coerce
           Errors.ignore_error)
      ~this_ty
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

and is_sub_type_alt ~ignore_generic_params ~no_top_bottom env ty1 ty2 =
  is_sub_type_alt_i
    ~ignore_generic_params
    ~no_top_bottom
    env
    (LoclType ty1)
    (LoclType ty2)

and is_sub_type env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:false
    ~coerce:None
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_coercion env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:false
    ~coerce:(Some TL.CoerceFromDynamic)
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_union env ?(coerce = None) ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~coerce
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_union_i env ?(coerce = None) ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt_i
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~coerce
    env
    ty1
    ty2
  = Some true

and can_sub_type env ty1 ty2 =
  let ( <> ) a b = not (Option.equal Bool.equal a b) in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~coerce:None
    env
    ty1
    ty2
  <> Some false

and is_sub_type_ignore_generic_params env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:true
    ~no_top_bottom:true
    ~coerce:None
    env
    ty1
    ty2
  = Some true

and is_sub_type_ignore_generic_params_i env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt_i
    ~ignore_generic_params:true
    ~no_top_bottom:true
    ~coerce:None
    env
    ty1
    ty2
  = Some true

(* Attempt to compute the intersection of a type with an existing list intersection.
 * If try_intersect env t [t1;...;tn] = [u1; ...; um]
 * then u1&...&um must be the greatest lower bound of t and t1&...&tn wrt subtyping.
 * For example:
 *   try_intersect nonnull [?C] = [C]
 *   try_intersect t1 [t2] = [t1]  if t1 <: t2
 * Note: it's acceptable to return [t;t1;...;tn] but the intention is that
 * we simplify (as above) wherever practical.
 * It can be assumed that the original list contains no redundancy.
 *)
and try_intersect_i env ty tyl =
  match tyl with
  | [] -> [ty]
  | ty' :: tyl' ->
    if is_sub_type_ignore_generic_params_i env ty ty' then
      try_intersect_i env ty tyl'
    else if is_sub_type_ignore_generic_params_i env ty' ty then
      tyl
    else
      let nonnull_ty = LoclType (MakeType.nonnull (reason ty)) in
      let (env, ty) = Env.expand_internal_type env ty in
      let (env, ty') = Env.expand_internal_type env ty' in
      let default env = ty' :: try_intersect_i env ty tyl' in
      (match (ty, ty') with
      | (LoclType lty, _)
        when is_sub_type_ignore_generic_params_i env ty' nonnull_ty ->
        begin
          match get_node lty with
          | Toption t -> try_intersect_i env (LoclType t) (ty' :: tyl')
          | _ -> default env
        end
      | (_, LoclType lty)
        when is_sub_type_ignore_generic_params_i env ty nonnull_ty ->
        begin
          match get_node lty with
          | Toption t -> try_intersect_i env (LoclType t) (ty :: tyl')
          | _ -> default env
        end
      | (_, _) -> default env)

and try_intersect env ty tyl =
  List.map
    (try_intersect_i
       env
       (LoclType ty)
       (List.map tyl ~f:(fun ty -> LoclType ty)))
    ~f:(function
      | LoclType ty -> ty
      | _ ->
        failwith
          "The intersection of two locl type should always be a locl type.")

(* Attempt to compute the union of a type with an existing list union.
 * If try_union env t [t1;...;tn] = [u1;...;um]
 * then u1|...|um must be the least upper bound of t and t1|...|tn wrt subtyping.
 * For example:
 *   try_union int [float] = [num]
 *   try_union t1 [t2] = [t1] if t2 <: t1
 *
 * Notes:
 * 1. It's acceptable to return [t;t1;...;tn] but the intention is that
 *    we simplify (as above) wherever practical.
 * 2. Do not use Tunion for a syntactic union - the caller can do that.
 * 3. It can be assumed that the original list contains no redundancy.
 * TODO: there are many more unions to implement yet.
 *)
and try_union_i env ty tyl =
  match tyl with
  | [] -> [ty]
  | ty' :: tyl' ->
    if is_sub_type_for_union_i env ty ty' then
      tyl
    else if is_sub_type_for_union_i env ty' ty then
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

and try_union env ty tyl =
  List.map
    (try_union_i env (LoclType ty) (List.map tyl ~f:(fun ty -> LoclType ty)))
    ~f:(function
      | LoclType ty -> ty
      | _ -> failwith "The union of two locl type should always be a locl type.")

let decompose_subtype_add_bound
    (env : env) (ty_sub : locl_ty) (ty_super : locl_ty) : env =
  let (env, ty_super) = Env.expand_type env ty_super in
  let (env, ty_sub) = Env.expand_type env ty_sub in
  match (get_node ty_sub, get_node ty_super) with
  | (_, Tany _) -> env
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (Tgeneric (name_sub, targs), _) when not (phys_equal ty_sub ty_super) ->
    (* TODO(T69551141) handle type arguments. Passing targs to get_lower_bounds,
      but the add_upper_bound call must be adapted *)
    log_subtype
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
      Env.add_upper_bound ~intersect:(try_intersect env) env name_sub ty_super
  (* ty_sub <: name_super so add a lower bound on name_super *)
  | (_, Tgeneric (name_super, targs)) when not (phys_equal ty_sub ty_super) ->
    (* TODO(T69551141) handle type arguments. Passing targs to get_lower_bounds,
      but the add_lower_bound call must be adapted *)
    log_subtype
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
      Env.add_lower_bound ~union:(try_union env) env name_super ty_sub
  | (_, _) -> env

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
let rec decompose_subtype
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.error_from_reasons_callback) : env =
  log_subtype
    ~level:2
    ~this_ty:None
    ~function_name:"decompose_subtype"
    env
    ty_sub
    ty_super;
  let (env, prop) =
    simplify_subtype
      ~subtype_env:(make_subtype_env ~ignore_generic_params:true on_error)
      ~this_ty:None
      ty_sub
      ty_super
      env
  in
  decompose_subtype_add_prop env prop

and decompose_subtype_add_prop env prop =
  match prop with
  | TL.Conj props ->
    List.fold_left ~f:decompose_subtype_add_prop ~init:env props
  | TL.Disj (_, []) -> Env.mark_inconsistent env
  | TL.Disj (_, [prop']) -> decompose_subtype_add_prop env prop'
  | TL.Disj _ ->
    Typing_log.log_prop 2 env.function_pos "decompose_subtype_add_prop" env prop;
    env
  | TL.Coerce _ -> failwith "Coercions should have been resolved beforehand"
  | TL.IsSubtype (LoclType ty1, LoclType ty2) ->
    decompose_subtype_add_bound env ty1 ty2
  | TL.IsSubtype _ ->
    failwith
      "Subtyping locl types should yield propositions involving locl types only."

(* Decompose a general constraint *)
and decompose_constraint
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
  log_subtype
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
    add_constraint env ck ty1 ty2 (Errors.unify_error_at p)
  in
  List.fold_left constraints ~f:add_constraint ~init:env

let sub_type_with_dynamic_as_bottom_res
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.error_from_reasons_callback) : (env, env) result =
  let env_change_log = Env.log_env_change "coercion" env in
  log_subtype
    ~level:1
    ~this_ty:None
    ~function_name:"coercion"
    env
    ty_sub
    ty_super;
  let old_env = env in
  let (env, prop) =
    simplify_subtype
      ~subtype_env:
        (make_subtype_env ~coerce:(Some TL.CoerceFromDynamic) on_error)
      ~this_ty:None
      ty_sub
      ty_super
      env
  in
  let (env, prop) = prop_to_env env prop on_error in
  let env = Env.add_subtype_prop env prop in
  let succeeded = process_simplify_subtype_result prop in
  if succeeded then
    Ok (env_change_log env)
  else
    Error (env_change_log old_env)

let sub_type_with_dynamic_as_bottom
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.error_from_reasons_callback) : env =
  match sub_type_with_dynamic_as_bottom_res env ty_sub ty_super on_error with
  | Ok env -> env
  | Error env -> env

let simplify_subtype_i env ty_sub ty_super ~on_error =
  simplify_subtype_i
    ~subtype_env:(make_subtype_env ~no_top_bottom:true on_error)
    ty_sub
    ty_super
    env

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let sub_type_i env ty1 ty2 on_error =
  sub_type_i ~subtype_env:(make_subtype_env ~coerce:None on_error) env ty1 ty2
  |> function
  | Ok env -> env
  | Error env -> env

let subtype_funs
    ~(check_return : bool)
    ~on_error
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env : env =
  let old_env = env in
  (* This is used for checking subtyping of function types for method override
   * (see Typing_subtype_method) so types are fully-explicit and therefore we
   * permit subtyping to dynamic when --enable-sound-dynamic-type is true
   *)
  let (env, prop) =
    simplify_subtype_funs
      ~subtype_env:(make_subtype_env ~coerce:(Some TL.CoerceToDynamic) on_error)
      ~check_return
      r_sub
      ft_sub
      r_super
      ft_super
      env
  in
  let (env, prop) = prop_to_env env prop on_error in
  let env = Env.add_subtype_prop env prop in
  let succeeded = process_simplify_subtype_result prop in
  if succeeded then
    env
  else
    old_env

let sub_type_or_fail env ty1 ty2 fail =
  sub_type env ty1 ty2 (fun ?code:_ _ -> fail ())

let set_fun_refs () =
  Typing_utils.sub_type_ref := sub_type;
  Typing_utils.sub_type_res_ref := sub_type_res;
  Typing_utils.sub_type_i_ref := sub_type_i;
  Typing_utils.sub_type_with_dynamic_as_bottom_ref :=
    sub_type_with_dynamic_as_bottom;
  Typing_utils.sub_type_with_dynamic_as_bottom_res_ref :=
    sub_type_with_dynamic_as_bottom_res;
  Typing_utils.add_constraint_ref := add_constraint;
  Typing_utils.is_sub_type_ref := is_sub_type;
  Typing_utils.is_sub_type_for_coercion_ref := is_sub_type_for_coercion;
  Typing_utils.is_sub_type_for_union_ref := is_sub_type_for_union;
  Typing_utils.is_sub_type_for_union_i_ref := is_sub_type_for_union_i;
  Typing_utils.is_sub_type_ignore_generic_params_ref :=
    is_sub_type_ignore_generic_params

let () = set_fun_refs ()
