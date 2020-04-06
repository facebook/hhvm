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
    let (env, lty) = Phase.localize ~ety_env env dty in
    (env, Some lty)

let reduce_pu_type_access env base enum atom name =
  let (env, base) = Env.expand_type env base in
  match class_get_pu_member_type env base (snd enum) atom (snd name) with
  | (_env, None) -> assert false (* already caught in localization *)
  | (env, Some lty) ->
    (* Not sure if this expand is necessary, ask Catg *)
    let (env, lty) = Env.expand_type env lty in
    (env, lty)

(* Expand a fully instanciated PU type to its definition.
   For example:
   class C {
     enum E {
      case type T;
      case T data;
      :@I (type T = int, data = 42);
     }
   }

   function f<TP as C:@E>(TP $atom) : TP:@T {
     return C:@E::data($atom);
   }

   // in here, f(:@I) has type :@I:@T in the C:@E context which
   // is defined to be int
   function testit() : void {
     expect_int(f(:@I));
   }
*)
let expand_pocket_universes env reason (base : locl_ty) enum member ty =
  let (_, member) = member in
  let (env, member) =
    let (env, res) = TUtils.class_get_pu_member env base (snd enum) member in
    if Option.is_some res then
      let member = mk (reason, Tprim (Aast_defs.Tatom member)) in
      (env, member)
    else (
      (* We dealt with type parameters during localization, so this must be
         an error *)
      Errors.pu_localize
        (Reason.to_pos reason)
        (Printf.sprintf "%s:@%s" (Typing_print.debug env base) (snd enum))
        (Printf.sprintf "%s:@%s" member (snd ty));
      (env, TUtils.terr env reason)
    )
  in
  (* We are trying to expand `member:@ty` to a known type, when `member` is
   * solved to a known atom of `base:@enum`
   *)
  match deref member with
  | (_, Tprim (Aast_defs.Tatom atom)) ->
    reduce_pu_type_access env base enum atom ty
  | _ -> (env, Typing_utils.terr env reason)

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_pocket_universes_ref := expand_pocket_universes
