(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Common
open Utils
open Typing_defs
open Typing_kinding_defs
module Env = Typing_env
module Cls = Decl_provider.Class
module KindDefs = Typing_kinding_defs
module TGenConstraint = Typing_generic_constraint
module TUtils = Typing_utils
module Subst = Decl_subst

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
        let args = List.map args ~f:(instantiate subst) in
        (match SMap.find_opt x subst with
        | Some x_ty -> merge_hk_type r x x_ty args
        | None -> mk (r, Tgeneric (x, args)))
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
    | (Tvar _ | Tdynamic | Tnonnull | Tany _ | Tprim _ | Tneg _) as x -> x
    | Ttuple tyl ->
      let tyl = List.map tyl ~f:(instantiate subst) in
      Ttuple tyl
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
    | Tunapplied_alias _ -> failwith "this shouldn't be here"
    | Tdependent (dep, ty) ->
      let ty = instantiate subst ty in
      Tdependent (dep, ty)
    | Taccess (ty, ids) ->
      let ty = instantiate subst ty in
      Taccess (ty, ids)
end

(* TODO(T70068435)
   This is a workaround for the problem that alias and newtype definitions do not spell out
   the constraints they may implicitly impose on their parameters.
   Consider:
   class Foo<T1 as num> {...}
   type Bar<T2> = Foo<T2>;

   Here, T2 of Bar implicitly has the bound T2 as num. However, in the current design, we only
   ever check that when expanding Bar, the argument in place of T2 satisfies all the
   implicit bounds.
   However, this is not feasible for using aliases and newtypes as higher-kinded types, where we
   use them without expanding them.
   In the long-term, we would like to be able to infer the implicit bounds and use those for
   the purposes of kind-checking. For now, we just detect if there *are* implicit bounds, and
   if so reject using the alias/newtype as an HK type.
*)
let check_typedef_usable_as_hk_type env use_pos typedef_name typedef_info =
  let report_constraint violating_type used_class used_class_tparam_name =
    let tparams_in_ty = Env.get_tparams env violating_type in
    let tparams_of_typedef =
      List.fold typedef_info.td_tparams ~init:SSet.empty ~f:(fun s tparam ->
          SSet.add (snd tparam.tp_name) s)
    in
    let intersection = SSet.inter tparams_in_ty tparams_of_typedef in
    if SSet.is_empty intersection then
      (* Just violated constraints inside the typedef that do not involve
         the type parameters of the typedef we are looking at. Nothing to report at this point *)
      None
    else
      (* We choose an arbitrary element. If a constraint violation were to contain multiple
         tparams of the typedef, we can live with only showing the user one of them. *)
      let typedef_tparam_name = SSet.min_elt intersection in
      let (used_class_in_def_pos, used_class_in_def_name) = used_class in
      let typedef_pos = typedef_info.td_pos in
      Some
        Typing_error.(
          Reasons_callback.always
          @@ primary
          @@ Primary.HKT_alias_with_implicit_constraints
               {
                 pos = use_pos;
                 typedef_pos;
                 used_class_in_def_pos;
                 typedef_name;
                 typedef_tparam_name;
                 used_class_in_def_name;
                 used_class_tparam_name;
               })
  in
  let check_tapply r class_sid type_args =
    let decl_ty = Typing_make_type.apply r class_sid type_args in
    let ((env, ty_err_opt), locl_ty) =
      TUtils.localize_no_subst env ~ignore_errors:true decl_ty
    in
    Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
    match get_node (TUtils.get_base_type env locl_ty) with
    | Tclass (cls_name, _, tyl) when not (List.is_empty tyl) ->
      (match Env.get_class env (snd cls_name) with
      | Decl_entry.Found cls ->
        let tc_tparams = Cls.tparams cls in
        let ety_env =
          { empty_expand_env with substs = Subst.make_locl tc_tparams tyl }
        in
        iter2_shortest
          begin
            fun { tp_name = (_p, x); tp_constraints = cstrl; _ } ty ->
              List.iter cstrl ~f:(fun (ck, cstr_ty) ->
                  let ((env, ty_err1), cstr_ty) =
                    TUtils.localize ~ety_env env cstr_ty
                  in
                  Option.iter
                    ~f:(Typing_error_utils.add_typing_error ~env)
                    ty_err1;
                  let (_env, ty_err2) =
                    TGenConstraint.check_constraint env ck ty ~cstr_ty
                    @@ report_constraint ty cls_name x
                  in
                  Option.iter
                    ty_err2
                    ~f:(Typing_error_utils.add_typing_error ~env))
          end
          tc_tparams
          tyl
      | _ -> ())
    | _ -> ()
  in

  let visitor =
    object
      inherit [unit] Type_visitor.decl_type_visitor

      method! on_tapply _ r name args = check_tapply r name args
    end
  in
  visitor#on_type () typedef_info.td_type;
  maybe visitor#on_type () typedef_info.td_as_constraint;
  maybe visitor#on_type () typedef_info.td_super_constraint

(* TODO(T70068435)
   This is a workaround until we support proper kind-checking of HK types that impose constraints
   on their arguments.
   For now, we reject using any class as a HK type that has any constraints on its type parameters.
*)
let check_class_usable_as_hk_type pos class_info =
  let class_name = Cls.name class_info in
  let tparams = Cls.tparams class_info in
  let has_tparam_constraints =
    List.exists tparams ~f:(fun tp -> not (List.is_empty tp.tp_constraints))
  in
  if has_tparam_constraints then
    Errors.add_error
      Naming_error.(
        to_user_error @@ HKT_class_with_constraints_used { pos; class_name })

let report_kind_error env ~use_pos ~def_pos ~tparam_name ~expected ~actual =
  let actual_kind = Simple.description_of_kind actual in
  let expected_kind = Simple.description_of_kind expected in
  Typing_error_utils.add_typing_error ~env
  @@ Typing_error.(
       primary
       @@ Primary.Kind_mismatch
            {
              pos = use_pos;
              decl_pos = def_pos;
              tparam_name;
              actual_kind;
              expected_kind;
            })

module Simple = struct
  (* TODO(T70068435) Once we support constraints on higher-kinded types, this should only be used
     during the localization of declaration site types, everything else should be doing full
     kind-checking (including constraints) *)

  let is_subkind _env ~(sub : Simple.kind) ~(sup : Simple.kind) =
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

  let rec check_targs_well_kinded
      ~allow_missing_targs
      ~in_signature
      ~def_pos
      ~use_pos
      env
      (tyargs : decl_ty list)
      (nkinds : Simple.named_kind list) =
    let exp_len = List.length nkinds in
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
    let (tyargs, nkinds) = (List.take tyargs length, List.take nkinds length) in
    List.iter2_exn tyargs nkinds ~f:(check_targ_well_kinded ~in_signature env)

  and check_targ_well_kinded ~in_signature env tyarg (nkind : Simple.named_kind)
      =
    let kind = snd nkind in
    match get_node tyarg with
    | Twildcard ->
      let is_higher_kinded = Simple.get_arity kind > 0 in
      if is_higher_kinded then (
        let pos =
          get_reason tyarg |> Reason.to_pos |> Pos_or_decl.unsafe_to_raw_pos
        in
        Errors.add_error Naming_error.(to_user_error @@ HKT_wildcard pos);
        check_well_kinded ~in_signature env tyarg nkind
      )
    | _ -> check_well_kinded ~in_signature env tyarg nkind

  and check_well_kinded_type
      ~allow_missing_targs ~in_signature env (ty : decl_ty) =
    let (r, ty_) = deref ty in
    let use_pos = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos in
    let check =
      check_well_kinded_type ~allow_missing_targs:false ~in_signature env
    in
    let check_against_tparams def_pos tyargs tparams =
      let kinds = Simple.named_kinds_of_decl_tparams tparams in

      check_targs_well_kinded
        ~allow_missing_targs
        ~def_pos
        ~use_pos
        env
        tyargs
        kinds
    in
    match ty_ with
    | Tany _
    | Tnonnull
    | Tprim _
    | Tdynamic
    | Tmixed
    | Twildcard
    | Tthis ->
      ()
    | Tvec_or_dict (tk, tv) ->
      check tk;
      check tv
    | Tlike ty
    | Toption ty ->
      check ty
    | Ttuple tyl
    | Tunion tyl
    | Tintersection tyl ->
      List.iter tyl ~f:check
    | Taccess (ty, _) ->
      (* Because type constants cannot depend on type parameters,
         we allow Foo::the_type even if Foo has type parameters *)
      check_well_kinded_type ~allow_missing_targs:true ~in_signature env ty
    | Trefinement (ty, rs) ->
      check ty;
      Class_refinement.iter check rs
    | Tshape { s_fields = map; _ } ->
      TShapeMap.iter (fun _ sft -> check sft.sft_ty) map
    | Tfun ft ->
      check ft.ft_ret;
      List.iter ft.ft_params ~f:(fun p -> check p.fp_type)
    (* FIXME shall we inspect tparams and where_constraints *)
    (* List.iter ft.ft_where_constraints (fun (ty1, _, ty2) -> check ty1; check ty2 ); *)
    | Tgeneric (name, targs) -> begin
      match Env.get_pos_and_kind_of_generic env name with
      | Some (def_pos, gen_kind) ->
        let param_nkinds =
          Simple.from_full_kind gen_kind |> Simple.get_named_parameter_kinds
        in
        check_targs_well_kinded
          ~allow_missing_targs:false
          ~in_signature
          ~def_pos
          ~use_pos
          env
          targs
          param_nkinds
      | None -> ()
    end
    | Tapply ((_p, cid), argl) -> begin
      match Env.get_class_or_typedef env cid with
      | Decl_entry.Found (Env.ClassResult class_info) ->
        Option.iter
          ~f:(Typing_error_utils.add_typing_error ~env)
          (Typing_visibility.check_top_level_access
             ~in_signature
             ~use_pos
             ~def_pos:(Cls.pos class_info)
             env
             (Cls.internal class_info)
             (Cls.get_module class_info));
        let tparams = Cls.tparams class_info in
        check_against_tparams ~in_signature (Cls.pos class_info) argl tparams
      | Decl_entry.Found (Env.TypedefResult typedef) ->
        Option.iter
          ~f:(Typing_error_utils.add_typing_error ~env)
          (Typing_visibility.check_top_level_access
             ~in_signature
             ~use_pos
             ~def_pos:typedef.td_pos
             env
             typedef.td_internal
             (Option.map typedef.td_module ~f:snd));
        check_against_tparams
          ~in_signature
          typedef.td_pos
          argl
          typedef.td_tparams
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ()
    end
    | Tnewtype (name, tyl, _) ->
      (match Env.get_typedef env name with
      | Decl_entry.Found typedef ->
        Option.iter
          ~f:(Typing_error_utils.add_typing_error ~env)
          (Typing_visibility.check_top_level_access
             ~in_signature
             ~use_pos
             ~def_pos:typedef.td_pos
             env
             typedef.td_internal
             (Option.map typedef.td_module ~f:snd));
        check_against_tparams
          ~in_signature
          typedef.td_pos
          tyl
          typedef.td_tparams
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        ())

  and check_well_kinded
      ~in_signature env (ty : decl_ty) (expected_nkind : Simple.named_kind) =
    let (expected_name, expected_kind) = expected_nkind in
    let r = get_reason ty in
    let use_pos = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos in
    let kind_error actual_kind env =
      let (def_pos, tparam_name) = expected_name in
      report_kind_error
        env
        ~use_pos
        ~def_pos
        ~tparam_name
        ~actual:actual_kind
        ~expected:expected_kind
    in
    let check_against_tparams tparams =
      let overall_kind = Simple.type_with_params_to_simple_kind tparams in
      if not (is_subkind env ~sub:overall_kind ~sup:expected_kind) then
        kind_error overall_kind env
    in

    if Int.( = ) (Simple.get_arity expected_kind) 0 then
      check_well_kinded_type ~allow_missing_targs:false ~in_signature env ty
    else
      match get_node ty with
      | Tapply ((_pos, name), []) -> begin
        match Env.get_class_or_typedef env name with
        | Decl_entry.Found (Env.ClassResult class_info) ->
          let tparams = Cls.tparams class_info in
          check_class_usable_as_hk_type use_pos class_info;
          check_against_tparams tparams
        | Decl_entry.Found (Env.TypedefResult typedef) ->
          let tparams = typedef.td_tparams in
          check_typedef_usable_as_hk_type env use_pos name typedef;
          check_against_tparams tparams
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          ()
      end
      | Tgeneric (name, []) -> begin
        match Env.get_pos_and_kind_of_generic env name with
        | Some (_pos, gen_kind) ->
          let get_kind = Simple.from_full_kind gen_kind in
          if not (is_subkind env ~sub:get_kind ~sup:expected_kind) then
            kind_error get_kind env
        | None -> ()
      end
      | Tgeneric (_, targs)
      | Tapply (_, targs) ->
        Errors.add_error
          Naming_error.(
            to_user_error
            @@ HKT_partial_application
                 {
                   pos = Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos;
                   count = List.length targs;
                 })
      | Tany _ -> ()
      | _ -> kind_error (Simple.fully_applied_type ()) env

  (* Export the version that doesn't expose allow_missing_targs *)
  let check_well_kinded_type ~in_signature env (ty : decl_ty) =
    check_well_kinded_type ~allow_missing_targs:false ~in_signature env ty

  let check_well_kinded_hint ~in_signature env hint =
    let decl_ty = Decl_hint.hint env.Typing_env_types.decl_env hint in
    check_well_kinded_type ~in_signature env decl_ty

  let check_well_kinded_context_hint ~in_signature env hint =
    let decl_ty = Decl_hint.context_hint env.Typing_env_types.decl_env hint in
    check_well_kinded_type ~in_signature env decl_ty
end
