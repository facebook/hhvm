(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs_core
module Env = Typing_env

let capability_id = Local_id.make_unscoped SN.Coeffects.capability

let local_capability_id = Local_id.make_unscoped SN.Coeffects.local_capability

let register_capabilities env (cap_ty : locl_ty) (unsafe_cap_ty : locl_ty) =
  let cap_pos = Typing_defs.get_pos cap_ty in
  (* Represents the capability for local operations inside a function body, excluding calls. *)
  let env = Env.set_local env local_capability_id cap_ty cap_pos in
  let (env, ty) =
    Typing_intersection.intersect
      env
      ~r:(Reason.Rhint cap_pos)
      cap_ty
      unsafe_cap_ty
  in
  let (env, ty) = Typing_intersection.simplify_intersections env ty in
  (* The implicit argument for ft_implicit_params.capability *)
  (Env.set_local env capability_id ty cap_pos, ty)

let get_type :
    Typing_env_types.env -> locl_ty capability -> Typing_env_types.env * locl_ty
    =
 fun env capability ->
  match capability with
  | CapTy cap -> (env, cap)
  | CapDefaults p ->
    Typing_make_type.default_capability (Reason.Rhint p)
    |> Typing_phase.localize_with_self env
