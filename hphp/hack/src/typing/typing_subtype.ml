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
module Subst = Decl_subst
module TUtils = Typing_utils
module SN = Naming_special_names
module Phase = Typing_phase
module TL = Typing_logic
module Cls = Decl_provider.Class
module ITySet = Internal_type_set
module MakeType = Typing_make_type
module Partial = Partial_provider
module ShapeMap = Nast.ShapeMap
module ShapeSet = Ast_defs.ShapeSet
module Nast = Aast

type subtype_env = {
  seen_generic_params: SSet.t option;
  no_top_bottom: bool;
  treat_dynamic_as_bottom: bool;
  on_error: Errors.typing_error_callback;
}

let empty_seen = Some SSet.empty

let make_subtype_env
    ?(seen_generic_params = empty_seen)
    ?(no_top_bottom = false)
    ?(treat_dynamic_as_bottom = false)
    on_error =
  { seen_generic_params; no_top_bottom; treat_dynamic_as_bottom; on_error }

let add_seen_generic subtype_env name =
  match subtype_env.seen_generic_params with
  | Some seen ->
    { subtype_env with seen_generic_params = Some (SSet.add name seen) }
  | None -> subtype_env

type reactivity_extra_info = {
  method_info: (* method_name *) (string * (* is_static *) bool) option;
  class_ty: phase_ty option;
  parent_class_ty: phase_ty option;
}

let empty_extra_info =
  { method_info = None; class_ty = None; parent_class_ty = None }

