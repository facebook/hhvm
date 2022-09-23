(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Typing_defs

let is_typedef ctx x =
  match Naming_provider.get_type_kind ctx x with
  | Some Naming_types.TTypedef -> true
  | _ -> false

type class_or_typedef_result =
  | ClassResult of Shallow_decl_defs.shallow_class
  | TypedefResult of Typing_defs.typedef_type

let get_class_or_typedef ctx x =
  if is_typedef ctx x then
    match Typedef_provider.get_typedef ctx x with
    | None -> None
    | Some td -> Some (TypedefResult td)
  else
    match Shallow_classes_provider.get ctx x with
    | None -> None
    | Some cd -> Some (ClassResult cd)

let is_enum (ctx : Provider_context.t) (x : string) =
  match Shallow_classes_provider.get ctx x with
  | None -> false
  | Some sc -> Option.is_some sc.Shallow_decl_defs.sc_enum_type

let get_enforcement ~return_from_async (ctx : Provider_context.t) (ty : decl_ty)
    : Typing_defs.enforcement =
  let tcopt = Provider_context.get_tcopt ctx in
  let enable_sound_dynamic = TypecheckerOptions.enable_sound_dynamic tcopt in
  (* hack to avoid yet another flag, just for data gathering for pessimisation *)
  let mixed_nonnull_unenforced = TypecheckerOptions.like_casts tcopt in
  (* is_dynamic_enforceable controls whether the type dynamic is considered enforceable.
     It isn't at the top-level of a type, but is as an argument to a reified generic. *)
  let rec enforcement ~is_dynamic_enforceable ctx visited ty =
    match get_node ty with
    | Tthis -> Unenforced
    (* Enums are only enforced at their underlying type, in contrast to as and is
     * tests which check for validity of values *)
    | Tapply ((_, name), _) when is_enum ctx name -> Unenforced
    | Tapply ((_, name), tyl) ->
      (* Cyclic type definition error will be produced elsewhere *)
      if SSet.mem name visited then
        Enforced
      else begin
        (* The pessimised definition depends on the class or typedef being referenced,
           but we aren't adding any dependency edges here. It is therefore critical that
           we are sure thay are added elsewhere. Currently, that is when we revisit this type
           in Typing_enforceability when we are typechecking the function/property definition
           it is part of. *)
        match get_class_or_typedef ctx name with
        | Some
            (TypedefResult
              { td_vis = Aast.Transparent; td_tparams = _; td_type; _ }) ->
          (* Expand type definition one step and compute its enforcement. *)
          enforcement
            ~is_dynamic_enforceable
            ctx
            (SSet.add name visited)
            td_type
        | Some (ClassResult sc) ->
          List.Or_unequal_lengths.(
            (match
               List.for_all2
                 tyl
                 sc.Shallow_decl_defs.sc_tparams
                 ~f:(fun targ tparam ->
                   match get_node targ with
                   | Tdynamic
                   (* We accept the inner type being dynamic regardless of reification *)
                   | Tlike _
                     when not enable_sound_dynamic ->
                     true
                   | _ ->
                     (match tparam.tp_reified with
                     | Aast.Erased -> false
                     | Aast.SoftReified -> false
                     | Aast.Reified ->
                       (match
                          enforcement
                            ~is_dynamic_enforceable:true
                            ctx
                            visited
                            targ
                        with
                       | Unenforced -> false
                       | Enforced -> true)))
             with
            | Ok false -> Unenforced
            | Ok true
            | Unequal_lengths ->
              Enforced))
        | Some (TypedefResult { td_vis = Aast.Opaque | Aast.OpaqueModule; _ })
        | None ->
          Unenforced
      end
    | Tgeneric _ ->
      (* Previously we allowed dynamic ~> T when T is an __Enforceable generic,
       * that is, when it's valid on the RHS of an `is` or `as` expression.
       * However, `is` / `as` checks have different behavior than runtime checks
       * for `tuple`s and `shapes`s; `is` / `as` will shallow-ly check declared
       * fields but typehint enforcement only checks that we have the right
       * array type (`varray` for `tuple`, `darray` for `shape`). This means
       * it's unsound to allow this coercion.
       *
       * Additionally, higher kinded generics (i.e., with type arguments) cannot
       * be enforced at the moment; they are disallowed to have upper bounds.
       *)
      Unenforced
    | Trefinement _ -> Unenforced
    | Taccess _ -> Unenforced
    | Tlike ty when enable_sound_dynamic ->
      enforcement ~is_dynamic_enforceable ctx visited ty
    | Tlike _ -> Unenforced
    | Tprim prim ->
      begin
        match prim with
        | Aast.Tvoid
        | Aast.Tnoreturn ->
          Unenforced
        | _ -> Enforced
      end
    | Tany _ -> Enforced
    | Terr -> Enforced
    | Tnonnull ->
      if mixed_nonnull_unenforced then
        Unenforced
      else
        Enforced
    | Tdynamic ->
      if (not enable_sound_dynamic) || is_dynamic_enforceable then
        Enforced
      else
        Unenforced
    | Tfun _ -> Unenforced
    | Ttuple _ -> Unenforced
    | Tunion [] -> Enforced
    | Tunion _ -> Unenforced
    | Tintersection _ -> Unenforced
    | Tshape _ -> Unenforced
    | Tmixed ->
      if mixed_nonnull_unenforced then
        Unenforced
      else
        Enforced
    | Tvar _ -> Unenforced
    (* With no parameters, we enforce varray_or_darray just like array *)
    | Tvec_or_dict (_, ty) ->
      if is_any ty then
        Enforced
      else
        Unenforced
    | Toption ty -> enforcement ~is_dynamic_enforceable ctx visited ty
  in
  if return_from_async then
    match get_node ty with
    | Tapply ((_, name), [ty])
      when String.equal Naming_special_names.Classes.cAwaitable name ->
      enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty
    | _ -> enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty
  else
    enforcement ~is_dynamic_enforceable:false ctx SSet.empty ty

let is_enforceable ~return_from_async (ctx : Provider_context.t) (ty : decl_ty)
    =
  match get_enforcement ~return_from_async ctx ty with
  | Enforced -> true
  | Unenforced -> false

let make_like_type ~return_from_async ty =
  let like_if_not_void ty =
    match get_node ty with
    | Tprim Aast.(Tvoid | Tnoreturn) -> ty
    | _ -> Typing_make_type.like (get_reason ty) ty
  in
  if return_from_async then
    match get_node ty with
    | Tapply ((pos, name), [ty])
      when String.equal Naming_special_names.Classes.cAwaitable name ->
      mk (get_reason ty, Tapply ((pos, name), [like_if_not_void ty]))
    | _ -> like_if_not_void ty
  else
    like_if_not_void ty

let make_supportdyn_type p r ty =
  mk (r, Tapply ((p, Naming_special_names.Classes.cSupportDyn), [ty]))

let supportdyn_mixed p r = make_supportdyn_type p r (mk (r, Tmixed))

let add_supportdyn_constraints p tparams =
  let r = Reason.Rwitness_from_decl p in
  List.map tparams ~f:(fun tparam ->
      if
        Naming_special_names.Coeffects.is_generated_generic (snd tparam.tp_name)
      then
        tparam
      else
        {
          tparam with
          tp_constraints =
            (Ast_defs.Constraint_as, supportdyn_mixed p r)
            :: tparam.tp_constraints;
        })

let maybe_add_supportdyn_constraints ctx p tparams =
  if TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx) then
    add_supportdyn_constraints p tparams
  else
    tparams

let pessimise_type ~is_xhp_attr ctx ty =
  if (not is_xhp_attr) && is_enforceable ~return_from_async:false ctx ty then
    ty
  else
    make_like_type ~return_from_async:false ty

let maybe_pessimise_type ~is_xhp_attr ctx ty =
  if TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx) then
    pessimise_type ~is_xhp_attr ctx ty
  else
    ty

let update_return_ty ft ty =
  { ft with ft_ret = { et_type = ty; et_enforced = Unenforced } }

let pessimise_fun_type ctx p ty =
  match get_node ty with
  | Tfun ft ->
    let return_from_async = get_ft_async ft in
    let ret_ty = ft.ft_ret.et_type in
    let ft =
      { ft with ft_tparams = add_supportdyn_constraints p ft.ft_tparams }
    in
    if is_enforceable ~return_from_async ctx ret_ty then
      mk (get_reason ty, Tfun ft)
    else
      mk
        ( get_reason ty,
          Tfun (update_return_ty ft (make_like_type ~return_from_async ret_ty))
        )
  | _ -> ty

let maybe_pessimise_fun_type ctx p ty =
  if TypecheckerOptions.everything_sdt (Provider_context.get_tcopt ctx) then
    pessimise_fun_type ctx p ty
  else
    ty
