(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val of_candidate :
  source_text:Full_fidelity_source_text.t ->
  path:Relative_path.t ->
  Extract_method_types.candidate ->
  Code_action_types.Refactor.t
