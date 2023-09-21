(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module used to enforce that Enum subclasses are used reasonably.
 * Exports the Enum type as the type of all constants, checks that constants
 * have the proper type, and restricts what types can be used for enums.
 *)
(*****************************************************************************)
open Hh_prelude
open Aast
open Typing_defs
module Phase = Typing_phase
module Cls = Decl_provider.Class
module MakeType = Typing_make_type

let member_type env member_ce =
  let (lazy default_result) = member_ce.ce_type in
  if Option.is_none (get_ce_xhp_attr member_ce) then
    default_result
  else
    let (stripped_ty, has_like) =
      match get_node default_result with
      | Tlike ty -> (ty, true)
      | _ -> (default_result, false)
    in
    match get_node stripped_ty with
    | Tapply (enum_id, _) ->
      (* XHP attribute type transform is necessary to account for
       * non-first class Enums:

       * attribute MyEnum x; // declaration: MyEnum
       * $this->:x;          // usage: MyEnumType
       *)
      let maybe_enum = Typing_env.get_class env (snd enum_id) in
      (match maybe_enum with
      | None -> default_result
      | Some tc ->
        (match
           Decl_enum.enum_kind
             (Cls.pos tc, Cls.name tc)
             ~is_enum_class:(Ast_defs.is_c_enum_class (Cls.kind tc))
             (Cls.enum_type tc)
             Option.(
               Cls.get_typeconst tc Naming_special_names.FB.tInner >>= fun t ->
               (* TODO(T88552052) This code was taking the default of abstract
                * type constants as a value *)
               match t.ttc_kind with
               | TCConcrete { tc_type = t }
               | TCAbstract { atc_default = Some t; _ } ->
                 Some t
               | TCAbstract { atc_default = None; _ } -> None)
             ~get_ancestor:(Cls.get_ancestor tc)
         with
        | None -> default_result
        | Some
            {
              Decl_enum.base = _;
              type_ = enum_ty;
              constraint_ = _;
              interface = _;
            } ->
          let ty = mk (get_reason default_result, get_node enum_ty) in
          if has_like then
            mk (get_reason default_result, Tlike ty)
          else
            ty))
    | _ -> default_result

let enum_check_const ty_exp env cc t =
  if Typing_utils.is_tyvar_error env ty_exp || Typing_utils.is_tyvar_error env t
  then
    (env, None)
  else
    let p = fst cc.cc_id in
    Typing_ops.sub_type
      p
      Reason.URenum
      env
      t
      ty_exp
      Typing_error.Callback.constant_does_not_match_enum_type

(* Check that the `as` bound or the underlying type of an enum is a subtype of
 * arraykey. For enum class, check that it is a denotable closed type:
 * no free type parameters are allowed, and we also prevent uncertain cases,
 * like any, or dynamic. The free status of type parameter is caught during
 * naming (Unbound name), so we only check the kind of type that is used.
 *)
let enum_check_type env (pos : Pos_or_decl.t) ur ty_interface ty _on_error =
  let sd env =
    (* Allow pessimised enum class types when sound dynamic is enabled *)
    TypecheckerOptions.enable_sound_dynamic (Typing_env.get_tcopt env)
  in
  let ty_arraykey =
    MakeType.arraykey (Reason.Rimplicit_upper_bound (pos, "arraykey"))
  in
  (* Enforcement of case types for enum/enum classes is wonky.
   * Forbid for now until we can more thoroughly audit the behavior *)
  let check_if_case_type env ty =
    match get_node ty with
    | Tnewtype (name, _, _) ->
      (match Typing_env.get_typedef env name with
      | Some { td_vis = Aast.CaseType; td_pos; _ } ->
        Typing_error_utils.add_typing_error ~env
        @@ Typing_error.(
             enum
             @@ Primary.Enum.Enum_type_bad_case_type
                  {
                    pos = Pos_or_decl.unsafe_to_raw_pos pos;
                    ty_name = lazy (Typing_print.full_strip_ns env ty);
                    case_type_decl_pos = td_pos;
                  })
      | _ -> ())
    | _ -> ()
  in
  let rec is_valid_base ty =
    Typing_utils.is_tyvar_error env ty
    ||
    match get_node ty with
    | Tprim _
    | Tnonnull ->
      true
    | Toption lty -> is_valid_base lty
    | Ttuple ltys -> List.for_all ~f:is_valid_base ltys
    | Tnewtype (_, ltys, lty) ->
      check_if_case_type env ty;
      List.for_all ~f:is_valid_base (lty :: ltys)
    | Tclass (_, _, ltys) -> List.for_all ~f:is_valid_base ltys
    | Tunion [ty1; ty2] when is_dynamic ty1 && sd env -> is_valid_base ty2
    | Tunion [ty1; ty2] when is_dynamic ty2 && sd env -> is_valid_base ty1
    | Tshape { s_fields = shapemap; _ } ->
      TShapeMap.for_all (fun _name sfty -> is_valid_base sfty.sft_ty) shapemap
    | Tany _
    | Tdynamic
    | Tfun _
    | Tvar _
    | Tgeneric _
    | Tunion _
    | Tintersection _
    | Tvec_or_dict _
    | Taccess _
    | Tunapplied_alias _
    | Tdependent _
    | Tneg _ ->
      false
  in
  let (env, ty_err_opt) =
    match ty_interface with
    | Some interface ->
      (if not (is_valid_base interface) then
        Typing_error_utils.add_typing_error ~env
        @@ Typing_error.(
             enum
             @@ Primary.Enum.Enum_type_bad
                  {
                    pos = Pos_or_decl.unsafe_to_raw_pos pos;
                    is_enum_class = true;
                    ty_name = lazy (Typing_print.full_strip_ns env interface);
                    trail = [];
                  }));
      (env, None)
    | None ->
      check_if_case_type env ty;
      let callback =
        let open Typing_error in
        Callback.always
          Primary.(
            Enum
              (Enum.Enum_type_bad
                 {
                   pos = Pos_or_decl.unsafe_to_raw_pos pos;
                   is_enum_class = false;
                   ty_name = lazy (Typing_print.full_strip_ns env ty);
                   trail = [];
                 }))
      in
      Typing_ops.sub_type
        (Pos_or_decl.unsafe_to_raw_pos pos)
        ur
        env
        ty
        ty_arraykey
        callback
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  env

