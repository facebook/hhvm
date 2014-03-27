(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module ParserHeap = SharedMem.NoCache(struct
  type t = Ast.program
  let prefix = Prefix.make()
end)

let find_class_in_file file_name class_name =
  List.fold_left begin fun acc def ->
    match def with
    | Ast.Class c when snd c.Ast.c_name = class_name -> Some c
    | _ -> acc
  end None (ParserHeap.find_unsafe file_name)

