(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Decl_cache_entry : sig
  type _ t =
    | Fun_decl : string -> Typing_defs.fun_elt t
    | Class_decl : string -> Obj.t t
    | Record_decl : string -> Typing_defs.record_def_type t
    | Typedef_decl : string -> Typing_defs.typedef_type t
    | Gconst_decl : string -> (Typing_defs.decl_ty * Errors.t) t

  type 'a key = 'a t

  type 'a value = 'a

  val get_size : key:'a key -> value:'a value -> int
end

(** Maps decl names to types. *)
module Decl_cache : sig
  include module type of Lru_cache.Cache (Decl_cache_entry)
end

module Shallow_decl_cache_entry : sig
  type _ t = Shallow_class_decl : string -> Shallow_decl_defs.shallow_class t

  type 'a key = 'a t

  type 'a value = 'a

  val get_size : key:'a key -> value:'a value -> int
end

module Shallow_decl_cache : sig
  include module type of Lru_cache.Cache (Shallow_decl_cache_entry)
end

type t =
  | Shared_memory
  | Local_memory of {
      decl_cache: Decl_cache.t;
      shallow_decl_cache: Shallow_decl_cache.t;
    }
  | Decl_service of Decl_service_client.t

val t_to_string : t -> string

val set_shared_memory_backend : unit -> unit

val set_local_memory_backend :
  max_num_decls:int -> max_num_shallow_decls:int -> unit

val set_local_memory_backend_with_defaults : unit -> unit

val set_decl_service_backend : Decl_service_client.t -> unit

val get : unit -> t
