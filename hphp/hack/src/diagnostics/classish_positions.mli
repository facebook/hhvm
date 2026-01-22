(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Positional information for a collection of classes *)
type 'pos t = 'pos Classish_positions_types.t

(** A relative position specifier in a class. Can later be
evaluated to an actual position in a file. *)
type 'pos pos = 'pos Classish_positions_types.pos

val extract :
  Full_fidelity_positioned_syntax.t ->
  Full_fidelity_source_text.t ->
  Relative_path.t ->
  Pos.t t

(** An empty positional map *)
val empty : 'pos t

(** Map over the embedded positions. *)
val map : f:('pos -> 'qos) -> 'pos t -> 'qos t

(** Map over the embedded positions *)
val map_pos : f:('pos -> 'qos) -> 'pos pos -> 'qos pos

(** Find a position in the map. *)
val find : 'pos pos -> 'pos t -> 'pos option

(** Return a map, mapping class-names to all the ranges in-between body
elements (i.e. methods, properties, ...) in that class *)
val inbetween_body_element_positions : Pos.t t -> Pos.t list SMap.t
