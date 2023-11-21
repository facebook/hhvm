(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val resolve_files :
  Provider_context.t -> ServerEnv.env -> Fanout.t -> Relative_path.Set.t
