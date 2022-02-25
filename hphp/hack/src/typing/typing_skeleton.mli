(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Generates source code for a method skeleton that matches [meth],
    with the appropriate static modifier and override attribute.*)
val of_method :
  string ->
  Typing_defs.class_elt ->
  is_static:bool ->
  is_override:bool ->
  string
