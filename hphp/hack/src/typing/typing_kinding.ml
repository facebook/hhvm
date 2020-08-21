open Hh_prelude
open Common
open Typing_defs
open Typing_kinding_defs
module Env = Typing_env

module Locl_Inst = struct
  let rec instantiate subst (ty : locl_ty) =
    let merge_hk_type orig_r orig_var ty args =
      let (r, ty_) = deref ty in
      let res_ty_ =
        match ty_ with
        | Tclass (n, exact, existing_args) ->
          (* We could insist on existing_args = [] here unless we want to support partial application. *)
          Tclass (n, exact, existing_args @ args)
        | Tnewtype (n, existing_args, bound) ->
          Tnewtype (n, existing_args @ args, bound)
        | Tunapplied_alias _ -> failwith "not implemented"
        | Tgeneric (n, existing_args) ->
          (* Same here *)
          Tgeneric (n, existing_args @ args)
        | _ ->
          (* We could insist on args = [] here, everything else is a kinding error *)
          ty_
      in
      mk (Reason.Rinstantiate (r, orig_var, orig_r), res_ty_)
    in

    (* PERF: If subst is empty then instantiation is a no-op. We can save a
     * significant amount of CPU by avoiding recursively deconstructing the ty
     * data type.
     *)
    if SMap.is_empty subst then
      ty
    else
      match deref ty with
      | (r, Tgeneric (x, args)) ->
        let args = List.map args (instantiate subst) in
        (match SMap.find_opt x subst with
        | Some x_ty -> merge_hk_type r x x_ty args
        | None -> mk (r, Tgeneric (x, args)))
      | (r, ty) ->
        let ty = instantiate_ subst ty in
        mk (r, ty)

  and instantiate_ subst x =
    match x with
    | Tgeneric _ -> assert false
    | Tdarray (ty1, ty2) ->
      Tdarray (instantiate subst ty1, instantiate subst ty2)
    | Tvarray ty -> Tvarray (instantiate subst ty)
    | Tvarray_or_darray (ty1, ty2) ->
      let ty1 = instantiate subst ty1 in
      let ty2 = instantiate subst ty2 in
      Tvarray_or_darray (ty1, ty2)
    | ( Tobject | Tvar _ | Tdynamic | Tnonnull | Tany _ | Terr | Tprim _
      | Tpu_type_access _ ) as x ->
      x
    | Ttuple tyl ->
      let tyl = List.map tyl (instantiate subst) in
      Ttuple tyl
    | Tunion tyl ->
      let tyl = List.map tyl (instantiate subst) in
      Tunion tyl
    | Tintersection tyl ->
      let tyl = List.map tyl (instantiate subst) in
      Tintersection tyl
    | Toption ty ->
      let ty = instantiate subst ty in
      (* we want to avoid double option: ??T *)
      (match get_node ty with
      | Toption _ as ty_node -> ty_node
      | _ -> Toption ty)
    | Tfun ft ->
      let tparams = ft.ft_tparams in
      let outer_subst = subst in
      let subst =
        List.fold_left
          ~f:
            begin
              fun subst t ->
              SMap.remove (snd t.tp_name) subst
            end
          ~init:subst
          tparams
      in
      let params =
        List.map ft.ft_params (fun param ->
            let ty = instantiate_possibly_enforced_ty subst param.fp_type in
            { param with fp_type = ty })
      in
      let arity =
        match ft.ft_arity with
        | Fvariadic ({ fp_type = var_ty; _ } as param) ->
          let var_ty = instantiate_possibly_enforced_ty subst var_ty in
          Fvariadic { param with fp_type = var_ty }
        | Fstandard as x -> x
      in
      let ret = instantiate_possibly_enforced_ty subst ft.ft_ret in
      let tparams =
        List.map tparams (fun t ->
            {
              t with
              tp_constraints =
                List.map t.tp_constraints (fun (ck, ty) ->
                    (ck, instantiate subst ty));
            })
      in
      let where_constraints =
        List.map ft.ft_where_constraints (fun (ty1, ck, ty2) ->
            (instantiate subst ty1, ck, instantiate outer_subst ty2))
      in
      Tfun
        {
          ft with
          ft_arity = arity;
          ft_params = params;
          ft_ret = ret;
          ft_tparams = tparams;
          ft_where_constraints = where_constraints;
        }
    | Tnewtype (x, tyl, bound) ->
      let tyl = List.map tyl (instantiate subst) in
      let bound = instantiate subst bound in
      Tnewtype (x, tyl, bound)
    | Tclass (x, exact, tyl) ->
      let tyl = List.map tyl (instantiate subst) in
      Tclass (x, exact, tyl)
    | Tshape (shape_kind, fdm) ->
      let fdm = ShapeFieldMap.map (instantiate subst) fdm in
      Tshape (shape_kind, fdm)
    | Tunapplied_alias _ -> failwith "this shouldn't be here"
    | Tdependent (dep, ty) ->
      let ty = instantiate subst ty in
      Tdependent (dep, ty)
    | Tpu (ty, sid) ->
      let ty = instantiate subst ty in
      Tpu (ty, sid)

  and instantiate_possibly_enforced_ty subst et =
    { et_type = instantiate subst et.et_type; et_enforced = et.et_enforced }
end

let is_subkind_simple _env ~(sub : Simple.kind) ~(sup : Simple.kind) =
  let rec is_subk subk superk =
    let param_compare =
      List.fold2
        (Simple.get_named_parameter_kinds subk)
        (Simple.get_named_parameter_kinds superk)
        ~init:true
        ~f:(fun ok (_, param_sub) (_, param_sup) ->
          (* Treating parameters contravariantly here. For simple subkinding, it doesn't make
             a difference, though *)
          ok && is_subk param_sup param_sub)
    in
    let open List.Or_unequal_lengths in
    match param_compare with
    | Unequal_lengths -> false
    | Ok r -> r
  in
  is_subk sub sup
