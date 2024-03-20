(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Positional information for a collection of classes *)
type 'pos t

val extract :
  Full_fidelity_positioned_syntax.t ->
  Full_fidelity_source_text.t ->
  Relative_path.t ->
  Pos.t t

(** An empty positional map *)
val empty : 'pos t

(** Position at the start of the class body. Will return a zero-length
position just after the opening brace. *)
val body_start_for : class_name:string -> 'pos t -> 'pos option

(** Position at the end of the class body. Will return a zero-length
position just before the closing brace. *)
val body_end_for : class_name:string -> 'pos t -> 'pos option
