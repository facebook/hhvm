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
  | Some (ety_env, (_, _, dty)) ->
    let (env, lty) = Phase.localize ~ety_env env dty in
    (env, Some lty)

let reduce_pu_type_access env base enum atom name =
  let (env, base) = Env.expand_type env base in
  match class_get_pu_member_type env base (snd enum) atom (snd name) with
  (* already caught in naming *)
  | (_env, None) -> (env, mk (Reason.Rwitness (fst name), TUtils.tany env))
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
let expand_atom env reason (base : locl_ty) enum member tyname =
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
        (Printf.sprintf "%s:@%s" member (snd tyname));
      (env, TUtils.terr env reason)
    )
  in
  (* We are trying to expand `member:@ty` to a known type, when `member` is
   * solved to a known atom of `base:@enum`
   *)
  match get_node member with
  | Tprim (Aast_defs.Tatom atom) ->
    reduce_pu_type_access env base enum atom tyname
  | _ -> (env, Typing_utils.terr env reason)

(* In the context of the PU base:@enum, we are trying to expand the type
 * projection ty:@tyname
 *)
let expand_pocket_universes env reason base enum (ty : locl_ty) tyname =
  let apply env r p gen tyname =
    (env, Some (mk (r, Tpu_type_access ((p, gen), tyname))))
  in
  let error env r =
    let pos = Reason.to_pos r in
    Errors.pu_expansion pos (Typing_print.debug env ty) (snd tyname);
    (env, Some (TUtils.terr env r))
  in
  let (env, ty) = Env.expand_type env ty in
  match deref ty with
  | (r, Tprim (Aast_defs.Tatom atom)) ->
    let member = (Reason.to_pos r, atom) in
    let (env, ty) = expand_atom env reason base enum member tyname in
    (env, Some ty)
  | (r, Tgeneric (s, [])) ->
    (* TODO(T70090664) not supporting HK generics, causing them to trigger error catchall case *)
    apply env reason (Reason.to_pos r) s tyname
  | (r, Tdependent (dep_ty, bound)) ->
    let (env, bound) = Env.expand_type env bound in
    (match deref bound with
    | (_, Tpu _) ->
      let new_r =
        (* Patch the location for better error reporting *)
        match r with
        | Reason.Rexpr_dep_type (_, pos, s) ->
          let r = Reason.Rwitness (fst tyname) in
          Reason.Rexpr_dep_type (r, pos, s)
        | _ -> r
      in
      let gen = DependentKind.to_string dep_ty in
      (* TODO(T59317869): play well with flow sensitivity *)
      let env = Env.add_upper_bound_global env gen bound in
      apply env new_r (Reason.to_pos r) gen tyname
    | (_, Tgeneric (gen, [])) ->
      (* TODO(T70090664) not supporting HK generics, causing them to trigger error catchall case *)
      (* Patch the location for better error reporting *)
      let rdep =
        match r with
        | Reason.Rexpr_dep_type (_, pos, _) ->
          let r = Reason.Rwitness (fst tyname) in
          Reason.Rexpr_dep_type (r, pos, Reason.ERpu (gen ^ ":@" ^ snd tyname))
        | _ -> r
      in
      apply env rdep (Reason.to_pos r) gen tyname
    | _ -> error env r)
  | (_, Tpu _) ->
    (* Non dependent Tpu can/will happen because it is used as upperbounds
     * for PU generics. These occurences should be ignored
     *)
    (env, None)
  | (r, _) -> error env r

(*****************************************************************************)
(* Exporting *)
(*****************************************************************************)

let () = Typing_utils.expand_pocket_universes_ref := expand_pocket_universes
