(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val collect_analysis_targets :
  Provider_context.t -> Tast.program -> potential_targets

val callable :
  Ast_defs.id_ ->
  Tast_env.t ->
  Tast.fun_param list ->
  return:Tast.type_hint ->
  Tast.func_body ->
  decorated_constraints

val program : Provider_context.t -> Tast.program -> decorated_constraints SMap.t
