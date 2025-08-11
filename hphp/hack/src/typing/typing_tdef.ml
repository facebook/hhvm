(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Utils
module Reason = Typing_reason
module Env = Typing_env
module SN = Naming_special_names
module Subst = Decl_subst
module TUtils = Typing_utils
module Phase = Typing_phase
module MakeType = Typing_make_type

type expansion =
  | Rhs of decl_ty
  | Opaque of (* constraint *) decl_ty

type expand_result = {
  tparams: decl_tparam list;
  expansion: expansion;
}

(** [expand_typedef_decl ~force_expand ~visibility_behavior env r name ty_args] looks up the type
  alias [name].
  It will not expand opaque type aliases (`newtype`) unless they're visible (in the current file)
  or [force_expand] is true.
  *)
let expand_typedef_decl
    ~force_expand ~visibility_behavior env (x : string) (td : typedef_type) :
    expand_result =
  let {
    td_pos;
    td_module = _;
    td_tparams;
    td_type_assignment;
    td_as_constraint;
    td_super_constraint = _;
    td_is_ctx = _;
    td_attributes = _;
    td_internal = _;
    td_docs_url = _;
    td_package = _;
  } =
    td
  in
  let should_expand =
    force_expand
    || Env.should_expand_type_alias ~visibility_behavior env ~name:x td
  in
  let expansion =
    if should_expand then
      let td_type =
        match td_type_assignment with
        | SimpleTypeDef (_vis, td_type) -> td_type
        | CaseType (variant, variants) ->
          Typing_utils.get_case_type_variants_as_type variant variants
      in
      Rhs td_type
    else
      Opaque
        (match td_as_constraint with
        | None ->
          MakeType.mixed (Reason.implicit_upper_bound (td_pos, "?nonnull"))
        | Some cstr -> cstr)
  in
  { tparams = td_tparams; expansion }

(** [expand_typedef_ ~force_expand ety_env env r name ty_args] looks up the type
  alias [name] in the decls and returns the value of the type alias as a locl_ty, with
  type parameters substituted with [ty_args].

  It will not expand opaque type aliases (`newtype`) unless they're visible (in the current file)
  or [force_expand] is true.
  *)
let expand_typedef_ ~force_expand ety_env env r (x : string) argl :
    Typing_utils.expand_typedef_result * expand_env =
  let td = unsafe_opt @@ Decl_entry.to_option (Env.get_typedef env x) in
  let { td_pos; td_as_constraint; _ } = td in
  match
    Typing_defs.add_type_expansion_check_cycles
      ety_env
      {
        Type_expansions.name = Type_expansions.Expandable.Type_alias x;
        use_pos = Reason.to_pos r;
        def_pos = Some td_pos;
      }
  with
  | Error cycle ->
    let r = Typing_reason.illegal_recursive_type (Reason.to_pos r) x in
    let mixed =
      match td_as_constraint with
      | Some ty ->
        (match get_node ty with
        | Tapply ((_pos, sdt), [_]) when String.equal sdt SN.Classes.cSupportDyn
          ->
          MakeType.supportdyn_mixed r
        | _ -> MakeType.mixed r)
      | _ -> MakeType.mixed r
    in
    let ty =
      (* For regular typechecking, we localize to mixed to prevent various unsoundness,
         when localizing recursive bounds. Other cycle-related wellformedness checks
         using Always_expand_newtype require to localize as an the opaque newtype to be
         able to make proper conclusions about the cycle. *)
      match ety_env.visibility_behavior with
      | Always_expand_newtype -> mk (r, Tnewtype (x, argl, mixed))
      | Never_expand_newtype
      | Expand_visible_newtype_only ->
        mixed
    in
    ( Typing_utils.
        { env; ty_err_opt = None; cycles = [cycle]; ty; bound = mixed },
      ety_env )
  | Ok ety_env ->
    let { tparams; expansion } =
      expand_typedef_decl
        ~force_expand
        ~visibility_behavior:ety_env.visibility_behavior
        env
        x
        td
    in
    let substs = Subst.make_locl tparams argl in
    let ety_env = { ety_env with substs } in
    let ((env, err, cycles), expanded_ty, bound) =
      match expansion with
      | Rhs ty ->
        let ((env, err, cycles), ty) = Phase.localize_rec ~ety_env env ty in
        ((env, err, cycles), ty, ty)
      | Opaque cstr_ty ->
        let ((env, err, cycles), cstr_ty) =
          (* Special case for supportdyn<T> defined with "as T" in order to
           * avoid supportdynamic.hhi appearing in reason *)
          if String.equal x SN.Classes.cSupportDyn then
            ((env, None, []), List.hd_exn argl)
          else
            Phase.localize_rec ~ety_env env cstr_ty
        in
        let ty = mk (r, Tnewtype (x, argl, cstr_ty)) in
        ((env, err, cycles), ty, cstr_ty)
    in
    ( { env; ty_err_opt = err; cycles; ty = with_reason expanded_ty r; bound },
      ety_env )

let expand_typedef ety_env env r type_name argl =
  let (res, _ety_env) =
    expand_typedef_ ~force_expand:false ety_env env r type_name argl
  in
  res

(** Expand a typedef, smashing abstraction and collecting a trail
  of where the typedefs come from.

  /!\ This only does something if passed a Tnewtype. Not sure if that's a bug.
  *)
let force_expand_typedef ~ety_env env (t : locl_ty) =
  let rec aux e1 ety_env env t =
    let default () =
      ((env, e1), t, Type_expansions.def_positions ety_env.type_expansions)
    in
    match deref t with
    | (_, Tnewtype (x, _, _)) when String.equal SN.Classes.cEnumClassLabel x ->
      (* Labels are Resources at runtime, so we don't want to force them
       * to string. MemberOf on the other hand are "typed alias" on the
       * underlying type so it's ok to force them. So we only special case
       * Labels here *)
      default ()
    | (r, Tnewtype (x, argl, _))
      when not (Env.is_enum env x || Env.is_enum_class env x) ->
      let ( Typing_utils.{ env; ty_err_opt = e2; cycles = _; ty; bound = _ },
            ety_env ) =
        expand_typedef_ ~force_expand:true ety_env env r x argl
      in
      aux (Option.merge e1 e2 ~f:Typing_error.both) ety_env env ty
    | _ -> default ()
  in
  aux None ety_env env t

let () = TUtils.expand_typedef_ref := expand_typedef
