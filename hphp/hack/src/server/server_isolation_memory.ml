(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let bytes_per_gib = 1024. *. 1024. *. 1024.

let gib bytes = float_of_int bytes /. bytes_per_gib

let rss_gib () =
  match Memory_stats.get_vm_rss () with
  | Some bytes -> gib bytes
  | None -> 0.

let heap_gib () = gib (Shared_mem.SMTelemetry.heap_size ())
