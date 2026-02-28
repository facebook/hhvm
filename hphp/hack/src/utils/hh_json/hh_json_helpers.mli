(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Jget : sig
  exception Parse of string

  val get_opt :
    ('a -> 'b * 'c list -> ('d * 'e, 'f) result) -> 'b option -> 'a -> 'd option

  val get_exn : ('a -> string -> 'b option) -> 'a -> string -> 'b

  val int_string_opt : string option -> int option

  val float_string_opt : string option -> float option

  val list_opt : 'a list option -> 'a option list option

  val string_opt : Hh_json.json option -> string -> string option

  val bool_opt : Hh_json.json option -> string -> bool option

  val obj_opt : Hh_json.json option -> string -> Hh_json.json option

  val val_opt : Hh_json.json option -> string -> Hh_json.json option

  val int_opt : Hh_json.json option -> string -> int option

  val float_opt : Hh_json.json option -> string -> float option

  val array_opt :
    Hh_json.json option -> string -> Hh_json.json option list option

  val string_d : Hh_json.json option -> string -> default:string -> string

  val bool_d : Hh_json.json option -> string -> default:bool -> bool

  val int_d : Hh_json.json option -> string -> default:int -> int

  val float_d : Hh_json.json option -> string -> default:float -> float

  val array_d :
    Hh_json.json option ->
    string ->
    default:Hh_json.json option list ->
    Hh_json.json option list

  val bool_exn : Hh_json.json option -> string -> bool

  val string_exn : Hh_json.json option -> string -> string

  val val_exn : Hh_json.json option -> string -> Hh_json.json

  val int_exn : Hh_json.json option -> string -> int

  val float_exn : Hh_json.json option -> string -> float

  val array_exn : Hh_json.json option -> string -> Hh_json.json option list

  val obj_exn : Hh_json.json option -> string -> Hh_json.json option

  val string_array_exn : Hh_json.json option -> string -> string list
end

module Jprint : sig
  val object_opt : (string * Hh_json.json option) list -> Hh_json.json

  val string_array : string list -> Hh_json.json
end

module AdhocJsonHelpers : sig
  exception Not_found

  val try_get_val : string -> Hh_json.json -> Hh_json.json option

  val get_string_val : string -> ?default:string -> Hh_json.json -> string

  val get_number_val : string -> ?default:string -> Hh_json.json -> string

  val get_bool_val : string -> ?default:bool -> Hh_json.json -> bool

  val get_array_val :
    string -> ?default:Hh_json.json list -> Hh_json.json -> Hh_json.json list

  val strlist : string list -> Hh_json.json

  val assoc_strlist : string -> string list -> Hh_json.json

  val pred : string -> Hh_json.json list -> Hh_json.json
end
