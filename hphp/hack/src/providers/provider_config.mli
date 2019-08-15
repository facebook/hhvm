(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decl_cache_key =
  | Fun_decl of string
  | Class_decl of string
  | Typedef_decl of string
  | Gconst_decl of string

type decl_cache = (decl_cache_key, Obj.t) Memory_bounded_lru_cache.t
(** Maps decl names to types. *)

type backend = private
  | Lru_shared_memory
  | Shared_memory
  | Local_memory of { decl_cache: decl_cache }

val set_lru_shared_memory_backend : unit -> unit

val set_shared_memory_backend : unit -> unit

val set_local_memory_backend : max_size_in_words:int -> unit

val get_backend : unit -> backend
