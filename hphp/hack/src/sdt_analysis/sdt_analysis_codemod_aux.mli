(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val insert_attribute :
  Relative_path.t ->
  attribute:string ->
  enclosing_node:Full_fidelity_editable_positioned_syntax.t option ->
  attributes_node:Full_fidelity_editable_positioned_syntax.t ->
  ServerRefactorTypes.patch option
