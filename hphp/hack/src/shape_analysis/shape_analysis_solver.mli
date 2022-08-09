(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types
module HT = Hips_types

val deduce : constraint_ list -> constraint_ list

val produce_results :
  Typing_env_types.env -> constraint_ list -> shape_result list

(** Equality of entity_ and HT.entity, which is embedded into the former *)
val is_same_entity : HT.entity -> entity_ -> bool