module ConditionTypes = struct
  let try_get_class_for_condition_type (env : env) (ty : decl_ty) =
    match TUtils.try_unwrap_class_type ty with
    | None -> None
    | Some (_, ((_, x) as sid), _) ->
      begin
        match Env.get_class env x with
        | None -> None
        | Some cls -> Some (sid, cls)
      end

  let try_get_method_from_condition_type
      (env : env) (ty : decl_ty) (is_static : bool) (method_name : string) =
    match try_get_class_for_condition_type env ty with
    | Some (_, cls) ->
      let get =
        if is_static then
          Cls.get_smethod
        else
          Cls.get_method
      in
      get cls method_name
    | None -> None

  let localize_condition_type (env : env) (ty : decl_ty) : locl_ty =
    (* if condition type is generic - we cannot specify type argument in attribute.
       For cases when we check if containing type is a subtype of condition type
       if condition type is generic instantiate it with TAny's *)
    let do_localize ty =
      let ty =
        match try_get_class_for_condition_type env ty with
        | None -> ty
        | Some (_, cls) when List.is_empty (Cls.tparams cls) -> ty
        | Some (((p, _) as sid), cls) ->
          let params =
            List.map (Cls.tparams cls) ~f:(fun { tp_name = (p, x); _ } ->
                MakeType.generic (Reason.Rwitness p) x)
          in
          let subst = Decl_instantiate.make_subst (Cls.tparams cls) [] in
          let ty = MakeType.apply (Reason.Rwitness p) sid params in
          Decl_instantiate.instantiate subst ty
      in
      let ety_env = Phase.env_with_self env in
      let (_, t) = Phase.localize ~ety_env env ty in
      t
    in
    match deref ty with
    | (r, Toption ty) -> mk (r, Toption (do_localize ty))
    | _ -> do_localize ty
end

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

(** Check that a mutability type is a subtype of another mutability type *)
let check_mutability
    ~(is_receiver : bool)
    ~subtype_env
    (p_sub : Pos.t)
    (mut_sub : param_mutability option)
    (p_super : Pos.t)
    (mut_super : param_mutability option)
    env =
  let str m =
    match m with
    | None -> "immutable"
    | Some Param_owned_mutable -> "owned mutable"
    | Some Param_borrowed_mutable -> "mutable"
    | Some Param_maybe_mutable -> "maybe-mutable"
  in
  (* maybe-mutable <------immutable
                      |
                       <--mutable <-- owned mutable  *)
  match (mut_sub, mut_super) with
  (* immutable is not compatible with mutable *)
  | (None, Some (Param_borrowed_mutable | Param_owned_mutable))
  (* mutable is not compatible with immutable  *)
  | (Some (Param_borrowed_mutable | Param_owned_mutable), None)
  (* borrowed mutable is not compatible with owned mutable *)
  | (Some Param_borrowed_mutable, Some Param_owned_mutable)
  (* maybe-mutable is not compatible with immutable/mutable *)
  | ( Some Param_maybe_mutable,
      (None | Some (Param_borrowed_mutable | Param_owned_mutable)) ) ->
    invalid
      ~fail:(fun () ->
        Errors.mutability_mismatch
          ~is_receiver
          p_sub
          (str mut_sub)
          p_super
          (str mut_super)
          subtype_env.on_error)
      env
  | _ -> valid env

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

let rec describe_ty_super env ?(short = false) ty =
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
        let prefix =
          if short then
            ""
          else
            "something "
        in
        let sep =
          match (locl_tyl, cstr_tyl) with
          | (_ :: _, _ :: _) -> " and "
          | _ -> ""
        in
        let locl_descr =
          match locl_tyl with
          | [] -> ""
          | tyl ->
            let prefix =
              if short then
                ""
              else
                "of type "
            in
            prefix ^ String.concat ~sep:" & " (List.map tyl ~f:print)
        in
        let cstr_descr =
          String.concat
            ~sep:" and "
            (List.map cstr_tyl ~f:(describe_ty_super env))
        in
        prefix ^ locl_descr ^ sep ^ cstr_descr)
    | Toption ty when is_tyvar ty ->
      "null or " ^ describe_ty_super env (LoclType ty)
    | _ -> default ())
  | ConstraintType ty ->
    (match deref_constraint_type ty with
    | (_, Thas_member hm) ->
      let { hm_name = (_, name); hm_type = _; hm_class_id = _ } = hm in
      Printf.sprintf "an object with member `%s`" name
    | (_, Tdestructure _) ->
      Typing_print.with_blank_tyvars (fun () ->
          Typing_print.full_strip_ns_i env (ConstraintType ty))
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
 * This last one would be left as T <: I if seen_generic_params is None
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
    let ty_super_descr = describe_ty_super env ety_super in
    let ty_sub_descr =
      Typing_print.with_blank_tyvars (fun () ->
          Typing_print.full_strip_ns_i env ety_sub)
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
  let invalid_with f = invalid ~fail:f env in
  let invalid_env env = invalid ~fail env in
  let invalid_env_with env f = invalid ~fail:f env in
  let invalid () = invalid_with fail in
  (* We *know* that the assertion is valid *)
  let valid_env env = valid env in
  let valid () = valid_env env in
  (* We don't know whether the assertion is valid or not *)
  let default () = (env, TL.IsSubtype (ety_sub, ety_super)) in
  (* This function contains typing rules that are based solely on the subtype
   * if you need to pattern match on the super type it should NOT be included
   * here
   *)
  let default_subtype env =
    let ty_sub = ety_sub in
    let ty_super = ety_super in
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
        | _ -> invalid ()
      end
    | LoclType ty_sub ->
      (*
       * t1 | ... | tn <: t
       *   if and only if
       * t1 <: t /\ ... /\ tn <: t
       * We want this even if t is a type variable e.g. consider
       *   int | v <: v
       *)
      begin
        match deref ty_sub with
        | (_, Tunion tyl) ->
          List.fold_left tyl ~init:(env, TL.valid) ~f:(fun res ty_sub ->
              res &&& simplify_subtype_i ~subtype_env (LoclType ty_sub) ty_super)
        | (_, Terr) ->
          if subtype_env.no_top_bottom then
            default ()
          else
            valid ()
        | (_, Tvar _) -> default ()
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
        | (_, Tgeneric name_sub) ->
          (match subtype_env.seen_generic_params with
          | None -> default ()
          | Some seen ->
            (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
             *)
            if SSet.mem name_sub seen then
              invalid ()
            else
              (* If the generic is actually an expression dependent type,
             we need to update this_ty
           *)
              let this_ty =
                if
                  DependentKind.is_generic_dep_ty name_sub
                  && Option.is_none this_ty
                then
                  Some ty_sub
                else
                  this_ty
              in
              let subtype_env = add_seen_generic subtype_env name_sub in
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
                    Reason.Rimplicit_upper_bound (get_pos ty_sub, "?nonnull")
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
                   (Typing_set.elements (Env.get_upper_bounds env name_sub)))
          |> (* Turn error into a generic error about the type parameter *)
          if_unsat invalid
        | (_, Tdynamic) when subtype_env.treat_dynamic_as_bottom -> valid ()
        | _ -> invalid ()
      end
  in
  (* We further refine the default subtype case for rules that apply to all
   * LoclTypes but not to ConstraintTypes
   *)
  let default_subtype env =
    let ty_sub = ety_sub in
    let ty_super = ety_super in
    match ty_super with
    | LoclType ty_super ->
      (match ty_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType ty_sub ->
        begin
          match deref ty_sub with
          | (_, Tvar _) when subtype_env.treat_dynamic_as_bottom ->
            (env, TL.Coerce (ty_sub, ty_super))
          | (_, Tnewtype (_, _, ty)) ->
            simplify_subtype ~subtype_env ~this_ty ty ty_super env
          | (_, Tdependent (_, ty)) ->
            let this_ty = Option.first_some this_ty (Some ty_sub) in
            simplify_subtype ~subtype_env ~this_ty ty ty_super env
          | (r_sub, Tany _) ->
            if subtype_env.no_top_bottom then
              default ()
            else
              let ty_sub = anyfy env r_sub ty_super in
              simplify_subtype ~subtype_env ~this_ty ty_sub ty_super env
          | (r_sub, Tprim Nast.Tvoid) ->
            let r =
              Reason.Rimplicit_upper_bound (Reason.to_pos r_sub, "?nonnull")
            in
            simplify_subtype
              ~subtype_env
              ~this_ty
              (MakeType.mixed r)
              ty_super
              env
            |> if_unsat invalid
          | _ -> default_subtype env
        end)
    | ConstraintType _ -> default_subtype env
  in
  match ety_super with
  (* First deal with internal constraint types *)
  | ConstraintType cty_super ->
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
            let (env, p1) =
              simplify_subtype_i
                ~subtype_env
                ~this_ty
                (LoclType ty_null)
                ty_super
                env
            in
            if TL.is_unsat p1 then
              invalid ()
            else
              let (env, p2) =
                simplify_subtype_i
                  ~subtype_env
                  ~this_ty
                  (LoclType ty)
                  ty_super
                  env
              in
              (env, TL.conj p1 p2)
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
                  fpos)
          | (SplatUnpack, [], None) ->
            invalid_env_with env (fun () ->
                Errors.unpack_array_variadic_argument
                  (Reason.to_pos r_super)
                  fpos)
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
              | Reason.Runpack_param (epos, fpos, c) -> (epos, fpos, c)
              | _ -> (Reason.to_pos r_super, Reason.to_pos r, 0)
            in
            invalid_env_with env (fun () ->
                f (prefix + len_required) (prefix + len_ts) epos fpos)
          in
          if len_ts < len_required then
            arity_error Errors.typing_too_few_args
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
            | ([], None) -> valid ()
            | (_, None) ->
              (* Elements remain but we have nowhere to put them *)
              arity_error Errors.typing_too_many_args
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
            | (_, Tarraykind (AKvarray elt_type)) ->
              env |> destructure_array elt_type
            | (_, Tdynamic) -> env |> destructure_array ty_sub
            (* TODO: should remove these any cases *)
            | (r, Tany _) ->
              let any = mk (r, Typing_defs.make_tany ()) in
              env |> destructure_array any
            | (_, (Tunion _ | Tintersection _ | Tgeneric _ | Tvar _)) ->
              default_subtype env
            | _ ->
              begin
                match d_kind with
                | SplatUnpack ->
                  (* Allow splatting of arbitrary Traversables *)
                  let (env, ty_inner) =
                    Env.fresh_type env (Reason.to_pos r_super)
                  in
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
                  |> if_unsat @@ fun () ->
                     invalid_with (fun () ->
                         Errors.invalid_destructure
                           (Reason.to_pos r_super)
                           (get_pos ty_sub)
                           ty_sub_descr)
              end)
        end
      | ( r,
          Thas_member
            { hm_name = name; hm_type = member_ty; hm_class_id = class_id } ) ->
        (match ety_sub with
        | ConstraintType cty ->
          begin
            match deref_constraint_type cty with
            | ( _,
                Thas_member
                  {
                    hm_name = name_sub;
                    hm_type = ty_sub;
                    hm_class_id = cid_sub;
                  } ) ->
              if Nast.equal_sid name_sub name && class_id_equal cid_sub class_id
              then
                simplify_subtype ~subtype_env ~this_ty ty_sub member_ty env
              else
                invalid ()
            | _ -> default_subtype env
          end
        | LoclType ty_sub ->
          (match deref ty_sub with
          | (_, (Tvar _ | Tunion _ | Terr)) -> default_subtype env
          | (_, Tintersection tyl)
            when let (_, non_ty_opt, _) =
                   find_type_with_exact_negation env tyl
                 in
                 Option.is_some non_ty_opt ->
            default_subtype env
          (* Ideally, we'd want this case to come after the case with an intersection
     on the left, to deal properly with (#1 & A) <: Thas_member(#2) by potentially
     adding an upper bound to #1, but that would result in a disjunction
     which we don't handle very well at the moment.
     TODO: when we have a better treatment of disjunctions, move that case after
     the case with an intersection on the left.
     For now, if there is an intersection on the left here,
     we rely on how obj_get itself treats intersections. If that
     intersection contains a type variable, this type variable will be eagerly
     solved. Once this case is moved, we can clean up obj_get from the Tvar and
     Tintersection cases *)
          | _ ->
            let (obj_get_ty, error_prop) =
              Errors.try_with_result
                (fun () ->
                  let (env, (obj_get_ty, _tal)) =
                    Typing_object_get.obj_get
                      ~obj_pos:(Reason.to_pos r)
                      ~is_method:false
                      ~coerce_from_ty:None
                      ~nullsafe:None
                      env
                      ty_sub
                      class_id
                      name
                      subtype_env.on_error
                  in
                  (obj_get_ty, valid_env env))
                (fun (obj_get_ty, _) error ->
                  (obj_get_ty, invalid_with (fun () -> Errors.add_error error)))
            in
            error_prop
            &&& simplify_subtype ~subtype_env ~this_ty obj_get_ty member_ty))
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
          default ()
        else
          valid ()
      | LoclType lty ->
        (match deref lty with
        | (_, Tunion _) -> default_subtype env
        | (_, Terr) -> valid ()
        | _ ->
          if subtype_env.no_top_bottom then
            default ()
          else
            valid ()))
    | (_, Tvar var_super) ->
      (match ety_sub with
      | ConstraintType cty when is_constraint_type_union cty ->
        default_subtype env
      | ConstraintType _ -> default ()
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (_, (Tunion _ | Terr)) -> default_subtype env
        | (_, Tdynamic) when subtype_env.treat_dynamic_as_bottom ->
          default_subtype env
        (* We want to treat nullable as a union with the same rule as above.
         * This is only needed for Tvar on right; other cases are dealt with specially as
         * derived rules.
         *)
        | (r, Toption t) ->
          let ty_null = MakeType.null r in
          env
          |> simplify_subtype ~subtype_env ~this_ty t ty_super
          &&& simplify_subtype ~subtype_env ~this_ty ty_null ty_super
        | (_, Tvar var_sub) when Ident.equal var_sub var_super -> valid ()
        | _ when subtype_env.treat_dynamic_as_bottom ->
          (env, TL.Coerce (ty_sub, ty_super))
        | _ -> default ()))
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
                | _ -> invalid ()
              end
            | _ -> invalid ())
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
        | (_, Tgeneric _) when Option.is_none subtype_env.seen_generic_params ->
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
          let (env, p1) =
            simplify_subtype_i
              ~subtype_env
              ~this_ty
              (LoclType ty_null)
              ety_super
              env
          in
          if TL.is_unsat p1 then
            invalid ()
          else
            let (env, p2) =
              simplify_subtype_i
                ~subtype_env
                ~this_ty
                (LoclType ty)
                ety_super
                env
            in
            (env, TL.conj p1 p2)
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
        valid ()
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
          | ((_, Tprim Nast.Tnull), _) -> valid ()
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
          | ((_, Tgeneric _), _)
            when Option.is_none subtype_env.seen_generic_params ->
            default_subtype env
          (* If t1 <: ?t2 and t1 is an abstract type constrained as t1',
           * then t1 <: t2 or t1' <: ?t2.  The converse is obviously
           * true as well.  We can fold the case where t1 is unconstrained
           * into the case analysis below.
           *)
          | ((_, (Tnewtype _ | Tdependent _ | Tgeneric _ | Tprim Nast.Tvoid)), _)
            ->
            env
            |> simplify_subtype ~subtype_env ~this_ty lty_sub arg_ty_super
            ||| default_subtype
          (* If ty_sub <: ?ty_super' and ty_sub does not contain null then we
           * must also have ty_sub <: ty_super'.  The converse follows by
           * widening and transitivity.  Therefore, this step preserves the set
           * of solutions.
           *)
          | ( ( _,
                ( Tdynamic | Tprim _ | Tnonnull | Tfun _ | Ttuple _ | Tshape _
                | Tobject | Tclass _ | Tarraykind _ | Tany _ | Terr | Tpu _
                | Tpu_type_access _ ) ),
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
            fail_with_suffix
              ( if equal_dependent_type d_sup (DTcls x) then
                Errors.exact_class_final id p
              else
                Errors.this_final id p )
            else
              fail
          in

          let class_def = Env.get_class env x in
          (match (d_sup, class_def) with
          | (DTthis, Some class_ty) ->
            let tyl_super =
              if
                List.is_empty tyl_super
                && not (Partial.should_check_error (Env.get_mode env) 4029)
              then
                List.map (Cls.tparams class_ty) (fun _ ->
                    mk (r_super, Typing_defs.make_tany ()))
              else
                tyl_super
            in
            if
              not
                (Int.equal
                   (List.length (Cls.tparams class_ty))
                   (List.length tyl_super))
            then
              invalid_with (fun () ->
                  Errors.expected_tparam
                    ~definition_pos:(Cls.pos class_ty)
                    ~use_pos:(Reason.to_pos r_super)
                    (List.length (Cls.tparams class_ty))
                    (Some subtype_env.on_error))
            else
              let ety_env =
                {
                  type_expansions = [];
                  substs = Subst.make_locl (Cls.tparams class_ty) tyl_super;
                  this_ty = Option.value this_ty ~default:ty_super;
                  from_class = None;
                  on_error = subtype_env.on_error;
                  quiet = true;
                }
              in
              let lower_bounds_super = Cls.lower_bounds_on_this class_ty in
              let rec try_constraints lower_bounds_super env =
                match lower_bounds_super with
                | [] -> invalid_with fail
                | ty_super :: lower_bounds_super ->
                  let (env, ty_super) = Phase.localize ~ety_env env ty_super in
                  env
                  |> simplify_subtype ~subtype_env ~this_ty ty_sub ty_super
                  ||| try_constraints lower_bounds_super
              in
              try_constraints lower_bounds_super env
          | _ -> invalid_with fail)
        | ((_, Tdependent (d_sub, bound_sub)), _) ->
          let this_ty = Option.first_some this_ty (Some ty_sub) in
          (* Dependent types are identical but bound might be different *)
          if equal_dependent_type d_sub d_sup then
            simplify_subtype ~subtype_env ~this_ty bound_sub bound_sup env
          else
            simplify_subtype ~subtype_env ~this_ty bound_sub ty_super env
        | _ -> default_subtype env))
    | (_, Tgeneric name_super) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      (* If subtype and supertype are the same generic parameter, we're done *)
      | LoclType ty_sub ->
        (match get_node ty_sub with
        | Tgeneric name_sub when String.equal name_sub name_super -> valid ()
        (* When decomposing subtypes for the purpose of adding bounds on generic
         * parameters to the context, (so seen_generic_params = None), leave
         * subtype so that the bounds get added *)
        | Tvar _
        | Tunion _
        | Terr ->
          default_subtype env
        | _ ->
          (match subtype_env.seen_generic_params with
          | None -> default ()
          | Some seen ->
            (* If we've seen this type parameter before then we must have gone
             * round a cycle so we fail
             *)
            if SSet.mem name_super seen then
              invalid ()
            else
              let subtype_env = add_seen_generic subtype_env name_super in
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
                   (Typing_set.elements (Env.get_lower_bounds env name_super))
              |> if_unsat invalid)))
    | (_, Tnonnull) ->
      (match ety_sub with
      | ConstraintType cty ->
        begin
          match deref_constraint_type cty with
          | (_, (Thas_member _ | Tdestructure _)) -> valid ()
          | _ -> default_subtype env
        end
      | LoclType lty ->
        (match deref lty with
        | ( _,
            ( Tprim
                Nast.(
                  ( Tint | Tbool | Tfloat | Tstring | Tresource | Tnum
                  | Tarraykey | Tnoreturn | Tatom _ ))
            | Tnonnull | Tfun _ | Ttuple _ | Tshape _ | Tobject | Tclass _
            | Tarraykind _ | Tpu _ | Tpu_type_access _ ) ) ->
          valid ()
        | _ -> default_subtype env))
    | (_, Tdynamic) ->
      (match ety_sub with
      | LoclType lty when is_dynamic lty -> valid ()
      | ConstraintType _
      | LoclType _ ->
        default_subtype env)
    | (_, Tprim prim_ty) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match (deref lty, prim_ty) with
        | ((_, Tprim (Nast.Tint | Nast.Tfloat)), Nast.Tnum) -> valid ()
        | ((_, Tprim (Nast.Tint | Nast.Tstring)), Nast.Tarraykey) -> valid ()
        | ((_, Tprim prim_sub), _) when Aast.equal_tprim prim_sub prim_ty ->
          valid ()
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
          valid ()
        | _ -> default_subtype env))
    | (r_super, Tany _) ->
      (match ety_sub with
      | ConstraintType cty ->
        begin
          match deref_constraint_type cty with
          | (_, (TCunion _ | TCintersection _)) -> default_subtype env
          | _ -> valid ()
        end
      | LoclType ty_sub ->
        (match deref ty_sub with
        | (_, Tany _) -> valid ()
        | (_, (Tunion _ | Tintersection _ | Tvar _)) -> default_subtype env
        | _ when subtype_env.no_top_bottom -> default ()
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
    | (_, Tpu (base_super, (_, enum_super))) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        (* TODO: document contravariance *)
        | (_, Tpu (base_sub, (_, enum_sub)))
          when String.equal enum_sub enum_super ->
          simplify_subtype ~subtype_env ~this_ty base_super base_sub env
        (* Atom vs Tpu: check for membership *)
        | (_, Tprim (Aast_defs.Tatom atom))
          when Option.is_some
               @@ snd
               @@ TUtils.class_get_pu_member env base_super enum_super atom ->
          valid ()
        | _ -> default_subtype env))
    | (_, Tpu_type_access (bsuper, esuper, msuper, nsuper)) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType ty_sub ->
        (match get_node ty_sub with
        | Tpu_type_access (bsub, esub, msub, nsub) ->
          if
            String.equal (snd esub) (snd esuper)
            && String.equal (snd nsub) (snd nsuper)
            && String.equal (snd msub) (snd msuper)
          then
            simplify_subtype ~subtype_env ~this_ty bsuper bsub env
          else
            default_subtype env
        | Tunion _
        | Tvar _
        | Tintersection _ ->
          default_subtype env
        | _ ->
          (* TODO: check if err is still needed *)
          let err () =
            let reason = get_reason ty_sub in
            if Typing_utils.is_any env ty_sub then
              ()
            else
              Errors.pu_typing_not_supported (Reason.to_pos reason)
          in
          invalid_env_with env err))
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
      (*
       * shape_field_type A <: shape_field_type B iff:
       *   1. A is no more optional than B
       *   2. A's type <: B.type
       *)
      let simplify_subtype_shape_field r_sub name res sft_sub sft_super =
        match (sft_sub.sft_optional, sft_super.sft_optional) with
        | (_, true)
        | (false, false) ->
          res
          &&& simplify_subtype
                ~subtype_env
                ~this_ty
                sft_sub.sft_ty
                sft_super.sft_ty
        | (true, false) ->
          res
          |> with_error (fun () ->
                 let printable_name =
                   TUtils.get_printable_shape_field_name name
                 in
                 match get_reason sft_sub.sft_ty with
                 | Reason.Rmissing_required_field _ ->
                   Errors.missing_field
                     (Reason.to_pos r_sub)
                     (Reason.to_pos r_super)
                     printable_name
                     subtype_env.on_error
                 | _ ->
                   Errors.required_field_is_optional
                     (Reason.to_pos r_sub)
                     (Reason.to_pos r_super)
                     printable_name
                     subtype_env.on_error)
      in
      let lookup_shape_field_type name r shape_kind fdm =
        match ShapeMap.find_opt name fdm with
        | Some sft -> sft
        | None ->
          let printable_name = TUtils.get_printable_shape_field_name name in
          let sft_ty =
            match shape_kind with
            | Closed_shape ->
              MakeType.nothing
                (Reason.Rmissing_required_field (Reason.to_pos r, printable_name))
            | Open_shape ->
              MakeType.mixed
                (Reason.Rmissing_optional_field (Reason.to_pos r, printable_name))
          in
          { sft_ty; sft_optional = true }
      in
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (r_sub, Tshape (Open_shape, _))
          when equal_shape_kind Closed_shape shape_kind_super ->
          invalid_with (fun () ->
              Errors.shape_fields_unknown
                (Reason.to_pos r_sub)
                (Reason.to_pos r_super)
                subtype_env.on_error)
        | (r_sub, Tshape (shape_kind_sub, fdm_sub)) ->
          ShapeSet.fold
            (fun name res ->
              simplify_subtype_shape_field
                r_sub
                name
                res
                (lookup_shape_field_type name r_sub shape_kind_sub fdm_sub)
                (lookup_shape_field_type
                   name
                   r_super
                   shape_kind_super
                   fdm_super))
            (ShapeSet.of_list (ShapeMap.keys fdm_sub @ ShapeMap.keys fdm_super))
            (env, TL.valid)
        | _ -> default_subtype env))
    | (_, Tarraykind ak_super) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (r_sub, Tarraykind ak_sub) ->
          begin
            match (ak_sub, ak_super) with
            | (AKvarray ty_sub, AKvarray ty_super) ->
              simplify_subtype ~subtype_env ~this_ty ty_sub ty_super env
            | ( AKvarray_or_darray (tk_sub, tv_sub),
                AKvarray_or_darray (tk_super, tv_super) )
            | (AKdarray (tk_sub, tv_sub), AKdarray (tk_super, tv_super))
            | ( AKdarray (tk_sub, tv_sub),
                AKvarray_or_darray (tk_super, tv_super) ) ->
              env
              |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
            | (AKvarray tv_sub, AKvarray_or_darray (tk_super, tv_super)) ->
              let pos = Reason.to_pos r_sub in
              let tk_sub = MakeType.int (Reason.Ridx_vector pos) in
              env
              |> simplify_subtype ~subtype_env ~this_ty tk_sub tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv_sub tv_super
            (* any other array subtyping is unsatisfiable *)
            | _ -> invalid ()
          end
        | _ -> default_subtype env))
    | (_, Tnewtype (name_super, tyl_super, _)) ->
      (match ety_sub with
      | ConstraintType _ -> default_subtype env
      | LoclType lty ->
        (match deref lty with
        | (_, Tclass ((_, name_sub), _, _)) ->
          if String.equal name_sub name_super && Env.is_enum env name_super then
            valid ()
          else
            default_subtype env
        | (_, Tnewtype (name_sub, tyl_sub, _))
          when String.equal name_sub name_super ->
          if List.is_empty tyl_sub then
            valid ()
          else if Env.is_enum env name_super && Env.is_enum env name_sub then
            valid ()
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
              | None -> invalid ()
            end
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
          valid ()
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
          valid ()
        | ( _,
            ( Tarraykind _
            | Tprim Nast.(Tstring | Tarraykey | Tint | Tfloat | Tnum) ) )
          when String.equal class_name SN.Classes.cXHPChild
               && equal_exact exact_super Nonexact ->
          valid ()
        | (_, Tprim Nast.Tstring)
          when String.equal class_name SN.Classes.cStringish
               && equal_exact exact_super Nonexact ->
          valid ()
        (* Match what's done in unify for non-strict code *)
        | (_, Tobject)
          when not @@ Partial.should_check_error (Env.get_mode env) 4110 ->
          valid ()
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
              valid ()
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
                invalid ()
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
                  invalid_with (fun () ->
                      Errors.type_arity_mismatch
                        (fst x_super)
                        n_super
                        (fst x_sub)
                        n_sub
                        subtype_env.on_error)
                else
                  let variance_reifiedl =
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
            invalid ()
          else
            let class_def_sub = Env.get_class env cid_sub in
            (match class_def_sub with
            | None ->
              (* This should have been caught already in the naming phase *)
              valid ()
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
              if
                not
                  (Int.equal
                     (List.length (Cls.tparams class_sub))
                     (List.length tyl_sub))
              then
                invalid_with (fun () ->
                    Errors.expected_tparam
                      ~definition_pos:(Cls.pos class_sub)
                      ~use_pos:(Reason.to_pos r_sub)
                      (List.length (Cls.tparams class_sub))
                      (Some subtype_env.on_error))
              else
                let ety_env =
                  {
                    type_expansions = [];
                    substs = Subst.make_locl (Cls.tparams class_sub) tyl_sub;
                    (* TODO: do we need this? *)
                    this_ty = Option.value this_ty ~default:ty_sub;
                    from_class = None;
                    quiet = true;
                    on_error = subtype_env.on_error;
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
                    || Ast_defs.(
                         equal_class_kind (Cls.kind class_sub) Cinterface)
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
                    invalid ()))
        | (r_sub, Tarraykind akind) ->
          (match (exact_super, tyl_super) with
          | (Nonexact, [tv_super])
            when String.equal class_name SN.Collections.cTraversable
                 || String.equal class_name SN.Rx.cTraversable
                 || String.equal class_name SN.Collections.cContainer ->
            (match akind with
            (* vec<tv> <: Traversable<tv_super>
             * iff tv <: tv_super
             * Likewise for vec<tv> <: Container<tv_super>
             *          and map<_,tv> <: Traversable<tv_super>
             *          and map<_,tv> <: Container<tv_super>
             *)
            | AKvarray tv
            | AKdarray (_, tv)
            | AKvarray_or_darray (_, tv) ->
              simplify_subtype ~subtype_env ~this_ty tv tv_super env)
          | (Nonexact, [tk_super; tv_super])
            when String.equal class_name SN.Collections.cKeyedTraversable
                 || String.equal class_name SN.Rx.cKeyedTraversable
                 || String.equal class_name SN.Collections.cKeyedContainer ->
            (match akind with
            | AKvarray tv ->
              env
              |> simplify_subtype
                   ~subtype_env
                   ~this_ty
                   (MakeType.int r_sub)
                   tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv tv_super
            | AKvarray_or_darray (tk, tv)
            | AKdarray (tk, tv) ->
              env
              |> simplify_subtype ~subtype_env ~this_ty tk tk_super
              &&& simplify_subtype ~subtype_env ~this_ty tv tv_super)
          | (Nonexact, [])
            when String.equal class_name SN.Collections.cKeyedTraversable
                 || String.equal class_name SN.Rx.cKeyedTraversable
                 || String.equal class_name SN.Collections.cKeyedContainer ->
            (* All arrays are subtypes of the untyped KeyedContainer / Traversables *)
            valid ()
          | (_, _) -> default_subtype env)
        | _ -> default_subtype env)))

