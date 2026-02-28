(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let resolve : Tast_env.env -> Pos_or_decl.t -> Pos.t =
 (fun env p -> Naming_provider.resolve_position (Tast_env.get_ctx env) p)
