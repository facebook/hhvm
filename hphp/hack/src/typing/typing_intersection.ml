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
module MkType = Typing_make_type
module Reason = Typing_reason
module TySet = Typing_set
module Utils = Typing_utils

exception Nothing

module Log = struct
  let should_log env ~level =
    Typing_log.should_log env ~category:"intersection" ~level

  let log_intersection env r ty1 ty2 =
    Typing_log.log_function
      (Reason.to_pos r)
      ~function_name:"Typing_intersection.intersect"
      ~arguments:
        [
          ("ty1", Typing_print.debug env ty1);
          ("ty2", Typing_print.debug env ty2);
        ]
      ~result:(fun (env, ty) ->
        let (env, ty) = Typing_env.expand_type env ty in
        Some (Typing_print.debug env ty))
end

module TrackedOperation = struct
  type t = {
    ty1: locl_ty;
    ty2: locl_ty;
  }
  [@@deriving eq]
end

module Recursion_tracker : sig
  type t

  val empty : t

  val check_infinite_recursion :
    t -> locl_ty -> locl_ty -> (t, TrackedOperation.t) result
end = struct
  include Recursion_tracker.Make (TrackedOperation)

  let is_type_alias (ty : locl_ty) : bool =
    match get_node ty with
    | Tnewtype _ -> true
    | _ -> false

  let check_infinite_recursion t (ty1 : locl_ty) ty2 =
    if is_type_alias ty1 || is_type_alias ty2 then
      (* There can be infinite recursion with recursive type aliases, so we track them *)
      add_op_and_check_infinite_recursion t { TrackedOperation.ty1; ty2 }
    else
      Ok t
end

(** Computes the negation of a type when it is known, which is currently the case
for null, nonnull, mixed, nothing, primitives, and classes.
Otherwise approximate up or down according to
`approx` parameter: If approx is `ApproxUp`, return mixed, else if it is `ApproxDown`,
return nothing. *)
let negate_type env r ty ~approx =
  let (env, ty) = Env.expand_type env ty in
  let approximated =
    if Utils.equal_approx approx Utils.ApproxUp then
      MkType.mixed r
    else
      MkType.nothing r
  in
  match get_node ty with
  | Tprim Aast.Tnull -> (env, MkType.nonnull r)
  | Tprim _ -> begin
    match Result.ok @@ Typing_refinement.TyPredicate.of_ty env ty with
    | Some predicate -> (env, MkType.neg r predicate)
    | None -> (env, approximated)
    (* void, noreturn *)
  end
  | Tneg (r, IsTag (ClassTag (c, []))) when Utils.class_has_no_params env c ->
    (env, MkType.class_type r c [])
  | Tneg predicate ->
    ( env,
      Typing_refinement.TyPredicate.to_ty_without_instantiation_opt predicate
      |> Option.value ~default:approximated )
  | Tnonnull -> (env, MkType.null r)
  | Tclass ((_, c), Nonexact _, args) ->
    let tparams =
      match Env.get_class env c with
      | Decl_entry.Found cls -> Folded_class.tparams cls
      | _ -> []
    in
    let is_fresh_generic ty =
      match get_node ty with
      | Tgeneric name -> Env.is_fresh_generic_parameter name
      | _ -> false
    in
    let rec is_all_filled_reified tparams args =
      match (tparams, args) with
      | ([], []) -> true
      | (tparam :: tparams, arg :: args)
        when (not (Aast.is_erased tparam.tp_reified))
             && not (is_fresh_generic arg) ->
        is_all_filled_reified tparams args
      (* too few args counts as unfilled, too many args is malformed, but treat that like the tparam is erased *)
      | _ -> false
    in
    ( env,
      MkType.neg
        r
        ( r,
          IsTag
            (ClassTag
               ( c,
                 if is_all_filled_reified tparams args then
                   List.map args ~f:(fun ty -> Filled ty)
                 else
                   [] )) ) )
  | _ -> (env, approximated)

(** Decompose nullable types into unions with null

      decompose_nullable ?A = null | A
      decompose_nullable ?(A | ?B) = null | A | B

  The implementation has the side-effect of flattening unions if the type is
  nullable, e.g.,

    decompose_optional ?(A | (B | C)) = null | A | B | C
  *)
