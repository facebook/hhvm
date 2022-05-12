(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module A = Aast

val stmt_name : ('a, 'b) A.stmt_ -> string

val expr_name : ('a, 'b) A.expr_ -> string
