(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

let unify_param_modes param1 param2 on_error =
  let { fp_pos = pos1; _ } = param1 in
  let { fp_pos = pos2; _ } = param2 in
  match (get_fp_mode param1, get_fp_mode param2) with
  | (FPnormal, FPnormal)
  | (FPinout, FPinout) ->
    ()
  | (FPnormal, FPinout) -> Errors.inoutness_mismatch pos2 pos1 on_error
  | (FPinout, FPnormal) -> Errors.inoutness_mismatch pos1 pos2 on_error