let decompose_nullable ty =
  let rec has_option (ty : 'a ty) : bool =
    match deref ty with
    | (_, Toption _) -> true
    | (_, Tunion tyl) -> List.exists ~f:has_option tyl
    | _ -> false
  in
  let rec peel (ty_acc : 'a ty list) (ty : 'a ty) : 'a ty list =
    match deref ty with
    | (_, Toption ty) -> peel ty_acc ty
    | (_, Tunion tyl) -> List.fold ~init:ty_acc ~f:peel tyl
    | _ -> ty :: ty_acc
  in
  if has_option ty then
    let r = get_reason ty in
    let null_ty = MkType.null r in
    let tyl = peel [null_ty] ty in
    let tyl = TySet.elements (TySet.of_list tyl) in
    MkType.union r tyl
  else
    ty

(** Build a union from a list of types. Flatten if any of the types themselves are union.
    Also, pull nullable out to a top-level Toption. Undoes decompose atomic.
*)
let recompose_atomic env r tyl =
  let rec traverse nullable dynamic tyl_acc tyl =
    match tyl with
    | [] -> (nullable, dynamic, List.rev tyl_acc)
    | ty :: tyl ->
      if Utils.is_nothing env ty then
        traverse nullable dynamic tyl_acc tyl
      else (
        match deref ty with
        | (r, Toption ty) -> traverse (Some r) dynamic (ty :: tyl_acc) tyl
        | (r, Tprim Aast.Tnull) -> traverse (Some r) dynamic tyl_acc tyl
        | (r, Tdynamic) -> traverse nullable (Some r) tyl_acc tyl
        | (_, Tunion tyl') -> traverse nullable dynamic tyl_acc (tyl' @ tyl)
        | _ -> traverse nullable dynamic (ty :: tyl_acc) tyl
      )
  in
  let (nullable_r, dynamic_r, tyl) = traverse None None [] tyl in
  Utils.make_union env r tyl nullable_r dynamic_r

(* Destructure an intersection into a list of its sub-types,
   decending into sub-intersections.
*)
let destruct_inter_list env tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec dest_inter env ty tyl tyl_res r_inter =
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (r, Tintersection tyl') ->
      destruct_inter_list env (tyl' @ tyl) tyl_res (orr r_inter r)
    | _ -> destruct_inter_list env tyl (ty :: tyl_res) r_inter
  and destruct_inter_list env tyl tyl_res r_union =
    match tyl with
    | [] -> (env, (tyl_res, r_union))
    | ty :: tyl -> dest_inter env ty tyl tyl_res r_union
  in
  destruct_inter_list env tyl [] None

(** Number of '&' symbols in an intersection representation. E.g. for (A & B),
returns 1, for A, returns 0. *)
let number_of_inter_symbols env ty =
  let rec n_inter env tyl n =
    match tyl with
    | [] -> (env, n)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      begin
        match get_node ty with
        | Tintersection tyl' ->
          n_inter env (tyl' @ tyl) (List.length tyl' - 1 + n)
        | Tunion tyl' -> n_inter env (tyl' @ tyl) n
        | Toption ty -> n_inter env (ty :: tyl) n
        | _ -> n_inter env tyl n
      end
  in
  n_inter env [ty] 0

let collapses env ty1 ty2 ~inter_ty =
  let (env, n_inter_inter) = number_of_inter_symbols env inter_ty in
  let (env, n_inter1) = number_of_inter_symbols env ty1 in
  let (env, n_inter2) = number_of_inter_symbols env ty2 in
  (env, n_inter_inter <= n_inter1 + n_inter2)

let make_intersection env r tyl =
  let ty = MkType.intersection r tyl in
  let (env, ty) = Utils.wrap_union_inter_ty_in_var env r ty in
  (env, ty)

let rec intersect env rec_tracker ~r ty1 ty2 =
  if Log.should_log env ~level:2 then
    Log.log_intersection env r ty1 ty2 @@ fun () ->
    intersect_ env rec_tracker ~r ty1 ty2
  else
    intersect_ env rec_tracker ~r ty1 ty2

(** Computes the intersection (greatest lower bound) of two types.
For the intersection of unions, attempt to simplify by using the distributivity
of intersection over union. Uses the the `collapses` test function to make sure
the resulting type is no greater in size than the trivial `Tintersection [ty1; ty2]`. *)
and intersect_ env (rec_tracker : Recursion_tracker.t) ~r ty1 ty2 =
  if ty_equal ty1 ty2 then
    (env, ty1)
  else
    let (env, ty1) = Env.expand_type env ty1 in
    let (env, ty2) = Env.expand_type env ty2 in
    if Utils.is_sub_type_for_union env ty1 ty2 then
      (env, ty1)
    else if Utils.is_sub_type_for_union env ty2 ty1 then
      (env, ty2)
    else if Utils.is_type_disjoint env ty1 ty2 then
      (env, MkType.nothing r)
    else
      match Recursion_tracker.check_infinite_recursion rec_tracker ty1 ty2 with
      | Error { TrackedOperation.ty1 = _; ty2 = _ } ->
        (env, MkType.intersection r [ty1; ty2])
      | Ok rec_tracker ->
        (* Attempt to simplify any case types involved in the intersection.
           If simplification occurs we re-run [intersect] with the simplified type
           until no more simplications can be performed.

           Note: Re-running [intersect] when neither [ty1] or [ty2] has been
           simplified will lead to infinite recursion.
        *)
        let (env, simplified_ty1_opt) =
          try_simplifying_case_type
            env
            rec_tracker
            ~case_type:ty1
            ~intersect_ty:ty2
        in
        let (env, simplified_ty2_opt) =
          try_simplifying_case_type
            env
            rec_tracker
            ~case_type:ty2
            ~intersect_ty:ty1
        in
        (match (simplified_ty1_opt, simplified_ty2_opt) with
        | (Some simplified_ty1, None) ->
          intersect env rec_tracker ~r simplified_ty1 ty2
        | (None, Some simplified_ty2) ->
          intersect env rec_tracker ~r ty1 simplified_ty2
        | (Some simplified_ty1, Some simplified_ty2) ->
          intersect env rec_tracker ~r simplified_ty1 simplified_ty2
        | (None, None) ->
          let ty1 = decompose_nullable ty1 in
          let ty2 = decompose_nullable ty2 in
          let (env, inter_ty) =
            try
              match (deref ty1, deref ty2) with
              (* TODO: optional and variadic fields T201398626 T201398652 *)
              | ( ( _,
                    Ttuple
                      {
                        t_required = t_required1;
                        t_extra =
                          Textra { t_optional = []; t_variadic = t_variadic1 };
                      } ),
                  ( _,
                    Ttuple
                      {
                        t_required = t_required2;
                        t_extra =
                          Textra { t_optional = []; t_variadic = t_variadic2 };
                      } ) )
                when Int.equal
                       (List.length t_required1)
                       (List.length t_required2) ->
                let (env, t_required) =
                  List.map2_env env t_required1 t_required2 ~f:(function
                      | env -> intersect ~r env rec_tracker)
                in
                let (env, t_variadic) =
                  intersect ~r env rec_tracker t_variadic1 t_variadic2
                in
                ( env,
                  mk
                    ( r,
                      Ttuple
                        {
                          t_required;
                          t_extra = Textra { t_optional = []; t_variadic };
                        } ) )
              (* Runtime representation of tuples is vec, which can be observed in Hack via refinement.
               * Therefore it's sound to simplify vec<t> & (t1,...,tn) to (t & t1, ..., t & tn)
               * but because we don't support subtyping directly between tuples and vecs, we need
               * to keep the vec conjunct i.e. simplify to vec<t> & (t & t1, ..., t & tn)
               *)
              | ( ((_, Tclass ((_, cn), _, [ty])) as vty),
                  ( rt,
                    Ttuple
                      {
                        t_required;
                        t_extra = Textra { t_optional; t_variadic };
                      } ) )
              | ( ( rt,
                    Ttuple
                      {
                        t_required;
                        t_extra = Textra { t_optional; t_variadic };
                      } ),
                  ((_, Tclass ((_, cn), _, [ty])) as vty) )
                when String.equal cn Naming_special_names.Collections.cVec ->
                let (env, t_required) =
                  List.map_env env t_required ~f:(fun env ty' ->
                      intersect ~r env rec_tracker ty ty')
                in
                let (env, t_optional) =
                  List.map_env env t_optional ~f:(fun env ty' ->
                      intersect ~r env rec_tracker ty ty')
                in
                let (env, t_variadic) =
                  intersect ~r env rec_tracker ty t_variadic
                in
                make_intersection
                  env
                  r
                  [
                    mk vty;
                    mk
                      ( rt,
                        Ttuple
                          {
                            t_required;
                            t_extra = Textra { t_variadic; t_optional };
                          } );
                  ]
              | ( ( _,
                    Tshape
                      {
                        s_origin = _;
                        s_unknown_value = shape_kind1;
                        s_fields = fdm1;
                      } ),
                  ( _,
                    Tshape
                      {
                        s_origin = _;
                        s_unknown_value = shape_kind2;
                        s_fields = fdm2;
                      } ) ) ->
                let (env, shape_kind, fdm) =
                  intersect_shapes
                    env
                    rec_tracker
                    r
                    (shape_kind1, fdm1)
                    (shape_kind2, fdm2)
                in
                ( env,
                  mk
                    ( r,
                      Tshape
                        {
                          s_origin = Missing_origin;
                          s_unknown_value = shape_kind;
                          s_fields = fdm;
                        } ) )
              | ((_, Tclass_ptr ty_c1), (_, Tclass_ptr ty_c2)) ->
                let (env, ty) = intersect ~r env rec_tracker ty_c1 ty_c2 in
                (env, mk (r, Tclass_ptr ty))
              | ((_, Tintersection tyl1), (_, Tintersection tyl2)) ->
                intersect_lists env rec_tracker r tyl1 tyl2
              (* Simplify `supportdyn<t> & u` to `supportdyn<t & u>`. Do not apply if `u` is
               * a type variable, else we end up with recursion in constraints. *)
              | ((r, Tnewtype (name1, [ty1arg], _)), _)
                when String.equal name1 Naming_special_names.Classes.cSupportDyn
                     && not (is_tyvar ty2) ->
                let (env, ty) = intersect ~r env rec_tracker ty1arg ty2 in
                let (env, res) = Utils.simple_make_supportdyn r env ty in
                (env, res)
              | (_, (r, Tnewtype (name1, [ty2arg], _)))
                when String.equal name1 Naming_special_names.Classes.cSupportDyn
                     && not (is_tyvar ty1) ->
                let (env, ty) = intersect ~r env rec_tracker ty1 ty2arg in
                let (env, res) = Utils.simple_make_supportdyn r env ty in
                (env, res)
              (* If class<T> <: classname<T>, class<U> & classname<V> -> classname<U & V> *)
              | ((_, Tnewtype (cn, [ty_cn], _)), (_, Tclass_ptr ty_c))
              | ((_, Tclass_ptr ty_c), (_, Tnewtype (cn, [ty_cn], _)))
                when TypecheckerOptions.class_sub_classname (Env.get_tcopt env)
                     && String.equal cn Naming_special_names.Classes.cClassname
                ->
                let (env, ty) = intersect ~r env rec_tracker ty_c ty_cn in
                (env, MkType.classname r [ty])
              | ( (_, Tnewtype (cn1, [ty_cn1], _)),
                  (_, Tnewtype (cn2, [ty_cn2], _)) )
                when String.equal cn1 Naming_special_names.Classes.cClassname
                     && String.equal cn2 Naming_special_names.Classes.cClassname
                ->
                let (env, ty) = intersect ~r env rec_tracker ty_cn1 ty_cn2 in
                (env, MkType.classname r [ty])
              | ((_, Tintersection tyl), _) ->
                intersect_lists env rec_tracker r [ty2] tyl
              | (_, (_, Tintersection tyl)) ->
                intersect_lists env rec_tracker r [ty1] tyl
              | ((r1, Tunion tyl1), (r2, Tunion tyl2)) ->
                let (common_tyl, tyl1', tyl2') =
                  Typing_algebra.factorize_common_types tyl1 tyl2
                in
                let (env, not_common_tyl) =
                  intersect_unions env rec_tracker r (r1, tyl1') (r2, tyl2')
                in
                recompose_atomic env r (common_tyl @ not_common_tyl)
              | ((r_union, Tunion tyl), ty)
              | (ty, (r_union, Tunion tyl)) ->
                let (env, inter_tyl) =
                  intersect_ty_union env rec_tracker r (mk ty) (r_union, tyl)
                in
                recompose_atomic env r inter_tyl
              | ((_, Tprim Aast.Tnum), (_, Tprim Aast.Tarraykey))
              | ((_, Tprim Aast.Tarraykey), (_, Tprim Aast.Tnum)) ->
                (env, MkType.int r)
              | ((neg_reason, Tneg predicate), ty)
              | (ty, (neg_reason, Tneg predicate)) ->
                let (env, partition) =
                  Typing_refinement.partition_ty env (mk ty) predicate
                in
                if List.is_empty partition.Typing_refinement.span then
                  if List.is_empty partition.Typing_refinement.left then
                    (* This is logically the same result as the else case handling
                       would produce but may be simpler *)
                    (env, mk ty)
                  else
                    let (env, intersections) =
                      List.fold_left
                        partition.Typing_refinement.right
                        ~init:(env, [])
                        ~f:(fun (env, acc) tys ->
                          (* recursion here should be safe since, while the type
                             may not be structurally smaller, it should be
                             logically smaller and we shouldn't repeat !predicate
                          *)
                          let (env, intersection) =
                            intersect_list env rec_tracker neg_reason tys
                          in
                          (env, intersection :: acc))
                    in
                    let (env, right_ty) =
                      Utils.simplify_unions env
                      @@ MkType.union neg_reason intersections
                    in
                    (env, right_ty)
                else
                  make_intersection env r [ty1; ty2]
              | _ -> make_intersection env r [ty1; ty2]
            with
            | Nothing -> (env, MkType.nothing r)
          in
          (env, inter_ty))

(** Attempts to simplify an intersection involving a case type and another type. If [case_type]
 is a case type, we fetch the list of variant types, then filter this list down to only the types
 that intersect with the data types associated with [ty]. If the resulting filtered type is
 not a union type, we consider it to be a simplified version of the case type and return it.
 Otherwise simplification fails and [None] is returned.

 As an example consider:
   [case_type] = int | vec<int> | string
   [ty] = not int & not string

  This will result in simplifying the case type to `vec<int>`

  If instead we had:
    [ty] = not int

  This would fail to simplify because the result filtered type would be `vec<int> | string`

  Finally if we had:
    [ty] = not vec

  This would succeed, because the union `int | string` would simplify to `arraykey`
  *)
and try_simplifying_case_type
    env rec_tracker ~(case_type : locl_ty) ~(intersect_ty : locl_ty) :
    Typing_env_types.env * locl_ty option =
  match deref case_type with
  | (r, Tnewtype (name, ty_args, _)) ->
    let (env, variants_opt) =
      Typing_case_types.get_variant_tys env name ty_args
    in
    begin
      match variants_opt with
      | Some variants ->
        let (env, filtered_ty) =
          Typing_case_types.filter_variants_using_datatype
            ~safe_for_are_disjoint:false
            env
            r
            variants
            intersect_ty
        in
        if Typing_defs.is_union filtered_ty then
          (env, None)
        else
          (env, Some filtered_ty)
      | None -> (env, None)
    end
  | (r, Tintersection tyl) ->
    (* For each type in [tyl] try to simplify all case types that are present, filtering based on
       the intersection of all other types in the intersection.

         [acc] will contain the rebuilt list with any case types that could be simplified replaced with their simplified form
         [changed] will be `true` if a case type in the list was simplified, otherwise it will be `false*)
    let rec simplify_all_case_types env tyl (acc, changed) =
      match tyl with
      | ty :: tyl ->
        let (env, simplified_ty_opt) =
          try_simplifying_case_type
            env
            rec_tracker
            ~case_type:ty
            ~intersect_ty:(MkType.intersection r ((intersect_ty :: tyl) @ acc))
        in
        let result =
          match simplified_ty_opt with
          | Some simplified_ty -> (simplified_ty :: acc, true)
          | None -> (ty :: acc, changed)
        in
        simplify_all_case_types env tyl result
      | [] -> (env, acc, changed)
    in
    let (env, simplified_tyl, changed) =
      simplify_all_case_types env tyl ([], false)
    in
    if changed then
      let (env, simplified_ty) =
        intersect_list env rec_tracker r simplified_tyl
      in
      (env, Some simplified_ty)
    else
      (env, None)
  | _ -> (env, None)

and intersect_shapes env rec_tracker r (shape_kind1, fdm1) (shape_kind2, fdm2) =
  let (env, fdm) =
    TShapeMap.merge_env env fdm1 fdm2 ~combine:(fun env _sfn sft1 sft2 ->
        match
          ((is_nothing shape_kind1, sft1), (is_nothing shape_kind2, sft2))
        with
        | ((_, None), (_, None))
        | ((_, Some { sft_optional = true; _ }), (true, None))
        | ((true, None), (_, Some { sft_optional = true; _ })) ->
          (env, None)
        | ((_, Some { sft_optional = false; _ }), (true, None))
        | ((true, None), (_, Some { sft_optional = false; _ })) ->
          raise Nothing
        | ((_, Some sft), (_, None)) ->
          let (env, ty) = intersect env rec_tracker ~r shape_kind2 sft.sft_ty in
          (env, Some { sft with sft_ty = ty })
        | ((_, None), (_, Some sft)) ->
          let (env, ty) = intersect env rec_tracker ~r shape_kind1 sft.sft_ty in
          (env, Some { sft with sft_ty = ty })
        | ( (_, Some { sft_optional = opt1; sft_ty = ty1 }),
            (_, Some { sft_optional = opt2; sft_ty = ty2 }) ) ->
          let opt = opt1 && opt2 in
          let (env, ty) = intersect env rec_tracker ~r ty1 ty2 in
          (env, Some { sft_optional = opt; sft_ty = ty }))
  in
  let (env, shape_kind) =
    intersect env rec_tracker ~r shape_kind1 shape_kind2
  in
  (env, shape_kind, fdm)

and intersect_lists env rec_tracker r tyl1 tyl2 =
  let rec intersect_lists env tyl1 tyl2 acc_tyl =
    match (tyl1, tyl2) with
    | ([], _) -> (env, tyl2 @ acc_tyl)
    | (_, []) -> (env, tyl1 @ acc_tyl)
    | (ty1 :: tyl1', _) ->
      let (env, (inter_ty, missed_inter_tyl2)) =
        intersect_ty_tyl env rec_tracker r ty1 tyl2
      in
      intersect_lists env tyl1' missed_inter_tyl2 (inter_ty :: acc_tyl)
  in
  let (env, tyl) = intersect_lists env tyl1 tyl2 [] in
  make_intersection env r tyl

and intersect_ty_tyl env rec_tracker r ty tyl =
  (* try negs last because we expect them to be more likely to fail try_intersect *)
  let (negs, others) = List.partition_tf tyl ~f:is_neg in
  let tyl = others @ negs in
  let rec intersect_ty_tyl env ty tyl missed_inter_tyl =
    match tyl with
    | [] -> (env, (ty, missed_inter_tyl))
    | ty' :: tyl' ->
      let (env, ty_opt) = try_intersect env rec_tracker r ty ty' in
      begin
        match ty_opt with
        | None -> intersect_ty_tyl env ty tyl' (ty' :: missed_inter_tyl)
        | Some inter_ty -> intersect_ty_tyl env inter_ty tyl' missed_inter_tyl
      end
  in
  intersect_ty_tyl env ty tyl []

and try_intersect env rec_tracker r ty1 ty2 =
  let (env, ty) = intersect env rec_tracker ~r ty1 ty2 in
  let (env, ty) = Env.expand_type env ty in
  match get_node ty with
  | Tintersection _ -> (env, None)
  | _ -> (env, Some ty)

and intersect_unions env rec_tracker r (r1, tyl1) (r2, tyl2) =
  (* The order matters. (A | B | C) & (A | B) gets simplified to (A | B)
     while (A | B) & (A | B | C) would become A | B | ((A | B) & C), so we
     put the longest union first as a heuristic. *)
  let ((r1, tyl1), (r2, tyl2)) =
    if List.length tyl1 >= List.length tyl2 then
      ((r1, tyl1), (r2, tyl2))
    else
      ((r2, tyl2), (r1, tyl1))
  in
  let union_ty1 = MkType.union r1 tyl1 in
  intersect_ty_union env rec_tracker r union_ty1 (r2, tyl2)

(** For (A1 | .. | An) & B, compute each of the A1 & B, .. , An & B.
Keep those which collapse (see `collapses` function) with B
and leave the others unchanged.
So if I is the set of indices i such that Ai collapses with B,
and J the set of indices such that this is not the case,
the result would be
  (|_{i in I} (Ai & B)) | (B & (|_{j in J} Aj))
*)
and intersect_ty_union env rec_tracker r (ty1 : locl_ty) (r_union, tyl2) =
  let (env, inter_tyl) =
    List.map_env env tyl2 ~f:(fun env ty2 ->
        intersect env rec_tracker ~r ty1 ty2)
  in
  let zipped = List.zip_exn tyl2 inter_tyl in
  let (collapsed, not_collapsed) =
    List.partition_tf zipped ~f:(fun (ty2, inter_ty) ->
        snd @@ collapses env ty1 ty2 ~inter_ty)
  in
  let collapsed = List.map collapsed ~f:snd in
  let not_collapsed =
    match not_collapsed with
    | [] -> []
    | [(_ty2, inter_ty)] -> [inter_ty]
    | _ ->
      let not_collapsed = List.map not_collapsed ~f:fst in
      [MkType.intersection r [ty1; MkType.union r_union not_collapsed]]
  in
  (env, not_collapsed @ collapsed)

and intersect_list env rec_tracker r tyl =
  (* We need to match tyl here because we'd mess the reason if tyl was just
     [mixed] *)
  match tyl with
  | [] -> (env, MkType.mixed r)
  | ty :: tyl ->
    List.fold_left_env env tyl ~init:ty ~f:(fun env ->
        intersect env rec_tracker ~r)

let normalize_intersection env ?on_tyvar tyl =
  let orr r_opt r = Some (Option.value r_opt ~default:r) in
  let rec normalize_intersection env r tyl tys_acc =
    match tyl with
    | [] -> (env, r, tys_acc)
    | ty :: tyl ->
      let (env, ty) = Env.expand_type env ty in
      let proceed env ty =
        normalize_intersection env r tyl (TySet.add ty tys_acc)
      in
      begin
        match (deref ty, on_tyvar) with
        | ((r', Tvar v), Some on_tyvar) ->
          let (env, ty') = on_tyvar env r' v in
          if ty_equal ty ty' then
            proceed env ty
          else
            normalize_intersection env r (ty' :: tyl) tys_acc
        | ((r', Tintersection tyl'), _) ->
          normalize_intersection env (orr r r') (tyl' @ tyl) tys_acc
        | ((_, Tunion _), _) ->
          let (env, ty) = Utils.simplify_unions env ty ?on_tyvar in
          let (env, ety) = Env.expand_type env ty in
          begin
            match get_node ety with
            | Tunion _ -> proceed env ty
            | _ -> normalize_intersection env r (ty :: tyl) tys_acc
          end
        | _ -> proceed env ty
      end
  in
  normalize_intersection env None tyl TySet.empty

let simplify_intersections env ?on_tyvar ty =
  let r = get_reason ty in
  let (env, r', tys) = normalize_intersection env [ty] ?on_tyvar in
  let r = Option.value r' ~default:r in
  intersect_list env Recursion_tracker.empty r (TySet.elements tys)

let intersect env ~r ty1 ty2 =
  let rec_tracker = Recursion_tracker.empty in
  if Log.should_log env ~level:1 then
    Log.log_intersection env r ty1 ty2 @@ fun () ->
    intersect env rec_tracker ~r ty1 ty2
  else
    intersect env rec_tracker ~r ty1 ty2

let intersect_list env = intersect_list env Recursion_tracker.empty

let intersect_with_nonnull env pos ty =
  let r = Reason.witness_from_decl pos in
  intersect env ~r ty (Typing_make_type.nonnull r)

let () = Utils.negate_type_ref := negate_type

let () = Utils.simplify_intersections_ref := simplify_intersections

let () = Utils.intersect_list_ref := intersect_list
