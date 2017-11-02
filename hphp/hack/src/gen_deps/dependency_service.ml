(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Hh_core

let dependency_files popt _acc fnl =
  List.iter fnl ~f:(fun fn ->
    let ast = Parser_heap.get_from_parser_heap popt fn in
    Dependency_visitors.gen_deps popt ast
  )

let go workers ~get_next popt =
  MultiWorker.call
      workers
      ~job:(dependency_files popt)
      ~neutral:()
      ~merge:(fun _ _ -> ())
      ~next:get_next