and simplify_subtype_variance
    ~(subtype_env : subtype_env)
    (cid : string)
    (variance_reifiedl : (Ast_defs.variance * Aast.reify_kind) list)
    (children_tyl : locl_ty list)
    (super_tyl : locl_ty list) : env -> env * TL.subtype_prop =
 fun env ->
  let simplify_subtype reify_kind =
    (* When doing coercions we treat dynamic as a bottom type. This is generally
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
      if not Aast.(equal_reify_kind reify_kind Erased) then
        { subtype_env with treat_dynamic_as_bottom = false }
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
    ?(is_method : bool = false)
    ?(check_params_reactivity = false)
    ?(check_params_mutability = false)
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
    ( env
    |> begin
         if check_params_reactivity then
           simplify_subtype_fun_params_reactivity ~subtype_env sub super
         else
           valid
       end
    &&&
    if check_params_mutability then
      check_mutability
        ~is_receiver:false
        ~subtype_env
        sub.fp_pos
        sub.fp_mutability
        super.fp_pos
        super.fp_mutability
    else
      valid )
    &&& fun env ->
    let { fp_type = ty_sub; _ } = sub in
    let { fp_type = ty_super; _ } = super in
    (* Check that the calling conventions of the params are compatible. *)
    env
    |> simplify_param_modes ~subtype_env sub super
    &&& simplify_param_accept_disposable ~subtype_env sub super
    &&& begin
          fun env ->
          match (sub.fp_kind, super.fp_kind) with
          | (FPinout, FPinout) ->
            (* Inout parameters are invariant wrt subtyping for function types. *)
            env
            |> simplify_subtype_possibly_enforced ty_super ty_sub
            &&& simplify_subtype_possibly_enforced ty_sub ty_super
          | _ -> env |> simplify_subtype_possibly_enforced ty_sub ty_super
        end
    &&& simplify_subtype_params
          ~is_method
          subl
          superl
          variadic_sub_ty
          variadic_super_ty

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

and simplify_subtype_reactivity
    ~subtype_env
    ?(extra_info : reactivity_extra_info option)
    ?(is_call_site = false)
    p_sub
    (r_sub : reactivity)
    p_super
    (r_super : reactivity)
    (env : env) : env * TL.subtype_prop =
  let fail () =
    let msg_super =
      "This function is " ^ TUtils.reactivity_to_string env r_super ^ "."
    in
    let msg_sub =
      "This function is " ^ TUtils.reactivity_to_string env r_sub ^ "."
    in
    subtype_env.on_error [(p_super, msg_super); (p_sub, msg_sub)]
  in
  let ( ||| ) = ( ||| ) ~fail in
  let invalid () = invalid ~fail env in
  let valid () = valid env in
  let maybe_localize t =
    match t with
    | DeclTy t ->
      let ety_env = Phase.env_with_self env in
      let (_, t) = Phase.localize ~ety_env env t in
      t
    | LoclTy t -> t
  in
  let class_ty =
    Option.bind extra_info (fun { class_ty = cls; _ } ->
        Option.map cls ~f:maybe_localize)
  in
  (* for method declarations check if condition type for r_super includes
     reactive method with a matching name. If yes - then it will act as a guarantee
     that derived class will have to redefine the method with a shape required
     by condition type (reactivity of redefined method must be subtype of reactivity
     of method in interface) *)
  let check_condition_type_has_matching_reactive_method env =
    (* function type TSub of method M with arbitrary reactivity in derive class
     can be subtype of conditionally reactive function type TSuper of method M
     defined in base class when condition type has reactive method M.
     interface Rx {
       <<__Rx>>
       public function f(): int;
     }
     class A {
       <<__RxIfImplements(Rx::class)>>
       public function f(): int { ... }
     }
     class B extends A {
       public function f(): int { ... }
     }
     This should be OK because:
     - B does not implement Rx (B::f is not compatible with Rx::f) which means
     that calling ($b : B)->f() will not be treated as reactive
     - if one of subclasses of B will decide to implement B - they will be forced
     to redeclare f which now will shadow B::f. Note that B::f will still be
     accessible as parent::f() but will be treated as non-reactive call.
     *)
    match (r_super, extra_info) with
    | ( ( Reactive (Some condition_type_super)
        | Shallow (Some condition_type_super)
        | Local (Some condition_type_super) ),
        Some { method_info = Some (method_name, is_static); _ } ) ->
      let m =
        ConditionTypes.try_get_method_from_condition_type
          env
          condition_type_super
          is_static
          method_name
      in
      begin
        (* check that reactivity of interface method (effectively a promised
       reactivity of a method in derived class) is a subtype of r_super.
       NOTE: we check only for unconditional reactivity since conditional
       version does not seems to yield a lot and will requre implementing
       cycle detection for condition types *)
        match m with
        | Some { ce_type = (lazy ty); _ } ->
          begin
            match get_node ty with
            | Tfun
                {
                  ft_reactive =
                    (Reactive None | Shallow None | Local None) as fr;
                  _;
                } ->
              let extra_info =
                {
                  empty_extra_info with
                  parent_class_ty = Some (DeclTy condition_type_super);
                }
              in
              simplify_subtype_reactivity
                ~subtype_env
                ~extra_info
                p_sub
                fr
                p_super
                r_super
                env
            | _ -> invalid ()
          end
        | _ -> invalid ()
      end
    | _ -> invalid ()
  in
  match (r_sub, r_super) with
  (* anything is a subtype of nonreactive functions *)
  | (_, Nonreactive) -> valid ()
  (* to compare two maybe reactive values we need to unwrap them *)
  | (MaybeReactive sub, MaybeReactive super) ->
    simplify_subtype_reactivity
      ~subtype_env
      ?extra_info
      ~is_call_site
      p_sub
      sub
      p_super
      super
      env
  (* for explicit checks at callsites implicitly unwrap maybereactive value:
     function f(<<__AtMostRxAsFunc>> F $f)
     f(<<__RxLocal>> () ==> {... })
     here parameter will be maybereactive and argument - rxlocal
     *)
  | (sub, MaybeReactive super) when is_call_site ->
    simplify_subtype_reactivity
      ~subtype_env
      ?extra_info
      ~is_call_site
      p_sub
      sub
      p_super
      super
      env
  (* if is_call_site is falst ignore maybereactive flavors.
     This usually happens during subtype checks for arguments and when target
     function is conditionally reactive we'll do the proper check
     in typing_reactivity.check_call. *)
  | (_, MaybeReactive _) when not is_call_site -> valid ()
  (* ok:
    class A { function f((function(): int) $f) {} }
    class B extends A {
      <<__Rx>>
      function f(<<__AtMostRxAsFunc>> (function(): int) $f);
    }
    reactivity for arguments is checked contravariantly *)
  | (_, RxVar None)
  (* ok:
     <<__Rx>>
     function f(<<__AtMostRxAsFunc>> (function(): int) $f) { return $f() }  *)
  | (RxVar None, RxVar _) ->
    valid ()
  | (RxVar (Some sub), RxVar (Some super))
  | (sub, RxVar (Some super)) ->
    simplify_subtype_reactivity
      ~subtype_env
      ?extra_info
      ~is_call_site
      p_sub
      sub
      p_super
      super
      env
  | (RxVar _, _) -> invalid ()
  | ((Local cond_sub | Shallow cond_sub | Reactive cond_sub), Local cond_super)
  | ((Shallow cond_sub | Reactive cond_sub), Shallow cond_super)
  | (Reactive cond_sub, Reactive cond_super) ->
    env
    |> simplify_subtype_param_rx_if_impl
         ~subtype_env
         ~is_param:false
         p_sub
         cond_sub
         class_ty
         p_super
         cond_super
    ||| check_condition_type_has_matching_reactive_method
  (* call_site specific cases *)
  (* shallow can call into local *)
  | (Local cond_sub, Shallow cond_super) when is_call_site ->
    simplify_subtype_param_rx_if_impl
      ~subtype_env
      ~is_param:false
      p_sub
      cond_sub
      class_ty
      p_super
      cond_super
      env
  (* local can call into non-reactive *)
  | (Nonreactive, Local _) when is_call_site -> valid ()
  | _ -> check_condition_type_has_matching_reactive_method env

and should_check_fun_params_reactivity (ft_super : locl_fun_type) =
  not (equal_reactivity ft_super.ft_reactive Nonreactive)

(* checks condition described by OnlyRxIfImpl condition on parameter is met  *)
and simplify_subtype_param_rx_if_impl
    ~subtype_env
    ~is_param
    p_sub
    (cond_type_sub : decl_ty option)
    (declared_type_sub : locl_ty option)
    p_super
    (cond_type_super : decl_ty option)
    (env : env) : env * TL.subtype_prop =
  let cond_type_sub =
    Option.map cond_type_sub ~f:(ConditionTypes.localize_condition_type env)
  in
  let cond_type_super =
    Option.map cond_type_super ~f:(ConditionTypes.localize_condition_type env)
  in
  let invalid () =
    ( env,
      TL.invalid ~fail:(fun () ->
          Errors.rx_parameter_condition_mismatch
            SN.UserAttributes.uaOnlyRxIfImpl
            p_sub
            p_super
            subtype_env.on_error) )
  in
  match (cond_type_sub, cond_type_super) with
  (* no condition types - do nothing *)
  | (None, None) -> valid env
  (* condition type is specified only for super - ok for receiver case (is_param is false)
    abstract class A {
      <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
      public abstract function condlocalrx(): int;
    }
    abstract class B extends A {
      // ok to override cond local with local (if condition is not met - method
      // in base class is non-reactive )
      <<__Override, __RxLocal>>
      public function condlocalrx(): int {
        return 1;
      }
    }
    for parameters we need to verify that declared type of sub is a subtype of
    conditional type for super. Here is an example where this is violated:

    interface A {}
    interface RxA {}

    class C1 {
      <<__Rx>>
      public function f(A $a): void {
      }
    }

    class C2 extends C1 {
      // ERROR: invariant f body is reactive iff $a instanceof RxA can be violated
      <<__Rx, __AtMostRxAsArgs>>
      public function f(<<__OnlyRxIfImpl(RxA::class)>>A $a): void {
      }
    }
    here declared type of sub is A
    and cond type of super is RxA
  *)
  | (None, Some _) when not is_param -> valid env
  | (None, Some cond_type_super) ->
    begin
      match declared_type_sub with
      | None -> invalid ()
      | Some declared_type_sub ->
        simplify_subtype ~subtype_env declared_type_sub cond_type_super env
    end
  (* condition types are set for both sub and super types: contravariant check
    interface A {}
    interface B extends A {}
    interface C extends B {}

    interface I1 {
      <<__Rx, __OnlyRxIfImpl(B::class)>>
      public function f(): void;
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(B::class)>> A $a): void;
    }
    interface I2 extends I1 {
      // OK since condition in I1::f covers I2::f
      <<__Rx, __OnlyRxIfImpl(A::class)>>
      public function f(): void;
      // OK since condition in I1::g covers I2::g
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(A::class)>> A $a): void;
    }
    interface I3 extends I1 {
      // Error since condition in I1::f is less strict that in I3::f
      <<__Rx, __OnlyRxIfImpl(C::class)>>
      public function f(): void;
      // Error since condition in I1::g is less strict that in I3::g
      <<__Rx>>
      public function g(<<__OnlyRxIfImpl(C::class)>> A $a): void;
    }
   *)
  | (Some cond_type_sub, Some cond_type_super) ->
    if is_param then
      simplify_subtype ~subtype_env cond_type_sub cond_type_super env
    else
      simplify_subtype ~subtype_env cond_type_super cond_type_sub env
  (* condition type is set for super type, check if declared type of
     subtype is a subtype of condition type
     interface Rx {
       <<__Rx>>
       public function f(int $a): void;
     }
     class A<T> {
       <<__Rx, __OnlyRxIfImpl(Rx::class)>>
       public function f(T $a): void {
       }
     }
     // B <: Rx so B::f is completely reactive
     class B extends A<int> implements Rx {
     } *)
  | (Some cond_type_sub, None) ->
    if is_param then
      valid env
    else begin
      match declared_type_sub with
      | None -> invalid ()
      | Some declared_type_sub ->
        simplify_subtype ~subtype_env declared_type_sub cond_type_sub env
    end

