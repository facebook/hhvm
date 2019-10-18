(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* NOTE: we can't simply use a string as a key. In the case of a name conflict,
we may put e.g. a function named 'foo' into the cache whose value is one type,
and then later try to withdraw a class named 'foo' whose value is another type.

The problem can be solved with a GADT, but making a GADT with references to
types like `Typing_defs.ty` causes dependency cycles, since `typing` ends up
depending on `Provider_config` transitively.
*)
type decl_cache_key =
  | Fun_decl of string
  | Class_decl of string
  | Record_decl of string
  | Typedef_decl of string
  | Gconst_decl of string

type decl_cache = (decl_cache_key, Obj.t) Memory_bounded_lru_cache.t

type backend =
  | Lru_shared_memory
  | Shared_memory
  | Local_memory of { decl_cache: decl_cache }
  | Decl_service of Decl_service_client.t

let backend_ref = ref Shared_memory

let set_lru_shared_memory_backend () : unit = backend_ref := Lru_shared_memory

let set_shared_memory_backend () : unit = backend_ref := Shared_memory

let set_local_memory_backend ~(max_size_in_words : int) : unit =
  backend_ref :=
    Local_memory
      { decl_cache = Memory_bounded_lru_cache.make ~max_size_in_words }

let set_decl_service_backend (decl : Decl_service_client.t) : unit =
  backend_ref := Decl_service decl

let get_backend () : backend = !backend_ref
