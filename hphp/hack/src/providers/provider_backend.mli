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
  | Record_decl of string
  | Typedef_decl of string
  | Gconst_decl of string

(** Maps decl names to types. *)
type decl_cache = (decl_cache_key, Obj.t) Memory_bounded_lru_cache.t

type t = private
  | Shared_memory
  | Local_memory of { decl_cache: decl_cache }
  (* In Decl_service, 'unit' left for further expansion *)
  | Decl_service of Decl_service_client.t

val t_to_string : t -> string

val set_shared_memory_backend : unit -> unit

val set_local_memory_backend : max_size_in_words:int -> unit

val set_decl_service_backend : Decl_service_client.t -> unit

val get : unit -> t
