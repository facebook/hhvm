(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t

val create : unit -> t

val to_string : t -> string

val to_json : t -> Hh_json.json

(** `diff current ~prev` is for when `current` and `prev` have the same structure.
It produces a hybrid telemetry object where, element by element, if they're the same
then we only see the current element, but if they're different then we see both.
It works with nested telemetry objects. In places where the structure differs,
only `current` is kept. *)
val diff : t -> prev:t -> t

val string_ : ?truncate:int -> t -> key:string -> value:string -> t

val string_opt : ?truncate:int -> t -> key:string -> value:string option -> t

val array_ :
  ?truncate_elems:int ->
  ?truncate_len:int ->
  t ->
  key:string ->
  value:string list ->
  t

val bool_ : t -> key:string -> value:bool -> t

val int_ : t -> key:string -> value:int -> t

val int_opt : t -> key:string -> value:int option -> t

val object_ : t -> key:string -> value:t -> t

val duration : t -> start_time:float -> t

val float_ : t -> key:string -> value:float -> t

val float_opt : t -> key:string -> value:float option -> t

val error : t -> e:string -> t

val error_with_stack : t -> stack:string -> e:string -> t

val exception_ : t -> e:Exception.t -> t

val quick_gc_stat : unit -> t
