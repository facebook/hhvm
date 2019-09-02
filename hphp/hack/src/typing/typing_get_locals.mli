(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val lvalue :
  Namespace_env.env * Pos.t SMap.t ->
  Nast.expr ->
  Namespace_env.env * Pos.t SMap.t

val stmt :
  Namespace_env.env * Pos.t SMap.t ->
  Nast.stmt ->
  Namespace_env.env * Pos.t SMap.t
