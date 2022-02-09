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
open Tast
open Typing_defs
open Utils
module TUtils = Typing_utils
module Reason = Typing_reason
module Env = Typing_env
module Union = Typing_union
module Inter = Typing_intersection
module SN = Naming_special_names
module TVis = Typing_visibility
module Phase = Typing_phase
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

(** Common arguments to internal `obj_get_...` functions *)
type obj_get_args = {
  inst_meth: bool;
  meth_caller: bool;
  is_method: bool;
  is_nonnull: bool;
  nullsafe: Typing_reason.t option;
  obj_pos: pos;
  coerce_from_ty:
    (MakeType.Nast.pos * Reason.ureason * Typing_defs.locl_ty) option;
  explicit_targs: Nast.targ list;
  this_ty: locl_ty;
  this_ty_conjunct: locl_ty;
  is_parent_call: bool;
  dep_kind: Reason.t * Typing_dependent_type.ExprDepTy.dep;
}

let log_obj_get env helper id_pos ty this_ty =
  let (fn_name, ty_name) =
    match helper with
    | `concrete -> ("obj_get_concrete_ty", "concrete_ty")
    | `inner -> ("obj_get_inner", "receiver_ty")
  in
  Typing_log.(
    log_with_level env "obj_get" ~level:2 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos id_pos)
          env
          [
            Log_head
              (fn_name, [Log_type (ty_name, ty); Log_type ("this_ty", this_ty)]);
          ]))

let mk_ety_env class_info paraml this_ty =
  {
    empty_expand_env with
    this_ty;
    substs = TUtils.make_locl_subst_for_class_tparams class_info paraml;
  }

let mk_intersection_err env errs_res =
  Result.fold
    errs_res
    ~ok:(fun tys ->
      let (env, ty) = Typing_intersection.intersect_list env Reason.none tys in
      (env, Ok ty))
    ~error:(fun (actuals, expecteds) ->
      let (env, ty_actual) =
        Typing_intersection.intersect_list env Reason.none actuals
      in
      let (env, ty_expect) =
        Typing_intersection.intersect_list env Reason.none expecteds
      in
      (env, Error (ty_actual, ty_expect)))

let mk_union_err env =
  Result.fold
    ~ok:(fun tys ->
      let (env, ty) = Typing_union.union_list env Reason.none tys in
      (env, Ok ty))
    ~error:(fun (actuals, expecteds) ->
      let (env, ty_acutal) = Typing_union.union_list env Reason.none actuals in
      let (env, ty_expect) =
        Typing_union.union_list env Reason.none expecteds
      in
      (env, Error (ty_acutal, ty_expect)))

let fold_errs errs =
  List.fold_left errs ~init:(Ok []) ~f:(fun acc err ->
      match (acc, err) with
      | (Ok xs, Ok x) -> Ok (x :: xs)
      | (Ok xs, Error (x, y)) -> Error (x :: xs, y :: xs)
      | (Error (xs, ys), Ok x) -> Error (x :: xs, x :: ys)
      | (Error (xs, ys), Error (x, y)) -> Error (x :: xs, y :: ys))

let fold_opt_errs opt_errs = Option.(map ~f:fold_errs @@ all opt_errs)

let err_witness env p = TUtils.terr env (Reason.Rwitness p)

let smember_not_found
    pos ~is_const ~is_method ~is_function_pointer class_ member_name on_error =
  let kind =
    if is_const then
      `class_constant
    else if is_method then
      `static_method
    else
      `class_variable
  in
  let error hint =
    let (class_pos, class_name) = (Cls.pos class_, Cls.name class_) in
    Typing_error.(
      apply ~on_error
      @@ primary
      @@ Primary.Smember_not_found
           { pos; kind; class_name; class_pos; member_name; hint })
  in
  let static_suggestion =
    Env.suggest_static_member is_method class_ member_name
  in
  let method_suggestion = Env.suggest_member is_method class_ member_name in
  match (static_suggestion, method_suggestion) with
  (* If there is a normal method of the same name and the
   * syntax is a function pointer, suggest meth_caller *)
  | (_, Some (_, v)) when is_function_pointer && String.equal v member_name ->
    Typing_error.(
      primary
      @@ Primary.Consider_meth_caller
           { pos; class_name = Cls.name class_; meth_name = member_name })
  (* If there is a normal method of the same name, suggest it *)
  | (Some _, Some (def_pos, v)) when String.equal v member_name ->
    error (Some (`instance, def_pos, v))
  (* Otherwise suggest a different static method *)
  | (Some (def_pos, v), _) -> error (Some (`static, def_pos, v))
  (* Fallback to closest normal method *)
  | (None, Some (def_pos, v)) -> error (Some (`instance, def_pos, v))
  | (None, None) -> error None

