(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_reifiable :
  Type_validator.Env.env -> Typing_defs.typeconst_type -> Pos_or_decl.t -> unit
