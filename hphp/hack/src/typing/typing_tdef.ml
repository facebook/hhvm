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

(*****************************************************************************)
(* Expanding type definition *)
(*****************************************************************************)

let expand_typedef_ ?(force_expand = false) ety_env env r (x : string) argl =
  let pos = Reason.to_pos r in
  let td = unsafe_opt @@ Decl_entry.to_option (Env.get_typedef env x) in
  let {
    td_pos;
    td_module = _;
    td_vis = _;
    td_tparams;
    td_type;
    td_as_constraint;
    td_super_constraint = _;
    td_is_ctx = _;
    td_attributes = _;
    td_internal = _;
    td_docs_url = _;
  } =
    td
  in
  let (ety_env, has_cycle) =
    Typing_defs.add_type_expansion_check_cycles ety_env (td_pos, x)
  in
  match has_cycle with
  | Some initial_taccess_pos_opt ->
    (* Only report a cycle if it's through the specified definition *)
    let ty_err_opt =
      Option.map initial_taccess_pos_opt ~f:(fun initial_taccess_pos ->
          Typing_error.(
            primary
            @@ Primary.Cyclic_typedef
                 { pos = initial_taccess_pos; decl_pos = pos }))
    in
    let (env, ty) =
      Env.fresh_type_error env (Pos_or_decl.unsafe_to_raw_pos pos)
    in
    ((env, ty_err_opt), (ety_env, ty))
  | None ->
    let should_expand =
      force_expand
      || Env.is_typedef_visible
           env
           ~expand_visible_newtype:ety_env.expand_visible_newtype
           ~name:x
           td
    in
    let ety_env =
      {
        ety_env with
        substs = Subst.make_locl td_tparams argl;
        on_error =
          (* Don't report errors in expanded definition.
           * These will have been reported at the definition site already. *)
          None;
      }
    in
    let (env, expanded_ty) =
      if should_expand then
        Phase.localize ~ety_env env td_type
      else
        let (env, td_as_constraint) =
          match td_as_constraint with
          | None ->
            let r_cstr =
              Reason.Rimplicit_upper_bound (Reason.to_pos r, "?nonnull")
            in
            let cstr = MakeType.mixed r_cstr in
            ((env, None), cstr)
          | Some cstr ->
            (* Special case for supportdyn<T> defined with "as T" in order to
             * avoid supportdynamic.hhi appearing in reason *)
            if String.equal x SN.Classes.cSupportDyn then
              ((env, None), List.hd_exn argl)
            else
              Phase.localize ~ety_env env cstr
        in
        (* TODO: update Tnewtype and pass in super constraint as well *)
        (env, mk (r, Tnewtype (x, argl, td_as_constraint)))
    in
    (env, (ety_env, with_reason expanded_ty r))

let expand_typedef ety_env env r type_name argl =
  let (env, (_ety_env, ty)) = expand_typedef_ ety_env env r type_name argl in
  (env, ty)

(* Expand a typedef, smashing abstraction and collecting a trail
 * of where the typedefs come from. *)
let force_expand_typedef ~ety_env env (t : locl_ty) =
  let rec aux e1 ety_env env t =
    let default () =
      ( (env, e1),
        t,
        Typing_defs.Type_expansions.positions ety_env.type_expansions )
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
      let ((env, e2), (ety_env, ty)) =
        expand_typedef_ ~force_expand:true ety_env env r x argl
      in
      aux (Option.merge e1 e2 ~f:Typing_error.both) ety_env env ty
    | _ -> default ()
  in
  aux None ety_env env t

let () = TUtils.expand_typedef_ref := expand_typedef
