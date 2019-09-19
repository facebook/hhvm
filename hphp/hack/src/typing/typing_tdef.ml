(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
open Utils
module Reason = Typing_reason
module Env = Typing_env
module Subst = Decl_subst
module TUtils = Typing_utils
module Phase = Typing_phase
module MakeType = Typing_make_type

(*****************************************************************************)
(* Expanding type definition *)
(*****************************************************************************)

let expand_typedef_ ?(force_expand = false) ety_env env r x argl =
  let pos = Reason.to_pos r in
  if Typing_defs.has_expanded ety_env x then (
    Errors.cyclic_typedef pos;
    (env, (ety_env, (r, Terr)))
  ) else
    let {
      td_pos;
      td_vis;
      td_tparams;
      td_type;
      td_constraint;
      td_decl_errors = _;
    } =
      unsafe_opt @@ Typing_env.get_typedef env x
    in
    let should_expand =
      force_expand
      ||
      match td_vis with
      | Aast.Opaque -> Pos.filename td_pos = Env.get_file env
      | Aast.Transparent -> true
    in
    let ety_env =
      {
        ety_env with
        type_expansions = (td_pos, x) :: ety_env.type_expansions;
        substs = Subst.make_locl td_tparams argl;
      }
    in
    let (env, expanded_ty) =
      if should_expand then
        Phase.localize ~ety_env env td_type
      else
        let (env, td_constraint) =
          match td_constraint with
          | None ->
            let r_cstr =
              Reason.Rimplicit_upper_bound (Reason.to_pos r, "?nonnull")
            in
            let cstr = (r_cstr, Toption (r_cstr, Tnonnull)) in
            (env, Some cstr)
          | Some cstr ->
            let (env, cstr) = Phase.localize ~ety_env env cstr in
            (env, Some cstr)
        in
        (env, (r, Tabstract (AKnewtype (x, argl), td_constraint)))
    in
    if Naming_special_names.Classes.is_format_string x then
      (env, (ety_env, MakeType.class_type r x argl))
    else
      (env, (ety_env, (r, snd expanded_ty)))

let expand_typedef ety_env env r x argl =
  let (env, (_, ty)) = expand_typedef_ ety_env env r x argl in
  (env, ty)

(* Expand a typedef, smashing abstraction and collecting a trail
 * of where the typedefs come from. *)
let rec force_expand_typedef ~ety_env env (t : locl_ty) =
  match t with
  | (r, Tabstract (AKnewtype (x, argl), _)) when not (Env.is_enum env x) ->
    let (env, (ety_env, ty)) =
      expand_typedef_ ~force_expand:true ety_env env r x argl
    in
    force_expand_typedef ~ety_env env ty
  | ty -> (env, ty, List.rev_map ety_env.type_expansions fst)

(*****************************************************************************)
(*****************************************************************************)

let () = TUtils.expand_typedef_ref := expand_typedef
