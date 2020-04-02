(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Hh_core
open Typing_defs
module Env = Typing_env
module Reason = Typing_reason
module TySet = Typing_set
module Utils = Typing_utils
module MakeType = Typing_make_type
module Nast = Aast

exception Dont_simplify

module Log = struct
  let log_union r ty1 ty2 (env, result) =
    Typing_log.(
      log_with_level env "union" 1 (fun () ->
          log_types
            (Reason.to_pos r)
            env
            [
              Log_head
                ( "Typing_union.union",
                  [
                    Log_type ("ty1", ty1);
                    Log_type ("ty2", ty2);
                    Log_type ("result", result);
                  ] );
            ]));
    (env, result)

  let log_simplify_union r ty1 ty2 (env, result) =
    Typing_log.(
      log_with_level env "union" 2 (fun () ->
          log_types
            (Reason.to_pos r)
            env
            [
              Log_head
                ( "Typing_union.simplify_union",
                  [
                    Log_type ("ty1", ty1);
                    Log_type ("ty2", ty2);
                    (match result with
                    | None -> Log_head ("result: None", [])
                    | Some ty -> Log_type ("result: Some", ty));
                  ] );
            ]));
    (env, result)

  let log_union_list r tyl (env, result) =
    Typing_log.(
      log_with_level env "union" 1 (fun () ->
          log_types
            (Reason.to_pos r)
            env
            [
              Log_head
                ( "Typing_union.union_list",
                  List.map tyl ~f:(fun ty -> Log_type ("ty", ty))
                  @ [Log_type ("result", result)] );
            ]));
    (env, result)

  let log_simplify_unions r ty (env, result) =
    Typing_log.(
      log_with_level env "union" 1 (fun () ->
          log_types
            (Reason.to_pos r)
            env
            [
              Log_head
                ( "Typing_union.simplify_unions",
                  [Log_type ("ty", ty); Log_type ("result", result)] );
            ]));
    (env, result)
end

(* Two types are "equivalent" if when expanded they are equal.
 * If they are equivalent, this function returns a type which is equivalent to
 * both, otherwise returns None.
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
  let (env, ety1) = Env.expand_type env ty1 in
  let (env, ety2) = Env.expand_type env ty2 in
  let ty =
    match ((get_node ety1, ty1), (get_node ety2, ty2)) with
    | ((Tunion [], _), (_, ty))
    | ((_, ty), (Tunion [], _))
      when are_ty_param ->
      Some ty
    | _ when ty_equal ety1 ety2 ->
      begin
        match get_node ty1 with
        | Tvar _ -> Some ty1
        | _ -> Some ty2
      end
    | _ -> None
  in
  (env, ty)

(** Constructor for unions and options, taking a list of types and whether the
result should be nullable. Simplify things like singleton unions or nullable nothing. *)
let make_union env r tyl reason_nullable_opt reason_dyn_opt =
  let ty =
    match tyl with
    | [ty] -> ty
    | tyl -> MakeType.union r tyl
  in
  (* Given that unions with null are encoded specially in the type system - as options -
  we still need to simplify unions of null and nonnull here.
  For example, we may have to construct ?(nonnull | any) which should result in mixed.
  For something more complex like ?(A | (B & nonnull)), one can
  distribute the option to give (?A | (?B & ?nonnull)) which then simplifies to ?(A | B).
  This function performs those simplifications. *)
  let simplify_nonnull env ty =
    let is_nonnull ty = ty_equal ty (MakeType.nonnull r) in
    let rec simplify env ty =
      let (env, ty) = Env.expand_type env ty in
      match deref ty with
      | (_, Toption ty) ->
        (* ok to remove the Toption since we'll re-add it at the end. *)
        simplify env ty
      | (r, Tunion tyl) ->
        let (env, tyl) = List.map_env env tyl ~f:simplify in
        if List.exists tyl ~f:is_nonnull then
          (env, MakeType.nonnull r)
        else
          (env, MakeType.union r tyl)
      | (r, Tintersection tyl) ->
        let (env, tyl) = List.map_env env tyl ~f:simplify in
        let tyl = List.filter tyl ~f:(fun ty -> not (is_nonnull ty)) in
        (env, MakeType.intersection r tyl)
      | _ -> (env, ty)
    in
    simplify env ty
  in
  let (env, ty) =
    if Option.is_some reason_nullable_opt then
      simplify_nonnull env ty
    else
      (env, ty)
  in
  let like_ty dyn_r ty = MakeType.union dyn_r [MakeType.dynamic dyn_r; ty] in
  (* Some code is sensitive to dynamic appearing first in the union. *)
  let ty =
    match (reason_nullable_opt, reason_dyn_opt, deref ty) with
    | (None, Some dyn_r, (_, Tunion [])) -> MakeType.dynamic dyn_r
    | (Some null_r, None, (_, Tunion [])) -> MakeType.null null_r
    | (Some null_r, Some dyn_r, (_, Tunion [])) ->
      like_ty dyn_r (MakeType.null null_r)
    | (None, None, _) -> ty
    | (None, Some dyn_r, (r, Tunion tyl)) ->
      MakeType.union r (MakeType.dynamic dyn_r :: tyl)
    | (None, Some dyn_r, _) -> like_ty dyn_r ty
    | (Some null_r, None, _) -> MakeType.nullable_locl null_r ty
    | (Some null_r, Some dyn_r, _) ->
      like_ty dyn_r (MakeType.nullable_locl null_r ty)
  in
  let (env, ty) = Typing_utils.wrap_union_inter_ty_in_var env r ty in
  (env, ty)

let exact_least_upper_bound e1 e2 =
  match (e1, e2) with
  | (Exact, Exact) -> Exact
  | (_, _) -> Nonexact

let rec union env ty1 ty2 =
  let r1 = get_reason ty1 in
  let r2 = get_reason ty2 in
  Log.log_union r2 ty1 ty2
  @@
  if ty_equal ty1 ty2 then
    (env, ty1)
  else if Typing_utils.is_sub_type_for_union env ty1 ty2 then
    (env, ty2)
  else if Typing_utils.is_sub_type_for_union env ty2 ty1 then
    (env, ty1)
  else
    let r = union_reason r1 r2 in
    union_ env ty1 ty2 r

and union_ env ty1 ty2 r = union_lists env [ty1] [ty2] r

(** Simplify the union of two types, for example (int|float) as num.
Returns None if there is no simplification.
Does not deal with null, options, unions and intersections, which are dealt with by union_lists. *)
and simplify_union env ty1 ty2 r =
  Log.log_simplify_union r ty1 ty2
  @@
  if ty_equal ty1 ty2 then
    (env, Some ty1)
  else if Typing_utils.is_sub_type_for_union env ty1 ty2 then
    (env, Some ty2)
  else if Typing_utils.is_sub_type_for_union env ty2 ty1 then
    (env, Some ty1)
  else
    simplify_union_ env ty1 ty2 r

and is_class ty =
  match get_node ty with
  | Tclass _ -> true
  | _ -> false

and simplify_union_ env ty1 ty2 r =
  let (env, ty1) = Env.expand_type env ty1 in
  let (env, ty2) = Env.expand_type env ty2 in
  try
    match (deref ty1, deref ty2) with
    | ((r1, Tprim Nast.Tint), (r2, Tprim Nast.Tfloat))
    | ((r1, Tprim Nast.Tfloat), (r2, Tprim Nast.Tint)) ->
      let r = union_reason r1 r2 in
      (env, Some (MakeType.num r))
    | ((_, Tclass ((p, id1), e1, tyl1)), (_, Tclass ((_, id2), e2, tyl2)))
      when String.equal id1 id2 ->
      let e = exact_least_upper_bound e1 e2 in
      if List.is_empty tyl1 then
        (env, Some (mk (r, Tclass ((p, id1), e, tyl1))))
      else
        let (env, tyl) = union_class env id1 tyl1 tyl2 in
        (env, Some (mk (r, Tclass ((p, id1), e, tyl))))
    | ((_, Tgeneric name1), (_, Tgeneric name2)) when String.equal name1 name2
      ->
      (env, Some (mk (r, Tgeneric name1)))
    | ((_, Tarraykind ak1), (_, Tarraykind ak2)) ->
      let (env, ak) = union_arraykind env ak1 ak2 in
      (env, Some (mk (r, Tarraykind ak)))
    | ((_, Tdependent (dep1, tcstr1)), (_, Tdependent (dep2, tcstr2)))
      when equal_dependent_type dep1 dep2 ->
      let (env, tcstr) = union env tcstr1 tcstr2 in
      (env, Some (mk (r, Tdependent (dep1, tcstr))))
    | ((_, Tdependent (_, ty1)), _) when is_class ty1 ->
      ty_equiv env ty1 ty2 ~are_ty_param:false
    | (_, (_, Tdependent (_, ty2))) when is_class ty2 ->
      ty_equiv env ty2 ty1 ~are_ty_param:false
    | ((_, Tnewtype (id1, tyl1, tcstr1)), (_, Tnewtype (id2, tyl2, tcstr2)))
      when String.equal id1 id2 ->
      if List.is_empty tyl1 then
        (env, Some ty1)
      else
        let (env, tyl) = union_newtype env id1 tyl1 tyl2 in
        let (env, tcstr) = union env tcstr1 tcstr2 in
        (env, Some (mk (r, Tnewtype (id1, tyl, tcstr))))
    | ((_, Ttuple tyl1), (_, Ttuple tyl2)) ->
      if Int.equal (List.length tyl1) (List.length tyl2) then
        let (env, tyl) = List.map2_env env tyl1 tyl2 ~f:union in
        (env, Some (mk (r, Ttuple tyl)))
      else
        (env, None)
    | ((r1, Tshape (shape_kind1, fdm1)), (r2, Tshape (shape_kind2, fdm2))) ->
      let (env, ty) =
        union_shapes env (shape_kind1, fdm1, r1) (shape_kind2, fdm2, r2)
      in
      (env, Some (mk (r, ty)))
    | ((_, Tfun ft1), (_, Tfun ft2)) ->
      let (env, ft) = union_funs env ft1 ft2 in
      (env, Some (mk (r, Tfun ft)))
    (* TODO with Tclass, union type arguments if covariant *)
    | ( ( _,
          ( ( Tarraykind _ | Tprim _ | Tdynamic | Tgeneric _ | Tnewtype _
            | Tdependent _ | Tclass _ | Ttuple _ | Tfun _ | Tobject | Tshape _
            | Terr
            | Tvar _
            (* If T cannot be null, `union T nonnull = nonnull`. However, it's hard
             * to say whether a given T can be null - e.g. opaque newtypes, dependent
             * types, etc. - so for now we leave it here.
             * TODO improve that. *)
            | Tnonnull | Tany _ | Tintersection _ | Tpu _ | Tpu_type_access _
            | Toption _ | Tunion _ ) as ty1_ ) ),
        (_, ty2_) ) ->
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
        | _ -> ()
      in
      add env ty1_;
      add env ty2_;
      ty_equiv env ty1 ty2 ~are_ty_param:false
  with Dont_simplify -> (env, None)

and union_lists env tyl1 tyl2 r =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec decompose env ty tyl tyl_res r_null r_union r_dyn =
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (r, Tunion tyl') ->
      decompose_list env (tyl' @ tyl) tyl_res r_null (orr r_union r) r_dyn
    | (r, Toption ty) ->
      decompose env ty tyl tyl_res (orr r_null r) r_union r_dyn
    | (r, Tprim Aast.Tnull) ->
      decompose_list env tyl tyl_res (orr r_null r) r_union r_dyn
    | (r, Tdynamic) ->
      decompose_list env tyl tyl_res r_null r_union (orr r_dyn r)
    | _ -> decompose_list env tyl (ty :: tyl_res) r_null r_union r_dyn
  and decompose_list env tyl tyl_res r_null r_union r_dyn =
    match tyl with
    | [] -> (env, (tyl_res, r_null, r_union, r_dyn))
    | ty :: tyl -> decompose env ty tyl tyl_res r_null r_union r_dyn
  in
  let decompose env tyl = decompose_list env tyl [] None None None in
  let (env, (tyl1, r_null1, _r_union1, r_dyn1)) = decompose env tyl1 in
  let (env, (tyl2, r_null2, _r_union2, r_dyn2)) = decompose env tyl2 in
  let product_ty_tyl env ty1 tyl2 =
    let rec product_ty_tyl env ty1 tyl2 tyl_res =
      match tyl2 with
      | [] -> (env, ty1 :: tyl_res)
      | ty2 :: tyl2 ->
        let (env, ty_opt) = simplify_union env ty1 ty2 r in
        (match ty_opt with
        | None -> product_ty_tyl env ty1 tyl2 (ty2 :: tyl_res)
        | Some ty -> product_ty_tyl env ty tyl2 tyl_res)
    in
    product_ty_tyl env ty1 tyl2 []
  in
  let rec product env tyl1 tyl2 =
    match (tyl1, tyl2) with
    | ([], tyl)
    | (tyl, []) ->
      (env, tyl)
    | (ty1 :: tyl1, tyl2) ->
      let (env, tyl2) = product_ty_tyl env ty1 tyl2 in
      product env tyl1 tyl2
  in
  let (env, tyl) = product env tyl1 tyl2 in
  let r_null = Option.first_some r_null1 r_null2 in
  let r_dyn = Option.first_some r_dyn1 r_dyn2 in
  make_union env r tyl r_null r_dyn

and union_arraykind env ak1 ak2 =
  match (ak1, ak2) with
  | (AKvarray ty1, AKvarray ty2) ->
    let (env, ty) = union env ty1 ty2 in
    (env, AKvarray ty)
  | (AKdarray (tk1, tv1), AKdarray (tk2, tv2)) ->
    let (env, tk) = union env tk1 tk2 in
    let (env, tv) = union env tv1 tv2 in
    (env, AKdarray (tk, tv))
  | (AKvarray_or_darray (tk1, tv1), AKvarray_or_darray (tk2, tv2)) ->
    let (env, tk) = union env tk1 tk2 in
    let (env, tv) = union env tv1 tv2 in
    (env, AKvarray_or_darray (tk, tv))
  | _ -> raise Dont_simplify

and union_funs env fty1 fty2 =
  (* TODO: If we later add fields to ft, they will be forgotten here. *)
  if
    equal_locl_fun_arity fty1 fty2
    && equal_reactivity fty1.ft_reactive fty2.ft_reactive
    && Int.equal fty1.ft_flags fty2.ft_flags
    && Int.equal (ft_params_compare fty1.ft_params fty2.ft_params) 0
  then
    let (env, ft_ret) =
      union_possibly_enforced_tys env fty1.ft_ret fty2.ft_ret
    in
    (env, { fty1 with ft_ret })
  else
    raise Dont_simplify

and union_possibly_enforced_tys env ety1 ety2 =
  let (env, et_type) = union env ety1.et_type ety2.et_type in
  (env, { ety1 with et_type })

and union_class env name tyl1 tyl2 =
  let tparams =
    match Env.get_class env name with
    | None -> []
    | Some c -> Decl_provider.Class.tparams c
  in
  union_tylists_w_variances env tparams tyl1 tyl2

and union_newtype env typename tyl1 tyl2 =
  let tparams =
    match Env.get_typedef env typename with
    | None -> []
    | Some t -> t.td_tparams
  in
  union_tylists_w_variances env tparams tyl1 tyl2

and union_tylists_w_variances env tparams tyl1 tyl2 =
  let variances = List.map tparams (fun t -> t.tp_variance) in
  let variances =
    let adjust_list_length l newlen filler =
      let len = List.length l in
      if len < newlen then
        l @ List.init (newlen - len) (fun _ -> filler)
      else
        List.sub l 0 newlen
    in
    adjust_list_length variances (List.length tyl1) Ast_defs.Invariant
  in
  let merge_ty_params env ty1 ty2 variance =
    match variance with
    | Ast_defs.Covariant -> union env ty1 ty2
    | _ ->
      let (env, ty_opt) = ty_equiv env ty1 ty2 ~are_ty_param:true in
      (match ty_opt with
      | Some ty -> (env, ty)
      | None -> raise Dont_simplify)
  in
  let (env, tyl) =
    try List.map3_env env tyl1 tyl2 variances ~f:merge_ty_params
    with Invalid_argument _ -> raise Dont_simplify
  in
  (env, tyl)

and union_shapes env (shape_kind1, fdm1, r1) (shape_kind2, fdm2, r2) =
  let shape_kind = union_shape_kind shape_kind1 shape_kind2 in
  let ((env, shape_kind), fdm) =
    Nast.ShapeMap.merge_env
      (env, shape_kind)
      fdm1
      fdm2
      ~combine:(fun (env, shape_kind) k fieldopt1 fieldopt2 ->
        match ((shape_kind1, fieldopt1, r1), (shape_kind2, fieldopt2, r2)) with
        | ((_, None, _), (_, None, _)) -> ((env, shape_kind), None)
        (* key is present on one side but not the other *)
        | ((_, Some { sft_ty; _ }, _), (shape_kind_other, None, r))
        | ((shape_kind_other, None, r), (_, Some { sft_ty; _ }, _)) ->
          let sft_ty =
            match shape_kind_other with
            | Closed_shape -> sft_ty
            | Open_shape ->
              let r =
                Reason.Rmissing_optional_field
                  (Reason.to_pos r, Utils.get_printable_shape_field_name k)
              in
              MakeType.mixed r
          in
          ((env, shape_kind), Some { sft_optional = true; sft_ty })
        (* key is present on both sides *)
        | ( (_, Some { sft_optional = optional1; sft_ty = ty1 }, _),
            (_, Some { sft_optional = optional2; sft_ty = ty2 }, _) ) ->
          let sft_optional = optional1 || optional2 in
          let (env, sft_ty) = union env ty1 ty2 in
          ((env, shape_kind), Some { sft_optional; sft_ty }))
  in
  (env, Tshape (shape_kind, fdm))

and union_shape_kind shape_kind1 shape_kind2 =
  match (shape_kind1, shape_kind2) with
  | (Closed_shape, Closed_shape) -> Closed_shape
  | _ -> Open_shape

(* TODO: add a new reason with positions of merge point and possibly merged
 * envs.*)
and union_reason r1 r2 =
  if Reason.(equal r1 none) then
    r2
  else if Reason.(equal r2 none) then
    r1
  else if Reason.compare r1 r2 <= 0 then
    r1
  else
    r2

let normalize_union env ?on_tyvar tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec normalize_union env tyl r_null r_union r_dyn =
    match tyl with
    | [] -> (env, r_null, r_union, r_dyn, TySet.empty)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      let proceed env ty = (env, r_null, r_union, r_dyn, TySet.singleton ty) in
      let (env, r_null, r_union, r_dyn, tys') =
        match (deref ty, on_tyvar) with
        | ((r, Tvar v), Some on_tyvar) ->
          let (env, ty') = on_tyvar env r v in
          if ty_equal ty ty' then
            proceed env ty'
          else
            normalize_union env [ty'] r_null r_union r_dyn
        | ((r, Tprim Nast.Tnull), _) ->
          normalize_union env [] (orr r_null r) r_union r_dyn
        | ((r, Toption ty), _) ->
          normalize_union env [ty] (orr r_null r) r_union r_dyn
        | ((r, Tunion tyl'), _) ->
          normalize_union env tyl' r_null (orr r_union r) r_dyn
        | ((r, Tdynamic), _) ->
          normalize_union env [] r_null r_union (orr r_dyn r)
        | ((_, Tintersection _), _) ->
          let (env, ty) = Utils.simplify_intersections env ty ?on_tyvar in
          let (env, ety) = Env.expand_type env ty in
          begin
            match deref ety with
            | (_, Tintersection _) -> proceed env ty
            | _ -> normalize_union env [ty] r_null r_union r_dyn
          end
        | _ -> proceed env ty
      in
      let (env, r_null, r_union, r_dyn, tys) =
        normalize_union env tyl r_null r_union r_dyn
      in
      (env, r_null, r_union, r_dyn, TySet.union tys' tys)
  in
  normalize_union env tyl None None None

let union_list_2_by_2 env r tyl =
  let (env, tyl) =
    List.fold tyl ~init:(env, []) ~f:(fun (env, tyl) ty ->
        let rec union_ty_w_tyl env ty tyl tyl_acc =
          match tyl with
          | [] -> (env, ty :: tyl_acc)
          | ty' :: tyl ->
            let (env, union_opt) = simplify_union env ty ty' r in
            (match union_opt with
            | None -> union_ty_w_tyl env ty tyl (ty' :: tyl_acc)
            | Some union -> (env, tyl @ (union :: tyl_acc)))
        in
        union_ty_w_tyl env ty tyl [])
  in
  (env, tyl)

let union_list env r tyl =
  Log.log_union_list r tyl
  @@
  let (env, r_null, _r_union, r_dyn, tys) = normalize_union env tyl in
  let (env, tyl) = union_list_2_by_2 env r (TySet.elements tys) in
  make_union env r tyl r_null r_dyn

let fold_union env r tyl =
  List.fold_left_env env tyl ~init:(MakeType.nothing r) ~f:union

let simplify_unions env ?on_tyvar ty =
  let r = get_reason ty in
  Log.log_simplify_unions r ty
  @@
  let (env, r_null, r_union, r_dyn, tys) = normalize_union env [ty] ?on_tyvar in
  let (env, tyl) = union_list_2_by_2 env r (TySet.elements tys) in
  let r = Option.value r_union ~default:r in
  make_union env r tyl r_null r_dyn

let rec union_i env r ty1 lty2 =
  let ty2 = LoclType lty2 in
  if Typing_utils.is_sub_type_for_union_i env ty1 ty2 then
    (env, ty2)
  else if Typing_utils.is_sub_type_for_union_i env ty2 ty1 then
    (env, ty1)
  else
    let (env, ty) =
      match ty1 with
      | LoclType lty1 ->
        let (env, ty) = union env lty1 lty2 in
        (env, LoclType ty)
      | ConstraintType cty1 ->
        (match deref_constraint_type cty1 with
        | (_, TCunion (lty1, cty1)) ->
          let (env, lty) = union env lty1 lty2 in
          (env, ConstraintType (mk_constraint_type (r, TCunion (lty, cty1))))
        | (r', TCintersection (lty1, cty1)) ->
          (* Distribute union over intersection.
          At the moment local types in TCintersection can only be
          unions or intersections involving only null and nonnull,
          so applying distributivity allows for simplifying the types. *)
          let (env, lty) = union env lty1 lty2 in
          let (env, ty) = union_i env r (ConstraintType cty1) lty2 in
          (match ty with
          | LoclType ty ->
            let (env, ty) = Typing_intersection.intersect env r' lty ty in
            (env, LoclType ty)
          | ConstraintType cty ->
            ( env,
              ConstraintType
                (mk_constraint_type (r', TCintersection (lty, cty))) ))
        | (_, Thas_member _)
        | (_, Tdestructure _) ->
          (env, ConstraintType (mk_constraint_type (r, TCunion (lty2, cty1)))))
    in
    match ty with
    | LoclType _ -> (env, ty)
    | ConstraintType ty -> Utils.simplify_constraint_type env ty

let () = Typing_utils.union_ref := union

let () = Typing_utils.union_i_ref := union_i

let () = Typing_utils.union_list_ref := union_list

let () = Typing_utils.fold_union_ref := fold_union

let () = Typing_utils.simplify_unions_ref := simplify_unions

let () =
  (* discard optional parameter. *)
  let simplify_unions env ty = simplify_unions env ty in
  Typing_env.simplify_unions_ref := simplify_unions
