(* Copyright (c) 2020, Facebook, Inc.
   All rights reserved. *)
type t

val pp : Format.formatter -> t -> unit

val equal : t -> t -> bool

val compare : t -> t -> int

val alloc : unit -> t
