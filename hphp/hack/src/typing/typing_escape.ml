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
open Typing_defs_constraints
module Env = Typing_env
module ITySet = Internal_type_set
module Reason = Typing_reason
module TUtils = Typing_utils
module TySet = Typing_set

(** This module contains functions to clear rigid type variables from types
    and from the inference environment. We say that a type variable is
    "rigid" when it only unifies with itself. Examples of such type variables
    include type parameters and expression-dependent types. Any such rigid
    type variable needs to be cleared after type-checking lambdas & loops
    for soundness.
*)

(* Here is an example program that would bogusly type check if we did not
   clear rigid type variables properly:

   $f = (mixed $arg) ==> ($arg as Box<_>);
   $bs = $f($str_box as mixed) : Box<T#1>
   $bi = $f($int_box as mixed) : Box<T#1>  // unsound!
   $bi->set($bs->get());  // should not work

   We do not want the type of the two calls to $f to unify; this module
   prevents unsoundness by clearing T#1 from the result of the lambda.
   In this particular case, an error is generated.
*)

(* TODO(T92111151): Adding existential types to Hack locl types might
   help in simplifying the logic below by harnessing Typing_subtype.

   Namely, elimination of a rigid type T#1 in a type C[T#1] would
   simply quantify T#1 away (preserving its constraints).
*)

type rigid_tvar =
  | Rtv_tparam of string
  | Rtv_dependent of Expression_id.t

(********************************************************************)
(* Eliminating rigid type variables *)
(********************************************************************)