(* checks reactivity conditions for function parameters *)
and simplify_subtype_fun_params_reactivity
    ~subtype_env (p_sub : locl_fun_param) (p_super : locl_fun_param) env =
  match (p_sub.fp_rx_annotation, p_super.fp_rx_annotation) with
  (* no conditions on parameters - do nothing *)
  | (None, None) -> valid env
  (* both parameters are conditioned to be rx function - no need to check anything *)
  | (Some Param_rx_var, Some Param_rx_var) -> valid env
  | (None, Some Param_rx_var) ->
    (* parameter is conditionally reactive in supertype and missing condition
      in subtype - this is ok only if parameter in subtype is reactive
      <<__Rx>>
      function super((function(): int) $f);
      <<__Rx>>
      function sub(<<__AtMostRxAsFunc>> (function(): int) $f);
      We check if sub <: super. parameters are checked contravariantly
      so we need to verify that
      (function(): int) $f <: <<__AtMostRxAsFunc>> (function(): int) $f

      Suppose this is legal, then this will be allowed (in pseudo-code)

      function sub(<<__AtMostRxAsFunc>> (function(): int) $f) {
        $f(); // can call function here since it is conditionally reactive
      }
      <<__Rx>>
      function g() {
        $f: super = sub;
        // invoke non-reactive code in reactive context which is bad
        $f(() ==> { echo 1; });
      }
    }
    It will be safe if parameter in super will be completely reactive,
    hence check below *)
    let (_, p_sub_type) = Env.expand_type env p_sub.fp_type.et_type in
    begin
      match get_node p_sub_type with
      | Tfun tfun when not (equal_reactivity tfun.ft_reactive Nonreactive) ->
        valid env
      | Tfun _ ->
        ( env,
          TL.invalid ~fail:(fun () ->
              Errors.rx_parameter_condition_mismatch
                SN.UserAttributes.uaAtMostRxAsFunc
                p_sub.fp_pos
                p_super.fp_pos
                subtype_env.on_error) )
      (* parameter type is not function - error will be reported in different place *)
      | _ -> valid env
    end
  | (cond_sub, cond_super) ->
    let cond_type_sub =
      match cond_sub with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None
    in
    let cond_type_super =
      match cond_super with
      | Some (Param_rx_if_impl t) -> Some t
      | _ -> None
    in
    let subtype_env =
      {
        subtype_env with
        on_error =
          (fun ?code:_ _ ->
            Errors.rx_parameter_condition_mismatch
              SN.UserAttributes.uaOnlyRxIfImpl
              p_sub.fp_pos
              p_super.fp_pos
              subtype_env.on_error);
      }
    in
    simplify_subtype_param_rx_if_impl
      ~subtype_env
      ~is_param:true
      p_sub.fp_pos
      cond_type_sub
      (Some p_sub.fp_type.et_type)
      p_super.fp_pos
      cond_type_super
      env

