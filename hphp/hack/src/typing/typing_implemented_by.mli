(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Validate the __ImplementedBy attribute if present.
 *)
val check_method :
  Typing_env_types.env -> Aast_defs.sid -> Nast.method_ -> Typing_env_types.env
