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
  if Option.is_none (get_ce_flags_xhp_attr member_ce.ce_flags) then
    default_result
  else
    match get_node default_result with
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
             (Cls.enum_type tc)
             Option.(
               Cls.get_typeconst tc Naming_special_names.FB.tInner >>= fun t ->
               t.ttc_type)
             (Cls.get_ancestor tc)
         with
        | None -> default_result
        | Some (_base, enum_ty, _constraint) ->
          let ty = mk (get_reason default_result, get_node enum_ty) in
          ty))
    | _ -> default_result

(* Check that a type is something that can be used as an array index
 * (int or string), blowing through typedefs to do it. Takes a function
 * to call to report the error if it isn't. *)
let check_valid_array_key_type f_fail ~allow_any env p t =
  let rec check_valid_array_key_type env t =
    let ety_env = Phase.env_with_self env in
    let (env, t, trail) = Typing_tdef.force_expand_typedef ~ety_env env t in
    match get_node t with
    | Tprim (Tint | Tstring) -> (env, None)
    (* Enums have to be valid array keys *)
    | Tnewtype (id, _, _) when Typing_env.is_enum env id -> (env, None)
    | Terr
    | Tany _
      when allow_any ->
      (env, None)
    | Tintersection tyl ->
      (* Ok if at least one element of the intersection is ok. *)
      let (env, errors) =
        List.fold_map tyl ~init:env ~f:check_valid_array_key_type
      in
      if List.exists errors ~f:Option.is_none then
        (env, None)
      else
        (env, Option.value ~default:None (List.find errors ~f:Option.is_some))
    | Terr
    | Tany _
    | Tnonnull
    | Tarraykind _
    | Tprim _
    | Toption _
    | Tdynamic
    | Tvar _
    | Tgeneric _
    | Tnewtype _
    | Tdependent _
    | Tclass _
    | Ttuple _
    | Tfun _
    | Tunion _
    | Tobject
    | Tshape _
    | Tpu _
    | Tpu_type_access _ ->
      ( env,
        Some (fun () -> f_fail p (get_pos t) (Typing_print.error env t) trail)
      )
  in
  let (env, err) = check_valid_array_key_type env t in
  Option.iter err (fun f -> f ());
  env

let enum_check_const ty_exp env cc t =
  let p = fst cc.cc_id in
  Typing_ops.sub_type
    p
    Reason.URenum
    env
    t
    ty_exp
    Errors.constant_does_not_match_enum_type

(* Check that the `as` bound or the underlying type of an enum is a subtype of arraykey *)
let enum_check_type env pos ur ty _on_error =
  let ty_arraykey =
    MakeType.arraykey (Reason.Rimplicit_upper_bound (pos, "arraykey"))
  in
  Typing_ops.sub_type pos ur env ty ty_arraykey (fun ?code:_ _ ->
      Errors.enum_type_bad pos (Typing_print.full_strip_ns env ty) [])

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
      (Cls.enum_type tc)
      Option.(
        Cls.get_typeconst tc Naming_special_names.FB.tInner >>= fun t ->
        t.ttc_type)
      (Cls.get_ancestor tc)
  in
  match enum_info_opt with
  | Some (ty_exp, _, ty_constraint) ->
    let ety_env = Phase.env_with_self env in
    let (env, ty_exp) = Phase.localize ~ety_env env ty_exp in
    (* Check that ty_exp <: arraykey *)
    let env =
      enum_check_type
        env
        pos
        Reason.URenum_underlying
        ty_exp
        Errors.enum_underlying_type_must_be_arraykey
    in
    (* Check that ty_exp <: ty_constraint <: arraykey *)
    let env =
      match ty_constraint with
      | None -> env
      | Some ty ->
        let (env, ty) = Phase.localize ~ety_env env ty in
        let env =
          enum_check_type
            env
            pos
            Reason.URenum_cstr
            ty
            Errors.enum_constraint_must_be_arraykey
        in
        Typing_ops.sub_type
          pos
          Reason.URenum_incompatible_cstr
          env
          ty_exp
          ty
          Errors.enum_subtype_must_have_compatible_constraint
    in
    List.fold2_exn ~f:(enum_check_const ty_exp) ~init:env consts const_types
  | None -> env
