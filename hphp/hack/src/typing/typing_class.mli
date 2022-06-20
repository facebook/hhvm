(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val class_def :
  Provider_context.t ->
  Nast.class_ ->
  (Tast.class_ * Typing_inference_env.t_global_with_pos list) option
