(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = { analyses: string list }

let prepare ~server:_ _ = { analyses = [] }

let set_json_mode opt _ = opt

let modify_shared_mem_sizes global_size heap_size dep_table_pow hash_table_pow _
    =
  (global_size, heap_size, dep_table_pow, hash_table_pow, 0)
