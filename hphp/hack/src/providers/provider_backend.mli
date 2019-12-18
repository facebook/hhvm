(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Decl_cache_entry : sig
  type key =
    | Fun_decl of string
    | Class_decl of string
    | Record_decl of string
    | Typedef_decl of string
    | Gconst_decl of string

  type value = Obj.t

  val get_size : value -> int
end

(** Maps decl names to types. *)
module Decl_cache : sig
  include module type of Lru_cache.Cache (Decl_cache_entry)
end

type t = private
  | Shared_memory
  | Local_memory of { decl_cache: Decl_cache.t }
  (* In Decl_service, 'unit' left for further expansion *)
  | Decl_service of Decl_service_client.t

val t_to_string : t -> string

val set_shared_memory_backend : unit -> unit

val set_local_memory_backend : max_num_decls:int -> unit

val set_decl_service_backend : Decl_service_client.t -> unit

val get : unit -> t
