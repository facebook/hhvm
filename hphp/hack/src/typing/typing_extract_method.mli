(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val extract_static_method :
  Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type ->
  class_name:Typing_defs_core.pos_id ->
  folded_class:Folded_class.t ->
  env:Typing_env_types.env ->
  Typing_env_types.env * Typing_defs_core.decl_ty Typing_defs_core.fun_type

val extract_instance_method :
  Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type ->
  class_name:Typing_defs_core.pos_id ->
  folded_class:Folded_class.t ->
  env:Typing_env_types.env ->
  Typing_env_types.env * Typing_defs_core.decl_ty Typing_defs_core.fun_type
