(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_env_types

val merge_graphs : env -> Typing_inference_env.t_global_with_pos list -> env

val solve_env : env -> (Ident.t -> Errors.error_from_reasons_callback) -> env