(* Check an enum declaration of the form
 *    enum E : <ty_exp> as <ty_constraint>
 * or
 *    class E extends Enum<ty_exp>
 * where the absence of <ty_constraint> is assumed to default to arraykey.
 *
 * Check that <ty_exp> is int or string, and that
 *   ty_exp <: ty_constraint <: arraykey
 * Also that each type constant is of type ty_exp.
 *)
let enum_class_check env tc consts const_types =
  let pos = Cls.pos tc in
  let enum_info_opt =
    Decl_enum.enum_kind
      (pos, Cls.name tc)
      ~is_enum_class:(Ast_defs.is_c_enum_class (Cls.kind tc))
      (Cls.enum_type tc)
      Option.(
        Cls.get_typeconst tc Naming_special_names.FB.tInner >>= fun t ->
        (* TODO(T88552052) This code was taking the default of abstract
         * type constants as a value *)
        match t.ttc_kind with
        | TCConcrete { tc_type = t }
        | TCAbstract { atc_default = Some t; _ } ->
          Some t
        | TCAbstract { atc_default = None; _ } -> None)
      ~get_ancestor:(Cls.get_ancestor tc)
  in
  let (env, ty_err_opt) =
    match enum_info_opt with
    | Some
        {
          Decl_enum.base = ty_exp;
          type_ = _;
          constraint_ = ty_constraint;
          interface = ty_interface;
        } ->
      let ((env, ty_err1), ty_exp) =
        Phase.localize_no_subst env ~ignore_errors:false ty_exp
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
      let ((env, ty_err2), ty_interface) =
        match ty_interface with
        | Some dty ->
          let (env, lty) =
            Phase.localize_no_subst env ~ignore_errors:false dty
          in
          (env, Some lty)
        | None -> ((env, None), None)
      in
      Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err2;
      (* Check that ty_exp <: arraykey *)
      let env =
        enum_check_type
          env
          pos
          Reason.URenum_underlying
          ty_interface
          ty_exp
          Typing_error.Callback.enum_underlying_type_must_be_arraykey
      in
      (* Check that ty_exp <: ty_constraint <: arraykey *)
      let (env, ty_err_opt) =
        match ty_constraint with
        | None -> (env, None)
        | Some ty ->
          let ((env, ty_err1), ty) =
            Phase.localize_no_subst env ~ignore_errors:false ty
          in
          Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err1;
          let env =
            enum_check_type
              env
              pos
              Reason.URenum_cstr
              None (* Enum classes do not have constraints *)
              ty
              Typing_error.Callback.enum_constraint_must_be_arraykey
          in
          Typing_ops.sub_type
            (Pos_or_decl.unsafe_to_raw_pos pos)
            Reason.URenum_incompatible_cstr
            env
            ty_exp
            ty
            Typing_error.Callback.enum_subtype_must_have_compatible_constraint
      in
      let (env, ty_errs) =
        List.fold2_exn
          ~f:(fun (env, ty_errs) c cty ->
            match enum_check_const ty_exp env c cty with
            | (env, Some ty_err) -> (env, ty_err :: ty_errs)
            | (env, _) -> (env, ty_errs))
          ~init:(env, Option.to_list ty_err_opt)
          consts
          const_types
      in
      (env, Typing_error.multiple_opt ty_errs)
    | None -> (env, None)
  in
  Option.iter ~f:(Typing_error_utils.add_typing_error ~env) ty_err_opt;
  env
