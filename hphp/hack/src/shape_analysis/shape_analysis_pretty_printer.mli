(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

val show_constraint_ : Typing_env_types.env -> constraint_ -> string

val show_shape_result : Typing_env_types.env -> shape_result -> string
