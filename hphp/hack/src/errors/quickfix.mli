(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t [@@deriving eq, ord, show]

val make : title:string -> new_text:string -> Pos.t -> t

val make_classish : title:string -> new_text:string -> classish_name:string -> t

val get_pos : classish_starts:Pos.t SMap.t -> t -> Pos.t

val get_title : t -> string

val get_new_text : t -> string

val apply_all : string -> Pos.t SMap.t -> t list -> string
