(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val is_empty : 'phase Typing_defs.class_refinement -> bool

val has_refined_const :
  'pos * string -> 'phase Typing_defs.class_refinement -> bool

val get_refined_const :
  'pos * string ->
  'phase Typing_defs.class_refinement ->
  'phase Typing_defs.refined_const option

val add_refined_const :
  string ->
  'phase Typing_defs.refined_const ->
  'phase Typing_defs.class_refinement ->
  'phase Typing_defs.class_refinement

val iter :
  ('phase Typing_defs.ty -> unit) -> 'phase Typing_defs.class_refinement -> unit

val map :
  ('ph1 Typing_defs.ty -> 'ph2 Typing_defs.ty) ->
  'ph1 Typing_defs.class_refinement ->
  'ph2 Typing_defs.class_refinement

val fold_map :
  ('acc -> Typing_defs.decl_ty -> 'acc * Typing_defs.decl_ty) ->
  'acc ->
  Typing_defs.decl_phase Typing_defs.class_refinement ->
  'acc * Typing_defs.decl_phase Typing_defs.class_refinement

val fold :
  'phase Typing_defs.class_refinement ->
  init:'acc ->
  f:('acc -> 'phase Typing_defs.ty -> 'acc) ->
  'acc

val fold_refined_consts :
  'phase Typing_defs_core.class_refinement ->
  init:'acc ->
  f:(string -> 'phase Typing_defs_core.refined_const -> 'acc -> 'acc) ->
  'acc

val to_string :
  ('phase Typing_defs.ty -> string) ->
  'phase Typing_defs.class_refinement ->
  string
