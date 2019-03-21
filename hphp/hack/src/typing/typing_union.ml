(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Hh_core
open Typing_defs

module TO = TypecheckerOptions
module Env = Typing_env
module Reason = Typing_reason
module TySet = Typing_set
module Unify = Typing_unify
module URec = Typing_unify_recursive
module Utils = Typing_utils
module MakeType = Typing_make_type

exception Not_equiv
exception Dont_unify

(* Two types are "equivalent" if when expanded they are equal.
 * If they are equivalent, this function returns a type which is equivalent to
 * both, otherwise it raises Not_equiv.
 *
 * We need this instead of simply ty_equal, otherwise a lot of things aren't
 * unioned properly - e.g. we get things like `array<^(int)> | array<^(int)>` -
 * which potentially creates huge unresolved types, damaging perf.
 *
 * Furthermore, when unioning classes with type parameters, we allow
 *   A<Unresolved[]> | A<B> = A<B>
 * This is because collections are often initialized like
 *   $myvec = get_int_vec() ?? vec[]
 * If we don't do this simplification, this potentially produces exponentially
 * growing Unresolved, like in test map_corner_case.php.
 *)
let ty_equiv env ty1 ty2 ~are_ty_param =
  let env, ety1 = Env.expand_type env ty1 in
  let env, ety2 = Env.expand_type env ty2 in
  let ty = match (ety1, ty1), (ety2, ty2) with
    | ((_, Tunresolved []), _), (_, ty)
    | (_, ty), ((_, Tunresolved []), _) when are_ty_param -> ty
    | _ when ty_equal ety1 ety2 ->
      begin match ty1 with
      | _, Tvar _ -> ty1
      | _ -> ty2
      end
    | _ -> raise Not_equiv in
  env, ty

let make_union r tyl reason_nullable_opt ~discard_singletons =
  match discard_singletons, reason_nullable_opt, tyl with
  | _, Some null_r, [] ->  (null_r, Tprim Nast.Tnull)
  | true, None, [ty] -> ty
  | _, None, tyl -> (r, Tunresolved tyl)
  | true, Some null_r, [ty] -> (null_r, Toption ty)
  | _, Some null_r, tyl -> (null_r, Toption (r, Tunresolved tyl))

let rec union env (r1, _ as ty1) (r2, _ as ty2) =
  if ty_equal ty1 ty2 then env, ty1
  else
    let is_sub_type = Typing_utils.is_sub_type_alt ~no_top_bottom:true in
    if is_sub_type env ty1 ty2 = Some true then env, ty2
    else if is_sub_type env ty2 ty1 = Some true then env, ty1
    else
      let r = union_reason r1 r2 in
      union_ env ty1 ty2 r

and union_ env ty1 ty2 r =
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  let (env, ty1) = if new_inference then Env.expand_type env ty1 else env, ty1 in
  let (env, ty2) = if new_inference then Env.expand_type env ty2 else env, ty2 in
  try
  begin match ty1, ty2 with
  | (r1, Tprim Nast.Tint), (r2, Tprim Nast.Tfloat)
  | (r1, Tprim Nast.Tfloat), (r2, Tprim Nast.Tint) ->
    let r = union_reason r1 r2 in
    env, MakeType.num r
  | (r, Tprim Nast.Tnull), ty
  | ty, (r, Tprim Nast.Tnull) ->
    env, (r, Toption ty)
  | (_, Tclass ((p, id1), e1, tyl1)), (_, Tclass ((_, id2), e2, tyl2))
    when id1 = id2 ->
    let e = Typing_ops.LeastUpperBound.exact_least_upper_bound e1 e2 in
    let env, tyl = union_class env id1 tyl1 tyl2 in
    env, (r, Tclass ((p, id1), e, tyl))
  | (r, Toption ty1), ty2
  | ty2, (r, Toption ty1)->
    let env, ty = union env ty1 ty2 in
    begin match ty with
    | _, Toption _ -> env, ty
    | _ -> env, (r, Toption ty)
    end
  | (_, Tvar n1), (_, Tvar n2) when not new_inference ->
    let env, n1 = Env.get_var env n1 in
    let env, n2 = Env.get_var env n2 in
    if n1 = n2 then env, (r, Tvar n1) else
    let env, ty1 = Env.get_type_unsafe env n1 in
    let env, ty2 = Env.get_type_unsafe env n2 in
    let n' = Env.fresh () in
    let env, ty = union env ty1 ty2 in
    let env = URec.add env n' ty in
    env, (r, Tvar n')
  | (r, Tvar n), ty2
  | ty2, (r, Tvar n) when not new_inference ->
    let env, ty1 = Env.get_type env r n in
    let n' = Env.fresh () in
    let env, ty = union env ty1 ty2 in
    let env = URec.add env n' ty in
    env, (r, Tvar n')
  | (_, Tunresolved tyl1), (_, Tunresolved tyl2) ->
    union_unresolved env tyl1 tyl2 r
  | (r, Tunresolved tyl), ty2
  | ty2, (r, Tunresolved tyl) ->
    union_unresolved env tyl [ty2] r
  | (_, Tarraykind ak1), (_, Tarraykind ak2) ->
    let env, ak = union_arraykind env ak1 ak2 in
    env, (r, Tarraykind ak)
  | (_, Tabstract (ak1, tcstr1)), (_, Tabstract (ak2, tcstr2)) ->
    let env, ak = union_ak env ak1 ak2 in
    let env, tcstr = union_tconstraints env tcstr1 tcstr2 in
    env, (r, Tabstract (ak, tcstr))
  (* TODO AKgeneric will be killed and replaces with Tgeneric*)
  | (_, Tabstract (AKgeneric x, _)), ty
    when Unify.generic_param_matches ~opts:Utils.default_unify_opt env x ty ->
    env, ty
  | ty, (_, Tabstract (AKgeneric x, _))
    when Unify.generic_param_matches ~opts:Utils.default_unify_opt env x ty ->
    env, ty
  | (_, Tabstract (AKdependent _, Some (_, Tclass _ as ty1))), ty2
  | ty2, (_, Tabstract (AKdependent _, Some (_, Tclass _ as ty1))) ->
    begin try ty_equiv env ty1 ty2 ~are_ty_param:false
    with Not_equiv -> raise Dont_unify
    end
  | (_, Ttuple tyl1), (_, Ttuple tyl2) ->
    if List.length tyl1 = List.length tyl2
    then
      let env, tyl = List.map2_env env tyl1 tyl2 ~f:union in
      env, (r, Ttuple tyl)
    else
      raise Dont_unify
  | (r1, Tshape (fk1, fdm1)), (r2, Tshape (fk2, fdm2)) ->
    let env, ty = union_shapes env (fk1, fdm1, r1) (fk2, fdm2, r2) in
    env, (r, ty)
  | (_, Tfun ft1), (_, Tfun ft2) ->
    let env, ft = union_funs env ft1 ft2 in
    env, (r, Tfun ft)
  | (_, Tanon (_, id1)), (_, Tanon (_, id2)) when id1 = id2 -> env, ty1
  (* TODO with Tclass, union type arguments if covariant *)
  | (_, ((Tarraykind _ | Tprim _ | Tdynamic | Tabstract _ | Tclass _
    | Ttuple _ | Tanon _ | Tfun _ | Tobject | Tshape _ | Terr | Tvar _
    (* If T cannot be null, `union T nonnull = nonnull`. However, it's hard
     * to say whether a given T can be null - e.g. opaque newtypes, dependent
     * types, etc. - so for now we leave it here.
     * TODO improve that. *)
    | Tnonnull | Tany) as ty1_)),
    (_, ty2_) ->
    (* Make sure to add a dependency on any classes referenced here, even if
     * we're in an error state (i.e., where we are right now). The need for
     * this is extremely subtle. Consider this function:
     *
     * function f(): blah {
     *   // ...
     * }
     *
     * Suppose that "blah" isn't currently defined, and we send the result
     * of f() into a function that expects an int. We'll hit a unification
     * error here, as we should. But, we might later define "blah" to be a
     * type alias, "type blah = int", in another file. In that case, f()
     * needs to be rechecked with the new definition of "blah" present.
     *
     * Normally this isn't a problem. The presence of the error in f() in
     * the first place will cause it to be rechecked when "blah" pops into
     * existance anyways. (And in strict mode, or with assume_php=false, you
     * can't refer to the undefined "blah" anyways.) But there's one
     * important case where this does matter: the JS cross-compile of the
     * typechecker. The JS driver code uses the presence of dependencies to
     * figure out what code to pull into the browser, and it's pretty aggro
     * about not pulling in things it doesn't need. If this dep is missing,
     * it will never pull in "blah" -- which actually does exist, but is
     * "undefined" as far as the typechecker is concerned because the JS
     * driver hasn't pulled it into the browser *yet*. The presence of this
     * dep causes that to happen.
     *
     * Another way to do this might be to look up blah and see if it's
     * defined (and doing this will add the dep for us), and suppress the
     * error if it isn't. We typically say that undefined classes could live
     * in PHP and thus be anything -- but the only way it could unify with
     * a non-class is if it's a type alias, which isn't a PHP feature, so
     * the strictness (and subtlety) is warranted here.
     *
     * And the dep is correct anyways: if there weren't a unification error
     * like this, we'd be pulling in the declaration of "blah" (and adding
     * the dep) anyways.
     *)
    let add env = function
      | Tclass ((_, cid), _, _) -> Env.add_wclass env cid
      | _ -> () in
    add env ty1_;
    add env ty2_;
    try ty_equiv env ty1 ty2 ~are_ty_param:false
    with Not_equiv -> raise Dont_unify
  end
  with Dont_unify ->
    env, (r, Tunresolved [ty1; ty2])

and try_union env ty1 ty2 =
  match union env ty1 ty2 with
  | _, (_, Tunresolved _) -> env, None
  | env, ty -> env, Some ty

and union_unresolved env tyl1 tyl2 r =
  let attempt_union env tyl ty2 res_is_opt =
    let rec go env tyl ty2 missed res_is_opt =
      match tyl with
      | [] -> env, ty2::missed, res_is_opt
      | ty1::tyl ->
        let env, ety1 = Env.expand_type env ty1 in
        begin match ety1 with
        | _, Tunresolved tyl1 ->
          go env (tyl1@tyl) ty2 missed res_is_opt
        | r, Toption ty ->
          go env (ty::tyl) ty2 missed (Some r)
        | r, Tprim Nast.Tnull ->
          go env tyl ty2 missed (Some r)
        | _ ->
          begin match try_union env ety1 ty2 with
          | _, None -> go env tyl ty2 (ety1::missed) res_is_opt
          | env, Some ty -> env, missed @ (ty::tyl), res_is_opt
          end
        end in
    go env tyl ty2 [] res_is_opt in

  let rec normalize_union env tyl1 tyl2 res_is_opt =
    match tyl2 with
    | [] -> env, tyl1, res_is_opt
    | ty2::tyl2 ->
      let env, ety2 = Env.expand_type env ty2 in
      begin match ety2 with
      | _, Tunresolved tyl ->
        normalize_union env tyl1 (tyl@tyl2) res_is_opt
      | r, Toption ty ->
        normalize_union env tyl1 (ty::tyl2) (Some r)
      | r, Tprim Nast.Tnull ->
        normalize_union env tyl1 tyl2 (Some r)
      | _ ->
        let env, tyl1, res_is_opt = attempt_union env tyl1 ety2 res_is_opt in
        normalize_union env tyl1 tyl2 res_is_opt
      end in
  let env, tyl, res_is_opt = normalize_union env tyl1 tyl2 None in
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  env, make_union r tyl res_is_opt ~discard_singletons:new_inference

and union_arraykind env ak1 ak2 =
  match ak1, ak2 with
  | AKempty, ak
  | ak, AKempty -> env, ak
  | AKany, AKany -> env, AKany
  | AKany, ak
  | ak, AKany ->
    if (TypecheckerOptions.safe_array (Env.get_tcopt env))
    then raise Dont_unify
    else env, ak
  | AKvarray ty1, AKvarray ty2 ->
    let env, ty = union env ty1 ty2 in
    env, AKvarray ty
  | (AKvarray ty1 | AKvec ty1), (AKvarray ty2 | AKvec ty2) ->
    let env, ty = union env ty1 ty2 in
    env, AKvec ty
  | AKdarray (tk1, tv1), AKdarray (tk2, tv2) ->
    let env, tk = union env tk1 tk2 in
    let env, tv = union env tv1 tv2 in
    env, AKdarray (tk, tv)
  | (AKdarray (tk1, tv1) | AKmap (tk1, tv1)),
    (AKdarray (tk2, tv2) | AKmap (tk2, tv2)) ->
    let env, tk = union env tk1 tk2 in
    let env, tv = union env tv1 tv2 in
    env, AKmap (tk, tv)
  | AKvarray_or_darray ty1, AKvarray_or_darray ty2 ->
    let env, ty = union env ty1 ty2 in
    env, AKvarray_or_darray ty
  | _ -> raise Dont_unify

and union_funs env fty1 fty2 =
  (* TODO: If we later add fields to ft, they will be forgotten here. *)
  if (fty1.ft_is_coroutine, fty1.ft_arity, fty1.ft_reactive,
    fty1.ft_return_disposable, fty1.ft_returns_mutable) =
    (fty2.ft_is_coroutine, fty2.ft_arity, fty2.ft_reactive,
    fty2.ft_return_disposable, fty2.ft_returns_mutable) &&
    ft_params_compare fty1.ft_params fty2.ft_params = 0
  then
    let env, ft_ret = union env fty1.ft_ret fty2.ft_ret in
    env, { fty1 with ft_ret }
  else raise Dont_unify

and union_class env name tyl1 tyl2 =
  let tparams = match Env.get_class env name with
    | None -> []
    | Some c -> Typing_classes_heap.tparams c in
  union_tylists_w_variances env tparams tyl1 tyl2

and union_ak env ak1 ak2 =
  match ak1, ak2 with
  | AKnewtype (id1, tyl1), AKnewtype (id2, tyl2) when id1 = id2 ->
    let env, tyl = union_newtype env id1 tyl1 tyl2 in
    env, AKnewtype (id1, tyl)
  | _ ->
    if abstract_kind_compare ak1 ak2 = 0 then env, ak1 else raise Dont_unify

and union_newtype env typename tyl1 tyl2 =
  let tparams = match Env.get_typedef env typename with
    | None -> []
    | Some t -> t.td_tparams in
  union_tylists_w_variances env tparams tyl1 tyl2

and union_tylists_w_variances env tparams tyl1 tyl2 =
  let variances = List.map tparams (fun t -> t.tp_variance) in
  let variances =
    let adjust_list_length l newlen filler =
      let len = List.length l in
      if len < newlen then l @ (List.init (newlen - len) (fun _ -> filler))
      else List.sub l 0 newlen in
    adjust_list_length variances (List.length tyl1) Ast.Invariant in
  let merge_ty_params env ty1 ty2 variance =
    match variance with
    | Ast.Covariant -> union env ty1 ty2
    | _ -> ty_equiv env ty1 ty2 ~are_ty_param:true in
  let env, tyl = try List.map3_env env tyl1 tyl2 variances ~f:merge_ty_params
    with Not_equiv | Invalid_argument _ -> raise Dont_unify in
  env, tyl

and union_tconstraints env tcstr1 tcstr2 =
  match tcstr1, tcstr2 with
  | Some ty1, Some ty2 ->
    let env, ty = union env ty1 ty2 in
    env, Some ty
  | _ -> env, None

and union_shapes env (fields_known1, fdm1, r1) (fields_known2, fdm2, r2) =
  let fields_known = union_fields_known fields_known1 fields_known2 in
  let (env, fields_known), fdm =
    Nast.ShapeMap.merge_env (env, fields_known) fdm1 fdm2 ~combine:
      (fun (env, fields_known) k fieldopt1 fieldopt2 ->
        match
          (fields_known1, fieldopt1, r1),
          (fields_known2, fieldopt2, r2) with
        | (_, None, _), (_, None, _) -> (env, fields_known), None
        (* key is present on one side but not the other *)
        | (_, Some { sft_ty; _ }, _), (fields_known_other, None, r)
        | (fields_known_other, None, r), (_, Some { sft_ty; _ }, _) ->
          if TO.experimental_feature_enabled (Env.get_tcopt env)
            TO.experimental_disable_optional_and_unknown_shape_fields
          then raise Dont_unify;
          let fields_known = begin match fields_known with
            | FieldsPartiallyKnown unset_fields ->
              FieldsPartiallyKnown (Nast.ShapeMap.remove k unset_fields)
            | FieldsFullyKnown -> FieldsFullyKnown end in
          let sft_ty = match fields_known_other with
            | FieldsFullyKnown ->
              sft_ty
            | FieldsPartiallyKnown l when Nast.ShapeMap.has_key k l ->
              sft_ty
            (* If the fields are partially known in the shape not containing
             * the key k, this shape may actually contain this key, so after
             * unioning, we only know that the key is (optionally) present but
             * can say nothing about its type. Therefore we assign the top
             * type. *)
            | FieldsPartiallyKnown _ ->
              let r = Reason.Rmissing_optional_field (
                Reason.to_pos r,
                Utils.get_printable_shape_field_name k) in
              (MakeType.mixed r) in
          (env, fields_known), Some { sft_optional = true; sft_ty }
        (* key is present on both sides *)
        | (_, Some { sft_optional = optional1; sft_ty = ty1 }, _),
          (_, Some { sft_optional = optional2; sft_ty = ty2 }, _) ->
          let sft_optional = optional1 || optional2 in
          let env, sft_ty = union env ty1 ty2 in
          (env, fields_known), Some { sft_optional; sft_ty }) in
    env, Tshape (fields_known, fdm)

and union_fields_known fields_known1 fields_known2 =
  match fields_known1, fields_known2 with
  | FieldsFullyKnown, FieldsFullyKnown -> FieldsFullyKnown
  | FieldsFullyKnown, FieldsPartiallyKnown l
  | FieldsPartiallyKnown l, FieldsFullyKnown -> FieldsPartiallyKnown l
  | FieldsPartiallyKnown unset_fields1, FieldsPartiallyKnown unset_fields2 ->
    (* intersect sets of unset fields *)
    let unset_fields = Nast.ShapeMap.merge (fun _k posopt1 posopt2 ->
      match posopt1, posopt2 with
      | Some p, Some _ -> Some p
      | _ -> None) unset_fields1 unset_fields2 in
    FieldsPartiallyKnown unset_fields

(* TODO: add a new reason with positions of merge point and possibly merged
 * envs.*)
and union_reason r1 r2 =
  if r1 = Reason.none then r2 else
    if r2 = Reason.none then r1 else
      if (Reason.compare r1 r2) <= 0 then r1
      else r2

let normalize_union env ?(on_tyvar = fun env r v -> env, (r, Tvar v)) tyl =
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec normalize_union env tyl r_null r_union =
    match tyl with
    | [] -> env, r_null, r_union, TySet.empty
    | ty :: tyl ->
      let ty = if new_inference then Typing_expand.fully_expand env ty else ty in
      let env, r_null, r_union, tys' = match ty with
        | (r, Tvar v) ->
          let env, ty' = on_tyvar env r v in
          if ty_equal ty ty' then env, r_null, r_union, TySet.singleton ty' else
          normalize_union env [ty'] r_null r_union
        | (r, Tprim Nast.Tnull) -> normalize_union env [] (orr r_null r) r_union
        | (r, Toption ty) -> normalize_union env [ty] (orr r_null r) r_union
        | (r, Tunresolved tyl') -> normalize_union env tyl' r_null (orr r_union r)
        | ty -> env, r_null, r_union, TySet.singleton ty in
      let env, r_null, r_union, tys = normalize_union env tyl r_null r_union in
      env, r_null,  r_union, TySet.union tys' tys in
  normalize_union env tyl None None

let union_list_2_by_2 env tyl =
  let max_n_iter = 20 in
  let env, tyl, _ = List.fold tyl ~init:(env, [], 0)
    ~f:(fun (env, tyl, iter) ty ->
      let rec union_ty_w_tyl env ty tyl tyl_acc iter =
        match tyl with
        | [] -> env, ty :: tyl_acc, iter
        | tyl when iter >= max_n_iter -> env, tyl @ (ty :: tyl_acc), iter
        | ty' :: tyl ->
          let env, union_opt = try_union env ty ty' in
          let iter = iter + 1 in
          match union_opt with
          | None -> union_ty_w_tyl env ty tyl (ty' :: tyl_acc) iter
          | Some union -> env, tyl @ (union :: tyl_acc), iter in
      union_ty_w_tyl env ty tyl [] iter) in
  env, tyl

let union_list env r tyl =
  let env, r_null, _r_union, tys = normalize_union env tyl in
  let env, tyl = union_list_2_by_2 env (TySet.elements tys) in
  let new_inference = TypecheckerOptions.new_inference (Env.get_tcopt env) in
  env, make_union r tyl r_null ~discard_singletons:new_inference

let simplify_unions env ?on_tyvar (r, _ as ty) =
  let env, r_null, r_union, tys = normalize_union env [ty] ?on_tyvar in
  let env, tyl = union_list_2_by_2 env (TySet.elements tys) in
  let r = Option.value r_union ~default:r in
  env, make_union r tyl r_null ~discard_singletons:true

let diff ty1 ty2 =
  let filter tyl = List.filter tyl ~f:(fun ty -> not (ty_equal ty ty2)) in
  match ty1 with
  | r, Toption (r', Tunresolved tyl) -> r, Toption (r', Tunresolved (filter tyl))
  | r, Tunresolved tyl -> r, Tunresolved (filter tyl)
  | _ -> ty1

let () = Typing_utils.union_ref := union
let () = Typing_utils.union_list_ref := union_list
let () = Typing_utils.simplify_unions_ref := simplify_unions
let () = Typing_utils.diff_ref := diff
