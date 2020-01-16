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

let unify_arities ~ellipsis_is_variadic anon_arity func_arity : bool =
  match (anon_arity, func_arity) with
  | (Fellipsis (a_min, _), Fvariadic (f_min, _)) when ellipsis_is_variadic ->
    (* we want to allow use the "..." syntax in the declaration of
     * anonymous function types to match named variadic arguments
     * of the "...$args" form as well as unnamed ones *)
    Int.equal a_min f_min
  | (Fvariadic (a_min, _), Fstandard (f_min, _))
  | (Fvariadic (a_min, _), Fvariadic (f_min, _))
  | (Fellipsis (a_min, _), Fstandard (f_min, _))
  | (Fellipsis (a_min, _), Fellipsis (f_min, _)) ->
    a_min <= f_min
  | (Fstandard (a_min, a_max), Fstandard (f_min, f_max)) ->
    Int.equal a_min f_min && Int.equal a_max f_max
  | (_, _) -> false

let unify_param_modes param1 param2 on_error =
  let { fp_pos = pos1; fp_kind = mode1; _ } = param1 in
  let { fp_pos = pos2; fp_kind = mode2; _ } = param2 in
  match (mode1, mode2) with
  | (FPnormal, FPnormal)
  | (FPinout, FPinout) ->
    ()
  | (FPnormal, FPinout) -> Errors.inoutness_mismatch pos2 pos1 on_error
  | (FPinout, FPnormal) -> Errors.inoutness_mismatch pos1 pos2 on_error

let unify_accept_disposable param1 param2 on_error =
  let { fp_pos = pos1; fp_accept_disposable = mode1; _ } = param1 in
  let { fp_pos = pos2; fp_accept_disposable = mode2; _ } = param2 in
  match (mode1, mode2) with
  | (true, false) -> Errors.accept_disposable_invariant pos1 pos2 on_error
  | (false, true) -> Errors.accept_disposable_invariant pos2 pos1 on_error
  | (_, _) -> ()
