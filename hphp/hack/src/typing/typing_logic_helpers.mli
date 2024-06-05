(**
 * Copyright (c) Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

val valid :
  Typing_env_types.env -> Typing_env_types.env * Typing_logic.subtype_prop

val ( &&& ) :
  Typing_env_types.env * Typing_logic.subtype_prop ->
  (Typing_env_types.env -> Typing_env_types.env * Typing_logic.subtype_prop) ->
  Typing_env_types.env * Typing_logic.subtype_prop

val if_unsat :
  (Typing_env_types.env -> Typing_env_types.env * Typing_logic.subtype_prop) ->
  Typing_env_types.env * Typing_logic.subtype_prop ->
  Typing_env_types.env * Typing_logic.subtype_prop

val ( ||| ) :
  fail:Typing_error.t option ->
  Typing_env_types.env * Typing_logic.subtype_prop ->
  (Typing_env_types.env -> Typing_env_types.env * Typing_logic.subtype_prop) ->
  Typing_env_types.env * Typing_logic.subtype_prop

val invalid : fail:Typing_error.t option -> 'a -> 'a * Typing_logic.subtype_prop
