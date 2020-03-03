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
module Env = Typing_env
module Phase = Typing_phase
module TUtils = Typing_utils

let class_get_pu_member_type ?from_class env ty enum member name =
  let (env, dty) =
    TUtils.class_get_pu_member_type ?from_class env ty enum member name
  in
  match dty with
  | None -> (env, None)
  | Some (ety_env, (_, dty)) ->
    let (env, lty) = Typing_phase.localize ~ety_env env dty in
    (env, Some lty)

let reduce_pu_type_access env base enum atom name =
  let (env, base) = Env.expand_type env base in
  match class_get_pu_member_type env base (snd enum) atom (snd name) with
  | (_env, None) -> assert false (* already caught in localization *)
  | (env, Some lty) ->
    (* Not sure if this expand is necessary, ask Catg *)
    let (env, lty) = Env.expand_type env lty in
    (env, lty)

(* We are trying to expand `base:@enum:@member:@ty` to:
 * - a known type if `member` is a known atom of `base:@enum`
 * - a fresh type variable if `member` is a Tvar, meaning we don't know yet
 *   what it should be solved to
 * - a rigid PU dependent type if `member` is a Tgeneric (with no
 *   substitution), which can only be compared to itself.
 *)
let expand_member env reason (base : locl_ty) enum (member : locl_ty) ty =
  match deref member with
  | (r, Tgeneric s) ->
    let p = Reason.to_pos r in
    (env, mk (reason, Tpu_type_access (base, enum, (p, s), ty)))
  | (_, Tvar v) ->
    let (env, lty) =
      Typing_subtype_pocket_universes.get_tyvar_pu_access
        env
        reason
        base
        enum
        v
        ty
    in
    (env, lty)
  | (_, Tprim (Aast_defs.Tatom atom)) ->
    reduce_pu_type_access env base enum atom ty
  | _ -> (env, Typing_utils.terr env reason)

let expand_dep_ty env ~ety_env reason (base : locl_ty) enum member ty =
  let ((mpos, member), pu_loc) = member in
  let (env, member) =
    match pu_loc with
    | Aast_defs.TypeParameter ->
      (* If we already checked that member was a type parameter.
       * It's the most recent binder. In this case,
       * either there's a substitution in `ety_env`, or it is a rigid generic.
       *)
      let mreason = Reason.Rwitness mpos in
      let member = mk (mreason, Tgeneric member) in
      (* We might have a substitution pending for this Tgeneric, so let's
       * localize it.
       *)
      Phase.localize ~ety_env env member
    | _ ->
      (* Ok, now it might be an atom, or an error *)
      let (env, res) = TUtils.class_get_pu_member env base (snd enum) member in
      if Option.is_some res then
        let (env, member) =
          Phase.localize
            ~ety_env
            env
            (mk (reason, Tprim (Aast_defs.Tatom member)))
        in
        (env, member)
      else (
        (* If it is neither a type parameter nor an atom, report an error *)
        Errors.pu_localize
          (Reason.to_pos reason)
          (Printf.sprintf
             "%s:@%s:@%s:@%s"
             (Typing_print.debug env base)
             (snd enum)
             member
             (snd ty))
          member;
        (env, TUtils.terr env reason)
      )
  in
  expand_member env reason base enum member ty

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_pocket_universes_ref := expand_dep_ty
