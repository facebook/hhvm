(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 **)

open Typing_defs

val check_class_access:
  Pos.t -> Typing_env.env -> (Pos.t * visibility * bool) -> Nast.class_id_ ->
  Typing_classes_heap.t -> unit

val check_obj_access:
  Pos.t -> Typing_env.env -> (Pos.t * visibility) -> unit

val is_visible:
  Typing_env.env -> (visibility * bool) -> Nast.class_id_ option -> Typing_classes_heap.t -> bool

val min_vis_opt:
  (Pos.t * visibility) option -> (Pos.t * visibility) option ->
  (Pos.t * visibility) option
