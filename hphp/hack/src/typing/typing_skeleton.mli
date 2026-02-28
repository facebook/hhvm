(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

(** Convert the skeleton to a string *)
val to_string : t -> string

(** Add a suffix to the skeleton *)
val add_suffix : string -> t -> t

(** Strip at the end of the skeleton. Noop if the skeleton
does not end in the given suffix *)
val strip_suffix : string -> t -> t

(** Add a prefix to the skeleton *)
val add_prefix : string -> t -> t

(** Make a change to the prefix of the skeleton *)

(** Where to put the cursor of the skeleton was inserted at the given position *)
val cursor_after_insert : Pos.t -> t -> Pos.t

(** Generates source code for a method skeleton that matches [meth],
    with the appropriate static modifier and override attribute.*)
val of_method :
  string ->
  Typing_defs.class_elt ->
  is_static:bool ->
  is_override:bool ->
  open_braces:bool ->
  t