and simplify_param_modes ~subtype_env param1 param2 env =
  let { fp_pos = pos1; fp_kind = mode1; _ } = param1 in
  let { fp_pos = pos2; fp_kind = mode2; _ } = param2 in
  match (mode1, mode2) with
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
  let { fp_pos = pos1; fp_accept_disposable = mode1; _ } = param1 in
  let { fp_pos = pos2; fp_accept_disposable = mode2; _ } = param2 in
  match (mode1, mode2) with
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

(* Helper function for subtyping on function types: performs all checks that
 * don't involve actual types:
 *   <<__ReturnDisposable>> attribute
 *   <<__MutableReturn>> attribute
 *   <<__Rx>> attribute
 *   <<__Mutable>> attribute
 *   whether or not the function is a coroutine
 *   variadic arity
 *)
and simplify_subtype_funs_attributes
    ~subtype_env
    ?(extra_info : reactivity_extra_info option)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env =
  let p_sub = Reason.to_pos r_sub in
  let p_super = Reason.to_pos r_super in
  let on_error ?code:_ _ =
    Errors.fun_reactivity_mismatch
      p_super
      (TUtils.reactivity_to_string env ft_super.ft_reactive)
      p_sub
      (TUtils.reactivity_to_string env ft_sub.ft_reactive)
      subtype_env.on_error
  in
  simplify_subtype_reactivity
    ~subtype_env:{ subtype_env with on_error }
    ?extra_info
    p_sub
    ft_sub.ft_reactive
    p_super
    ft_super.ft_reactive
    env
  |> check_with
       (Bool.equal (get_ft_is_coroutine ft_sub) (get_ft_is_coroutine ft_super))
       (fun () ->
         Errors.coroutinness_mismatch
           (get_ft_is_coroutine ft_super)
           p_super
           p_sub
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
  |> (* it is ok for subclass to return mutably owned value and treat it as immutable -
  the fact that value is mutably owned guarantees it has only single reference so
  as a result this single reference will be immutable. However if super type
  returns mutable value and subtype yields immutable value - this is not safe.
  NOTE: error is not reported if child is non-reactive since it does not have
  immutability-by-default behavior *)
  check_with
    ( Bool.equal
        (get_ft_returns_mutable ft_sub)
        (get_ft_returns_mutable ft_super)
    || (not (get_ft_returns_mutable ft_super))
    || equal_reactivity ft_sub.ft_reactive Nonreactive )
    (fun () ->
      Errors.mutable_return_result_mismatch
        (get_ft_returns_mutable ft_super)
        p_super
        p_sub
        subtype_env.on_error)
  |> check_with
       ( equal_reactivity ft_super.ft_reactive Nonreactive
       || get_ft_returns_void_to_rx ft_super
       || not (get_ft_returns_void_to_rx ft_sub) )
       (fun () ->
         (*  __ReturnsVoidToRx can be omitted on subtype, in this case using subtype
       via reference to supertype in rx context will be ok since result will be
       discarded. The opposite is not true:
       class A {
         <<__Rx, __Mutable>>
         public function f(): A { return new A(); }
       }
       class B extends A {
         <<__Rx, __Mutable, __ReturnsVoidToRx>>
         public function f(): A { return $this; }
       }

       <<__Rx, __MutableReturn>>
       function f(): A { return new B(); }
       $a = HH\Rx\mutable(f());
       $a1 = $a->f(); // immutable alias to mutable reference *)
         Errors.return_void_to_rx_mismatch
           ~pos1_has_attribute:true
           p_sub
           p_super
           subtype_env.on_error)
  |>
  (* check mutability only for reactive functions *)
  let check_params_mutability =
    (not (equal_reactivity ft_super.ft_reactive Nonreactive))
    && not (equal_reactivity ft_sub.ft_reactive Nonreactive)
  in
  fun (env, prop) ->
    ( if check_params_mutability (* check mutability of receivers *) then
      (env, prop)
      &&& check_mutability
            ~is_receiver:true
            ~subtype_env
            p_super
            (get_ft_param_mutable ft_super)
            p_sub
            (get_ft_param_mutable ft_sub)
    else
      (env, prop) )
    |> check_with
         (arity_min ft_sub.ft_arity <= arity_min ft_super.ft_arity)
         (fun () -> Errors.fun_too_many_args p_sub p_super subtype_env.on_error)
    |> fun res ->
    match (ft_sub.ft_arity, ft_super.ft_arity) with
    | (Fellipsis _, Fvariadic _) ->
      (* The HHVM runtime ignores "..." entirely, but knows about
       * "...$args"; for contexts for which the runtime enforces method
       * compatibility (currently, inheritance from abstract/interface
       * methods), letting "..." override "...$args" would result in method
       * compatibility errors at runtime. *)
      with_error
        (fun () ->
          Errors.fun_variadicity_hh_vs_php56 p_sub p_super subtype_env.on_error)
        res
    | (Fstandard _, Fstandard _) ->
      let sub_max = List.length ft_sub.ft_params in
      let super_max = List.length ft_super.ft_params in
      if sub_max < super_max then
        with_error
          (fun () -> Errors.fun_too_few_args p_sub p_super subtype_env.on_error)
          res
      else
        res
    | (Fstandard _, _) ->
      with_error
        (fun () ->
          Errors.fun_unexpected_nonvariadic p_sub p_super subtype_env.on_error)
        res
    | (_, _) -> res

and simplify_subtype_possibly_enforced
    ~(subtype_env : subtype_env) et_sub et_super =
  simplify_subtype ~subtype_env et_sub.et_type et_super.et_type

(* This implements basic subtyping on non-generic function types:
 *   (1) return type behaves covariantly
 *   (2) parameter types behave contravariantly
 *   (3) special casing for variadics, and various reactivity and mutability attributes
 *)
and simplify_subtype_funs
    ~(subtype_env : subtype_env)
    ~(check_return : bool)
    ?(extra_info : reactivity_extra_info option)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    env : env * TL.subtype_prop =
  let variadic_subtype =
    match ft_sub.ft_arity with
    | Fvariadic (_, { fp_type = var_sub; _ }) -> Some var_sub
    | _ -> None
  in
  let variadic_supertype =
    match ft_super.ft_arity with
    | Fvariadic (_, { fp_type = var_super; _ }) -> Some var_super
    | _ -> None
  in
  let simplify_subtype_possibly_enforced =
    simplify_subtype_possibly_enforced ~subtype_env
  in
  let simplify_subtype_params = simplify_subtype_params ~subtype_env in
  (* First apply checks on attributes, coroutine-ness and variadic arity *)
  env
  |> simplify_subtype_funs_attributes
       ~subtype_env
       ?extra_info
       r_sub
       ft_sub
       r_super
       ft_super
  &&& (* Now do contravariant subtyping on parameters *)
  begin
    match (variadic_subtype, variadic_supertype) with
    | (Some var_sub, Some var_super) ->
      simplify_subtype_possibly_enforced var_super var_sub
    | _ -> valid
  end
  &&& begin
        let check_params_mutability =
          (not (equal_reactivity ft_super.ft_reactive Nonreactive))
          && not (equal_reactivity ft_sub.ft_reactive Nonreactive)
        in
        let is_method =
          Option.equal
            Bool.equal
            (Option.map extra_info (fun i -> Option.is_some i.method_info))
            (Some true)
        in
        simplify_subtype_params
          ~is_method
          ~check_params_reactivity:(should_check_fun_params_reactivity ft_super)
          ~check_params_mutability
          ft_super.ft_params
          ft_sub.ft_params
          variadic_subtype
          variadic_supertype
      end
  &&&
  (* Finally do covariant subtryping on return type *)
  if check_return then
    simplify_subtype_possibly_enforced ft_sub.ft_ret ft_super.ft_ret
  else
    valid

(* One of the main entry points to this module *)
and sub_type_i
    ~subtype_env (env : env) (ty_sub : internal_type) (ty_super : internal_type)
    : env =
  Env.log_env_change "sub_type" env
  @@
  let old_env = env in
  let (env, success) =
    sub_type_inner ~subtype_env env ~this_ty:None ty_sub ty_super
  in
  if success then
    env
  else
    old_env

and sub_type env (ty_sub : locl_ty) (ty_super : locl_ty) on_error =
  sub_type_i
    ~subtype_env:(make_subtype_env ~treat_dynamic_as_bottom:false on_error)
    env
    (LoclType ty_sub)
    (LoclType ty_super)

(* Add a new upper bound ty on var.  Apply transitivity of sutyping,
 * so if we already have tyl <: var, then check that for each ty_sub
 * in tyl we have ty_sub <: ty.
 *)
and add_tyvar_upper_bound_and_close
    ~treat_dynamic_as_bottom (env, prop) var ty on_error =
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
        let env =
          Typing_subtype_pocket_universes.make_all_pu_equal
            env
            var
            upper_bound
            ~on_error
        in
        ITySet.fold
          (fun lower_bound (env, prop1) ->
            let (env, prop2) =
              simplify_subtype_i
                ~subtype_env:
                  (make_subtype_env ~treat_dynamic_as_bottom on_error)
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
and add_tyvar_lower_bound_and_close
    ~treat_dynamic_as_bottom (env, prop) var ty on_error =
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
        let env =
          Typing_subtype_pocket_universes.make_all_pu_equal
            env
            var
            lower_bound
            ~on_error
        in
        ITySet.fold
          (fun upper_bound (env, prop1) ->
            let (env, prop2) =
              simplify_subtype_i
                ~subtype_env:
                  (make_subtype_env ~treat_dynamic_as_bottom on_error)
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
              ~treat_dynamic_as_bottom:false
              (valid env)
              var_sub
              ty_super
              on_error
          in
          let (env, prop2) =
            add_tyvar_lower_bound_and_close
              ~treat_dynamic_as_bottom:false
              (valid env)
              var_super
              ty_sub
              on_error
          in
          props_to_env env remain (prop1 :: prop2 :: props) on_error
        | (Some var, _) ->
          let (env, prop) =
            add_tyvar_upper_bound_and_close
              ~treat_dynamic_as_bottom:false
              (valid env)
              var
              ty_super
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | (_, Some var) ->
          let (env, prop) =
            add_tyvar_lower_bound_and_close
              ~treat_dynamic_as_bottom:false
              (valid env)
              var
              ty_sub
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | _ -> props_to_env env (prop :: remain) props on_error
      end
    | TL.Coerce (ty_sub, ty_super) ->
      begin
        match (get_node ty_sub, get_node ty_super) with
        | (Tvar var_sub, Tvar var_super) ->
          let (env, prop1) =
            add_tyvar_upper_bound_and_close
              ~treat_dynamic_as_bottom:true
              (valid env)
              var_sub
              (LoclType ty_super)
              on_error
          in
          let (env, prop2) =
            add_tyvar_lower_bound_and_close
              ~treat_dynamic_as_bottom:true
              (valid env)
              var_super
              (LoclType ty_sub)
              on_error
          in
          props_to_env env remain (prop1 :: prop2 :: props) on_error
        | (Tvar var, _) ->
          let (env, prop) =
            add_tyvar_upper_bound_and_close
              ~treat_dynamic_as_bottom:true
              (valid env)
              var
              (LoclType ty_super)
              on_error
          in
          props_to_env env remain (prop :: props) on_error
        | (_, Tvar var) ->
          let (env, prop) =
            add_tyvar_lower_bound_and_close
              ~treat_dynamic_as_bottom:true
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
    ~function_name:"sub_type_inner"
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

and is_sub_type_alt_i
    ~ignore_generic_params ~no_top_bottom ~treat_dynamic_as_bottom env ty1 ty2 =
  let (this_ty, pos) =
    match ty1 with
    | LoclType ty1 -> (Some ty1, get_pos ty1)
    | ConstraintType _ -> (None, Pos.none)
  in
  let (_env, prop) =
    simplify_subtype_i
      ~subtype_env:
        {
          seen_generic_params =
            ( if ignore_generic_params then
              None
            else
              empty_seen );
          no_top_bottom;
          treat_dynamic_as_bottom;
          on_error = Errors.unify_error_at pos;
        }
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
    ~treat_dynamic_as_bottom:false
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_coercion env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:false
    ~treat_dynamic_as_bottom:true
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_union env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~treat_dynamic_as_bottom:false
    env
    ty1
    ty2
  = Some true

and is_sub_type_for_union_i env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt_i
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~treat_dynamic_as_bottom:false
    env
    ty1
    ty2
  = Some true

and can_sub_type env ty1 ty2 =
  let ( <> ) a b = not (Option.equal Bool.equal a b) in
  is_sub_type_alt
    ~ignore_generic_params:false
    ~no_top_bottom:true
    ~treat_dynamic_as_bottom:false
    env
    ty1
    ty2
  <> Some false

and is_sub_type_ignore_generic_params env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt
    ~ignore_generic_params:true
    ~no_top_bottom:true
    ~treat_dynamic_as_bottom:false
    env
    ty1
    ty2
  = Some true

and is_sub_type_ignore_generic_params_i env ty1 ty2 =
  let ( = ) = Option.equal Bool.equal in
  is_sub_type_alt_i
    ~ignore_generic_params:true
    ~no_top_bottom:true
    ~treat_dynamic_as_bottom:false
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
      let default () = ty' :: try_intersect_i env ty tyl' in
      (match (ty, ty') with
      | (LoclType lty, _)
        when is_sub_type_ignore_generic_params_i env ty' nonnull_ty ->
        begin
          match get_node lty with
          | Toption t -> try_intersect_i env (LoclType t) (ty' :: tyl')
          | _ -> default ()
        end
      | (_, LoclType lty)
        when is_sub_type_ignore_generic_params_i env ty nonnull_ty ->
        begin
          match get_node lty with
          | Toption t -> try_intersect_i env (LoclType t) (ty :: tyl')
          | _ -> default ()
        end
      | (_, _) -> default ())

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

let subtype_reactivity
    ?extra_info ?is_call_site env p_sub r_sub p_super r_super on_error =
  let subtype_env = make_subtype_env ~treat_dynamic_as_bottom:false on_error in
  let (env, prop) =
    simplify_subtype_reactivity
      ~subtype_env
      ?extra_info
      ?is_call_site
      p_sub
      r_sub
      p_super
      r_super
      env
  in
  let (env, prop) = prop_to_env env prop subtype_env.on_error in
  ignore (process_simplify_subtype_result prop);
  env

let decompose_subtype_add_bound
    (env : env) (ty_sub : locl_ty) (ty_super : locl_ty) : env =
  let (env, ty_super) = Env.expand_type env ty_super in
  let (env, ty_sub) = Env.expand_type env ty_sub in
  match (get_node ty_sub, get_node ty_super) with
  | (_, Tany _) -> env
  (* name_sub <: ty_super so add an upper bound on name_sub *)
  | (Tgeneric name_sub, _) when not (phys_equal ty_sub ty_super) ->
    log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_upper_bounds env name_sub in
    (* Don't add the same type twice! *)
    if Typing_set.mem ty_super tys then
      env
    else
      Env.add_upper_bound ~intersect:(try_intersect env) env name_sub ty_super
  (* ty_sub <: name_super so add a lower bound on name_super *)
  | (_, Tgeneric name_super) when not (phys_equal ty_sub ty_super) ->
    log_subtype
      ~level:2
      ~this_ty:None
      ~function_name:"decompose_subtype_add_bound"
      env
      ty_sub
      ty_super;
    let tys = Env.get_lower_bounds env name_super in
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
    p
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.typing_error_callback) : env =
  log_subtype
    ~level:2
    ~this_ty:None
    ~function_name:"decompose_subtype"
    env
    ty_sub
    ty_super;
  let (env, prop) =
    simplify_subtype
      ~subtype_env:(make_subtype_env ~seen_generic_params:None on_error)
      ~this_ty:None
      ty_sub
      ty_super
      env
  in
  decompose_subtype_add_prop p env prop

and decompose_subtype_add_prop p env prop =
  match prop with
  | TL.Conj props ->
    List.fold_left ~f:(decompose_subtype_add_prop p) ~init:env props
  | TL.Disj (_, []) -> Env.mark_inconsistent env
  | TL.Disj (_, [prop']) -> decompose_subtype_add_prop p env prop'
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
    p
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty) : env =
  (* constraints are caught based on reason, not error callback. Using unify_error *)
  match ck with
  | Ast_defs.Constraint_as ->
    decompose_subtype p env ty_sub ty_super (Errors.unify_error_at p)
  | Ast_defs.Constraint_super ->
    decompose_subtype p env ty_super ty_sub (Errors.unify_error_at p)
  | Ast_defs.Constraint_eq ->
    let env =
      decompose_subtype p env ty_sub ty_super (Errors.unify_error_at p)
    in
    decompose_subtype p env ty_super ty_sub (Errors.unify_error_at p)

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
    p
    (env : env)
    (ck : Ast_defs.constraint_kind)
    (ty_sub : locl_ty)
    (ty_super : locl_ty) : env =
  log_subtype
    ~level:1
    ~this_ty:None
    ~function_name:"add_constraint"
    env
    ty_sub
    ty_super;
  let oldsize = Env.get_tpenv_size env in
  let env = decompose_constraint p env ck ty_sub ty_super in
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
                (Typing_set.elements (Env.get_lower_bounds env x))
                ~init:env
                ~f:(fun env ty_sub' ->
                  List.fold_left
                    (Typing_set.elements (Env.get_upper_bounds env x))
                    ~init:env
                    ~f:(fun env ty_super' ->
                      decompose_subtype
                        p
                        env
                        ty_sub'
                        ty_super'
                        (Errors.unify_error_at p))))
        in
        if Int.equal (Env.get_tpenv_size env) oldsize then
          env
        else
          iter (n + 1) env
    in
    iter 0 env

let add_constraints p env constraints =
  let add_constraint env (ty1, ck, ty2) = add_constraint p env ck ty1 ty2 in
  List.fold_left constraints ~f:add_constraint ~init:env

(** Check that the method with signature ft_sub can be used to override
 * (is a subtype of) method with signature ft_super.
 *
 * This goes beyond subtyping on function types because methods can have
 * generic parameters with bounds, and `where` constraints.
 *
 * Suppose ft_super is of the form
 *    <T1 csuper1, ..., Tn csupern>(tsuper1, ..., tsuperm) : tsuper where wsuper
 * and ft_sub is of the form
 *    <T1 csub1, ..., Tn csubn>(tsub1, ..., tsubm) : tsub where wsub
 * where csuperX and csubX are constraints on type parameters and wsuper and
 * wsub are 'where' constraints. Note that all types in the superclass,
 * including constraints (so csuperX, tsuperX, tsuper and wsuper) have had
 * any class type parameters instantiated appropriately according to
 * the actual arguments of the superclass. For example, suppose we have
 *
 *   class Super<T> {
 *     function foo<Tu as A<T>>(T $x) : B<T> where T super C<T>
 *   }
 *   class Sub extends Super<D> {
 *     ...override of foo...
 *   }
 * then the actual signature in the superclass that we need to check is
 *     function foo<Tu as A<D>>(D $x) : B<D> where D super C<D>
 * Note in particular the general form of the 'where' constraint.
 *
 * (Currently, this instantiation happens in
 *   Typing_extends.check_class_implements which in turn calls
 *   Decl_instantiate.instantiate_ce)
 *
 * Then for ft_sub to be a subtype of ft_super it must be the case that
 * (1) tsuper1 <: tsub1, ..., tsupern <: tsubn (under constraints
 *     T1 csuper1, ..., Tn csupern and wsuper).
 *
 *     This is contravariant subtyping on parameter types.
 *
 * (2) tsub <: tsuper (under constraints T1 csuper1, ..., Tn csupern and wsuper)
 *     This is covariant subtyping on result type. For constraints consider
 *       e.g. consider ft_super = <T super I>(): T
 *                 and ft_sub = <T>(): I
 *
 * (3) The constraints for ft_super entail the constraints for ft_sub, because
 *     we might be calling the function having checked that csuperX are
 *     satisfied but the definition of the function (e.g. consider an override)
 *     has been checked under csubX.
 *     More precisely, we must assume constraints T1 csuper1, ..., Tn csupern
 *     and wsuper, and check that T1 satisfies csub1, ..., Tn satisfies csubn
 *     and that wsub holds under those assumptions.
 *)
let subtype_method
    ~(check_return : bool)
    ~(extra_info : reactivity_extra_info)
    (env : env)
    (r_sub : Reason.t)
    (ft_sub : locl_fun_type)
    (r_super : Reason.t)
    (ft_super : locl_fun_type)
    (on_error : Errors.typing_error_callback) : env =
  Env.log_env_change "subtype_method" env
  @@
  let old_tpenv = Env.get_tpenv env in
  (* We check constraint entailment and contravariant parameter/covariant result
   * subtyping in the context of the ft_super constraints. But we'd better
   * restore tpenv afterwards *)
  let add_tparams_constraints env (tparams : locl_tparam list) =
    let add_bound env { tp_name = (pos, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, ty) ->
          let tparam_ty = MakeType.generic (Reason.Rwitness pos) name in
          add_constraint pos env ck tparam_ty ty)
    in
    List.fold_left tparams ~f:add_bound ~init:env
  in
  let p_sub = Reason.to_pos r_sub in
  let add_where_constraints env (cstrl : locl_where_constraint list) =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        add_constraint p_sub env ck ty1 ty2)
  in
  let env = add_tparams_constraints env ft_super.ft_tparams in
  let env = add_where_constraints env ft_super.ft_where_constraints in
  let (env, prop) =
    simplify_subtype_funs
      ~subtype_env:(make_subtype_env on_error)
      ~check_return
      ~extra_info
      r_sub
      ft_sub
      r_super
      ft_super
      env
  in
  let (env, prop) = prop_to_env env prop on_error in
  let env = Env.add_subtype_prop env prop in
  ignore (process_simplify_subtype_result prop);

  (* This is (3) above *)
  let check_tparams_constraints env tparams =
    let check_tparam_constraints
        env { tp_name = (p, name); tp_constraints = cstrl; _ } =
      List.fold_left cstrl ~init:env ~f:(fun env (ck, cstr_ty) ->
          let tgeneric = MakeType.generic (Reason.Rwitness p) name in
          Typing_generic_constraint.check_constraint
            env
            ck
            tgeneric
            ~cstr_ty
            on_error)
    in
    List.fold_left tparams ~init:env ~f:check_tparam_constraints
  in
  let check_where_constraints env cstrl =
    List.fold_left cstrl ~init:env ~f:(fun env (ty1, ck, ty2) ->
        Typing_generic_constraint.check_constraint
          env
          ck
          ty1
          ~cstr_ty:ty2
          on_error)
  in
  (* We only do this if the ft_tparam lengths match. Currently we don't even
   * report this as an error, indeed different names for type parameters.
   * TODO: make it an error to override with wrong number of type parameters
   *)
  let env =
    if
      Int.( <> )
        (List.length ft_sub.ft_tparams)
        (List.length ft_super.ft_tparams)
    then
      env
    else
      check_tparams_constraints env ft_sub.ft_tparams
  in
  let env = check_where_constraints env ft_sub.ft_where_constraints in
  Env.env_with_tpenv env old_tpenv

let sub_type_with_dynamic_as_bottom
    (env : env)
    (ty_sub : locl_ty)
    (ty_super : locl_ty)
    (on_error : Errors.typing_error_callback) : env =
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
      ~subtype_env:(make_subtype_env ~treat_dynamic_as_bottom:true on_error)
      ~this_ty:None
      ty_sub
      ty_super
      env
  in
  let (env, prop) = prop_to_env env prop on_error in
  let env = Env.add_subtype_prop env prop in
  let succeeded = process_simplify_subtype_result prop in
  let env =
    if succeeded then
      env
    else
      old_env
  in
  env_change_log env

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
  sub_type_i
    ~subtype_env:(make_subtype_env ~treat_dynamic_as_bottom:false on_error)
    env
    ty1
    ty2

let sub_type_or_fail env ty1 ty2 fail =
  sub_type env ty1 ty2 (fun ?code:_ _ -> fail ())

let set_fun_refs () =
  Typing_utils.sub_type_ref := sub_type;
  Typing_utils.sub_type_i_ref := sub_type_i;
  Typing_utils.sub_type_with_dynamic_as_bottom_ref :=
    sub_type_with_dynamic_as_bottom;
  Typing_utils.add_constraint_ref := add_constraint;
  Typing_utils.is_sub_type_ref := is_sub_type;
  Typing_utils.is_sub_type_for_union_ref := is_sub_type_for_union;
  Typing_utils.is_sub_type_for_union_i_ref := is_sub_type_for_union_i;
  Typing_utils.is_sub_type_ignore_generic_params_ref :=
    is_sub_type_ignore_generic_params

let () = set_fun_refs ()
