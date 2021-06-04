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
module Subst = Decl_subst
module TUtils = Typing_utils
module Phase = Typing_phase
module MakeType = Typing_make_type

(*****************************************************************************)
(* Expanding type definition *)
(*****************************************************************************)

let expand_typedef_ ?(force_expand = false) ety_env env r (x : string) argl =
  let pos = Reason.to_pos r in
  let td = unsafe_opt @@ Typing_env.get_typedef env x in
  let { td_pos; td_module = _; td_vis = _; td_tparams; td_type; td_constraint }
      =
    td
  in
  let (ety_env, has_cycle) =
    Typing_defs.add_type_expansion_check_cycles ety_env (td_pos, x)
  in
  match has_cycle with
  | Some initial_taccess_pos_opt ->
    (* Only report a cycle if it's through the specified definition *)
    Option.iter initial_taccess_pos_opt ~f:(fun initial_taccess_pos ->
        Errors.cyclic_typedef initial_taccess_pos pos);
    (env, (ety_env, MakeType.err r))
  | None ->
    let should_expand =
      force_expand
      || Typing_env.is_typedef_visible
           env
           ~expand_visible_newtype:ety_env.expand_visible_newtype
           td
    in
    let ety_env =
      {
        ety_env with
        substs = Subst.make_locl td_tparams argl;
        on_error =
          (* Don't report errors in expanded definition.
           * These will have been reported at the definition site already. *)
          Errors.ignore_error;
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
            let cstr = MakeType.mixed r_cstr in
            (env, cstr)
          | Some cstr ->
            let (env, cstr) = Phase.localize ~ety_env env cstr in
            (env, cstr)
        in
        (env, mk (r, Tnewtype (x, argl, td_constraint)))
    in
    (env, (ety_env, with_reason expanded_ty r))

let expand_typedef ety_env env r type_name argl =
  let (env, (_ety_env, ty)) = expand_typedef_ ety_env env r type_name argl in
  (env, ty)

(* Expand a typedef, smashing abstraction and collecting a trail
 * of where the typedefs come from. *)
let rec force_expand_typedef ~ety_env env (t : locl_ty) =
  match deref t with
  | (r, Tnewtype (x, argl, _)) when not (Env.is_enum env x) ->
    let (env, (ety_env, ty)) =
      expand_typedef_ ~force_expand:true ety_env env r x argl
    in
    force_expand_typedef ~ety_env env ty
  | _ -> (env, t, Typing_defs.Type_expansions.positions ety_env.type_expansions)

(*****************************************************************************)
(*****************************************************************************)

let () = TUtils.expand_typedef_ref := expand_typedef
