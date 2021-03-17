(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env

val handler :
  Provider_context.t ->
  env Stateful_aast_visitor.default_nast_visitor_with_state
