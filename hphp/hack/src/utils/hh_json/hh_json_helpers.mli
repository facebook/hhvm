(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Jget : sig
  exception Parse of string

  val string_opt : Yojson.Safe.t option -> string -> string option

  val bool_opt : Yojson.Safe.t option -> string -> bool option

  val obj_opt : Yojson.Safe.t option -> string -> Yojson.Safe.t option

  val val_opt : Yojson.Safe.t option -> string -> Yojson.Safe.t option

  val int_opt : Yojson.Safe.t option -> string -> int option

  val float_opt : Yojson.Safe.t option -> string -> float option

  val array_opt :
    Yojson.Safe.t option -> string -> Yojson.Safe.t option list option

  val string_d : Yojson.Safe.t option -> string -> default:string -> string

  val bool_d : Yojson.Safe.t option -> string -> default:bool -> bool

  val int_d : Yojson.Safe.t option -> string -> default:int -> int

  val float_d : Yojson.Safe.t option -> string -> default:float -> float

  val array_d :
    Yojson.Safe.t option ->
    string ->
    default:Yojson.Safe.t option list ->
    Yojson.Safe.t option list

  val bool_exn : Yojson.Safe.t option -> string -> bool

  val string_exn : Yojson.Safe.t option -> string -> string

  val val_exn : Yojson.Safe.t option -> string -> Yojson.Safe.t

  val int_exn : Yojson.Safe.t option -> string -> int

  val float_exn : Yojson.Safe.t option -> string -> float

  val array_exn : Yojson.Safe.t option -> string -> Yojson.Safe.t option list

  val obj_exn : Yojson.Safe.t option -> string -> Yojson.Safe.t option

  val string_array_exn : Yojson.Safe.t option -> string -> string list
end

module Jprint : sig
  val object_opt : (string * Yojson.Safe.t option) list -> Yojson.Safe.t

  val string_array : string list -> Yojson.Safe.t
end

module AdhocJsonHelpers : sig
  exception Not_found

  val try_get_val : string -> Yojson.Safe.t -> Yojson.Safe.t option

  val get_string_val : string -> ?default:string -> Yojson.Safe.t -> string

  val get_number_val : string -> ?default:string -> Yojson.Safe.t -> string

  val get_bool_val : string -> ?default:bool -> Yojson.Safe.t -> bool

  val get_array_val :
    string -> ?default:Yojson.Safe.t list -> Yojson.Safe.t -> Yojson.Safe.t list

  val strlist : string list -> Yojson.Safe.t

  val pred : string -> Yojson.Safe.t list -> Yojson.Safe.t
end

val get_object_exn : Yojson.Safe.t -> (string * Yojson.Safe.t) list

val get_array_exn : Yojson.Safe.t -> Yojson.Safe.t list

val get_string_exn : Yojson.Safe.t -> string

val get_number_exn : Yojson.Safe.t -> string

val get_number_int_exn : Yojson.Safe.t -> int

val get_bool_exn : Yojson.Safe.t -> bool

type json_type =
  | Object_t
  | Array_t
  | String_t
  | Number_t
  | Integer_t
  | Bool_t

module type Access = sig
  type keytrace = string list

  type access_failure =
    | Not_an_object of keytrace
    | Missing_key_error of string * keytrace
    | Wrong_type_error of keytrace * json_type

  type 'a m = ('a * keytrace, access_failure) result

  val keytrace_to_string : keytrace -> string

  val access_failure_to_string : access_failure -> string

  val return : 'a -> 'a m

  val ( >>= ) : 'a m -> ('a * keytrace -> 'b m) -> 'b m

  val counit_with : (access_failure -> 'a) -> 'a m -> 'a

  val to_option : 'a m -> 'a option

  val get_obj : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t m

  val get_bool : string -> Yojson.Safe.t * keytrace -> bool m

  val get_string : string -> Yojson.Safe.t * keytrace -> string m

  val get_number : string -> Yojson.Safe.t * keytrace -> string m

  val get_number_int : string -> Yojson.Safe.t * keytrace -> int m

  val get_array : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t list m

  val get_val : string -> Yojson.Safe.t * keytrace -> Yojson.Safe.t m
end

module Access : Access

(** Truncate fields of a json object.
    String fields will be truncated according to [max_string_length].
    [max_object_child_count] determines the maximum number of children of objects.
    [max_array_elt_count] determines the maximum number of array elements.
    Fields at depth greater than [max_depth] will be removed.
    [max_total_count] is the maximum total number of children of arrays and objects
    aggregated over all arrays and objects *)
val json_truncate :
  ?max_string_length:int ->
  ?max_object_child_count:int ->
  ?max_array_elt_count:int ->
  ?max_depth:int ->
  ?max_total_count:int ->
  ?has_changed:bool ref ->
  Yojson.Safe.t ->
  Yojson.Safe.t

val json_truncate_string :
  ?max_string_length:int ->
  ?max_child_count:int ->
  ?max_depth:int ->
  ?max_total_count:int ->
  ?allowed_total_length:int ->
  ?if_reformat_multiline:bool ->
  string ->
  string

val get_field :
  (Yojson.Safe.t * Access.keytrace -> 'a Access.m) ->
  (string -> 'a) ->
  Yojson.Safe.t ->
  'a

val get_field_opt :
  (Yojson.Safe.t * Access.keytrace -> 'a Access.m) -> Yojson.Safe.t -> 'a option
