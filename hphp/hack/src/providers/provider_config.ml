(**
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type decl_cache = (string, Obj.t) Memory_bounded_lru_cache.t

type backend =
  | Shared_memory
  | Local_memory of { decl_cache: decl_cache }

let backend_ref = ref Shared_memory

let set_shared_memory_backend (): unit =
  backend_ref := Shared_memory

let set_local_memory_backend ~(max_size_in_words : int): unit =
  backend_ref := Local_memory {
    decl_cache = Memory_bounded_lru_cache.make ~max_size_in_words;
  }

let get_backend (): backend =
  !backend_ref
