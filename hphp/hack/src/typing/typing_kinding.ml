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

(* TODO(T70068435)
  This is a workaround for the problem that alias and newtype definitions do not spell out
  the constraints they may implicitly impose on their parameters.
  Consider:
  class Foo<T1 as num> {...}
  type Bar<T2> = Foo<T2>;

  Here, T2 of Bar implicitly has the bound T2 as num. However, in the current design, we only
  ever check that when expaning Bar, the argument in place of T2 satisfies all the
  implicit bounds.
  However, this is not feasible for using aliases and newtypes as higher-kinded types, where we
  use them without expanding them.
  In the long-term, we would like to be able to infer the implicit bounds and use those for
  the purposes of kind-checking. For now, we just detect if there *are* implicit bounds, and
  if so reject using the alias/newtype as an HK type.
  *)
let check_typedef_usable_as_hk_type env use_pos typedef_name typedef_info =
  let report_constraint violating_type used_class used_class_tparam_name =
    let tparams_in_ty = Typing_env.get_tparams env violating_type in
    let tparams_of_typedef =
      List.fold typedef_info.td_tparams ~init:SSet.empty ~f:(fun s tparam ->
          SSet.add (snd tparam.tp_name) s)
    in
    let intersection = SSet.inter tparams_in_ty tparams_of_typedef in
    if SSet.is_empty intersection then
      (* Just violated constraints inside the typedef that do not involve
      the type parameters of the typedef we are looking at. Nothing to report at this point *)
      ()
    else
      (* We choose an arbitrary element. If a constraint violation were to contain multiple
      tparams of the typedef, we can live with only showing the user one of them. *)
      let typedef_tparam_name = SSet.min_elt intersection in
      let (used_class_in_def_pos, used_class_in_def_name) = used_class in
      let typedef_pos = typedef_info.td_pos in
      Errors.alias_with_implicit_constraints_as_hk_type
        ~use_pos
        ~typedef_pos
        ~used_class_in_def_pos
        ~typedef_name
        ~typedef_tparam_name
        ~used_class_in_def_name
        ~used_class_tparam_name
  in
  let check_tapply r class_sid type_args =
    let decl_ty = Typing_make_type.apply r class_sid type_args in
    let (env, locl_ty) = TUtils.localize_with_self env decl_ty in
    match get_node (TUtils.get_base_type env locl_ty) with
    | Tclass (cls_name, _, tyl) ->
      (match Env.get_class env (snd cls_name) with
      | Some cls ->
        let tc_tparams = Cls.tparams cls in
        let ety_env =
          {
            (TUtils.env_with_self env) with
            substs = Subst.make_locl tc_tparams tyl;
          }
        in
        iter2_shortest
          begin
            fun { tp_name = (_p, x); tp_constraints = cstrl; _ } ty ->
            List.iter cstrl (fun (ck, cstr_ty) ->
                let (env, cstr_ty) = TUtils.localize ~ety_env env cstr_ty in
                let (_ : Typing_env_types.env) =
                  TGenConstraint.check_constraint
                    env
                    ck
                    ty
                    ~cstr_ty
                    (fun ?code:_ _l -> report_constraint ty cls_name x)
                in
                ())
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
  maybe visitor#on_type () typedef_info.td_constraint

(* TODO(T70068435)
  This is a workaround until we support proper kind-checking of HK types that impose constraints
  on their arguments.
  For now, we reject using any class as a HK type that has any constraints on its type parameters.
  *)
let check_class_usable_as_hk_type use_pos class_info =
  let name = Cls.name class_info in
  let tparams = Cls.tparams class_info in
  let has_tparam_constraints =
    List.exists tparams (fun tp -> not (List.is_empty tp.tp_constraints))
  in
  if has_tparam_constraints then
    Errors.class_with_constraints_used_as_hk_type use_pos name

let report_kind_error ~use_pos ~def_pos ~tparam_name ~expected ~actual =
  let actual_kind_repr = Simple.description_of_kind actual in
  let expected_kind_repr = Simple.description_of_kind expected in
  Errors.kind_mismatch
    ~use_pos
    ~def_pos
    ~tparam_name
    ~expected_kind_repr
    ~actual_kind_repr

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
      ~def_pos
      ~use_pos
      env
      (tyargs : decl_ty list)
      (nkinds : Simple.named_kind list) =
    let exp_len = List.length nkinds in
    let act_len = List.length tyargs in
    let arity_mistmach_okay =
      Int.equal act_len 0
      && ( allow_missing_targs
         || (* 4101 is Error_codes.Typing.TypeArityMismatch error code *)
         (not (Partial_provider.should_check_error (Env.get_mode env) 4101))
         && not
              (TypecheckerOptions.experimental_feature_enabled
                 (Env.get_tcopt env)
                 TypecheckerOptions.experimental_generics_arity) )
    in
    if Int.( <> ) exp_len act_len && not arity_mistmach_okay then
      Errors.type_arity use_pos def_pos ~expected:exp_len ~actual:act_len;
    let length = min exp_len act_len in
    let (tyargs, nkinds) = (List.take tyargs length, List.take nkinds length) in
    List.iter2_exn tyargs nkinds ~f:(check_targ_well_kinded env)

  and check_targ_well_kinded env tyarg (nkind : Simple.named_kind) =
    let kind = snd nkind in
    match get_node tyarg with
    | Tapply ((_, x), _argl) when String.equal x SN.Typehints.wildcard ->
      let is_higher_kinded = Simple.get_arity kind > 0 in
      if is_higher_kinded then (
        let pos = get_reason tyarg |> Reason.to_pos in
        Errors.wildcard_for_higher_kinded_type pos;
        check_well_kinded env tyarg nkind
      )
    | _ -> check_well_kinded env tyarg nkind

  and check_possibly_enforced_ty env enf_ty =
    check_well_kinded_type ~allow_missing_targs:false env enf_ty.et_type

  and check_well_kinded_type ~allow_missing_targs env (ty : decl_ty) =
    let check_opt =
      Option.value_map
        ~default:()
        ~f:(check_well_kinded_type ~allow_missing_targs:false env)
    in
    let (r, ty_) = deref ty in
    let use_pos = Reason.to_pos r in
    let check = check_well_kinded_type ~allow_missing_targs:false env in
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
    | Terr
    | Tany _
    | Tvar _
    (* Tvar must not be higher-kinded yet *)
    | Tnonnull
    | Tprim _
    | Tdynamic
    | Tmixed
    | Tthis ->
      ()
    | Tarray (ty1, ty2) ->
      check_opt ty1;
      check_opt ty2
    | Tdarray (tk, tv) ->
      check tk;
      check tv
    | Tvarray tv -> check tv
    | Tvarray_or_darray (tk, tv) ->
      check tk;
      check tv
    | Tpu_access (ty, _)
    | Tlike ty
    | Toption ty ->
      check ty
    | Ttuple tyl
    | Tunion tyl
    | Tintersection tyl ->
      List.iter tyl check
    | Taccess (ty, _) ->
      (* Because type constants cannot depend on type parameters,
       we allow Foo::the_type even if Foo has type parameters *)
      check_well_kinded_type ~allow_missing_targs:true env ty
    | Tshape (_, map) -> Nast.ShapeMap.iter (fun _ sft -> check sft.sft_ty) map
    | Tfun ft ->
      check_possibly_enforced_ty env ft.ft_ret;
      List.iter ft.ft_params (fun p -> check_possibly_enforced_ty env p.fp_type)
    (* FIXME shall we inspect tparams and where_constraints *)
    (* List.iter ft.ft_where_constraints (fun (ty1, _, ty2) -> check ty1; check ty2 ); *)
    | Tgeneric (name, targs) ->
      begin
        match Env.get_pos_and_kind_of_generic env name with
        | Some (def_pos, gen_kind) ->
          let param_nkinds =
            Simple.from_full_kind gen_kind |> Simple.get_named_parameter_kinds
          in
          check_targs_well_kinded
            ~allow_missing_targs:false
            ~def_pos
            ~use_pos
            env
            targs
            param_nkinds
        | None -> ()
      end
    | Tapply ((_p, cid), argl) ->
      begin
        match Env.get_class env cid with
        | Some class_info ->
          let tparams = Cls.tparams class_info in
          check_against_tparams (Cls.pos class_info) argl tparams
        | None ->
          begin
            match Env.get_typedef env cid with
            | Some typedef ->
              check_against_tparams typedef.td_pos argl typedef.td_tparams
            | None -> ()
          end
      end

  and check_well_kinded env (ty : decl_ty) (expected_nkind : Simple.named_kind)
      =
    let (expected_name, expected_kind) = expected_nkind in
    let r = get_reason ty in
    let use_pos = Reason.to_pos r in
    let kind_error actual_kind =
      let (def_pos, tparam_name) = expected_name in

      report_kind_error
        ~use_pos
        ~def_pos
        ~tparam_name
        ~actual:actual_kind
        ~expected:expected_kind
    in
    let check_against_tparams tparams =
      let overall_kind = Simple.type_with_params_to_simple_kind tparams in
      if not (is_subkind env ~sub:overall_kind ~sup:expected_kind) then
        kind_error overall_kind
    in

    if Int.( = ) (Simple.get_arity expected_kind) 0 then
      check_well_kinded_type ~allow_missing_targs:false env ty
    else
      match get_node ty with
      | Tapply ((_pos, name), []) ->
        begin
          match Env.get_class env name with
          | Some class_info ->
            let tparams = Cls.tparams class_info in
            check_class_usable_as_hk_type use_pos class_info;
            check_against_tparams tparams
          | None ->
            begin
              match Env.get_typedef env name with
              | Some typedef ->
                let tparams = typedef.td_tparams in
                check_typedef_usable_as_hk_type env use_pos name typedef;
                check_against_tparams tparams
              | None -> ()
            end
        end
      | Tgeneric (name, []) ->
        begin
          match Env.get_pos_and_kind_of_generic env name with
          | Some (_pos, gen_kind) ->
            let get_kind = Simple.from_full_kind gen_kind in
            if not (is_subkind env ~sub:get_kind ~sup:expected_kind) then
              kind_error get_kind
          | None -> ()
        end
      | Tgeneric (_, targs)
      | Tapply (_, targs) ->
        Errors.higher_kinded_partial_application
          (Reason.to_pos r)
          (List.length targs)
      | Terr
      | Tany _ ->
        ()
      | _ -> kind_error (Simple.fully_applied_type ())

  (* Export the version that doesn't expose allow_missing_targs *)
  let check_well_kinded_type env (ty : decl_ty) =
    check_well_kinded_type ~allow_missing_targs:false env ty

  let check_well_kinded_hint env hint =
    let decl_ty = Decl_hint.hint env.Typing_env_types.decl_env hint in
    check_well_kinded_type env decl_ty
end
