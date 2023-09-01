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
module Env = Typing_env
module Reason = Typing_reason
module TySet = Typing_set
module Utils = Typing_utils
module MakeType = Typing_make_type
module Nast = Aast
module Cls = Decl_provider.Class

exception Dont_simplify

module Log = struct
  let log_union r ty1 ty2 (env, result) =
    Typing_log.(
      log_with_level env "union" ~level:1 (fun () ->
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
      log_with_level env "union" ~level:2 (fun () ->
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
      log_with_level env "union" ~level:1 (fun () ->
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
      log_with_level env "union" ~level:1 (fun () ->
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
    | _ when ty_equal ety1 ety2 -> begin
      match get_node ty1 with
      | Tvar _ -> Some ty1
      | _ -> Some ty2
    end
    | _ -> None
  in
  (env, ty)

(* Destructure a union into a list of its sub-types, decending into sub-unions,
   and also pulling out null and dynamic.
*)
let dest_union_list env tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec dest_union env ty tyl tyl_res r_null r_union r_dyn =
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (r, Tunion tyl') ->
      dest_union_list env (tyl' @ tyl) tyl_res r_null (orr r_union r) r_dyn
    | (r, Toption ty) ->
      dest_union env ty tyl tyl_res (orr r_null r) r_union r_dyn
    | (r, Tprim Aast.Tnull) ->
      dest_union_list env tyl tyl_res (orr r_null r) r_union r_dyn
    | (r, Tdynamic) ->
      dest_union_list env tyl tyl_res r_null r_union (orr r_dyn r)
    | _ -> dest_union_list env tyl (ty :: tyl_res) r_null r_union r_dyn
  and dest_union_list env tyl tyl_res r_null r_union r_dyn =
    match tyl with
    | [] -> (env, (tyl_res, r_null, r_union, r_dyn))
    | ty :: tyl -> dest_union env ty tyl tyl_res r_null r_union r_dyn
  in
  dest_union_list env tyl [] None None None

(** Constructor for unions and options, taking a list of types and whether the
result should be nullable. Simplify things like singleton unions or nullable nothing.
This allows the result of any unioning function from this module to be flattened and normalized,
with options being "pushed out" of the union (e.g. we return ?(A|B) instead of (A | ?B)
or (A | null | B), and similarly with like types). *)
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
    | (Some null_r, None, _) -> MakeType.nullable null_r ty
    | (Some null_r, Some dyn_r, _) ->
      like_ty dyn_r (MakeType.nullable null_r ty)
  in
  let (env, ty) = Utils.wrap_union_inter_ty_in_var env r ty in
  (env, ty)

let exact_least_upper_bound e1 e2 =
  match (e1, e2) with
  | (Exact, Exact) -> Exact
  | (_, _) -> nonexact

let rec union env ?(approx_cancel_neg = false) ty1 ty2 =
  let r1 = get_reason ty1 in
  let r2 = get_reason ty2 in
  Log.log_union r2 ty1 ty2
  @@
  if ty_equal ty1 ty2 then
    (env, ty1)
  else if Utils.is_sub_type_for_union env ty1 ty2 then
    (env, ty2)
  else if Utils.is_sub_type_for_union env ty2 ty1 then
    (env, ty1)
  else
    let r = union_reason r1 r2 in
    let (env, non_ty2) =
      Typing_intersection.negate_type env Reason.none ty2 ~approx:Utils.ApproxUp
    in
    if Utils.is_sub_type_for_union env non_ty2 ty1 then
      (env, MakeType.mixed r)
    else
      let (env, non_ty1) =
        Typing_intersection.negate_type
          env
          Reason.none
          ty1
          ~approx:Utils.ApproxUp
      in
      if Utils.is_sub_type_for_union env non_ty1 ty2 then
        (env, MakeType.mixed r)
      else
        union_ ~approx_cancel_neg env ty1 ty2 r

and union_ ~approx_cancel_neg env ty1 ty2 r =
  union_lists ~approx_cancel_neg env [ty1] [ty2] r

(** Simplify the union of two types, for example (int|float) as num.
Returns None if there is no simplification.
Does not deal with null, options, unions and intersections, which are dealt with by union_lists.
If approx_cancel_neg is true, then some unions with negations are treated over-approximately:
C<t> | not C simplifies to mixed.*)
and simplify_union ~approx_cancel_neg env ty1 ty2 r =
  Log.log_simplify_union r ty1 ty2
  @@
  if ty_equal ty1 ty2 then
    (env, Some ty1)
  else if Utils.is_sub_type_for_union env ty1 ty2 then
    (env, Some ty2)
  else if Utils.is_sub_type_for_union env ty2 ty1 then
    (env, Some ty1)
  else
    simplify_union_ ~approx_cancel_neg env ty1 ty2 r

and simplify_union_ ~approx_cancel_neg env ty1 ty2 r =
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
        let (env, tyl) = union_class ~approx_cancel_neg env id1 tyl1 tyl2 in
        (env, Some (mk (r, Tclass ((p, id1), e, tyl))))
    | ((_, Tgeneric (name1, [])), (_, Tgeneric (name2, [])))
      when String.equal name1 name2 ->
      (* TODO(T69551141) handle type arguments above and below properly *)
      (env, Some (mk (r, Tgeneric (name1, []))))
    | ((_, Tvec_or_dict (tk1, tv1)), (_, Tvec_or_dict (tk2, tv2))) ->
      let (env, tk) = union ~approx_cancel_neg env tk1 tk2 in
      let (env, tv) = union ~approx_cancel_neg env tv1 tv2 in
      (env, Some (mk (r, Tvec_or_dict (tk, tv))))
    | ((_, Tdependent (dep1, tcstr1)), (_, Tdependent (dep2, tcstr2)))
      when equal_dependent_type dep1 dep2 ->
      let (env, tcstr) = union ~approx_cancel_neg env tcstr1 tcstr2 in
      (env, Some (mk (r, Tdependent (dep1, tcstr))))
    | ((_, Tdependent (_, ty1)), _) when Utils.is_class ty1 ->
      ty_equiv env ty1 ty2 ~are_ty_param:false
    | (_, (_, Tdependent (_, ty2))) when Utils.is_class ty2 ->
      ty_equiv env ty2 ty1 ~are_ty_param:false
    | ((r1, Tnewtype (id1, [tyl1], _)), (r2, Tnewtype (id2, [tyl2], _)))
      when String.equal id1 id2
           && String.equal id1 Naming_special_names.Classes.cSupportDyn ->
      let r = union_reason r1 r2 in
      let (env, try_union) =
        simplify_union ~approx_cancel_neg env tyl1 tyl2 r
      in
      (env, Option.map ~f:(MakeType.supportdyn r) try_union)
    | ((_, Tnewtype (id1, tyl1, tcstr1)), (_, Tnewtype (id2, tyl2, tcstr2)))
      when String.equal id1 id2
           && not (String.equal id1 Naming_special_names.Classes.cSupportDyn) ->
      if List.is_empty tyl1 then
        (env, Some ty1)
      else
        let (env, tyl) = union_newtype ~approx_cancel_neg env id1 tyl1 tyl2 in
        let (env, tcstr) = union ~approx_cancel_neg env tcstr1 tcstr2 in
        (env, Some (mk (r, Tnewtype (id1, tyl, tcstr))))
    | ((_, Ttuple tyl1), (_, Ttuple tyl2)) ->
      if Int.equal (List.length tyl1) (List.length tyl2) then
        let (env, tyl) =
          List.map2_env env tyl1 tyl2 ~f:(union ~approx_cancel_neg)
        in
        (env, Some (mk (r, Ttuple tyl)))
      else
        (env, None)
    | ( ( r1,
          Tshape
            { s_origin = _; s_unknown_value = shape_kind1; s_fields = fdm1 } ),
        ( r2,
          Tshape
            { s_origin = _; s_unknown_value = shape_kind2; s_fields = fdm2 } )
      ) ->
      let (env, ty) =
        union_shapes
          ~approx_cancel_neg
          env
          (shape_kind1, fdm1, r1)
          (shape_kind2, fdm2, r2)
      in
      (env, Some (mk (r, ty)))
    | ((_, Tfun ft1), (_, Tfun ft2)) ->
      let (env, ft) = union_funs ~approx_cancel_neg env ft1 ft2 in
      (env, Some (mk (r, Tfun ft)))
    | ((_, Tunapplied_alias _), _) ->
      Typing_defs.error_Tunapplied_alias_in_illegal_context ()
    | ((_, Tneg (Neg_class (_, c1))), (_, Tclass ((_, c2), Exact, [])))
    | ((_, Tclass ((_, c2), Exact, [])), (_, Tneg (Neg_class (_, c1))))
      when String.equal c1 c2 ->
      (env, Some (MakeType.mixed r))
    | ((_, Tneg (Neg_class (_, c1))), (_, Tclass ((_, c2), Nonexact _, [])))
    | ((_, Tclass ((_, c2), Nonexact _, [])), (_, Tneg (Neg_class (_, c1))))
      when Utils.is_sub_class_refl env c1 c2 ->
      (* This union is mixed iff for all objects o,
         o not in complement (union tyl. c1<tyl>) implies o in c2,
         which is equivalent to
         (union tyl. c1<tyl>) subset c2.
         This is certainly the case when c1 is a sub-class of c2, since
         c2 has no type parameters.
      *)
      (env, Some (MakeType.mixed r))
    | ((_, Tneg (Neg_class (_, c1))), (_, Tclass ((_, c2), Nonexact _, _ :: _)))
    | ((_, Tclass ((_, c2), Nonexact _, _ :: _)), (_, Tneg (Neg_class (_, c1))))
      when approx_cancel_neg && Utils.is_sub_class_refl env c1 c2 ->
      (* Unlike the case where c2 has no parameters, here we can get a situation
         where c1 is a sub-class of c2, but they don't union to mixed. For example,
         Vector<int> | not Vector, which doesn't include Vector<string>, etc. However,
         we can still approximate up to mixed when it would be both sound and convenient,
         as controlled by the approx_cancel_neg flag. *)
      (env, Some (MakeType.mixed r))
    | ((_, Tneg (Neg_class (_, c1))), (_, Tclass ((_, c2), Exact, _ :: _)))
    | ((_, Tclass ((_, c2), Exact, _ :: _)), (_, Tneg (Neg_class (_, c1))))
      when approx_cancel_neg && String.equal c1 c2 ->
      (env, Some (MakeType.mixed r))
    | ((_, Tneg (Neg_prim tp1)), (_, Tprim tp2))
    | ((_, Tprim tp1), (_, Tneg (Neg_prim tp2)))
      when Aast_defs.equal_tprim tp1 tp2 ->
      (env, Some (MakeType.mixed r))
    | ((_, Tneg (Neg_prim Aast.Tnum)), (_, Tprim Aast.Tarraykey))
    | ((_, Tprim Aast.Tarraykey), (_, Tneg (Neg_prim Aast.Tnum))) ->
      (env, Some (MakeType.neg r (Neg_prim Aast.Tfloat)))
    | ((_, Tneg (Neg_prim Aast.Tarraykey)), (_, Tprim Aast.Tnum))
    | ((_, Tprim Aast.Tnum), (_, Tneg (Neg_prim Aast.Tarraykey))) ->
      (env, Some (MakeType.neg r (Neg_prim Aast.Tstring)))
    | ((r1, Tintersection tyl1), (r2, Tintersection tyl2)) ->
      (match Typing_algebra.factorize_common_types tyl1 tyl2 with
      | ([], _, _) ->
        (* No common types, fall back to default case *)
        ty_equiv env ty1 ty2 ~are_ty_param:false
      | (common_tyl, tyl1', tyl2') ->
        (match (tyl1', tyl2') with
        | ([], _)
        | (_, []) ->
          (* If one of the intersections is now empty, then the union is mixed,
             and we just return the common types from the intersetion *)
          (env, Some (MakeType.intersection r common_tyl))
        | _ ->
          let (env, union_ty) =
            match (tyl1', tyl2') with
            | ([ty1], [ty2]) ->
              (* It seems like it is wasteful to simplify with the main union function which checks
                 subtyping. If there was a subtype relationship between these two, there
                 would have been one with the common types in, and we wouldn't get here.
                 However, in some cases, that is not the case. E.g., ?#1 & t <: #1 & t will
                 not pass the initial sub-type check, but will here *)
              union ~approx_cancel_neg env ty1 ty2
            | ([ty], tyl)
            | (tyl, [ty]) ->
              union_lists
                ~approx_cancel_neg
                env
                [ty]
                [MakeType.intersection r1 tyl]
                r
            | _ ->
              ( env,
                MakeType.union
                  r
                  [
                    MakeType.intersection r1 tyl1';
                    MakeType.intersection r2 tyl2';
                  ] )
          in
          (match common_tyl with
          | [common_ty] ->
            let (env, inter_ty) =
              Typing_intersection.intersect env ~r common_ty union_ty
            in
            (env, Some inter_ty)
          | _ -> (env, Some (MakeType.intersection r (union_ty :: common_tyl))))))
    (* TODO with Tclass, union type arguments if covariant *)
    | ( ( _,
          ( Tprim _ | Tdynamic | Tgeneric _ | Tnewtype _ | Tdependent _
          | Tclass _ | Ttuple _ | Tfun _ | Tshape _ | Tvar _ | Tvec_or_dict _
          (* If T cannot be null, `union T nonnull = nonnull`. However, it's hard
           * to say whether a given T can be null - e.g. opaque newtypes, dependent
           * types, etc. - so for now we leave it here.
           * TODO improve that. *)
          | Tnonnull | Tany _ | Tintersection _ | Toption _ | Tunion _
          | Taccess _ | Tneg _ ) ),
        (_, _) ) ->
      ty_equiv env ty1 ty2 ~are_ty_param:false
  with
  | Dont_simplify -> (env, None)

(* Recognise a pattern t1 | t2 | t3 | (t4 & t5 & t6) which happens frequently
   at control flow join points, and check for types that drop out of the intersection
   because they union to mixed with types in the union. Assume that all of the types
   in tyl1 and tyl2 are not unions, having been processed by decompose in the
   union_lists function. The semantics are that tyl1 and tyl2 are the union of their
   elements, and we are building the union of the two lists. Instead of just appending,
   we look for the special case where one of the elements of tyl1 or tyl2 is itself
   an intersection, and try to cancel things out. We return the simplified
   lists for further processing.

   For example if tyl1 = [bool; int; string] represents bool | int | string, and
   tyl2 = [float; (dynamic & not int & not string)],
   then to union tyl1 and tyl2, we can remove the not int and not string conjuncts
   from tyl2 because they union to mixed with elements of tyl1. Thus we end up with
   a result [bool; int; string], [float; dynamic]
*)
and try_special_union_of_intersection ~approx_cancel_neg env tyl1 tyl2 r :
    Typing_env_types.env * locl_ty list * locl_ty list =
  (* Assume that inter_ty is an intersection type and look for a pairs of a conjunct in
     inter_ty and an element of tyl that union to mixed. Remove these from the intersection,
     and return the remaining elements of the intersection,
     or return None indicating that no pairs were found.

     If inter_ty is wrapped in supportdyn, remove it.
     This is justified because supportdyn<t1 & t2> | u = (supportdyn<mixed> | u) & (t1 | u) & (t2 | u)
     and if (say) t1 | u = mixed, then we get (supportdyn<mixed> | u) & (t2 | u) and we can pull out
     u to get supportdyn<t2> | u.
     If any of the types in tyl are supportdyn, then we can remove that too because
     in supportdyn<t1 & t2> | supportdyn<u> we can lift the supportdyn out, simplify and then push it back.
  *)
  let simplify_inter env inter_ty tyl : Typing_env_types.env * locl_ty option =
    let changed = ref false in
    let (had_supportdyn_inter, env, stripped_inter_ty) =
      Typing_utils.strip_supportdyn env inter_ty
    in
    let sd_r = get_reason inter_ty in
    let (env, (inter_tyl, inter_r)) =
      Typing_intersection.destruct_inter_list env [stripped_inter_ty]
    in
    let inter_r = Option.value inter_r ~default:r in
    let (env, new_inter_tyl) =
      List.filter_map_env env inter_tyl ~f:(fun env inter_ty ->
          match
            List.exists_env env tyl ~f:(fun env ty ->
                let (_, env, stripped_ty) =
                  if had_supportdyn_inter then
                    Typing_utils.strip_supportdyn env ty
                  else
                    (false, env, ty)
                in
                match
                  simplify_union_ ~approx_cancel_neg env inter_ty stripped_ty r
                with
                | (env, None) -> (env, false)
                | (env, Some ty) -> (env, Utils.is_mixed env ty))
          with
          | (env, true) ->
            changed := true;
            (env, None)
          | (env, false) -> (env, Some inter_ty))
    in
    ( env,
      if !changed then
        let ty = MakeType.intersection inter_r new_inter_tyl in
        if had_supportdyn_inter then
          Some (MakeType.supportdyn sd_r ty)
        else
          Some ty
      else
        None )
  in
  let is_intersection env ty =
    let (_env, ty) = Env.expand_type env ty in
    match get_node ty with
    | Tnewtype (n, [ty], _)
      when String.equal n Naming_special_names.Classes.cSupportDyn ->
      Utils.is_tintersection env ty
    | _ -> Utils.is_tintersection env ty
  in
  let (inter_tyl1, not_inter_tyl1) =
    List.partition_tf tyl1 ~f:(is_intersection env)
  in
  let (inter_tyl2, not_inter_tyl2) =
    List.partition_tf tyl2 ~f:(is_intersection env)
  in
  let (env, res_tyl1) =
    match inter_tyl1 with
    | [inter_ty1] ->
      (match simplify_inter env inter_ty1 not_inter_tyl2 with
      | (env, None) -> (env, tyl1)
      | (env, Some new_inter1) -> (env, new_inter1 :: not_inter_tyl1))
    | _ -> (env, tyl1)
  in
  let (env, res_tyl2) =
    match inter_tyl2 with
    | [inter_ty2] ->
      (match simplify_inter env inter_ty2 not_inter_tyl1 with
      | (env, None) -> (env, tyl2)
      | (env, Some new_inter1) -> (env, new_inter1 :: not_inter_tyl2))
    | _ -> (env, tyl2)
  in
  (env, res_tyl1, res_tyl2)

(** Union two lists of types together.
This has complexity N*M where N, M are the sized of the two lists.
The two lists are first flattened and null and dynamic are extracted from them,
then we attempt to simplify each pair of types. *)
and union_lists ~approx_cancel_neg env tyl1 tyl2 r =
  let product_ty_tyl env ty1 tyl2 =
    let rec product_ty_tyl env ty1 tyl2 tyl_res =
      match tyl2 with
      | [] -> (env, ty1 :: tyl_res)
      | ty2 :: tyl2 ->
        let (env, ty_opt) = simplify_union ~approx_cancel_neg env ty1 ty2 r in
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
  let (env, (tyl1, r_null1, _r_union1, r_dyn1)) = dest_union_list env tyl1 in
  let (env, (tyl2, r_null2, _r_union2, r_dyn2)) = dest_union_list env tyl2 in
  let (env, tyl1, tyl2) =
    try_special_union_of_intersection ~approx_cancel_neg env tyl1 tyl2 r
  in
  let (env, tyl) = product env tyl1 tyl2 in
  let r_null = Option.first_some r_null1 r_null2 in
  let r_dyn = Option.first_some r_dyn1 r_dyn2 in
  make_union env r tyl r_null r_dyn

and union_funs ~approx_cancel_neg env fty1 fty2 =
  (* TODO: If we later add fields to ft, they will be forgotten here. *)
  if
    Int.equal fty1.ft_flags fty2.ft_flags
    && Int.equal (ft_params_compare fty1.ft_params fty2.ft_params) 0
  then
    let (env, ft_ret) =
      union_possibly_enforced_tys ~approx_cancel_neg env fty1.ft_ret fty2.ft_ret
    in
    (env, { fty1 with ft_ret })
  else
    raise Dont_simplify

and union_possibly_enforced_tys ~approx_cancel_neg env ety1 ety2 =
  let (env, et_type) = union ~approx_cancel_neg env ety1.et_type ety2.et_type in
  (env, { ety1 with et_type })

and union_class ~approx_cancel_neg env name tyl1 tyl2 =
  let tparams =
    match Env.get_class env name with
    | None -> []
    | Some c -> Cls.tparams c
  in
  union_tylists_w_variances ~approx_cancel_neg env tparams tyl1 tyl2

and union_newtype ~approx_cancel_neg env typename tyl1 tyl2 =
  let tparams =
    match Env.get_typedef env typename with
    | None -> []
    | Some t -> t.td_tparams
  in
  union_tylists_w_variances ~approx_cancel_neg env tparams tyl1 tyl2

and union_tylists_w_variances ~approx_cancel_neg env tparams tyl1 tyl2 =
  let variances = List.map tparams ~f:(fun t -> t.tp_variance) in
  let variances =
    let adjust_list_length l newlen filler =
      let len = List.length l in
      if len < newlen then
        l @ List.init (newlen - len) ~f:(fun _ -> filler)
      else
        List.sub l ~pos:0 ~len:newlen
    in
    adjust_list_length variances (List.length tyl1) Ast_defs.Invariant
  in
  let merge_ty_params env ty1 ty2 variance =
    match variance with
    | Ast_defs.Covariant -> union ~approx_cancel_neg env ty1 ty2
    | _ ->
      let (env, ty_opt) = ty_equiv env ty1 ty2 ~are_ty_param:true in
      (match ty_opt with
      | Some ty -> (env, ty)
      | None -> raise Dont_simplify)
  in
  let (env, tyl) =
    try List.map3_env env tyl1 tyl2 variances ~f:merge_ty_params with
    | Invalid_argument _ -> raise Dont_simplify
  in
  (env, tyl)

and union_shapes
    ~approx_cancel_neg env (shape_kind1, fdm1, r1) (shape_kind2, fdm2, r2) =
  let (env, shape_kind) =
    union_shape_kind ~approx_cancel_neg env shape_kind1 shape_kind2
  in
  let ((env, shape_kind), fdm) =
    TShapeMap.merge_env
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
            if is_nothing shape_kind_other then
              sft_ty
            else
              let r =
                Reason.Rmissing_optional_field
                  (Reason.to_pos r, Utils.get_printable_shape_field_name k)
              in
              with_reason shape_kind_other r
          in
          ((env, shape_kind), Some { sft_optional = true; sft_ty })
        (* key is present on both sides *)
        | ( (_, Some { sft_optional = optional1; sft_ty = ty1 }, _),
            (_, Some { sft_optional = optional2; sft_ty = ty2 }, _) ) ->
          let sft_optional = optional1 || optional2 in
          let (env, sft_ty) = union ~approx_cancel_neg env ty1 ty2 in
          ((env, shape_kind), Some { sft_optional; sft_ty }))
  in
  ( env,
    Tshape
      {
        s_origin = Missing_origin;
        s_unknown_value = shape_kind;
        s_fields = fdm;
      } )

and union_shape_kind ~approx_cancel_neg env shape_kind1 shape_kind2 =
  union ~approx_cancel_neg env shape_kind1 shape_kind2

(* TODO: add a new reason with positions of merge point and possibly merged
 * envs.*)
and union_reason r1 r2 =
  if Reason.is_none r1 then
    r2
  else if Reason.is_none r2 then
    r1
  else if Reason.compare r1 r2 <= 0 then
    r1
  else
    r2

(** Takes a list of types, possibly including unions, and flattens it.
Returns a set of types.
If null is encountered (possibly in an option), get its reason and return it
in a separate optional reason (do not add null in the resulting set of types).
Do the same if dynamic is encountered.
This is because options and dynamic have to be "pushed out" of the resulting union. *)
let normalize_union env ?on_tyvar tyl :
    Typing_env_types.env
    * Reason.t option
    * Reason.t option
    * Reason.t option
    * TySet.t =
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
        | ((r, Tnewtype (n, [ty], _)), _)
          when String.equal n Naming_special_names.Classes.cSupportDyn ->
          let (_, env, ty) = Utils.strip_supportdyn env ty in
          let (env, ty) = Utils.simplify_intersections env ty ?on_tyvar in
          let (env, ty) = Utils.simple_make_supportdyn r env ty in
          proceed env ty
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

(* Is this type minimal with respect to the ground subtype ordering, if we ignore the nothing type?
 * Examples are exact or final non-generic classes.
 * Non-examples (surprisingly) are int or string (because an enum can be a subtype).
 *
 * Another example is classname<C>, if C is minimal, and this pattern occurs
 * frequently e.g. vec[C::class, D::class].
 *
 * Note that doesn't work for general covariant classes or built-ins e.g. vec<exact C> is not
 * minimal because vec<nothing> <: vec<exact C>. But classname<nothing> is uninhabited so it's
 * ok to suppose that it is equivalent to nothing.
 *
 * Finally note that if we did treat string, int, and float as minimal, then the code in
 * union_list_2_by_2 would not simplify int|float to num or string|classname<C> to string.
 *)
let rec is_minimal env ty =
  match get_node ty with
  | Tclass (_, Exact, []) -> true
  | Tclass ((_, name), _, []) -> begin
    match Env.get_class env name with
    | Some cd -> Cls.final cd
    | None -> false
  end
  | Tnewtype (name, [ty], _)
    when String.equal name Naming_special_names.Classes.cClassname ->
    is_minimal env ty
  | Tdependent _ -> true
  | _ -> false

(** Construct union of a list of types by applying binary union pairwise.
 * This uses a quadratic number of subtype tests so we apply the following
 * optimization that's particularly useful for the common pattern of vec[C1::class,...,Cn::class].
 *
 * First split the list of types into "final" types (those known to have no non-empty subtypes)
 * and non-final types (that might have a non-empty subtype).
 * For the non-final types, we apply binary union pointwise, which is quadratic in the number
 * of types. Then for each of the final types we drop those that subtype any of the types
 *  in the result.
 *
 * If m is the number of final types, and n is the number of non-final types, the first
 * stage uses O(n^2) subtype tests and the second stage uses O(mn) subtype tests.
 * One common pattern is a union of exact classnames: constructing this will involve no
 * subtype tests at all.
 *)
let union_list_2_by_2 ~approx_cancel_neg env r tyl =
  let (tyl_final, tyl_nonfinal) = List.partition_tf tyl ~f:(is_minimal env) in
  let (env, reduced_nonfinal) =
    List.fold tyl_nonfinal ~init:(env, []) ~f:(fun (env, res_tyl) ty ->
        let rec union_ty_w_tyl env ty tyl tyl_acc =
          match tyl with
          | [] -> (env, ty :: tyl_acc)
          | ty' :: tyl ->
            let (env, union_opt) =
              simplify_union ~approx_cancel_neg env ty ty' r
            in
            (match union_opt with
            | None -> union_ty_w_tyl env ty tyl (ty' :: tyl_acc)
            | Some union -> (env, tyl @ (union :: tyl_acc)))
        in
        union_ty_w_tyl env ty res_tyl [])
  in
  let reduced_final =
    List.filter tyl_final ~f:(fun fty ->
        not
          (List.exists
             reduced_nonfinal
             ~f:(Utils.is_sub_type_for_union env fty)))
  in
  (env, reduced_final @ reduced_nonfinal)

let union_list env ?(approx_cancel_neg = false) r tyl =
  Log.log_union_list r tyl
  @@
  let (env, r_null, _r_union, r_dyn, tys) = normalize_union env tyl in
  let (env, tyl) =
    union_list_2_by_2 ~approx_cancel_neg env r (TySet.elements tys)
  in
  make_union env r tyl r_null r_dyn

let fold_union env ?(approx_cancel_neg = false) r tyl =
  List.fold_left_env
    env
    tyl
    ~init:(MakeType.nothing r)
    ~f:(union ~approx_cancel_neg)

(* See documentation in mli file *)
let simplify_unions env ?(approx_cancel_neg = false) ?on_tyvar ty =
  let r = get_reason ty in
  Log.log_simplify_unions r ty
  @@
  let (env, r_null, r_union, r_dyn, tys) = normalize_union env [ty] ?on_tyvar in
  let (env, tyl) =
    union_list_2_by_2 ~approx_cancel_neg env r (TySet.elements tys)
  in
  let r = Option.value r_union ~default:r in
  make_union env r tyl r_null r_dyn

let rec union_i env ?(approx_cancel_neg = false) r ty1 lty2 =
  let ty2 = LoclType lty2 in
  if Utils.is_sub_type_for_union_i env ty1 ty2 then
    (env, ty2)
  else if Utils.is_sub_type_for_union_i env ty2 ty1 then
    (env, ty1)
  else
    let (env, ty) =
      match ty1 with
      | LoclType lty1 ->
        let (env, ty) = union ~approx_cancel_neg env lty1 lty2 in
        (env, LoclType ty)
      | ConstraintType cty1 ->
        (match deref_constraint_type cty1 with
        | (_, TCunion (lty1, cty1)) ->
          let (env, lty) = union ~approx_cancel_neg env lty1 lty2 in
          (env, ConstraintType (mk_constraint_type (r, TCunion (lty, cty1))))
        | (r', TCintersection (lty1, cty1)) ->
          (* Distribute union over intersection.
             At the moment local types in TCintersection can only be
             unions or intersections involving only null and nonnull,
             so applying distributivity allows for simplifying the types. *)
          let (env, lty) = union ~approx_cancel_neg env lty1 lty2 in
          let (env, ty) =
            union_i ~approx_cancel_neg env r (ConstraintType cty1) lty2
          in
          (match ty with
          | LoclType ty ->
            let (env, ty) = Typing_intersection.intersect env ~r:r' lty ty in
            (env, LoclType ty)
          | ConstraintType cty ->
            ( env,
              ConstraintType
                (mk_constraint_type (r', TCintersection (lty, cty))) ))
        | (_, Thas_member _)
        | (_, Thas_type_member _)
        | (_, Tcan_index _)
        | (_, Tcan_traverse _)
        | (_, Tdestructure _) ->
          (env, ConstraintType (mk_constraint_type (r, TCunion (lty2, cty1)))))
    in
    match ty with
    | LoclType _ -> (env, ty)
    | ConstraintType ty -> Utils.simplify_constraint_type env ty

let () = Utils.union_ref := union

let () = Utils.union_i_ref := union_i

let () = Utils.union_list_ref := union_list

let () = Utils.fold_union_ref := fold_union

let () = Utils.simplify_unions_ref := simplify_unions

let () = Utils.make_union_ref := make_union

let () =
  (* discard optional parameters. *)
  let simplify_unions env ty = simplify_unions env ty in
  Env.simplify_unions_ref := simplify_unions
