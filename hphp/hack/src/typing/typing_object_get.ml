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
module Partial = Partial_provider

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
    let cid = (Cls.pos class_, Cls.name class_) in
    Errors.smember_not_found kind pos cid member_name hint on_error
  in
  let static_suggestion =
    Env.suggest_static_member is_method class_ member_name
  in
  let method_suggestion = Env.suggest_member is_method class_ member_name in
  match (static_suggestion, method_suggestion) with
  (* If there is a normal method of the same name and the
   * syntax is a function pointer, suggest meth_caller *)
  | (_, Some (_, v)) when is_function_pointer && String.equal v member_name ->
    Errors.consider_meth_caller pos (Cls.name class_) member_name
  (* If there is a normal method of the same name, suggest it *)
  | (Some _, Some (def_pos, v)) when String.equal v member_name ->
    error (`closest (def_pos, v))
  (* Otherwise suggest a different static method *)
  | (Some (def_pos, v), _) -> error (`did_you_mean (def_pos, v))
  (* Fallback to closest normal method *)
  | (None, Some (def_pos, v)) -> error (`closest (def_pos, v))
  (* no error in this case ... the member might be present
   * in one of the parents of class_ that the typing cannot see *)
  | (None, None) when not (Cls.members_fully_known class_) -> ()
  | (None, None) -> error `no_hint

let member_not_found
    (env : Typing_env_types.env) pos ~is_method class_ member_name r on_error =
  let cls_name = strip_ns (Cls.name class_) in
  if env.Typing_env_types.in_expr_tree && is_method then
    Errors.expr_tree_unsupported_operator cls_name member_name pos
  else
    let kind =
      if is_method then
        `method_
      else
        `property
    in
    let cid = (Cls.pos class_, Cls.name class_) in
    let reason =
      Reason.to_string
        ("This is why I think it is an object of type " ^ cls_name)
        r
    in
    let error hint =
      Errors.member_not_found kind pos cid member_name hint reason on_error
    in
    let method_suggestion = Env.suggest_member is_method class_ member_name in
    let static_suggestion =
      Env.suggest_static_member is_method class_ member_name
    in
    match (method_suggestion, static_suggestion) with
    (* Prefer suggesting a different method, unless there's a
      static method whose name matches exactly. *)
    | (Some _, Some (def_pos, v)) when String.equal v member_name ->
      error (`closest (def_pos, v))
    | (Some (def_pos, v), _) -> error (`did_you_mean (def_pos, v))
    | (None, Some (def_pos, v)) -> error (`closest (def_pos, v))
    | (None, None) when not (Cls.members_fully_known class_) ->
      (* no error in this case ... the member might be present
       * in one of the parents of class_ that the typing cannot see *)
      ()
    | (None, None) -> error `no_hint

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
                  type_expansions = Typing_defs.Type_expansions.empty;
                  substs =
                    TUtils.make_locl_subst_for_class_tparams class_info tyl;
                  this_ty = ty;
                  on_error = Errors.ignore_error;
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
        List.map_env env tyl (fun env ty ->
            make_nullable_member_type ~is_method env id_pos pos ty)
      in
      Union.union_list env r tyl
    | (r, Tintersection tyl) ->
      let (env, tyl) =
        List.map_env env tyl (fun env ty ->
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

(* We know that the receiver is a concrete class: not a generic with
 * bounds, or a Tunion. *)
let rec obj_get_concrete_ty
    ~inst_meth
    ~is_method
    ~coerce_from_ty
    ?(explicit_targs = [])
    ~this_ty
    ~dep_kind
    ~is_parent_call
    env
    concrete_ty
    (id_pos, id_str)
    on_error =
  let default () = (env, (Typing_utils.mk_tany env id_pos, [])) in
  (* We will substitute `this` in the function signature with `this_ty`. But first,
   * transform it according to the dependent kind dep_kind that was derived from the
   * class_id in the original call to obj_get. See Typing_dependent_type.ml for
   * more details.
   *)
  let (env, this_ty) =
    Typing_dependent_type.ExprDepTy.make_with_dep_kind env dep_kind this_ty
  in
  let mk_ety_env class_info paraml =
    {
      type_expansions = Typing_defs.Type_expansions.empty;
      this_ty;
      substs = TUtils.make_locl_subst_for_class_tparams class_info paraml;
      on_error = Errors.ignore_error;
    }
  in
  let read_context = Option.is_none coerce_from_ty in
  let (env, concrete_ty) = Env.expand_type env concrete_ty in
  match deref concrete_ty with
  | (r, Tclass (x, _, paraml)) ->
    let get_member_from_constraints env class_info =
      let ety_env = mk_ety_env class_info paraml in
      let upper_bounds = Cls.upper_bounds_on_this class_info in
      let (env, upper_bounds) =
        List.map_env env upper_bounds ~f:(fun env up ->
            Phase.localize ~ety_env env up)
      in
      let (env, inter_ty) =
        Inter.intersect_list env (Reason.Rwitness id_pos) upper_bounds
      in
      obj_get_inner
        ~inst_meth
        ~is_method
        ~nullsafe:None
        ~obj_pos:(Reason.to_pos r |> Pos_or_decl.unsafe_to_raw_pos)
        ~explicit_targs
        ~coerce_from_ty
        ~is_nonnull:true
        ~this_ty
        ~is_parent_call
        ~dep_kind
        env
        inter_ty
        (id_pos, id_str)
        on_error
    in
    begin
      match Env.get_class env (snd x) with
      | None -> default ()
      | Some class_info
        when (not is_method)
             && (not (Env.is_strict env))
             && (not (Partial.should_check_error (Env.get_mode env) 4053))
             && String.equal (Cls.name class_info) SN.Classes.cStdClass ->
        default ()
      | Some class_info ->
        let paraml =
          if List.is_empty paraml then
            List.map (Cls.tparams class_info) (fun _ ->
                Typing_utils.mk_tany env id_pos)
          else
            paraml
        in
        let old_member_info = Env.get_member is_method env class_info id_str in
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
              (match Env.get_member is_method env self_class id_str with
              | Some { ce_visibility = Vprivate _; _ } as member_info ->
                (member_info, true)
              | _ -> (old_member_info, false))
          else
            (old_member_info, false)
        in
        begin
          match member_info with
          | None when Cls.has_upper_bounds_on_this_from_constraints class_info
            ->
            Errors.try_with_error
              (fun () -> get_member_from_constraints env class_info)
              (fun () ->
                member_not_found
                  env
                  id_pos
                  ~is_method
                  class_info
                  id_str
                  r
                  on_error;
                default ())
          | None when not is_method ->
            if not (SN.Members.is_special_xhp_attribute id_str) then
              member_not_found
                env
                id_pos
                ~is_method
                class_info
                id_str
                r
                on_error;
            default ()
          | None when String.equal id_str SN.Members.__clone ->
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
                  {
                    et_type = MakeType.void Reason.Rnone;
                    et_enforced = Unenforced;
                  };
                ft_flags = 0;
                ft_ifc_decl = default_ifc_fun_decl;
              }
            in
            (env, (mk (Reason.Rnone, Tfun ft), []))
          | None when String.equal id_str SN.Members.__construct ->
            (* __construct is not an instance method and shouldn't be invoked directly *)
            let () = Errors.magic (id_pos, id_str) in
            default ()
          | None ->
            member_not_found env id_pos ~is_method class_info id_str r on_error;
            default ()
          | Some
              ( {
                  ce_visibility = vis;
                  ce_type = (lazy member_);
                  ce_deprecated;
                  _;
                } as member_ce ) ->
            let mem_pos = get_pos member_ in
            ( if shadowed then
              match old_member_info with
              | Some
                  {
                    ce_visibility = old_vis;
                    ce_type = (lazy old_member);
                    ce_origin;
                    _;
                  } ->
                begin
                  match get_node this_ty with
                  | Tdependent (DTthis, _) -> ()
                  | _ ->
                    if not (String.equal member_ce.ce_origin ce_origin) then
                      Errors.ambiguous_object_access
                        id_pos
                        id_str
                        (get_pos member_)
                        (TUtils.string_of_visibility old_vis)
                        (get_pos old_member)
                        self_id
                        (snd x)
                end
              | _ -> () );
            TVis.check_obj_access ~use_pos:id_pos ~def_pos:mem_pos env vis;
            TVis.check_deprecated ~use_pos:id_pos ~def_pos:mem_pos ce_deprecated;
            if is_parent_call && get_ce_abstract member_ce then
              Errors.parent_abstract_call id_str id_pos mem_pos;
            let member_decl_ty = Typing_enum.member_type env member_ce in
            let ety_env = mk_ety_env class_info paraml in
            let (env, member_ty, tal, et_enforced) =
              match deref member_decl_ty with
              | (r, Tfun ft) when is_method ->
                (* We special case function types here to be able to pass explicit type
                 * parameters. *)
                let (env, explicit_targs) =
                  Phase.localize_targs
                    ~check_well_kinded:true
                    ~is_method
                    ~use_pos:id_pos
                    ~def_pos:mem_pos
                    ~use_name:(strip_ns id_str)
                    env
                    ft.ft_tparams
                    (List.map ~f:snd explicit_targs)
                in
                let ft =
                  Typing_enforceability.compute_enforced_and_pessimize_fun_type
                    env
                    ft
                in
                let (env, ft) =
                  Phase.(
                    localize_ft
                      ~instantiation:
                        {
                          use_name = strip_ns id_str;
                          use_pos = id_pos;
                          explicit_targs;
                        }
                      ~ety_env:{ ety_env with on_error = Errors.ignore_error }
                      ~def_pos:mem_pos
                      env
                      ft)
                in
                let ft_ty = mk (Typing_reason.localize r, Tfun ft) in
                (env, ft_ty, explicit_targs, Unenforced)
              | _ ->
                let is_xhp_attr = Option.is_some (get_ce_xhp_attr member_ce) in
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
            if inst_meth then
              TVis.check_inst_meth_access ~use_pos:id_pos ~def_pos:mem_pos vis;
            let (env, (member_ty, tal)) =
              if Cls.has_upper_bounds_on_this_from_constraints class_info then
                let ((env, (ty, tal)), succeed) =
                  Errors.try_with_error
                    (fun () ->
                      (get_member_from_constraints env class_info, true))
                    (fun () ->
                      (* No eligible functions found in constraints *)
                      ((env, (MakeType.mixed Reason.Rnone, [])), false))
                in
                if succeed then
                  let (env, member_ty) =
                    Inter.intersect env (Reason.Rwitness id_pos) member_ty ty
                  in
                  (env, (member_ty, tal))
                else
                  (env, (member_ty, tal))
              else
                (env, (member_ty, tal))
            in
            let env =
              Option.value_map
                coerce_from_ty
                ~default:env
                ~f:(fun (p, ur, ty) ->
                  Typing_coercion.coerce_type
                    p
                    ur
                    env
                    ty
                    { et_type = member_ty; et_enforced }
                    Errors.unify_error)
            in
            (env, (member_ty, tal))
        end
        (* match member_info *)
    end
  (* match Env.get_class env (snd x) *)
  | (_, Tdynamic) ->
    ( if TypecheckerOptions.enable_sound_dynamic (Env.get_tcopt env) then
      (* Any access to a *private* member through dynamic might potentially
       * be unsound, if the receiver is an instance of a class that implements dynamic,
       * as we do no checks on enforceability or subtype-dynamic at the definition site
       * of private members.
       *)
      match Env.get_self_class env with
      | Some self_class
        when Cls.get_implements_dynamic self_class || not (Cls.final self_class)
        ->
        (match Env.get_member is_method env self_class id_str with
        | Some { ce_visibility = Vprivate _; ce_type = (lazy ty); _ }
          when not is_method ->
          ( if read_context then
            let (env, locl_ty) =
              Phase.localize_with_self ~ignore_errors:true env ty
            in
            Typing_dynamic.check_property_sound_for_dynamic_read
              ~on_error:Errors.private_property_is_not_dynamic
              env
              (Cls.name self_class)
              (id_pos, id_str)
              locl_ty );
          if not read_context then
            Typing_dynamic.check_property_sound_for_dynamic_write
              ~on_error:Errors.private_property_is_not_enforceable
              env
              (Cls.name self_class)
              (id_pos, id_str)
              ty
        | _ -> ())
      | _ -> () );
    let ty = MakeType.dynamic (Reason.Rdynamic_prop id_pos) in
    (env, (ty, []))
  | (_, Tobject)
  | (_, Tany _)
  | (_, Terr) ->
    default ()
  | (_, Tnonnull) ->
    let err =
      if read_context then
        Errors.top_member_read
      else
        Errors.top_member_write
    in
    err
      ~is_method
      ~is_nullable:false
      id_str
      id_pos
      (Typing_print.error env concrete_ty)
      (get_pos concrete_ty);
    default ()
  | _ ->
    let err =
      if read_context then
        Errors.non_object_member_read
      else
        Errors.non_object_member_write
    in
    err
      ~kind:
        ( if is_method then
          `method_
        else
          `property )
      id_str
      id_pos
      (Typing_print.error env concrete_ty)
      (get_pos concrete_ty)
      on_error;
    default ()

and nullable_obj_get
    ~inst_meth
    ~obj_pos
    ~is_method
    ~nullsafe
    ~explicit_targs
    ~coerce_from_ty
    ~is_nonnull
    ~this_ty
    ~is_parent_call
    ~dep_kind
    env
    ety1
    ((id_pos, id_str) as id)
    on_error
    ~read_context
    ty =
  match nullsafe with
  | Some r_null ->
    let (env, (method_, tal)) =
      obj_get_inner
        ~inst_meth
        ~obj_pos
        ~is_method
        ~nullsafe
        ~explicit_targs
        ~coerce_from_ty
        ~is_nonnull
        ~this_ty
        ~is_parent_call
        ~dep_kind
        env
        ty
        id
        on_error
    in
    let (env, ty) =
      match r_null with
      | Typing_reason.Rnullsafe_op p1 ->
        make_nullable_member_type ~is_method env id_pos p1 method_
      | _ -> (env, method_)
    in
    (env, (ty, tal))
  | None ->
    (match deref ety1 with
    | (r, Toption opt_ty) ->
      begin
        match get_node opt_ty with
        | Tnonnull ->
          let err =
            if read_context then
              Errors.top_member_read
            else
              Errors.top_member_write
          in
          err
            ~is_method
            ~is_nullable:true
            id_str
            id_pos
            (Typing_print.error env ety1)
            (Reason.to_pos r)
        | _ ->
          let err =
            if read_context then
              Errors.null_member_read
            else
              Errors.null_member_write
          in
          err ~is_method id_str id_pos (Reason.to_string "This can be null" r)
      end
    | (r, _) ->
      let err =
        if read_context then
          Errors.null_member_read
        else
          Errors.null_member_write
      in
      err ~is_method id_str id_pos (Reason.to_string "This can be null" r));
    (env, (TUtils.terr env (get_reason ety1), []))

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
and obj_get_inner
    ~inst_meth
    ~is_method
    ~nullsafe
    ~obj_pos
    ~coerce_from_ty
    ~is_nonnull
    ~explicit_targs
    ~this_ty
    ~is_parent_call
    ~dep_kind
    env
    ty1
    ((id_pos, id_str) as id)
    on_error =
  let (env, ety1) =
    if is_method then
      if TypecheckerOptions.method_call_inference (Env.get_tcopt env) then
        Env.expand_type env ty1
      else
        Typing_solver.expand_type_and_solve
          env
          ~description_of_expected:"an object"
          obj_pos
          ty1
    else
      Typing_solver.expand_type_and_narrow
        env
        ~description_of_expected:"an object"
        (widen_class_for_obj_get ~is_method ~nullsafe id_str)
        obj_pos
        ty1
  in
  let nullable_obj_get ~read_context ty =
    nullable_obj_get
      ~inst_meth
      ~obj_pos
      ~is_method
      ~nullsafe
      ~explicit_targs
      ~coerce_from_ty
      ~is_nonnull
      ~this_ty:ty
      ~is_parent_call
      ~dep_kind
      env
      ety1
      id
      on_error
      ~read_context
      ty
  in
  (* coerce_from_ty is used to store the source type for an assignment, so it
   * is a useful marker for whether we're reading or writing *)
  let read_context = Option.is_none coerce_from_ty in
  match deref ety1 with
  | (r, Tunion tyl) ->
    let (env, resultl) =
      List.map_env env tyl (fun env ty ->
          obj_get_inner
            ~inst_meth
            ~obj_pos
            ~is_method
            ~nullsafe
            ~explicit_targs
            ~coerce_from_ty
            ~is_nonnull
            ~this_ty:ty
            ~is_parent_call
            ~dep_kind
            env
            ty
            id
            on_error)
    in
    (* TODO: decide what to do about methods with differing generic arity.
     * See T55414751 *)
    let tal =
      match resultl with
      | [] -> []
      | (_, tal) :: _ -> tal
    in
    let tyl = List.map ~f:fst resultl in
    let (env, ty) = Union.union_list env r tyl in
    (env, (ty, tal))
  | (r, Tintersection tyl) ->
    let is_nonnull =
      is_nonnull
      || Typing_solver.is_sub_type
           env
           ty1
           (Typing_make_type.nonnull Reason.none)
    in
    let (env, resultl) =
      TUtils.run_on_intersection env tyl ~f:(fun env ty ->
          obj_get_inner
            ~inst_meth
            ~obj_pos
            ~is_method
            ~nullsafe
            ~explicit_targs
            ~coerce_from_ty
            ~is_nonnull
            ~this_ty
            ~is_parent_call
            ~dep_kind
            env
            ty
            id
            on_error)
    in
    (* TODO: decide what to do about methods with differing generic arity.
     * See T55414751 *)
    let tal =
      match resultl with
      | [] -> []
      | (_, tal) :: _ -> tal
    in
    let tyl = List.map ~f:fst resultl in
    let (env, ty) = Inter.intersect_list env r tyl in
    (env, (ty, tal))
  | (r, Tdependent (dep, ty)) ->
    let dep_kind =
      ( r,
        match dep with
        | DTexpr id -> Typing_dependent_type.ExprDepTy.Dep_Expr id
        | DTthis -> Typing_dependent_type.ExprDepTy.Dep_This )
    in
    obj_get_inner
      ~inst_meth
      ~obj_pos
      ~is_method
      ~nullsafe
      ~explicit_targs
      ~coerce_from_ty
      ~is_nonnull
      ~this_ty
      ~is_parent_call
      ~dep_kind
      env
      ty
      id
      on_error
  | (_, Tnewtype (_, _, ty)) ->
    obj_get_inner
      ~inst_meth
      ~obj_pos
      ~is_method
      ~nullsafe
      ~explicit_targs
      ~coerce_from_ty
      ~is_nonnull
      ~this_ty
      ~is_parent_call
      ~dep_kind
      env
      ty
      id
      on_error
  | (r, Tgeneric _) ->
    let (env, tyl) = TUtils.get_concrete_supertypes env ety1 in
    if List.is_empty tyl then (
      let err =
        if read_context then
          Errors.non_object_member_read
        else
          Errors.non_object_member_write
      in
      err
        ~kind:
          ( if is_method then
            `method_
          else
            `property )
        id_str
        id_pos
        (Typing_print.error env ety1)
        (Reason.to_pos r)
        on_error;
      (env, (err_witness env id_pos, []))
    ) else
      let (env, ty) = Typing_intersection.intersect_list env r tyl in
      let (env, ty) =
        if is_nonnull then
          Typing_solver.non_null env (Pos_or_decl.of_raw_pos obj_pos) ty
        else
          (env, ty)
      in
      obj_get_inner
        ~inst_meth
        ~obj_pos
        ~is_method
        ~nullsafe
        ~explicit_targs
        ~coerce_from_ty
        ~is_nonnull
        ~this_ty
        ~is_parent_call
        ~dep_kind
        env
        ty
        id
        on_error
  | (_, Toption ty) -> nullable_obj_get ~read_context ty
  | (r, Tprim Tnull) ->
    let ty = mk (r, Tunion []) in
    nullable_obj_get ~read_context ty
  (* We are trying to access a member through a value of unknown type *)
  | (r, Tvar _) ->
    Errors.unknown_object_member
      ~is_method
      id_str
      id_pos
      (Reason.to_string "It is unknown" r);
    (env, (TUtils.terr env r, []))
  | (_, _) ->
    obj_get_concrete_ty
      ~inst_meth
      ~is_method
      ~explicit_targs
      ~coerce_from_ty
      ~this_ty
      ~is_parent_call
      ~dep_kind
      env
      ety1
      id
      on_error

(* Look up the type of the property or method id in the type ty1 of the
 * receiver and use the function k to postprocess the result.
 * Return any fresh type variables that were substituted for generic type
 * parameters in the type of the property or method.
 *
 * Essentially, if ty1 is a concrete type, e.g., class C, then k is applied
 * to the type of the property id in C; and if ty1 is an unresolved type,
 * e.g., a union of classes (C1 | ... | Cn), then k is applied to the type
 * of the property id in each Ci and the results are collected into an
 * unresolved type.
 *
 *)
let obj_get
    ~obj_pos
    ~is_method
    ~inst_meth
    ~nullsafe
    ~coerce_from_ty
    ~explicit_targs
    ~class_id
    ~member_id
    ~on_error
    ?parent_ty
    env
    ty =
  let dep_kind =
    Typing_dependent_type.ExprDepTy.from_cid env (get_reason ty) class_id
  in
  let ty1 =
    match parent_ty with
    | Some ty -> ty
    | None -> ty
  in
  let is_parent_call = Nast.equal_class_id_ class_id Aast.CIparent in
  obj_get_inner
    ~inst_meth
    ~is_method
    ~nullsafe
    ~obj_pos
    ~explicit_targs
    ~coerce_from_ty
    ~is_nonnull:false
    ~is_parent_call
    ~this_ty:ty
    ~dep_kind
    env
    ty1
    member_id
    on_error
