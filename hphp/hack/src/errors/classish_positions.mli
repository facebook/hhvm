(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Positional information for a collection of classes *)
type 'pos t

(** A relative position specifier in a class. Can later be
evaluated to an actual position in a file. *)
type 'pos pos =
  | Precomputed of 'pos  (** Use the precomputed position. *)
  | Classish_end_of_body of string
      (** Position at the end of the class body. Will return a zero-length
          position just before the closing brace. *)
  | Classish_start_of_body of string
      (** Position at the start of the class body. Will return a zero-length
          position just after the opening brace. *)

val extract :
  Full_fidelity_positioned_syntax.t ->
  Full_fidelity_source_text.t ->
  Relative_path.t ->
  Pos.t t

(** An empty positional map *)
val empty : 'pos t

(** Find a position in the map. *)
val find : 'pos pos -> 'pos t -> 'pos option
