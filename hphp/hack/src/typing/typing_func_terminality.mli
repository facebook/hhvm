(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_noreturn : Typing_env_types.env -> bool

val typed_expression_exits : Tast.expr -> bool

val expression_exits :
  Typing_env_types.env -> ('a, 'b, 'c, 'd) Aast.expr -> bool
