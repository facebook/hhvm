(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type megabytes = int

let get_heap_size () = Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024
