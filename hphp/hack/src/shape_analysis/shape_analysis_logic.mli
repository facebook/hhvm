(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

(** Obtain a fresh result identifier *)
val fresh_result_id : unit -> ResultID.t

(** Create a singleton shape key, e.g., shape('a' => int) *)
val singleton : ResultID.t -> shape_key -> Typing_defs.locl_ty -> shape_keys

(** Merge shape keys, e.g.,

      shape('a' => int, 'b' => string)
      <>
      shape('a' => string, 'c' => mixed)
      =
      shape('a' => arraykey, 'b' => string, 'c' => mixed)
  *)
val ( <> ) : env:Typing_env_types.env -> shape_keys -> shape_keys -> shape_keys
