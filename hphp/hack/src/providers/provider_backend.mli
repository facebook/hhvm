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

  val key_to_log_string : 'a key -> string
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

  val key_to_log_string : 'a key -> string
end

module Shallow_decl_cache : sig
  include module type of Lru_cache.Cache (Shallow_decl_cache_entry)
end

(** A `fixme_map` associates:
    line number guarded by HH_FIXME =>
    error_node_number =>
    position of HH_FIXME comment *)
type fixme_map = Pos.t IMap.t IMap.t

module Fixme_store : sig
  type t

  val empty : t

  val get : t -> Relative_path.t -> fixme_map option

  val add : t -> Relative_path.t -> fixme_map -> unit

  val remove : t -> Relative_path.t -> unit

  val remove_batch : t -> Relative_path.Set.t -> unit
end

module Fixmes : sig
  (** The `hh_fixmes` and `decl_hh_fixmes` fields represent the HH_FIXMEs we
      detected in each file in a full-parse and a decl-parse, respectively. The
      former will be a superset of the latter, when both are populated and
      up-to-date. We have both because in some scenarios, we will have only
      performed a decl-parse on a given file, but wouldn't want to take the
      FIXMEs we collected during a decl-parse (i.e., one in which we throw away
      function bodies and any FIXMEs they might contain) during a future
      full-typecheck of that file. When performing a lookup (in
      `Fixme_provider`), we check `hh_fixmes` first, and use `decl_hh_fixmes`
      only if there is no entry in `hh_fixmes`. *)
  type t = private {
    hh_fixmes: Fixme_store.t;
    decl_hh_fixmes: Fixme_store.t;
    disallowed_fixmes: Fixme_store.t;
  }
end

type t =
  | Shared_memory
  | Local_memory of {
      decl_cache: Decl_cache.t;
      shallow_decl_cache: Shallow_decl_cache.t;
      fixmes: Fixmes.t;
    }
  | Decl_service of {
      decl: Decl_service_client.t;
      fixmes: Fixmes.t;
    }

val t_to_string : t -> string

val set_shared_memory_backend : unit -> unit

val set_local_memory_backend_with_defaults : unit -> unit

val set_decl_service_backend : Decl_service_client.t -> unit

val get : unit -> t