let member_not_found
    (env : Typing_env_types.env) pos ~is_method class_ member_name r on_error =
  let cls_name = strip_ns (Cls.name class_) in
  if
    env.Typing_env_types.in_expr_tree
    && is_method
    && String_utils.string_starts_with member_name "__"
  then
    Typing_error.(
      expr_tree
      @@ Primary.Expr_tree.Expression_tree_unsupported_operator
           { class_name = cls_name; member_name; pos })
  else
    let (class_pos, class_name) = (Cls.pos class_, Cls.name class_) in
    let reason =
      Reason.to_string
        ("This is why I think it is an object of type " ^ cls_name)
        r
    in
    let method_suggestion = Env.suggest_member is_method class_ member_name in
    let static_suggestion =
      Env.suggest_static_member is_method class_ member_name
    in
    let hint =
      lazy
        (match (method_suggestion, static_suggestion) with
        (* Prefer suggesting a different method, unless there's a
           static method whose name matches exactly. *)
        | (Some _, Some (def_pos, v)) when String.equal v member_name ->
          Some (`static, def_pos, v)
        | (Some (def_pos, v), _) -> Some (`instance, def_pos, v)
        | (None, Some (def_pos, v)) -> Some (`static, def_pos, v)
        | (None, None) -> None)
    in
    Typing_error.(
      apply ~on_error
      @@ primary
      @@ Primary.Member_not_found
           {
             pos;
             kind =
               (if is_method then
                 `method_
               else
                 `property);
             class_name;
             class_pos;
             member_name;
             hint;
             reason;
           })

let sound_dynamic_err_opt args env ((_, id_str) as id) read_context =
  if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
    (* Any access to a *private* member through dynamic might potentially
     * be unsound, if the receiver is an instance of a class that implements dynamic,
     * as we do no checks on enforceability or subtype-dynamic at the definition site
     * of private members.
     *)
    match Env.get_self_class env with
    | Some self_class
      when Cls.get_support_dynamic_type self_class || not (Cls.final self_class)
      ->
      (match Env.get_member args.is_method env self_class id_str with
      | Some { ce_visibility = Vprivate _; ce_type = (lazy ty); _ }
        when not args.is_method ->
        if read_context then
          let (env, locl_ty) =
            Phase.localize_no_subst ~ignore_errors:true env ty
          in
          Typing_dynamic.check_property_sound_for_dynamic_read
            ~on_error:(fun pos prop_name class_name (prop_pos, prop_type) ->
              Typing_error.(
                primary
                @@ Primary.Private_property_is_not_dynamic
                     { pos; prop_name; class_name; prop_pos; prop_type }))
            env
            (Cls.name self_class)
            id
            locl_ty
        else
          Typing_dynamic.check_property_sound_for_dynamic_write
            ~on_error:(fun pos prop_name class_name (prop_pos, prop_type) ->
              Typing_error.(
                primary
                @@ Primary.Private_property_is_not_enforceable
                     { pos; prop_name; class_name; prop_pos; prop_type }))
            env
            (Cls.name self_class)
            id
            ty
            None
      | _ -> None)
    | _ -> None
  else
    None

let widen_class_for_obj_get ~is_method ~nullsafe member_name env ty =
  match deref ty with
  | (_, Tprim Tnull) ->
    if Option.is_some nullsafe then
      (env, Some ty)
    else
      (env, None)
  | (r2, Tclass (((_, class_name) as class_id), _, tyl)) ->
    let default () =
      let ty = mk (r2, Tclass (class_id, Nonexact, tyl)) in
      (env, Some ty)
    in
    begin
      match Env.get_class env class_name with
      | None -> default ()
      | Some class_info ->
        (match Env.get_member is_method env class_info member_name with
        | Some { ce_origin; _ } ->
          (* If this member was inherited then we obtain the type from which
           * it is inherited as our wider type *)
          if String.equal ce_origin class_name then
            default ()
          else (
            match Cls.get_ancestor class_info ce_origin with
            | None -> default ()
            | Some basety ->
              let ety_env =
                {
                  empty_expand_env with
                  substs =
                    TUtils.make_locl_subst_for_class_tparams class_info tyl;
                  this_ty = ty;
                }
              in
              let (env, basety) = Phase.localize ~ety_env env basety in
              (env, Some basety)
          )
        | None -> (env, None))
    end
  | _ -> (env, None)

(* `ty` is expected to be the type for a property or method that has been
 * accessed using the nullsafe operatore e.g. $x?->prop or $x?->foo(...).
 *
 * For properties, just make the type nullable.
 * For methods, we expect a function type, and make the return type nullable.
 * But in the case that we have type nothing, we just use the type `null`.
 * The `call` helper will deal appropriately with it.
 * Similarly if we have dynamic, or err, or any, we leave as dynamic/err/any respectively.
 *)
let rec make_nullable_member_type env ~is_method id_pos pos ty =
  if is_method then
    let (env, ty) = Env.expand_type env ty in
    match deref ty with
    | (r, Tfun tf) ->
      let (env, ty) =
        make_nullable_member_type
          ~is_method:false
          env
          id_pos
          pos
          tf.ft_ret.et_type
      in
      (env, mk (r, Tfun { tf with ft_ret = { tf.ft_ret with et_type = ty } }))
    | (r, Tunion (_ :: _ as tyl)) ->
      let (env, tyl) =
        List.map_env env tyl ~f:(fun env ty ->
            make_nullable_member_type ~is_method env id_pos pos ty)
      in
      Union.union_list env r tyl
    | (r, Tintersection tyl) ->
      let (env, tyl) =
        List.map_env env tyl ~f:(fun env ty ->
            make_nullable_member_type ~is_method env id_pos pos ty)
      in
      Inter.intersect_list env r tyl
    | (_, (Terr | Tdynamic | Tany _)) -> (env, ty)
    | (_, Tunion []) -> (env, MakeType.null (Reason.Rnullsafe_op pos))
    | _ ->
      (* Shouldn't happen *)
      make_nullable_member_type ~is_method:false env id_pos pos ty
  else
    let (env, ty) =
      Typing_solver.non_null env (Pos_or_decl.of_raw_pos id_pos) ty
    in
    (env, MakeType.nullable_locl (Reason.Rnullsafe_op pos) ty)

(* Return true if the `this` type appears in a covariant
 * (resp. contravariant, if contra=true) position in ty.
 *)
let rec this_appears_covariantly ~contra env ty =
  match get_node ty with
  | Tthis -> not contra
  | Ttuple tyl
  | Tunion tyl
  | Tintersection tyl ->
    List.exists tyl ~f:(this_appears_covariantly ~contra env)
  | Tfun ft ->
    this_appears_covariantly ~contra env ft.ft_ret.et_type
    || List.exists ft.ft_params ~f:(fun fp ->
           this_appears_covariantly ~contra:(not contra) env fp.fp_type.et_type)
  | Tshape (_, fm) ->
    let fields = TShapeMap.elements fm in
    List.exists fields ~f:(fun (_, f) ->
        this_appears_covariantly ~contra env f.sft_ty)
  | Taccess (ty, _)
  | Tlike ty
  | Toption ty ->
    this_appears_covariantly ~contra env ty
  | Tvec_or_dict (ty1, ty2) ->
    this_appears_covariantly ~contra env ty1
    || this_appears_covariantly ~contra env ty2
  | Tapply (pos_name, tyl) ->
    let rec this_appears_covariantly_params tparams tyl =
      match (tparams, tyl) with
      | (tp :: tparams, ty :: tyl) ->
        begin
          match tp.tp_variance with
          | Ast_defs.Covariant -> this_appears_covariantly ~contra env ty
          | Ast_defs.Contravariant ->
            this_appears_covariantly ~contra:(not contra) env ty
          | Ast_defs.Invariant ->
            this_appears_covariantly ~contra env ty
            || this_appears_covariantly ~contra:(not contra) env ty
        end
        || this_appears_covariantly_params tparams tyl
      | _ -> false
    in
    let tparams =
      match Typing_env.get_class_or_typedef env (snd pos_name) with
      | Some (Typing_env.TypedefResult { td_tparams; _ }) -> td_tparams
      | Some (Typing_env.ClassResult cls) -> Cls.tparams cls
      | None -> []
    in
    this_appears_covariantly_params tparams tyl
  | Tmixed
  | Tany _
  | Terr
  | Tnonnull
  | Tdynamic
  | Tprim _
  | Tvar _
  | Tgeneric _ ->
    false

(** We know that the receiver is a concrete class, not a generic with
    bounds, or a Tunion. *)
let rec obj_get_concrete_ty
    args env concrete_ty ((id_pos, id_str) as id) on_error =
  log_obj_get env `concrete id_pos concrete_ty args.this_ty;
  let dflt_rval_err =
    Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty
  and dflt_lval_err = Ok concrete_ty in

  let default ?(lval_err = dflt_lval_err) ?(rval_err = dflt_rval_err) () =
    (env, (Typing_utils.mk_tany env id_pos, []), lval_err, rval_err)
  in
  let read_context = Option.is_none args.coerce_from_ty in
  let (env, concrete_ty) = Env.expand_type env concrete_ty in
  match deref concrete_ty with
  | (r, Tclass ((_, class_name), _, paraml)) ->
    obj_get_concrete_class
      args
      env
      concrete_ty
      (id_pos, id_str)
      r
      class_name
      paraml
      on_error
  | (_, Tdynamic) ->
    let err_opt = sound_dynamic_err_opt args env id read_context in
    Option.iter ~f:Errors.add_typing_error err_opt;
    let ty = MakeType.dynamic (Reason.Rdynamic_prop id_pos) in
    (env, (ty, []), dflt_lval_err, dflt_rval_err)
  | (_, Tany _)
  | (_, Terr) ->
    default ()
  | (_, Tnonnull) ->
    let err =
      Typing_error.(
        primary
        @@ Primary.Top_member
             {
               pos = id_pos;
               name = id_str;
               ctxt =
                 (if read_context then
                   `read
                 else
                   `write);
               kind =
                 (if args.is_method then
                   `method_
                 else
                   `property);
               is_nullable = false;
               decl_pos = get_pos concrete_ty;
               ty_name = lazy (Typing_print.error env concrete_ty);
             })
    in
    Errors.add_typing_error err;
    let ty_nothing = MakeType.nothing Reason.none in
    let lval_err = Error (concrete_ty, ty_nothing) in
    default ~lval_err ()
  | _ ->
    let err =
      Typing_error.(
        primary
        @@ Primary.Non_object_member
             {
               pos = id_pos;
               ctxt =
                 (if read_context then
                   `read
                 else
                   `write);
               kind =
                 (if args.is_method then
                   `method_
                 else
                   `property);
               member_name = id_str;
               ty_name = lazy (Typing_print.error env concrete_ty);
               decl_pos = get_pos concrete_ty;
             })
    in
    Errors.add_typing_error @@ Typing_error.apply ~on_error err;
    let ty_nothing = MakeType.nothing Reason.none in
    let lval_err = Error (concrete_ty, ty_nothing) in
    default ~lval_err ()

and get_member_from_constraints
    args env class_info ((id_pos, _) as id) reason params on_error =
  let ety_env = mk_ety_env class_info params args.this_ty in
  let upper_bounds = Cls.upper_bounds_on_this class_info in
  let (env, upper_bounds) =
    List.map_env env upper_bounds ~f:(fun env up ->
        Phase.localize ~ety_env env up)
  in
  let (env, inter_ty) =
    Inter.intersect_list env (Reason.Rwitness id_pos) upper_bounds
  in
  obj_get_inner
    {
      args with
      nullsafe = None;
      obj_pos = Reason.to_pos reason |> Pos_or_decl.unsafe_to_raw_pos;
      is_nonnull = true;
    }
    env
    inter_ty
    id
    on_error

and obj_get_concrete_class
    args
    env
    concrete_ty
    ((id_pos, id_str) as id)
    reason
    class_name
    params
    on_error =
  match Env.get_class env class_name with
  | None ->
    ( env,
      (Typing_utils.mk_tany env id_pos, []),
      Ok concrete_ty,
      Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty )
  | Some class_info ->
    let params =
      if List.is_empty params then
        List.map (Cls.tparams class_info) ~f:(fun _ ->
            Typing_utils.mk_tany env id_pos)
      else
        params
    in
    let old_member_info = Env.get_member args.is_method env class_info id_str in
    let self_id = Option.value (Env.get_self_id env) ~default:"" in
    let (member_info, shadowed) =
      if
        Cls.has_ancestor class_info self_id
        || Cls.requires_ancestor class_info self_id
      then
        (* We look up the current context to see if there is a field/method with
         * private visibility. If there is one, that one takes precedence *)
        match Env.get_self_class env with
        | None -> (old_member_info, false)
        | Some self_class ->
          (match Env.get_member args.is_method env self_class id_str with
          | Some { ce_visibility = Vprivate _; _ } as member_info ->
            (member_info, true)
          | _ -> (old_member_info, false))
      else
        (old_member_info, false)
    in
    begin
      match member_info with
      | None ->
        obj_get_concrete_class_without_member_info
          args
          env
          concrete_ty
          id
          reason
          class_info
          params
          on_error
      | Some member_info ->
        obj_get_concrete_class_with_member_info
          args
          env
          concrete_ty
          id
          self_id
          shadowed
          reason
          class_name
          class_info
          params
          old_member_info
          member_info
          on_error
    end

and obj_get_concrete_class_with_member_info
    args
    env
    concrete_ty
    ((id_pos, id_str) as id)
    self_id
    shadowed
    reason
    class_name
    class_info
    params
    old_member_info
    member_info
    on_error =
  let dflt_rval_err =
    Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty
  and dflt_lval_err = Ok concrete_ty in
  let { ce_visibility = vis; ce_type = (lazy member_); ce_deprecated; _ } =
    member_info
  in
  let mem_pos = get_pos member_ in

  let shadow_err_opt =
    Option.bind old_member_info ~f:(fun old_member_info ->
        if
          shadowed
          && not (String.equal member_info.ce_origin old_member_info.ce_origin)
        then
          let (lazy old_member) = old_member_info.ce_type in
          Some
            Typing_error.(
              primary
              @@ Primary.Ambiguous_object_access
                   {
                     pos = id_pos;
                     name = id_str;
                     self_pos = get_pos member_;
                     vis =
                       Typing_defs.string_of_visibility
                         old_member_info.ce_visibility;
                     subclass_pos = get_pos old_member;
                     class_self = self_id;
                     class_subclass = class_name;
                   })
        else
          None)
  in

  let vis_err_opts =
    [
      TVis.check_obj_access
        ~is_method:args.is_method
        ~use_pos:id_pos
        ~def_pos:mem_pos
        env
        vis;
      TVis.check_deprecated ~use_pos:id_pos ~def_pos:mem_pos ce_deprecated;
      TVis.check_expression_tree_vis ~use_pos:id_pos ~def_pos:mem_pos env vis;
      (if args.inst_meth then
        TVis.check_inst_meth_access ~use_pos:id_pos ~def_pos:mem_pos vis
      else
        None);
      (if
       args.meth_caller
       && TypecheckerOptions.meth_caller_only_public_visibility
            (Env.get_tcopt env)
      then
        TVis.check_meth_caller_access ~use_pos:id_pos ~def_pos:mem_pos vis
      else
        None);
    ]
  in

  let parent_abstract_call_err_opt =
    if args.is_parent_call && get_ce_abstract member_info then
      Some
        Typing_error.(
          primary
          @@ Primary.Parent_abstract_call
               { meth_name = id_str; pos = id_pos; decl_pos = mem_pos })
    else
      None
  in

  let typing_errs =
    List.filter_map
      ~f:Fn.id
      (shadow_err_opt :: parent_abstract_call_err_opt :: vis_err_opts)
  in

  List.iter ~f:Errors.add_typing_error typing_errs;

  let member_decl_ty = Typing_enum.member_type env member_info in
  let widen_this = this_appears_covariantly ~contra:true env member_decl_ty in
  let ety_env = mk_ety_env class_info params args.this_ty in
  let (env, member_ty, tal, et_enforced) =
    match deref member_decl_ty with
    | (r, Tfun ft) when args.is_method ->
      (* We special case function types here to be able to pass explicit type
       * parameters. *)
      let (env, explicit_targs) =
        Phase.localize_targs
          ~check_well_kinded:true
          ~is_method:args.is_method
          ~use_pos:id_pos
          ~def_pos:mem_pos
          ~use_name:(strip_ns id_str)
          env
          ft.ft_tparams
          (List.map ~f:snd args.explicit_targs)
      in
      let ft =
        Typing_enforceability.compute_enforced_and_pessimize_fun_type env ft
      in
      let (env, ft1) =
        Phase.(
          localize_ft
            ~instantiation:
              { use_name = strip_ns id_str; use_pos = id_pos; explicit_targs }
            ~ety_env:
              {
                ety_env with
                on_error = Typing_error.Reasons_callback.ignore_error;
              }
            ~def_pos:mem_pos
            env
            ft)
      in
      let ft_ty1 =
        Typing_dynamic.relax_method_type
          env
          (get_ce_support_dynamic_type member_info)
          r
          ft1
      in
      let (env, ft_ty) =
        if widen_this then
          let ety_env = { ety_env with this_ty = args.this_ty_conjunct } in
          let (env, ft2) =
            Phase.(
              localize_ft
                ~instantiation:
                  {
                    use_name = strip_ns id_str;
                    use_pos = id_pos;
                    explicit_targs;
                  }
                ~ety_env:
                  {
                    ety_env with
                    on_error = Typing_error.Reasons_callback.ignore_error;
                  }
                ~def_pos:mem_pos
                env
                ft)
          in
          let ft_ty2 = mk (Typing_reason.localize r, Tfun ft2) in
          Inter.intersect_list env (Typing_reason.localize r) [ft_ty1; ft_ty2]
        else
          (env, ft_ty1)
      in
      (env, ft_ty, explicit_targs, Unenforced)
    | _ ->
      let is_xhp_attr = Option.is_some (get_ce_xhp_attr member_info) in
      let { et_type; et_enforced } =
        Typing_enforceability.compute_enforced_and_pessimize_ty
          env
          member_decl_ty
          ~explicitly_untrusted:is_xhp_attr
      in
      let (env, member_ty) = Phase.localize ~ety_env env et_type in
      (* TODO(T52753871): same as for class_get *)
      (env, member_ty, [], et_enforced)
  in

  let (env, (member_ty, tal)) =
    if Cls.has_upper_bounds_on_this_from_constraints class_info then
      let ((env, (ty, tal), _, _), succeed) =
        Errors.try_
          (fun () ->
            ( get_member_from_constraints
                args
                env
                class_info
                id
                reason
                params
                on_error,
              true ))
          (fun _ ->
            (* No eligible functions found in constraints *)
            ( ( env,
                (MakeType.mixed Reason.Rnone, []),
                dflt_lval_err,
                dflt_rval_err ),
              false ))
      in
      if succeed then
        let (env, member_ty) =
          Inter.intersect env ~r:(Reason.Rwitness id_pos) member_ty ty
        in
        (env, (member_ty, tal))
      else
        (env, (member_ty, tal))
    else
      (env, (member_ty, tal))
  in
  let eff () =
    let open Typing_env_types in
    if env.in_support_dynamic_type_method_check then
      Typing_log.log_pessimise_prop env mem_pos id_str
  in
  let (env, rval_err) =
    Option.value_map
      args.coerce_from_ty
      ~default:(env, dflt_rval_err)
      ~f:(fun (p, ur, ty) ->
        Result.fold
          ~ok:(fun env -> (env, Some (Ok ty)))
          ~error:(fun env -> (env, Some (Error (ty, member_ty))))
        @@ Typing_coercion.coerce_type_res
             p
             ur
             env
             ty
             { et_type = member_ty; et_enforced }
             Typing_error.Callback.(
               (with_side_effect ~eff unify_error [@alert "-deprecated"])))
  in
  (env, (member_ty, tal), dflt_lval_err, rval_err)

and obj_get_concrete_class_without_member_info
    args
    env
    concrete_ty
    ((id_pos, id_str) as id)
    reason
    class_info
    params
    on_error =
  let dflt_rval_err =
    Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty
  and dflt_lval_err = Ok concrete_ty in
  let default ?(lval_err = dflt_lval_err) ?(rval_err = dflt_rval_err) () =
    (env, (Typing_utils.mk_tany env id_pos, []), lval_err, rval_err)
  in
  if Cls.has_upper_bounds_on_this_from_constraints class_info then
    Errors.try_
      (fun () ->
        get_member_from_constraints
          args
          env
          class_info
          id
          reason
          params
          on_error)
      (fun (_ : Errors.error) ->
        let err =
          member_not_found
            env
            id_pos
            ~is_method:args.is_method
            class_info
            id_str
            reason
            on_error
        in
        Errors.add_typing_error err;
        let ty_nothing = MakeType.nothing Reason.none in
        default ~lval_err:(Error (concrete_ty, ty_nothing)) ())
  else if not args.is_method then
    let lval_err =
      if not (SN.Members.is_special_xhp_attribute id_str) then (
        let err =
          member_not_found
            env
            id_pos
            ~is_method:args.is_method
            class_info
            id_str
            reason
            on_error
        in
        Errors.add_typing_error err;
        let ty_nothing = MakeType.nothing Reason.none in
        Error (concrete_ty, ty_nothing)
      ) else
        dflt_lval_err
    in
    default ~lval_err ()
  else if String.equal id_str SN.Members.__clone then
    (* Create a `public function __clone()[]: void {}` for classes that don't declare __clone *)
    let ft =
      {
        ft_arity = Fstandard;
        ft_tparams = [];
        ft_where_constraints = [];
        ft_params = [];
        ft_implicit_params =
          { capability = CapTy (MakeType.intersection Reason.Rnone []) };
        ft_ret =
          { et_type = MakeType.void Reason.Rnone; et_enforced = Unenforced };
        ft_flags = 0;
        ft_ifc_decl = default_ifc_fun_decl;
      }
    in
    (env, (mk (Reason.Rnone, Tfun ft), []), dflt_lval_err, dflt_rval_err)
  else if String.equal id_str SN.Members.__construct then (
    (* __construct is not an instance method and shouldn't be invoked directly
       Note that we already raise a NAST check error in `illegal_name_check` but
       we raise a related typing error here to properly keep track of failure.
       We prefer a specific error here since the generic 4053 `MemberNotFound`
       error, below, would be quite confusing telling us there is no instance
       method `__construct` *)
    let err =
      Typing_error.(primary @@ Primary.Construct_not_instance_method id_pos)
    in
    Errors.add_typing_error err;
    default ()
  ) else
    let err =
      member_not_found
        env
        id_pos
        ~is_method:args.is_method
        class_info
        id_str
        reason
        on_error
    in
    Errors.add_typing_error err;
    let ty_nothing = MakeType.nothing Reason.none in
    default ~lval_err:(Error (concrete_ty, ty_nothing)) ()

and nullable_obj_get
    args env ety1 ((id_pos, id_str) as id) on_error ~read_context ty =
  let dflt_rval_err =
    Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty
  in
  match args.nullsafe with
  | Some r_null ->
    let (env, (method_, tal), lval_err, rval_err) =
      obj_get_inner args env ty id on_error
    in
    let (env, ty) =
      match r_null with
      | Typing_reason.Rnullsafe_op p1 ->
        make_nullable_member_type
          ~is_method:args.is_method
          env
          id_pos
          p1
          method_
      | _ -> (env, method_)
    in
    (env, (ty, tal), lval_err, rval_err)
  | None ->
    let (ty_expect, err) =
      match deref ety1 with
      | (r, Toption opt_ty) ->
        begin
          match get_node opt_ty with
          | Tnonnull ->
            let err =
              Typing_error.(
                primary
                @@ Primary.Top_member
                     {
                       pos = id_pos;
                       name = id_str;
                       ctxt =
                         (if read_context then
                           `read
                         else
                           `write);
                       kind =
                         (if args.is_method then
                           `method_
                         else
                           `property);
                       is_nullable = true;
                       decl_pos = Reason.to_pos r;
                       ty_name = lazy (Typing_print.error env ety1);
                     })
            in
            (MakeType.nothing Reason.none, err)
          | _ ->
            let err =
              Typing_error.(
                primary
                @@ Primary.Null_member
                     {
                       pos = id_pos;
                       member_name = id_str;
                       reason = Reason.to_string "This can be null" r;
                       kind =
                         (if args.is_method then
                           `method_
                         else
                           `property);
                       ctxt =
                         (if read_context then
                           `read
                         else
                           `write);
                     })
            in

            (MakeType.nothing Reason.none, err)
        end
      | (r, _) ->
        let err =
          Typing_error.(
            primary
            @@ Primary.Null_member
                 {
                   pos = id_pos;
                   member_name = id_str;
                   reason = Reason.to_string "This can be null" r;
                   kind =
                     (if args.is_method then
                       `method_
                     else
                       `property);
                   ctxt =
                     (if read_context then
                       `read
                     else
                       `write);
                 })
        in
        (MakeType.nothing Reason.none, err)
    in
    Errors.add_typing_error err;
    ( env,
      (TUtils.terr env (get_reason ety1), []),
      Error (ety1, ty_expect),
      dflt_rval_err )

(* Helper method for obj_get that decomposes the type ty1.
 * The additional parameter this_ty represents the type that will be substitued
 * for `this` in the method signature.
 *
 * If ty1 is an intersection type, we distribute the member access through the
 * conjuncts but maintain the intersection type for this_ty. For example,
 * a call on (T & I & J) will recurse on T, I and J but with this_ty = (T & I & J),
 * so that we don't "lose" information when substituting for `this`.
 *
 * In contrast, if ty1 is a union type, we recurse on the disjuncts but pass these
 * through to this_ty, as the member access must be valid for each disjunct
 * separately. Likewise for nullables (special case of union).
 *)
and obj_get_inner args env receiver_ty ((id_pos, id_str) as id) on_error =
  log_obj_get env `inner id_pos receiver_ty args.this_ty;
  let (env, ety1') = Env.expand_type env receiver_ty in
  let was_var = is_tyvar ety1' in
  let dflt_lval_err = Ok receiver_ty
  and dflt_rval_err =
    Option.map ~f:(fun (_, _, ty) -> Ok ty) args.coerce_from_ty
  in
  let (env, ety1) =
    if args.is_method then
      if TypecheckerOptions.method_call_inference (Env.get_tcopt env) then
        Env.expand_type env receiver_ty
      else
        Typing_solver.expand_type_and_solve
          env
          ~description_of_expected:"an object"
          args.obj_pos
          receiver_ty
    else
      Typing_solver.expand_type_and_narrow
        env
        ~description_of_expected:"an object"
        (widen_class_for_obj_get
           ~is_method:args.is_method
           ~nullsafe:args.nullsafe
           id_str)
        args.obj_pos
        receiver_ty
  in
  let (env, ety1) =
    if was_var then
      Typing_dependent_type.ExprDepTy.make_with_dep_kind env args.dep_kind ety1
    else
      (env, ety1)
  in
  let nullable_obj_get ~read_context ty =
    nullable_obj_get
      { args with this_ty = ty; this_ty_conjunct = ty }
      env
      ety1
      id
      on_error
      ~read_context
      ty
  in
  (* coerce_from_ty is used to store the source type for an assignment, so it
   * is a useful marker for whether we're reading or writing *)
  let read_context = Option.is_none args.coerce_from_ty in
  match deref ety1 with
  | (r, Tunion tyl) -> obj_get_inner_union args env on_error id r tyl
  | (r, Tintersection tyl) ->
    let is_nonnull =
      args.is_nonnull
      || Typing_solver.is_sub_type
           env
           receiver_ty
           (Typing_make_type.nonnull Reason.none)
    in
    obj_get_inner_intersection { args with is_nonnull } env on_error id r tyl
  | (_, Tdependent (_, ty))
  | (_, Tnewtype (_, _, ty)) ->
    obj_get_inner args env ty id on_error
  | (r, Tgeneric (_name, _)) ->
    (match TUtils.get_concrete_supertypes ~abstract_enum:true env ety1 with
    | (env, []) ->
      let ctxt =
        if read_context then
          `read
        else
          `write
      and kind =
        if args.is_method then
          `method_
        else
          `property
      in
      let prim_err =
        Typing_error.(
          primary
          @@ Primary.Non_object_member
               {
                 pos = id_pos;
                 ctxt;
                 kind;
                 member_name = id_str;
                 ty_name = lazy (Typing_print.error env ety1);
                 decl_pos = Reason.to_pos r;
               })
      in
      Errors.add_typing_error @@ Typing_error.apply ~on_error prim_err;
      (env, (err_witness env id_pos, []), dflt_lval_err, dflt_rval_err)
    | (env, tyl) ->
      let (env, ty) = Typing_intersection.intersect_list env r tyl in
      let (env, ty) =
        if args.is_nonnull then
          Typing_solver.non_null env (Pos_or_decl.of_raw_pos args.obj_pos) ty
        else
          (env, ty)
      in
      obj_get_inner args env ty id on_error)
  | (_, Toption ty) -> nullable_obj_get ~read_context ty
  | (r, Tprim Tnull) ->
    let ty = mk (r, Tunion []) in
    nullable_obj_get ~read_context ty
  (* We are trying to access a member through a value of unknown type *)
  | (r, Tvar _) ->
    Errors.add_typing_error
      Typing_error.(
        primary
        @@ Primary.Unknown_object_member
             {
               elt =
                 (if args.is_method then
                   `meth
                 else
                   `prop);
               member_name = id_str;
               pos = id_pos;
               reason = Reason.to_string "It is unknown" r;
             });
    let ty_nothing = MakeType.nothing Reason.none in
    ( env,
      (TUtils.terr env r, []),
      Error (receiver_ty, ty_nothing),
      dflt_rval_err )
  | (_, _) -> obj_get_concrete_ty args env ety1 id on_error

and obj_get_inner_union args env on_error id reason tys =
  let (env, resultl, lval_errs, rval_err_opts) =
    List.fold_left
      tys
      ~init:(env, [], [], [])
      ~f:(fun (env, tys, lval_errs, rval_err_opts) ty ->
        let (env, ty, lval_err, rval_err_opt) =
          obj_get_inner
            { args with this_ty = ty; this_ty_conjunct = ty }
            env
            ty
            id
            on_error
        in
        (env, ty :: tys, lval_err :: lval_errs, rval_err_opt :: rval_err_opts))
  in
  let (env, lval_err) = mk_union_err env @@ fold_errs lval_errs in
  let (env, rval_err) =
    Option.value_map ~default:(env, None) ~f:(fun res ->
        let (env, r) = mk_union_err env res in
        (env, Some r))
    @@ fold_opt_errs rval_err_opts
  in

  (* TODO: decide what to do about methods with differing generic arity.
   * See T55414751 *)
  let tal =
    match resultl with
    | [] -> []
    | (_, tal) :: _ -> tal
  in
  let tyl = List.map ~f:fst resultl in
  let (env, ty) = Union.union_list env reason tyl in
  (env, (ty, tal), lval_err, rval_err)

and obj_get_inner_intersection args env on_error id reason tys =
  let (env, resultl, lval_errs, rval_err_opts) =
    TUtils.run_on_intersection_key_value_res env tys ~f:(fun env ty ->
        obj_get_inner { args with this_ty_conjunct = ty } env ty id on_error)
  in
  let (env, lval_err) = mk_intersection_err env @@ fold_errs lval_errs in
  let (env, rval_err) =
    Option.value_map ~default:(env, None) ~f:(fun res ->
        let (env, err) = mk_intersection_err env res in
        (env, Some err))
    @@ fold_opt_errs rval_err_opts
  in
  (* TODO: decide what to do about methods with differing generic arity.
   * See T55414751 *)
  let tal =
    match resultl with
    | [] -> []
    | (_, tal) :: _ -> tal
  in
  let tyl = List.map ~f:fst resultl in
  let (env, ty) = Inter.intersect_list env reason tyl in
  (env, (ty, tal), lval_err, rval_err)

(* Look up the type of the property or method id in the type receiver_ty of the
 * receiver and use the function k to postprocess the result.
 * Return any fresh type variables that were substituted for generic type
 * parameters in the type of the property or method.
 *
 * Essentially, if receiver_ty is a concrete type, e.g., class C, then k is applied
 * to the type of the property id in C; and if receiver_ty is an unresolved type,
 * e.g., a union of classes (C1 | ... | Cn), then k is applied to the type
 * of the property id in each Ci and the results are collected into an
 * unresolved type.
 *
 *)
let obj_get_with_err
    ~obj_pos
    ~is_method
    ~inst_meth
    ~meth_caller
    ~nullsafe
    ~coerce_from_ty
    ~explicit_targs
    ~class_id
    ~member_id
    ~on_error
    ?parent_ty
    env
    receiver_ty =
  Typing_log.(
    log_with_level env "obj_get" ~level:1 (fun () ->
        log_types
          (Pos_or_decl.of_raw_pos obj_pos)
          env
          [Log_head ("obj_get", [Log_type ("receiver_ty", receiver_ty)])]));
  let (env, receiver_ty) =
    if is_method then
      if TypecheckerOptions.method_call_inference (Env.get_tcopt env) then
        Env.expand_type env receiver_ty
      else
        Typing_solver.expand_type_and_solve
          env
          ~description_of_expected:"an object"
          obj_pos
          receiver_ty
    else
      Typing_solver.expand_type_and_narrow
        env
        ~description_of_expected:"an object"
        (widen_class_for_obj_get ~is_method ~nullsafe (snd member_id))
        obj_pos
        receiver_ty
  in

  (* We will substitute `this` in the function signature with `this_ty`. But first,
   * transform it according to the dependent kind dep_kind that was derived from the
   * class_id in the original call to obj_get. See Typing_dependent_type.ml for
   * more details.
   *)
  let dep_kind =
    Typing_dependent_type.ExprDepTy.from_cid
      env
      (get_reason receiver_ty)
      class_id
  in
  let (env, receiver_ty) =
    Typing_dependent_type.ExprDepTy.make_with_dep_kind env dep_kind receiver_ty
  in
  let receiver_or_parent_ty =
    match parent_ty with
    | Some ty -> ty
    | None -> receiver_ty
  in
  let is_parent_call = Nast.equal_class_id_ class_id Aast.CIparent in
  let args =
    {
      inst_meth;
      meth_caller;
      is_method;
      nullsafe;
      obj_pos;
      explicit_targs;
      coerce_from_ty;
      is_nonnull = false;
      is_parent_call;
      dep_kind;
      this_ty = receiver_ty;
      this_ty_conjunct = receiver_ty;
    }
  in
  let (env, ty, lval_err, rval_err_opt) =
    obj_get_inner args env receiver_or_parent_ty member_id on_error
  in
  let from_res = Result.fold ~ok:(fun _ -> None) ~error:(fun tys -> Some tys) in
  let lval_err_opt = from_res lval_err
  and rval_err_opt = Option.bind ~f:from_res rval_err_opt in
  (env, ty, lval_err_opt, rval_err_opt)

(* Look up the type of the property or method id in the type receiver_ty of the
 * receiver and use the function k to postprocess the result.
 * Return any fresh type variables that were substituted for generic type
 * parameters in the type of the property or method.
 *
 * Essentially, if receiver_ty is a concrete type, e.g., class C, then k is applied
 * to the type of the property id in C; and if receiver_ty is an unresolved type,
 * e.g., a union of classes (C1 | ... | Cn), then k is applied to the type
 * of the property id in each Ci and the results are collected into an
 * unresolved type.
 *
 *)
let obj_get
    ~obj_pos
    ~is_method
    ~inst_meth
    ~meth_caller
    ~nullsafe
    ~coerce_from_ty
    ~explicit_targs
    ~class_id
    ~member_id
    ~on_error
    ?parent_ty
    env
    receiver_ty =
  let (env, ty, _lval_err_opt, _rval_err_opt) =
    obj_get_with_err
      ~obj_pos
      ~is_method
      ~inst_meth
      ~meth_caller
      ~nullsafe
      ~coerce_from_ty
      ~explicit_targs
      ~class_id
      ~member_id
      ~on_error
      ?parent_ty
      env
      receiver_ty
  in
  (env, ty)
