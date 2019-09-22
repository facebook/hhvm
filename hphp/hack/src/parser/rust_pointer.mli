(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t [@@deriving show]

(* Assumptions:
 *  - the only pointers we leak are pointers to positioned syntax trees
 *  - we only leak one pointer at a time, calling register_leaked_pointer() immediately
 *    after, and unregister_leaked_pointer() immediately before passing it back to
 *    Rust function that will consume it
 *
 * This is meant as a temporary safeguard against memory leaks stemming from FFI
 * until entire parser (parser + error checker + lowerer) are in Rust.
 *)

val free_leaked_pointer : ?warn:bool -> unit -> unit

val register_leaked_pointer : t -> unit

val unregister_leaked_pointer : t -> unit
