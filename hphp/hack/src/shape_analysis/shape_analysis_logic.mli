(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module T = Typing_defs
open Shape_analysis_types

(** Create a singleton shape key, e.g., shape('a' => int) *)
val singleton : T.TShapeField.t -> T.locl_ty -> bool -> shape_keys

(** Merge shape keys disjunctively, e.g.,

      shape('a' => int, 'b' => string)
      <>
      shape(?'a' => string, 'c' => mixed)
      =
      shape(?'a' => arraykey, 'b' => string, 'c' => mixed)
  *)
val ( <> ) : env:Typing_env_types.env -> shape_keys -> shape_keys -> shape_keys