(* Many languages cannot "eliminate" escaping types and will simply raise
   an error when such an escape happens. In Hack, we can do a bit better
   using subtyping. For example, if a lambda returns [vec<T#1>] where
   T#1 must be eliminated, we can type the return value as [vec<mixed>]
   instead, or [vec<Foo>] if we happen to know that T#1 <: Foo. More
   generally, a type T to be eliminated can be replaced with the
   intersection of its upper bounds in covariant position, and with the
   union of its lower bounds in contravariant positions. This replacement
   is what the [eliminate] function performs.

   Because Hack is effectful, it is not sufficient to eliminate rigid type
   variables in the return type of lambdas, we must also make sure they
   do not escape in bounds of type variables that existed before typing
   a lambda. For example, consider:

     $v = Vector{}
     $f = (mixed $x) ==> {
       invariant($x is Box<_>, "");
       $v[] = $x->get();
     }
     // we do not want $v : Vector<T#1>; instead we want Vector<mixed>

   Improper rigid type variables handling would lead $v to have type
   Vector<T#1> leaving the door open to soundness holes. To prevent this
   problem we provide [refresh_*] functions that act on the inference
   environment. When doing so the question of the variance in constraints
   comes in.

   Inference can be understood as producing the following result: "under the
   constraints C on tyvars, the program P has type T"; which we will write
   more compactly as [C ==> P:T]. You can see that C stands on the LHS of an
   implication, i.e., it is in *contravariant* position.  This amounts to
   remarking that if C' is such that C'[tvs] ==> C[tvs] for all tvs then
   [C ==> P:T] implies [C' ==> P:T]. Consequently, it is always sound to
   make a constraints set stronger (harder to satisfy). Concretely, if we
   have a type variable #2021 with constraint #2021 <: arraykey, we can
   soundly replace the constraint with #2021 <: int; that is: upper bounds
   of type variables are *contravariant*. Dually, lower bounds are
   *covariant*. This remark underpins the implementation of [refresh_tvar].
*)

type elim_info = {
  pos: Pos_or_decl.t;
  upper_bounds: TySet.t;
  lower_bounds: TySet.t;
}

type remove_map = rigid_tvar -> elim_info option

type refresh_env = {
  env: Typing_env_types.env;  (** the underlying typing env *)
  tvars: Typing_error.Reasons_callback.t Tvid.Map.t;
      (** an accumulator used to remember all the type variabes
          that appeared when refreshing a type; the map is used as a
          set, and error callbacks are merely used for reporting *)
  remove: remove_map;
      (** the list of all the type parameters to eliminate with their
          bounds *)
  on_error: Typing_error.Reasons_callback.t;
      (** sometimes eliminating a rigid type is not possible and we must
          surface an error to the user *)
  scope_kind: string * Pos.t;
      (** a string and position describing the language construct for which
          we are preventing escapes (e.g., "lambda") *)
  elim_bogus_taccess: bool;
      (** a boolean indicating whether or not the elimination procedure
          should eliminate bogus Tgenerics of the form A::T where T is
          an abstract const type in A
          TODO(T91765587): kill bogus type access generics *)
}

(* refresh_ functions will return [Elim ...] when they eliminated a rigid
   type variable from their result. [Unchanged] is returned when nothing
   has changed in the data to refresh. *)
type changed =
  | Elim of Pos_or_decl.t * string
  | Unchanged

(* Using [with_default], all the refresh_ functions take care of returning
   *exactly* their argument if the refreshing did not change anything.

   I conjecture that this plays nice with the GC: if nothing changes all the
   temporary fresh objects will sit root-less in the minor heap and get
   collected for very cheap. If we instead used the fresh results as
   replacements of the originals we would create roots for them and may
   eventually have to collect the original copies with a constly compaction
   of the major heap *)
let with_default ~default (renv, res, changed) =
  match changed with
  | Elim _ -> (renv, res, changed)
  | Unchanged -> (renv, default, Unchanged)

let ( || ) ch1 ch2 =
  match ch1 with
  | Elim _ -> ch1
  | Unchanged -> ch2

let is_bogus_taccess tp =
  (* In typing_taccess.ml some type parameters of the form I::T are
     generated when T is abstract; these types are bogus and should
     be eliminated from our internal data structures but, in the
     meantime, we cope with them here by ignoring them.
     TODO(T91765587): kill bogus type access generics *)
  String.is_substring ~substring:"::" tp && Char.(tp.[0] <> '<')

let rec eliminate ~ty_orig ~rtv_pos ~name ~ubs ~lbs renv v =
  let r =
    let (what, wpos) = renv.scope_kind in
    Reason.rigid_tvar_escape (wpos, what, name, get_reason ty_orig)
  in
  match v with
  | Ast_defs.Contravariant ->
    let lbs = TySet.elements lbs in
    let (env, lbty) = Typing_union.union_list renv.env r lbs in
    (* a type that depends on type variables to be eliminated may be
       generated by the union computation; so we recursively eliminate
       them *)
    let elim_bogus_taccess = renv.elim_bogus_taccess in
    let renv = { renv with env; elim_bogus_taccess = true } in
    let (renv, lbty, _) = refresh_type renv v lbty in
    let lbty = with_reason lbty r in
    let renv = { renv with elim_bogus_taccess } in
    (renv, lbty, Elim (rtv_pos, name))
  | Ast_defs.Covariant ->
    let ubs = TySet.elements ubs in
    let (env, ubty) = Typing_intersection.intersect_list renv.env r ubs in
    let elim_bogus_taccess = renv.elim_bogus_taccess in
    let renv = { renv with env; elim_bogus_taccess = true } in
    (* ditto *)
    let (renv, ubty, _) = refresh_type renv v ubty in
    let ubty = with_reason ubty r in
    let renv = { renv with elim_bogus_taccess } in
    (renv, ubty, Elim (rtv_pos, name))
  | Ast_defs.Invariant ->
    let name = Markdown_lite.md_codify name in
    let snd_err =
      Typing_error.Secondary.Rigid_tvar_escape { pos = rtv_pos; name }
    in
    Typing_error_utils.add_typing_error
      ~env:renv.env
      Typing_error.(
        apply_reasons
          ~on_error:(Reasons_callback.retain_code renv.on_error)
          snd_err);
    (renv, ty_orig, Unchanged)

and refresh_type renv v ty_orig =
  let (env, ty) = Env.expand_type renv.env ty_orig in
  let renv = { renv with env } in
  with_default ~default:ty_orig
  @@
  match deref ty with
  | (_, (Tany _ | Tnonnull | Tdynamic | Tprim _ | Tneg _ | Tlabel _)) ->
    (renv, ty_orig, Unchanged)
  | (r, Toption ty1) ->
    let (renv, ty1, changed) = refresh_type renv v ty1 in
    (renv, mk (r, Toption ty1), changed)
  | (r, Tfun ft) ->
    let param_v = Ast_defs.swap_variance v in
    let ft_param
        (renv, changed)
        ({ fp_type; fp_pos = _; fp_name = _; fp_flags = _; fp_def_value = _ } as
        fp) =
      let (renv, fp_type, changed') = refresh_type renv param_v fp_type in
      ((renv, changed || changed'), { fp with fp_type })
    in
    let ((renv, changed), ft_params) =
      List.map_env (renv, Unchanged) ft.ft_params ~f:ft_param
    in
    let (renv, ft_ret, changed') = refresh_type renv v ft.ft_ret in
    (renv, mk (r, Tfun { ft with ft_params; ft_ret }), changed || changed')
  | (r, Ttuple { t_required; t_extra }) ->
    let (renv, t_required, changed) = refresh_types renv v t_required in
    let (renv, t_extra, changed') =
      match t_extra with
      | Textra { t_optional; t_variadic } ->
        let (renv, t_optional, changed) = refresh_types renv v t_optional in
        let (renv, t_variadic, changed') = refresh_type renv v t_variadic in
        (renv, Textra { t_optional; t_variadic }, changed || changed')
      | Tsplat t_splat ->
        let (renv, t_splat, changed) = refresh_type renv v t_splat in
        (renv, Tsplat t_splat, changed)
    in
    (renv, mk (r, Ttuple { t_required; t_extra }), changed || changed')
  | (r, Tshape { s_origin = _; s_unknown_value = sk; s_fields = sm }) ->
    let (renv, sm, ch) =
      TShapeMap.fold
        (fun sfn { sft_optional; sft_ty } (renv, sm, ch) ->
          let (renv, sft_ty, ch') = refresh_type renv v sft_ty in
          let sm = TShapeMap.add sfn { sft_optional; sft_ty } sm in
          (renv, sm, ch || ch'))
        sm
        (renv, TShapeMap.empty, Unchanged)
    in
    ( renv,
      mk
        ( r,
          Tshape
            {
              s_origin = Missing_origin;
              s_unknown_value = sk;
              (* TODO(shapes) refresh_type s_unknown_value *)
              s_fields = sm;
            } ),
      ch )
  | (_, Tvar v) ->
    let renv = { renv with tvars = Tvid.Map.add v renv.on_error renv.tvars } in
    (renv, ty_orig, Unchanged)
  | (r, Tgeneric name) -> begin
    (* look if the Tgeneric has to go away and kill it using its
       bounds if the variance of the current occurrence permits it *)
    match renv.remove (Rtv_tparam name) with
    | None -> (renv, ty_orig, Unchanged)
    | Some _ when is_bogus_taccess name && not renv.elim_bogus_taccess ->
      (renv, ty_orig, Unchanged)
    | Some { pos; lower_bounds = lbs; upper_bounds = ubs } ->
      let rtv_pos =
        if Pos_or_decl.(equal pos none) then
          Reason.to_pos r
        else
          pos
      in
      eliminate ~ty_orig ~rtv_pos ~name ~ubs ~lbs renv v
  end
  | (r, Tdependent ((DTexpr id as dt), ty1)) -> begin
    match renv.remove (Rtv_dependent id) with
    | None ->
      let (renv, ty1, ch1) = refresh_type renv v ty1 in
      (renv, mk (r, Tdependent (dt, ty1)), ch1)
    | Some _ ->
      let lbs = TySet.empty in
      let ubs = TySet.singleton ty1 in
      let rtv_pos = Reason.to_pos r in
      let name = DependentKind.to_string dt in
      eliminate ~ty_orig ~rtv_pos ~name ~ubs ~lbs renv v
  end
  | (r, Tunion l) ->
    let (renv, l, changed) = refresh_types renv v l in
    begin
      match changed with
      | Elim _ ->
        let (env, ty) = Typing_union.union_list renv.env r l in
        let renv = { renv with env } in
        (renv, ty, changed)
      | Unchanged -> (renv, ty_orig, Unchanged)
    end
  | (r, Tintersection l) ->
    let (renv, l, changed) = refresh_types renv v l in
    begin
      match changed with
      | Elim _ ->
        let (env, ty) = Typing_intersection.intersect_list renv.env r l in
        let renv = { renv with env } in
        (renv, ty, changed)
      | Unchanged -> (renv, ty_orig, Unchanged)
    end
  | (r, Tvec_or_dict (ty1, ty2)) ->
    let (renv, ty1, ch1) = refresh_type renv v ty1 in
    let (renv, ty2, ch2) = refresh_type renv v ty2 in
    (renv, mk (r, Tvec_or_dict (ty1, ty2)), ch1 || ch2)
  | (r, Taccess (ty1, id)) ->
    let (renv, ty1, ch1) = refresh_type renv Ast_defs.Invariant ty1 in
    (renv, mk (r, Taccess (ty1, id)), ch1)
  | (r, Tnewtype (name, l, bnd)) ->
    let tparams = Env.get_class_or_typedef_tparams env name in
    let (renv, l, ch) = refresh_types_w_variance renv v tparams l in
    (renv, mk (r, Tnewtype (name, l, bnd)), ch)
  | (r, Tclass ((p, cid), e, l)) ->
    let tparams = Env.get_class_or_typedef_tparams env cid in
    let (renv, l, ch) = refresh_types_w_variance renv v tparams l in
    (renv, mk (r, Tclass ((p, cid), e, l)), ch)
  | (r, Tclass_ptr ty1) ->
    let (renv, ty1, changed) = refresh_type renv v ty1 in
    (renv, mk (r, Tclass_ptr ty1), changed)

and refresh_types renv v l =
  let rec go renv changed tl acc =
    match tl with
    | [] -> (renv, List.rev acc, changed)
    | ty :: tl ->
      let (renv, ty, changed') = refresh_type renv v ty in
      go renv (changed || changed') tl (ty :: acc)
  in
  with_default ~default:l (go renv Unchanged l [])

and refresh_types_w_variance renv v tpl tl =
  let rec go renv changed tpl tl acc =
    match (tpl, tl) with
    | (_, []) -> (renv, List.rev acc, changed)
    | ([], ty :: tl) ->
      let (renv, ty, changed') = refresh_type renv Ast_defs.Invariant ty in
      go renv (changed || changed') [] tl (ty :: acc)
    | (tp :: tpl, ty :: tl) ->
      let v =
        match tp.tp_variance with
        | Ast_defs.Invariant -> Ast_defs.Invariant
        | Ast_defs.Covariant -> v
        | Ast_defs.Contravariant -> Ast_defs.swap_variance v
      in
      let (renv, ty, changed') = refresh_type renv v ty in
      go renv (changed || changed') tpl tl (ty :: acc)
  in
  with_default ~default:tl (go renv Unchanged tpl tl [])

let refresh_type_opt renv v tyo =
  match tyo with
  | None -> (renv, None, Unchanged)
  | Some ty ->
    let (renv, ty, ch) = refresh_type renv v ty in
    (renv, Some ty, ch)

let refresh_ctype renv v cty_orig =
  let inv = Ast_defs.Invariant in
  with_default ~default:cty_orig
  @@
  match deref_constraint_type cty_orig with
  | (r, Thas_member hm) ->
    let { hm_type; hm_name = _; hm_class_id = _; hm_method = _ } = hm in
    let (renv, hm_type, changed) = refresh_type renv inv hm_type in
    (renv, mk_constraint_type (r, Thas_member { hm with hm_type }), changed)
  | (r, Thas_type_member htm) ->
    let { htm_id; htm_lower; htm_upper } = htm in
    let v' = Ast_defs.swap_variance v in
    let (renv, htm_upper, ch1) = refresh_type renv v htm_upper in
    let (renv, htm_lower, ch2) = refresh_type renv v' htm_lower in
    let htm = { htm_id; htm_lower; htm_upper } in
    (renv, mk_constraint_type (r, Thas_type_member htm), ch1 || ch2)
  | (r, Tcan_index ci) ->
    let (renv, ci_val, ch1) = refresh_type renv inv ci.ci_val in
    let (renv, ci_key, ch2) = refresh_type renv inv ci.ci_key in
    ( renv,
      mk_constraint_type (r, Tcan_index { ci with ci_val; ci_key }),
      ch1 || ch2 )
  | (r, Tcan_index_assign cia) ->
    let (renv, cia_key, ch1) = refresh_type renv inv cia.cia_key in
    let (renv, cia_write, ch2) = refresh_type renv inv cia.cia_write in
    let (renv, cia_val, ch3) = refresh_type renv inv cia.cia_val in
    ( renv,
      mk_constraint_type
        (r, Tcan_index_assign { cia with cia_key; cia_write; cia_val }),
      ch1 || ch2 || ch3 )
  | (r, Tcan_traverse ct) ->
    let (renv, ct_val, ch1) = refresh_type renv inv ct.ct_val in
    let (renv, ct_key, ch2) =
      match ct.ct_key with
      | None -> (renv, None, Unchanged)
      | Some ct_key ->
        let (renv, ct_key, ch2) = refresh_type renv inv ct_key in
        (renv, Some ct_key, ch2)
    in
    ( renv,
      mk_constraint_type (r, Tcan_traverse { ct with ct_val; ct_key }),
      ch1 || ch2 )
  | (r, Tdestructure { d_required; d_optional; d_variadic; d_kind }) ->
    let (renv, d_required, ch1) = refresh_types renv inv d_required in
    let (renv, d_optional, ch2) = refresh_types renv inv d_optional in
    let (renv, d_variadic, ch3) = refresh_type_opt renv inv d_variadic in
    let des = { d_required; d_optional; d_variadic; d_kind } in
    (renv, mk_constraint_type (r, Tdestructure des), ch1 || ch2 || ch3)
  | (r, Ttype_switch { predicate; ty_true; ty_false }) ->
    let (renv, ty_true, ch1) = refresh_type renv v ty_true in
    let (renv, ty_false, ch2) = refresh_type renv v ty_false in
    ( renv,
      mk_constraint_type (r, Ttype_switch { predicate; ty_true; ty_false }),
      ch1 || ch2 )
  | (r, Thas_const { name; ty }) ->
    let (renv, ty, changed) = refresh_type renv inv ty in
    (renv, mk_constraint_type (r, Thas_const { name; ty }), changed)

let refresh_bounds renv v tys =
  ITySet.fold
    (fun ity (renv, del, add) ->
      match ity with
      | LoclType lty ->
        let (renv, lty, changed) = refresh_type renv v lty in
        begin
          match changed with
          | Unchanged -> (renv, del, add)
          | Elim (pos, name) ->
            (renv, ITySet.add ity del, (LoclType lty, pos, name) :: add)
        end
      | ConstraintType cty ->
        let (renv, cty, changed) = refresh_ctype renv v cty in
        begin
          match changed with
          | Unchanged -> (renv, del, add)
          | Elim (pos, name) ->
            (renv, ITySet.add ity del, (ConstraintType cty, pos, name) :: add)
        end)
    tys
    (renv, ITySet.empty, [])

let refresh_tvar tv (on_error : Typing_error.Reasons_callback.t) renv =
  (* restore the error context of one local variable causing us to visit
     this tvar *)
  let renv = { renv with on_error } in
  let tv_ity = LoclType (mk (Reason.none, Tvar tv)) in
  let elim_on_error pos name =
    let name = Markdown_lite.md_codify name in
    Some
      Typing_error.Reasons_callback.(
        with_reasons
          ~reasons:
            (lazy [(pos, "Could not remove rigid type variable " ^ name)])
        @@ retain_code
        @@ retain_quickfixes on_error)
  in
  let (renv, e1) =
    let ubs = Env.get_tyvar_upper_bounds renv.env tv in
    let var = Ast_defs.Contravariant in
    let add_bound (env, ty_errs) (ity, pos, name) =
      match TUtils.sub_type_i env tv_ity ity (elim_on_error pos name) with
      | (env, None) -> (env, ty_errs)
      | (env, Some ty_err) -> (env, ty_err :: ty_errs)
    in
    let (renv, del, add) = refresh_bounds renv var ubs in
    if ITySet.is_empty del && List.is_empty add then
      (renv, None)
    else
      let ubs = Env.get_tyvar_upper_bounds renv.env tv in
      let ubs = ITySet.diff ubs del in
      let env = Env.set_tyvar_upper_bounds renv.env tv ubs in
      let (env, ty_errs) = List.fold ~init:(env, []) ~f:add_bound add in

      ({ renv with env }, Typing_error.multiple_opt ty_errs)
  in
  let (renv, e2) =
    let lbs = Env.get_tyvar_lower_bounds renv.env tv in
    let var = Ast_defs.Covariant in
    let add_bound (env, ty_errs) (ity, pos, name) =
      match TUtils.sub_type_i env ity tv_ity (elim_on_error pos name) with
      | (env, None) -> (env, ty_errs)
      | (env, Some ty_err) -> (env, ty_err :: ty_errs)
    in
    let (renv, del, add) = refresh_bounds renv var lbs in
    if ITySet.is_empty del && List.is_empty add then
      (renv, None)
    else
      let lbs = Env.get_tyvar_lower_bounds renv.env tv in
      let lbs = ITySet.diff lbs del in
      let env = Env.set_tyvar_lower_bounds renv.env tv lbs in
      let (env, ty_errs) = List.fold ~init:(env, []) ~f:add_bound add in
      ({ renv with env }, Typing_error.multiple_opt ty_errs)
  in
  Option.(
    iter ~f:(Typing_error_utils.add_typing_error ~env:renv.env)
    @@ merge e1 e2 ~f:Typing_error.both);
  renv

let rec refresh_tvars seen renv =
  if Tvid.Map.is_empty renv.tvars then
    renv.env
  else
    let tvars = renv.tvars in
    let renv = { renv with tvars = Tvid.Map.empty } in
    let renv = Tvid.Map.fold refresh_tvar tvars renv in
    let seen = Tvid.Map.fold (fun v _ -> Tvid.Set.add v) tvars seen in
    let tvars = Tvid.Set.fold Tvid.Map.remove seen renv.tvars in
    let renv = { renv with tvars } in
    refresh_tvars seen renv

let refresh_locals renv =
  let locals =
    match Env.next_cont_opt renv.env with
    | None -> Typing_local_types.empty
    | Some { Typing_per_cont_env.local_types; _ } -> local_types
  in
  (* save the original error callback & use it to create one callback
     per local in the fold below *)
  let on_error = renv.on_error in
  Local_id.Map.fold
    (fun local
         Typing_local_types.
           { ty = lty; defined; bound_ty; pos; eid = _; macro_splice_vars = _ }
         renv ->
      if defined then
        let on_error =
          let pos = Pos_or_decl.of_raw_pos pos in
          let name = Markdown_lite.md_codify (Local_id.to_string local) in
          let reason = lazy (pos, "in the type of local " ^ name) in
          Typing_error.Reasons_callback.append_reason on_error ~reason
        in
        let renv = { renv with on_error } in
        let (renv, lty, changed) = refresh_type renv Ast_defs.Covariant lty in
        match changed with
        | Elim _ ->
          {
            renv with
            env =
              Env.set_local ~is_defined:true ~bound_ty renv.env local lty pos;
          }
        | Unchanged -> renv
      else
        renv)
    locals
    renv

let refresh_env_and_type ~remove:(types, remove) ~pos env ty =
  if List.is_empty types then
    (* nothing to clear, just return the inputs *)
    (env, ty)
  else (
    Typing_log.log_escape
      (Pos_or_decl.of_raw_pos pos)
      env
      "Clearing escaping types:"
      types;
    let what = "lambda" in
    let on_error =
      Typing_error.Reasons_callback.rigid_tvar_escape_at pos what
    in
    let renv =
      {
        env;
        tvars = Tvid.Map.empty;
        remove;
        on_error;
        scope_kind = (what, pos);
        elim_bogus_taccess = false;
      }
    in
    let renv = refresh_locals renv in
    let on_error =
      let pos = Pos_or_decl.of_raw_pos pos in
      Typing_error.Reasons_callback.append_reason
        on_error
        ~reason:(lazy (pos, "in the return type of this lambda"))
    in
    let renv = { renv with on_error } in
    let (renv, ty, _) = refresh_type renv Ast_defs.Covariant ty in
    (refresh_tvars Tvid.Set.empty renv, ty)
  )

(********************************************************************)
(* Computing escaping types *)
(********************************************************************)

type snapshot = {
  tpmap: (Pos_or_decl.t * Typing_kinding_defs.kind) SMap.t;
  nextid: Expression_id.t;
      (* nextid is used to detect if an expression-dependent type is fresh
         or not; we snapshot it at some time and all ids larger than the
         snapshot were allocated after the snapshot time *)
}

type escaping_rigid_tvars = string list * remove_map

let snapshot_env env =
  let gtp = Type_parameter_env.get_tparams (Env.get_global_tpenv env) in
  let ltp = Type_parameter_env.get_tparams (Env.get_tpenv env) in
  { tpmap = SMap.union gtp ltp; nextid = Env.make_expression_id env }

let escaping_from_snapshot snap env quants :
    string list * (rigid_tvar -> elim_info option) =
  let is_global tp =
    (* Oh, that's nice... *)
    String.length tp > 6 && String.(sub ~pos:0 ~len:6 tp = "this::")
  in
  let { nextid; _ } = snap in
  let is_old_dep_expr tp =
    (* but it gets better! *)
    let extract_id tp_name =
      let rec atoi s i acc =
        if Char.(s.[i] = '>') then
          acc
        else
          atoi s (i + 1) ((10 * acc) + Char.(to_int s.[i] - to_int '0'))
      in
      Expression_id.dodgy_from_int @@ atoi tp_name 6 0
    in
    String.length tp > 6
    && String.(sub ~pos:0 ~len:6 tp = "<expr#")
    && Expression_id.compare (extract_id tp) nextid < 0
  in
  let tpmap =
    Type_parameter_env.get_tparams (Env.get_global_tpenv env)
    |> Typing_continuations.Map.fold
         (fun _ c ->
           SMap.union
             (Type_parameter_env.get_tparams c.Typing_per_cont_env.tpenv))
         (Typing_lenv.get_all_locals env)
    |> SMap.fold (fun key _ acc -> SMap.remove key acc) snap.tpmap
    |> SMap.filter (fun tp _ ->
           (not (is_global tp)) && not (is_old_dep_expr tp))
  in
  let tpmap =
    List.fold_left quants ~init:tpmap ~f:(fun acc nm -> SMap.remove nm acc)
  in
  ( SMap.keys tpmap,
    function
    | Rtv_tparam name ->
      Option.map (SMap.find_opt name tpmap) ~f:(fun (pos, tpi) ->
          {
            pos;
            upper_bounds = tpi.Typing_kinding_defs.upper_bounds;
            lower_bounds = tpi.Typing_kinding_defs.lower_bounds;
          })
    | Rtv_dependent id ->
      let empty_info =
        {
          pos = Pos_or_decl.none;
          upper_bounds = TySet.empty;
          lower_bounds = TySet.empty;
        }
      in
      if Expression_id.compare id nextid > 0 then
        Some empty_info
      else
        None )
