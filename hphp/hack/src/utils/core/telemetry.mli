(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t [@@deriving show]

val create : unit -> t

val to_string : ?pretty:bool -> t -> string

val to_json : t -> Hh_json.json

(** `diff ~all current ~prev` is for when `current` and `prev` have the same structure.
It produces a hybrid telemetry object where, element by element, if they're the same
then we only see the current element, but if they're different then we see both.
(If you pass ~all:true then it hides elements that have remained the same.)
It works with nested telemetry objects. In places where the structure differs,
only `current` is kept. *)
val diff : all:bool -> ?suffix_keys:bool -> t -> prev:t -> t

val add : t -> t -> t

val string_ : ?truncate:int -> key:string -> value:string -> t -> t

val string_opt : ?truncate:int -> key:string -> value:string option -> t -> t

val string_list :
  ?truncate_list:int ->
  ?truncate_each_string:int ->
  key:string ->
  value:string list ->
  t ->
  t

val object_list : key:string -> value:t list -> t -> t

val bool_ : key:string -> value:bool -> t -> t

val int_ : key:string -> value:int -> t -> t

val int_opt : key:string -> value:int option -> t -> t

val int_list : ?truncate_list:int -> key:string -> value:int list -> t -> t

val json_ : key:string -> value:Hh_json.json -> t -> t

val object_ : key:string -> value:t -> t -> t

val object_opt : key:string -> value:t option -> t -> t

val duration : ?key:string -> start_time:float -> t -> t

val float_ : key:string -> value:float -> t -> t

val float_opt : key:string -> value:float option -> t -> t

val error : e:string -> t -> t

val error_with_stack : stack:string -> e:string -> t -> t

val exception_ : e:Exception.t -> t -> t

val quick_gc_stat : unit -> t
