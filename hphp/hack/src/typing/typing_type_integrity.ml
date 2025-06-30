(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
open Typing_defs
module Env = Typing_env
module Cls = Folded_class
module KindDefs = Typing_kinding_defs

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
        | Tgeneric n ->
          (* Same here *)
          Tgeneric n
        | _ ->
          (* We could insist on args = [] here, everything else is a kinding error *)
          ty_
      in
      mk (Reason.instantiate (r, orig_var, orig_r), res_ty_)
    in

    (* PERF: If subst is empty then instantiation is a no-op. We can save a
     * significant amount of CPU by avoiding recursively deconstructing the ty
     * data type.
     *)
    if SMap.is_empty subst then
      ty
    else
      match deref ty with
      | (r, Tgeneric x) ->
        let args = [] in
        (match SMap.find_opt x subst with
        | Some x_ty -> merge_hk_type r x x_ty args
        | None -> mk (r, Tgeneric x))
      | (r, ty) ->
        let ty = instantiate_ subst ty in
        mk (r, ty)

  and instantiate_ subst x =
    match x with
    | Tgeneric _ -> assert false
    | Tvec_or_dict (ty1, ty2) ->
      let ty1 = instantiate subst ty1 in
      let ty2 = instantiate subst ty2 in
      Tvec_or_dict (ty1, ty2)
    | (Tvar _ | Tdynamic | Tnonnull | Tany _ | Tprim _ | Tneg _ | Tlabel _) as x
      ->
      x
    | Ttuple { t_required; t_extra } ->
      let t_extra = instantiate_tuple_extra subst t_extra in
      let t_required = List.map t_required ~f:(instantiate subst) in
      Ttuple { t_required; t_extra }
    | Tunion tyl ->
      let tyl = List.map tyl ~f:(instantiate subst) in
      Tunion tyl
    | Tintersection tyl ->
      let tyl = List.map tyl ~f:(instantiate subst) in
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
              (fun subst t -> SMap.remove (snd t.tp_name) subst)
            end
          ~init:subst
          tparams
      in
      let params =
        List.map ft.ft_params ~f:(fun param ->
            let ty = instantiate subst param.fp_type in
            { param with fp_type = ty })
      in
      let ret = instantiate subst ft.ft_ret in
      let tparams =
        List.map tparams ~f:(fun t ->
            {
              t with
              tp_constraints =
                List.map t.tp_constraints ~f:(fun (ck, ty) ->
                    (ck, instantiate subst ty));
            })
      in
      let where_constraints =
        List.map ft.ft_where_constraints ~f:(fun (ty1, ck, ty2) ->
            (instantiate subst ty1, ck, instantiate outer_subst ty2))
      in
      Tfun
        {
          ft with
          ft_params = params;
          ft_ret = ret;
          ft_tparams = tparams;
          ft_where_constraints = where_constraints;
        }
    | Tnewtype (x, tyl, bound) ->
      let tyl = List.map tyl ~f:(instantiate subst) in
      let bound = instantiate subst bound in
      Tnewtype (x, tyl, bound)
    | Tclass (x, exact, tyl) ->
      let tyl = List.map tyl ~f:(instantiate subst) in
      Tclass (x, exact, tyl)
    | Tshape { s_origin = _; s_unknown_value = kind; s_fields = fdm } ->
      let fdm = ShapeFieldMap.map (instantiate subst) fdm in
      Tshape
        {
          s_origin = Missing_origin;
          s_unknown_value = kind;
          (* TODO(shapes) instantiate s_unknown_value *)
          s_fields = fdm;
        }
    | Tdependent (dep, ty) ->
      let ty = instantiate subst ty in
      Tdependent (dep, ty)
    | Taccess (ty, ids) ->
      let ty = instantiate subst ty in
      Taccess (ty, ids)
    | Tclass_ptr ty ->
      let ty = instantiate subst ty in
      Tclass_ptr ty

  and instantiate_tuple_extra subst e =
    match e with
    | Textra { t_optional; t_variadic } ->
      let t_optional = List.map t_optional ~f:(instantiate subst) in
      let t_variadic = instantiate subst t_variadic in
      Textra { t_optional; t_variadic }
    | Tsplat t_splat ->
      let t_splat = instantiate subst t_splat in
      Tsplat t_splat
end

(** Check for arity mismatch between type params and type arguments
  then check that each type argument is well-formed. *)
let rec check_targs_integrity
    ~allow_missing_targs
    ~in_signature
    ~def_pos
    ~use_pos
    env
    (tyargs : decl_ty list)
    (tparams : decl_tparam list) =
  let exp_len = List.length tparams in
  let act_len = List.length tyargs in
  let arity_mistmatch_okay = Int.equal act_len 0 && allow_missing_targs in
  if Int.( <> ) exp_len act_len && not arity_mistmatch_okay then
    Typing_error_utils.add_typing_error
      ~env
      Typing_error.(
        primary
        @@ Primary.Type_arity_mismatch
             {
               pos = use_pos;
               decl_pos = def_pos;
               expected = exp_len;
               actual = act_len;
             });
  let length = min exp_len act_len in
  let (tyargs, tparams) = (List.take tyargs length, List.take tparams length) in
  List.iter2_exn tyargs tparams ~f:(check_targ_well_formed ~in_signature env)

(** Checks that a type argument is well-kinded against its corresponding
  type parameter. Also checks that the wildcard type argument is not used
  if the type parameter is higher kinded (i.e. it expected type arguments itself) *)
and check_targ_well_formed ~in_signature env tyarg (tparam : decl_tparam) =
  let in_reified = not @@ Aast.is_erased tparam.tp_reified in
  let should_check_package_boundary =
    if in_reified && not (Env.package_v2_allow_reified_generics_violations env)
    then
      `Yes "reified generic"
    else if not (Env.package_v2_allow_all_generics_violations env) then
      `Yes "generic"
    else
      `No
  in
  check_type_integrity
    ~allow_missing_targs:false
    ~in_signature
    ~should_check_package_boundary
    env
    tyarg

and check_type_integrity
    ~allow_missing_targs
    ~in_signature
    ~should_check_package_boundary
    env
    (ty : decl_ty) =
  let (r, ty_) = deref ty in
  let use_pos = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos in
  let check ?(should_check_package_boundary = should_check_package_boundary) ty
      =
    check_type_integrity
      ~allow_missing_targs:false
      ~in_signature
      ~should_check_package_boundary
      env
      ty
  in
  let check_targs_integrity def_pos tyargs tparams =
    check_targs_integrity
      ~allow_missing_targs
      ~def_pos
      ~use_pos
      env
      tyargs
      tparams
  in
  match ty_ with
  (* Boring recursive cases first---------------------- *)
  | Tany _
  | Tnonnull
  | Tprim _
  | Tdynamic
  | Tmixed
  | Twildcard
  | Tthis ->
    ()
  | Tvec_or_dict (tk, tv) ->
    check ~should_check_package_boundary:`No tk;
    check ~should_check_package_boundary:`No tv
  | Tlike ty
  | Toption ty ->
    check ty
  | Ttuple { t_required; t_extra = Textra { t_optional; t_variadic } } ->
    List.iter t_required ~f:check;
    List.iter t_optional ~f:check;
    check t_variadic
  | Ttuple { t_required; t_extra = Tsplat t_splat } ->
    List.iter t_required ~f:check;
    check t_splat
  | Tunion tyl
  | Tintersection tyl ->
    List.iter tyl ~f:check
  | Taccess (ty, _) ->
    (* Because type constants cannot depend on type parameters,
       we allow Foo::the_type even if Foo has type parameters *)
    check_type_integrity
      ~allow_missing_targs:true
      ~in_signature
      ~should_check_package_boundary
      env
      ty
  | Trefinement (ty, rs) ->
    check ty;
    Class_refinement.iter check rs
  | Tshape { s_fields = map; _ } ->
    TShapeMap.iter
      (fun _ sft -> check ~should_check_package_boundary:`No sft.sft_ty)
      map
  | Tfun ({ ft_params; ft_ret; _ } : _ fun_type) ->
    (* FIXME shall we inspect tparams and where_constraints? *)
    check ~should_check_package_boundary:`No ft_ret;
    List.iter ft_params ~f:(fun p ->
        check ~should_check_package_boundary:`No p.fp_type)
  (* Interesting cases--------------------------------- *)
  | Tgeneric _ -> ()
  | Tapply ((_p, cid), argl) -> begin
    match Env.get_class_or_typedef env cid with
    | Decl_entry.Found (Env.ClassResult class_info) ->
      Typing_visibility.check_top_level_access
        ~should_check_package_boundary
        ~in_signature
        ~use_pos
        ~def_pos:(Cls.pos class_info)
        env
        (Cls.internal class_info)
        (Cls.get_module class_info)
        (Cls.get_package class_info)
        cid
      |> List.iter ~f:(Typing_error_utils.add_typing_error ~env);
      let tparams = Cls.tparams class_info in
      check_targs_integrity ~in_signature (Cls.pos class_info) argl tparams
    | Decl_entry.Found (Env.TypedefResult typedef) ->
      Typing_visibility.check_top_level_access
        ~should_check_package_boundary
        ~in_signature
        ~use_pos
        ~def_pos:typedef.td_pos
        env
        typedef.td_internal
        (Option.map typedef.td_module ~f:snd)
        typedef.td_package
        cid
      |> List.iter ~f:(Typing_error_utils.add_typing_error ~env);
      check_targs_integrity ~in_signature typedef.td_pos argl typedef.td_tparams
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      ()
  end
  | Tclass_ptr ty -> check ty

let check_type_integrity
    ~in_signature ~should_check_package_boundary env (ty : decl_ty) =
  check_type_integrity
    ~allow_missing_targs:false
    ~in_signature
    ~should_check_package_boundary
    env
    ty

let check_hint_integrity ~in_signature ~should_check_package_boundary env hint =
  let decl_ty = Decl_hint.hint env.Typing_env_types.decl_env hint in
  check_type_integrity ~in_signature ~should_check_package_boundary env decl_ty

let check_context_hint_integrity ~in_signature env hint =
  let decl_ty = Decl_hint.context_hint env.Typing_env_types.decl_env hint in
  check_type_integrity
    ~in_signature
    ~should_check_package_boundary:`No
    env
    decl_ty
