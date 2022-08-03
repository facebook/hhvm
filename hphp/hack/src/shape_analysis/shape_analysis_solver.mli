(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val deduce : constraint_ list -> constraint_ list

val produce_results :
  Typing_env_types.env -> constraint_ list -> shape_result list
