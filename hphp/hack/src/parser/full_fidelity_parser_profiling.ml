(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let parse_count_ref = ref 0

let start_profiling () = parse_count_ref := 0

let record_parse () = incr parse_count_ref

let stop_profiling () = !parse_count_ref
