(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type 'pos t [@@deriving eq, ord, show]

type classish_information = {
  classish_start: Pos.t;
  classish_end: Pos.t;
}

val make : title:string -> new_text:string -> Pos.t -> Pos.t t

val make_with_edits : title:string -> edits:(string * Pos.t) list -> Pos.t t

val make_classish_start :
  title:string -> new_text:string -> classish_name:string -> Pos.t t

val make_classish_end :
  title:string -> new_text:string -> classish_name:string -> Pos.t t

val get_edits :
  classish_information:classish_information SMap.t ->
  Pos.t t ->
  (string * Pos.t) list

val get_title : Pos.t t -> string

val apply_all : string -> classish_information SMap.t -> Pos.t t list -> string

val to_absolute : Pos.t t -> Pos.absolute t
