(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Decl_cache_entry = struct
  (* NOTE: we can't simply use a string as a key. In the case of a name conflict,
  we may put e.g. a function named 'foo' into the cache whose value is one type,
  and then later try to withdraw a class named 'foo' whose value is another type.

  The problem can be solved with a GADT, but making a GADT with references to
  types like `Typing_defs.ty` causes dependency cycles, since `typing` ends up
  depending on `Provider_backend` transitively.
  *)
  type key =
    | Fun_decl of string
    | Class_decl of string
    | Record_decl of string
    | Typedef_decl of string
    | Gconst_decl of string

  type value = Obj.t

  let get_size = Obj.reachable_words
end

module Decl_cache = Lru_cache.Cache (Decl_cache_entry)

type t =
  | Shared_memory
  | Local_memory of { decl_cache: Decl_cache.t }
  | Decl_service of Decl_service_client.t

let t_to_string (t : t) : string =
  match t with
  | Shared_memory -> "Shared_memory"
  | Local_memory _ -> "Local_memory"
  | Decl_service _ -> "Decl_service"

let backend_ref = ref Shared_memory

let set_shared_memory_backend () : unit = backend_ref := Shared_memory

let set_local_memory_backend ~(max_size_in_words : Lru_cache.size) : unit =
  backend_ref :=
    Local_memory { decl_cache = Decl_cache.make ~max_size:max_size_in_words }

let set_decl_service_backend (decl : Decl_service_client.t) : unit =
  backend_ref := Decl_service decl

let get () : t = !backend_ref
